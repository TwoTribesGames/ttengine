#include <tt/code/bufferutils.h>
#include <tt/code/DefaultValue.h>
#include <tt/pres/anim2d/AnimationStack2D.h>
#include <tt/pres/anim2d/ColorAnimationStack2D.h>
#include <tt/pres/anim2d/fwd.h>

#include <tt/pres/CallbackTrigger.h>
#include <tt/pres/FrameAnimationStack.h>
#include <tt/pres/ParticlesStack.h>
#include <tt/pres/PresentationLoader.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/Timer.h>
#include <tt/pres/TimerStack.h>
#include <tt/pres/TriggerFactoryInterface.h>
#include <tt/pres/TriggerStack.h>

#if !defined(TT_BUILD_FINAL)
#include <tt/system/Time.h>
#include <tt/platform/tt_printf.h>
#endif

#include <tt/xml/util/parse.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>

namespace tt {
namespace pres {


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 6);


PresentationLoader::PresentationLoader()
:
m_object(),
m_includes(),
m_triggerinfos(),
m_acceptedTags(),
m_particleCategory(0)
{
}


PresentationLoader::PresentationLoader(const Tags& p_acceptedTags, u32 p_particleCategory)
:
m_object(),
m_includes(),
m_triggerinfos(),
m_acceptedTags(p_acceptedTags),
m_particleCategory(p_particleCategory)
{
}


bool PresentationLoader::load(const PresentationObjectPtr& p_object,
                              const std::string& p_file,
                              const TriggerFactoryInterfacePtr& p_triggerFactory,
                              const Tags& p_requiredTags)
{
	TT_ERR_CREATE("Loading presentation file: '" << p_file << "'");
	m_object = p_object;
	
	DataTags emptyTags;
	if(p_requiredTags.empty())
	{
		load(p_file, p_triggerFactory, emptyTags, m_acceptedTags, true, &errStatus);
	}
	else
	{
		load(p_file, p_triggerFactory, emptyTags, p_requiredTags, true, &errStatus);
	}
	
	if(errStatus.hasError())
	{
		TT_PANIC("Loading Presentation failed: %s", errStatus.getErrorMessage().c_str());
		m_object.reset();
		return false;
	}
	
	if(p_requiredTags.empty() == false)
	{
		PresentationMgr::checkRequiredTags(p_requiredTags, m_object->getTags().getAllUsedTags(), p_file);
	}
	
	// reset all stacks to ignore initially suspended flags
	m_object->reset();
	// sort merged stacks.
	m_object->m_anim2dStack.sortAll();
	
	m_object.reset();
	
	return true;
}


bool PresentationLoader::loadXml( xml::XmlNode* p_rootNode, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading presentation file from xml");
	TT_ERR_ASSERT(m_object == 0);
	
	PresentationObjectPtr object(new PresentationObject(0));
	m_object = object;
	
	Tags emptyTags;
	DataTags emptyDataTags;
	loadXml(p_rootNode, TriggerFactoryInterfacePtr(), emptyDataTags, emptyTags, false, &errStatus);
	
	if(errStatus.hasError())
	{
		m_object.reset();
		TT_ERR_RETURN_ON_ERROR();
	}
	
	// reset all stacks to ignore initially suspended flags
	object->reset();
	// sort merged stack.
	object->m_anim2dStack.sortAll();
	
	return true;
}


bool PresentationLoader::saveBinary( u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                     code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Saving Presentation File Binary");
	TT_ERR_ASSERTMSG(m_object != 0, "Call load before save");

	if (save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
	{
		TT_ERR_AND_RETURN("saving of animationStack failed");
	}

	return true;
}


size_t PresentationLoader::getBufferSize() const
{
	size_t size(2); // version
	
	size += sizeof(bool); // permanent flag
	size += sizeof(bool); // don't precache flag
	
	size += sizeof(real); // fixed time step
	
	// includes
	size += 2; // count of includes
	for(IncludeInfos::const_iterator it(m_includes.begin()) ; it != m_includes.end() ; ++it)
	{
		size += 2 + it->includeFile.size();
		
		// tags
		size += it->tags.getBufferSize();
	}
	
	// object
	size += m_object->getBufferSize(); 
	
	// triggers
	size += 2; // count of triggers
	for(TriggerInfos::const_iterator it(m_triggerinfos.begin()) ; it != m_triggerinfos.end() ; ++it)
	{
		size += it->getBufferSize();
	}
	
	// presetCustomValues
	size += 2; // count of values
	for(PresentationObject::PresetCustomValues::const_iterator it(m_object->m_presetCustomValues.begin()) ; 
	    it != m_object->m_presetCustomValues.end() ; ++it)
	{
		size += 2 + it->first.size();
		size += it->second.getBufferSize();
	}
	return size;
}


bool PresentationLoader::load( const std::string& p_file, 
                               const TriggerFactoryInterfacePtr& p_factory, DataTags& p_applyTags, 
                               const Tags& p_acceptedTags,
                               bool p_loadForUsage, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Determine binary or xml loading.");
	TT_ERR_NULL_ASSERT(m_object);
	
	if(fs::fileExists(p_file + ".pres"))
	{
		fs::FilePtr file(fs::open(p_file + ".pres", fs::OpenMode_Read));
		size_t size = static_cast<size_t>(fs::getLength(file));
		u8* buffer = new u8[size];
		if (file->read(buffer, static_cast<fs::size_type>(size)) != static_cast<fs::size_type>(size))
		{
			delete[] buffer;
			TT_ERR_AND_RETURN("File '" << p_file << "' too small.");
		}
		const u8* scratch = buffer;
		
		loadBin(scratch, size, p_factory, p_applyTags, p_acceptedTags, &errStatus);
		delete[] buffer;
		TT_ERR_RETURN_ON_ERROR();
	}
	else 
	{
		TT_ERR_ASSERTMSG(fs::fileExists(p_file + ".xml"),
			("could not find presentation file: " + p_file).c_str());
		
		// load the xml
		xml::XmlDocument doc(p_file + ".xml");
		
		TT_ERR_ASSERTMSG(doc.getRootNode()->getName()=="presentation",
			"Presentation xml file's root node is not 'presentation'. it is '"
			<< doc.getRootNode()->getName() << "'");
		
		if (loadXml(doc.getRootNode(), p_factory, p_applyTags, p_acceptedTags, 
		            p_loadForUsage, &errStatus) == false)
		{
			TT_ERR_RETURN_ON_ERROR();
		}
	}
	
	return true;
}


bool PresentationLoader::loadXml(xml::XmlNode* p_rootNode,
	                             const TriggerFactoryInterfacePtr& p_factory,
	                             DataTags& p_applyTags,
	                             const Tags& p_acceptedTags,
	                             bool p_loadForUsage,
	                             code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading Presentation From XML");
	TT_ERR_ASSERT(p_rootNode->getName() == "presentation");
	TT_ERR_NULL_ASSERT(m_object);
	
	for (const xml::XmlNode* child = p_rootNode->getChild(); child != 0; child = child->getSibling())
	{
		// load includes
		if (child->getName() == "include")
		{
			if(parseIncludesXml(child, p_factory, p_applyTags, p_acceptedTags, p_loadForUsage, &errStatus) == false)
			{
				TT_ERR_RETURN_ON_ERROR();
			}
		}
		
		// load an anim2dstack and merge it
		else if (child->getName() == "anim2dstack")
		{
			anim2d::AnimationStack2D anim2dStack;
			anim2dStack.load(child, p_applyTags, p_acceptedTags, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			anim2dStack.reset();
			m_object->m_anim2dStack.appendStack(anim2dStack);
		}
		
		// load the startvalues and merge it
		else if (child->getName() == "startvalues")
		{
			anim2d::AnimationStack2D anim2dStack;
			anim2dStack.loadStartValues(child, p_applyTags, p_acceptedTags, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			anim2dStack.reset();
			m_object->m_anim2dStack.appendStack(anim2dStack);
		}
		
		// load a coloranimstack and merge it
		else if (child->getName() == "coloranimstack")
		{
			anim2d::ColorAnimationStack2D colorAnimStack;
			colorAnimStack.load(child, p_applyTags, p_acceptedTags, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			m_object->m_colorAnimStack.appendStack(colorAnimStack);
		}
		
		// load a frameanimstack and merge it
		else if (child->getName() == "frameanimstack")
		{
			FrameAnimationStack frameAnimStack;
			frameAnimStack.load(child, p_applyTags, p_acceptedTags, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			m_object->m_frameAnimStack.appendStack(frameAnimStack);
		}
		
		// load a particlesspawner and add it
		else if (child->getName() == "particles")
		{
			ParticleSpawnerPtr spawner(ParticleSpawner::loadXml(child, p_applyTags, 
			                                                    p_acceptedTags, m_object, 
			                                                    m_particleCategory,
			                                                    &errStatus));
			
			TT_ERR_RETURN_ON_ERROR();
			
			m_object->m_particleStack.push_back(spawner);
		}
		
		// load a trigger and add it
		else if (child->getName() == "trigger" )
		{
			if(p_loadForUsage && p_factory != 0)
			{
				
				TriggerInfo info(TriggerInfo::loadXml(child, p_applyTags, p_acceptedTags, &errStatus));
				
				TT_ERR_RETURN_ON_ERROR();
				
				TriggerInterfacePtr trigger(createTrigger(info, p_factory, m_object));
				if(trigger != 0)
				{
					m_object->m_triggerStack.push_back(trigger);
				}
			}
			else
			{
				m_triggerinfos.push_back(TriggerInfo::loadXml(child, p_applyTags,
				                                              p_acceptedTags,
				                                              &errStatus));
				
				TT_ERR_RETURN_ON_ERROR();
			}
		}
		
		// load a timer and add it
		else if (child->getName() == "time" )
		{
			TimerPtr timer(new Timer);
			timer->load(child, p_applyTags, p_acceptedTags, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			
			m_object->m_timerStack.push_back(timer);
		}
		
		// load preset custom values
		else if (child->getName() == "customvalue" )
		{
			std::string       name (xml::util::parseStr   (child, "name",  &errStatus));
			PresentationValue value(parsePresentationValue(child, "value", &errStatus));
			
			TT_ERR_RETURN_ON_ERROR();
			TT_ERR_ASSERTMSG(name.empty() == false, "custom value node has empty name attribute.");
			TT_ERR_ASSERTMSG(name.empty() == false && name.at(0) != '_',
			                 "The first character in the custom value '" << name << "' should not be '_'.");
			
			m_object->m_presetCustomValues.insert(std::make_pair(name, value));
		}
		else
		{
			TT_ERROR("Unknown element: '" << child->getName() << "'");
			return false;
		}
	}
	
	// Permanent flag
	{
		code::DefaultValue<bool> isPermanent(true);
		isPermanent = xml::util::parseOptionalBool(p_rootNode, "permanent", &errStatus);
		m_object->setPermanent(isPermanent.get());
	}
	
	// Don't precache flag
	{
		code::DefaultValue<bool> dontPrecache(false);
		dontPrecache = xml::util::parseOptionalBool(p_rootNode, "dont_precache", &errStatus);
		m_object->setDontPrecache(dontPrecache.get());
	}
	
	// Fixed time step
	{
		code::DefaultValue<real> fixedTimestep(0.0);
		fixedTimestep = xml::util::parseOptionalReal(p_rootNode, "fixed_timestep", &errStatus);
		m_object->setFixedTimestep(fixedTimestep.get());
	}
	
	return true;
}


bool PresentationLoader::parseIncludesXml( const xml::XmlNode* p_node, 
                                           const TriggerFactoryInterfacePtr& p_factory, 
                                           const DataTags& p_applyTags, const Tags& p_acceptedTags,
                                           bool p_loadForUsage, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Including Presentation File");
	TT_ERR_ASSERT(p_node->getName() == "include");
	TT_ERR_NULL_ASSERT(m_object);
	
	std::string file(p_node->getAttribute("file"));
	
	TT_ERR_ASSERTMSG(file.empty() == false,"No file attribute in Include element");
	
	if(p_loadForUsage)
	{
		DataTags includeTags;
		includeTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
		
		
		if(load(file, p_factory, includeTags, p_acceptedTags, p_loadForUsage, &errStatus) == false)
		{
			TT_ERR_RETURN_ON_ERROR();
		}
	}
	else
	{
		IncludeInfo includeInfo;
		includeInfo.includeFile = file;
		
		includeInfo.tags.load(p_node, DataTags(), p_acceptedTags, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
		
		m_includes.push_back(includeInfo);
	}
	return true;
}


bool PresentationLoader::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "saving Presentation");
	
	if(getBufferSize() > p_sizeOUT)
	{
		TT_ERR_AND_RETURN("Not enough space in buffer need " << getBufferSize() 
		                  << " got " << p_sizeOUT);
	}
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	// Permanent flag
	be_put(m_object->isPermanent(), p_bufferOUT, p_sizeOUT);
	
	// Don't precache flag
	be_put(m_object->dontPrecache(), p_bufferOUT, p_sizeOUT);
	
	// Fixed time step
	be_put(m_object->getFixedTimestep(), p_bufferOUT, p_sizeOUT);
	
	// includes
	be_put(static_cast<u16>(m_includes.size()), p_bufferOUT, p_sizeOUT); // count of includes
	for(IncludeInfos::const_iterator it(m_includes.begin()) ; it != m_includes.end() ; ++it)
	{
		// tags
		it->tags.save(p_bufferOUT, p_sizeOUT, &errStatus); 
		
		// include file string
		be_put(it->includeFile, p_bufferOUT, p_sizeOUT); 
	}
	
	// save anim2d
	if(m_object->m_anim2dStack.save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
	{
		TT_ERR_AND_RETURN("saving of animationStack failed");
	}
	
	// save color anim
	if(m_object->m_colorAnimStack.save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
	{
		TT_ERR_AND_RETURN("saving of colorAnimStack failed");
	}
	
	// save frame anim
	if(m_object->m_frameAnimStack.save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
	{
		TT_ERR_AND_RETURN("saving of frame anim stack failed");
	}
	
	// save particles
	if(m_object->m_particleStack.save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
	{
		TT_ERR_AND_RETURN("saving of particles failed");
	}
	
	// save timers
	if(m_object->m_timerStack.save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
	{
		TT_ERR_AND_RETURN("saving of timers failed");
	}
	
	// triggers
	be_put(static_cast<u16>(m_triggerinfos.size()), p_bufferOUT, p_sizeOUT);// count of triggers
	for(TriggerInfos::const_iterator it(m_triggerinfos.begin()) ; it != m_triggerinfos.end() ; ++it)
	{
		if (it->saveBin(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			TT_ERR_AND_RETURN("saving of timers failed");
		}
	}
	
	// preset custom values
	be_put(static_cast<u16>(m_object->m_presetCustomValues.size()), p_bufferOUT, p_sizeOUT); // count of custom values
	for(PresentationObject::PresetCustomValues::const_iterator it(m_object->m_presetCustomValues.begin()) ; 
	    it != m_object->m_presetCustomValues.end() ; ++it)
	{
		// name
		be_put(it->first, p_bufferOUT, p_sizeOUT);
		// PresentationValue
		it->second.save(p_bufferOUT, p_sizeOUT, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
	}
	
	
	return true;
}


bool PresentationLoader::loadBin( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                  const TriggerFactoryInterfacePtr& p_factory, 
                                  const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                                  code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading Presentation");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_version)
	{
		TT_ERR_AND_RETURN("Invalid version. Code "
		                  << GET_MAJOR_VERSION(s_version) << "." << GET_MINOR_VERSION(s_version) << ", data "
		                  << GET_MAJOR_VERSION(version)   << "." << GET_MINOR_VERSION(version)   << " -- "
		                  "please update your converter.");
	}
	
	// Permanent flag
	const bool isPermanent = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	// Dont precache flag
	const bool dontPrecache = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	// Fixed time step
	const real fixedTimestep = be_get<real>(p_bufferOUT, p_sizeOUT);
	
	// includes
	const u16 includeCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < includeCount; ++i)
	{
		DataTags tags;
		tags.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
		
		load(be_get<std::string>(p_bufferOUT, p_sizeOUT), p_factory, tags, p_acceptedTags, false, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
	}
	
	// Set the flags after loading the includes, otherwise pres will get the value of its last child
	m_object->setFixedTimestep(fixedTimestep);
	m_object->setPermanent(isPermanent);
	m_object->setDontPrecache(dontPrecache);
	
	// load anim2d
	anim2d::AnimationStack2D anim2d;
	anim2d.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	m_object->m_anim2dStack.appendStack(anim2d);
	
	// load color anim
	anim2d::ColorAnimationStack2D coloranim;
	coloranim.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	m_object->m_colorAnimStack.appendStack(coloranim);
	
	// load frame anim
	FrameAnimationStack frameAnim;
	frameAnim.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	m_object->m_frameAnimStack.appendStack(frameAnim);
	
	// load particles
	ParticlesStack particleStack;
	particleStack.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, m_object, 
	                    m_particleCategory, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	m_object->m_particleStack.appendStack(particleStack);
	
	// load timers
	TimerStack timerStack;
	timerStack.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	m_object->m_timerStack.appendStack(timerStack);
	
	// triggers
	const u16 triggerCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < triggerCount; ++i)
	{
		TriggerInfo info(TriggerInfo::loadBin(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus));
		
		TriggerInterfacePtr trigger(createTrigger(info, p_factory, m_object));
		if(trigger != 0)
		{
			m_object->m_triggerStack.push_back(trigger);
		}
	}
	
	// preset Custom values
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2,
	                 "Not enough space remaining in presentation data buffer. Expected at least 2 more bytes.");
	const u16 presetCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < presetCount; ++i)
	{
		std::string       name (be_get<std::string>(p_bufferOUT, p_sizeOUT));
		PresentationValue value;
		value.load(p_bufferOUT, p_sizeOUT, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
		
		m_object->m_presetCustomValues.insert(std::make_pair(name, value));
	}
	
	return true;
}


bool PresentationLoader::isTagAccepted( const Tag& p_tag, const Tags& p_acceptedTags )const
{
	// when accepted tags is empty this means we are converting, so all tags are accepted
	return p_acceptedTags.empty() || p_acceptedTags.find(p_tag) != p_acceptedTags.end();
}


TriggerInterfacePtr PresentationLoader::createTrigger(const TriggerInfo&                p_creationInfo,
                                                      const TriggerFactoryInterfacePtr& p_factory,
                                                      const PresentationObjectPtr&      p_object)
{
	
	if (p_creationInfo.type == "callback")
	{
		return tt::pres::TriggerInterfacePtr(new CallbackTrigger(p_creationInfo, p_object));
	}
	else if(p_factory != 0)
	{
		return p_factory->createTrigger(p_creationInfo, p_object);
	}
	return tt::pres::TriggerInterfacePtr();
}


PresentationObjectPtr PresentationLoader::createDefault(PresentationMgr* p_mgr) const
{
	PresentationObjectPtr object(new PresentationObject(p_mgr));
	object->m_anim2dStack.makeDefault();
	object->m_frameAnimStack.makeDefault();
	return object;
}


//namespace end
}
}
