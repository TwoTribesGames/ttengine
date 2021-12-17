#include <tt/code/bufferutils.h>
#include <tt/code/DefaultValue.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/pres/ParticleSpawner.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlNode.h>

namespace tt {
namespace pres {


std::string ParticleSpawner::ms_trySubDir;


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 6);


//--------------------------------------------------------------------------------------------------
// Public member functions


ParticleSpawnerPtr ParticleSpawner::loadBin(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                            const DataTags& p_applyTags, 
                                            const Tags& p_acceptedTags, 
                                            const PresentationObjectPtr& p_followObject,
                                            u32 p_particleCategory, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(ParticleSpawnerPtr, ParticleSpawnerPtr(), "loading particle spawner");
	
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_version)
	{
		TT_ERR_AND_RETURN("Invalid version for ParticleData, code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			", Please update your Presentation converter");
	}
	
	std::string triggerFile(be_get<std::string>(p_bufferOUT, p_sizeOUT));
	
	std::string layerName(be_get<std::string>(p_bufferOUT, p_sizeOUT));
	s32 renderGroup(-1);
	
	if (p_followObject != 0 && layerName.empty() == false && p_followObject->getManager() != 0)
	{
		renderGroup = p_followObject->getManager()->particleRenderGroupFromName(layerName);
	}
	
	PresentationValue positionOffsetX(0.0f);
	PresentationValue positionOffsetY(0.0f);
	PresentationValue positionOffsetZ(0.0f);
	positionOffsetX.load(p_bufferOUT, p_sizeOUT, &errStatus);
	positionOffsetY.load(p_bufferOUT, p_sizeOUT, &errStatus);
	positionOffsetZ.load(p_bufferOUT, p_sizeOUT, &errStatus);
	PositionType posType(static_cast<PositionType>(be_get<u8>(p_bufferOUT, p_sizeOUT)));
	
	PresentationValue scale(1.0f);
	scale.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	const bool useObjectFlip = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	TriggerInfo info(TriggerInfo::loadBin(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus));
	
	TT_ERR_RETURN_ON_ERROR();
	
	ParticleSpawnerPtr spawner(new ParticleSpawner(info, triggerFile, posType, positionOffsetX,
	                                               positionOffsetY, positionOffsetZ, scale,
	                                               p_followObject, p_particleCategory,
	                                               renderGroup, layerName ));
	
	spawner->m_useObjectFlip = useObjectFlip;
	
	return spawner;
}


ParticleSpawnerPtr ParticleSpawner::loadXml(const xml::XmlNode* p_node, const DataTags& p_applyTags, 
                                            const Tags& p_acceptedTags, 
                                            const PresentationObjectPtr& p_object, 
                                            u32 p_particleCategory,
                                            code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(ParticleSpawnerPtr, ParticleSpawnerPtr(), "Loading particle spawner");
	
	std::string file(xml::util::parseStr(p_node, "file", &errStatus));
	
	ParticleSpawner::PositionType positionType(ParticleSpawner::PositionType_Invalid);
	
	std::string positionTypeStr(xml::util::parseStr(p_node, "position_type", &errStatus));
	if      (positionTypeStr == "world")    positionType = ParticleSpawner::PositionType_WorldPosition;
	else if (positionTypeStr == "relative") positionType = ParticleSpawner::PositionType_RelativeOffset;
	else if (positionTypeStr == "follow")   positionType = ParticleSpawner::PositionType_FollowOffset;
	TT_ERR_RETURN_ON_ERROR();
	
	code::DefaultValue<std::string> particleLayer("nolayer");
	particleLayer = xml::util::parseOptionalStr(p_node, "layer", &errStatus);
	
	tt::code::OptionalValue<bool> useObjectFlip = xml::util::parseOptionalBool(p_node, "use_objects_flip", &errStatus);
	if (useObjectFlip.isValid() == false)
	{
		useObjectFlip = true; // default is true.
	}
	
	s32 renderGroup(-1);
	
	if(p_object != 0 && p_object->getManager() != 0)
	{
		renderGroup = p_object->getManager()->particleRenderGroupFromName(particleLayer);
	}
	
	PresentationValue positionOffsetX(0.0f);
	PresentationValue positionOffsetY(0.0f);
	PresentationValue positionOffsetZ(0.0f);
	PresentationValue scale(1.0f);
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "translation")
		{
			positionOffsetX = parseOptionalPresentationValue(child, "x", 0, &errStatus);
			positionOffsetY = parseOptionalPresentationValue(child, "y", 0, &errStatus);
			positionOffsetZ = parseOptionalPresentationValue(child, "z", 0, &errStatus);
			
			TT_ERR_RETURN_ON_ERROR();
		}
		else if (child->getName() == "scale")
		{
			scale = parsePresentationValue(child, "scale", 0, &errStatus);
		}
	}
	
	TriggerInfo info;
	
	info.loadXmlCommon(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	ParticleSpawnerPtr spawner(new ParticleSpawner(info, file, positionType, positionOffsetX,
	                                               positionOffsetY, positionOffsetZ, scale,
	                                               p_object, p_particleCategory,
	                                               renderGroup, particleLayer));
	
	spawner->m_useObjectFlip = useObjectFlip;
	
	return spawner;
}


math::Vector3 ParticleSpawner::getPosition() const
{
	switch (m_positionType)
	{
	case PositionType_FollowOffset:
		return getWorldPositionWithFollowObject();
		
	default:
		TT_PANIC("Not following so shouldn't call getPosition.");
		return math::Vector3::zero;
	}
}


real ParticleSpawner::getScale() const
{
	real scale(m_scale);
	
	PresentationObjectPtr followPtr(m_followObject.lock());
	TT_NULL_ASSERT(followPtr);
	if (followPtr != 0)
	{
		scale *= followPtr->getCombinedMatrix().getScaleVectorX();
	}
	
	return scale;
}


bool ParticleSpawner::isCulled() const
{
	switch (m_positionType)
	{
	case PositionType_FollowOffset:
		{
			PresentationObjectPtr followPtr(m_followObject.lock());
			TT_NULL_ASSERT(followPtr);
			if (followPtr != 0)
			{
				return followPtr->isCulled();
			}
		}
		break;
		
	default:
		break;
	}
	
	TT_PANIC("TODO: Implement isCulled for non-following objects.");
	return false;
}

int ParticleSpawner::getSortWeight() const
{
	TT_PANIC("Sorting not implemented for particles.");
	return 0;
}


void ParticleSpawner::trigger()
{
	if (m_particleEffect == 0)
	{
		real scale = m_scale;
		
		PresentationObjectPtr followPtr(m_followObject.lock());
		TT_NULL_ASSERT(followPtr);
		if (followPtr != 0)
		{
			// Scale scale of this spawner with scale of combined matrix (use X direction for uniform scale)
			scale *= followPtr->getCombinedMatrix().getScaleVectorX();
		}
		
		switch (m_positionType)
		{
		case PositionType_FollowOffset:
			// create a particle trigger that follows the object
			//FIXME: use weak pointers with Particles
			//m_particleEffect = particles::ParticleMgr::getInstance()->spawnContinuous(
			//	m_triggerFile, m_this, true);
			m_particleEffect = engine::particles::ParticleMgr::getInstance()->createEffect(
					m_triggerFile, this, m_particleCategory, scale, m_renderGroup);
			break;
			
		case PositionType_RelativeOffset:
			// create a particle trigger at the current position with an offset
			m_particleEffect = engine::particles::ParticleMgr::getInstance()->createEffect(
					m_triggerFile, getWorldPositionWithFollowObject(),
					m_particleCategory, scale, m_renderGroup);
			break;
			
		case PositionType_WorldPosition:
			// create a particle trigger in the world
			m_particleEffect = engine::particles::ParticleMgr::getInstance()->createEffect(
					m_triggerFile, getPositionOffset(), m_particleCategory, scale, m_renderGroup);
			break;
			
		default:
			TT_PANIC("Unsupported particle spawning PositionType: %d", m_positionType);
			break;
		}
		
		if (m_particleEffect != 0)
		{
			if (m_useObjectFlip)
			{
				m_particleEffect->flip(m_flipMask);
			}
			
			m_particleEffect->spawn();
		}
		else
		{
			TT_PANIC("Spawning presentation-triggered particle effect '%s' failed. "
			         "Particle effect may not exist.",
			         m_triggerFile.c_str());
		}
	}
}


void ParticleSpawner::presentationEnded()
{
	if(m_particleEffect != 0)
	{
		m_particleEffect->stop();
		
		// animation stopped so no need anymore for the particle effect
		m_particleEffect.reset();
	}
}


void ParticleSpawner::reTrigger()
{
	if (m_particleEffect != 0)
	{
		if (m_particleEffect->isActive() == false)
		{
			presentationEnded();
			trigger();
		}
	}
	else
	{
		trigger();
	}
}


bool ParticleSpawner::isVisible() const
{
	return m_particleEffect != 0 && (m_particleEffect->isActive() || m_particleEffect->isCulled() == false);
}


bool ParticleSpawner::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Saving particle spawner");
	
	if(getBufferSize() > p_sizeOUT)
	{
		TT_ERR_AND_RETURN("Not enough space in buffer need " << getBufferSize() 
		                  << " got " << p_sizeOUT);
	}
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	be_put(m_triggerFile,     p_bufferOUT, p_sizeOUT);
	be_put(m_renderGroupName, p_bufferOUT, p_sizeOUT);
	
	m_positionOffsetX.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_positionOffsetY.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_positionOffsetZ.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	be_put(static_cast<u8>(m_positionType), p_bufferOUT, p_sizeOUT);
	m_scale.save(p_bufferOUT, p_sizeOUT, &errStatus);
	be_put(m_useObjectFlip, p_bufferOUT, p_sizeOUT);
	
	return m_triggerInfo.saveBin(p_bufferOUT, p_sizeOUT, &errStatus);
}


void ParticleSpawner::stopParticles(bool p_kill)
{
	if (m_particleEffect != 0)
	{
		m_particleEffect->stop(p_kill == false);
		m_particleEffect.reset();
	}
}


size_t ParticleSpawner::getBufferSize() const
{
	// triggerfileSize + fileString + layerNameSize + layerString + 
	return 2 + m_triggerFile.size() + 2 + m_renderGroupName.size() + 
		// positionOffset x, y & z + 
		m_positionOffsetX.getBufferSize() + m_positionOffsetY.getBufferSize() + m_positionOffsetZ.getBufferSize() + 
		m_scale.getBufferSize() + 
		// positionType + triggerinfo + version
		1 + m_triggerInfo.getBufferSize() + 2 +
		1; // m_useObjectFlip(bool) 
}


engine::renderer::TextureContainer ParticleSpawner::getAndLoadAllUsedTextures() const
{
	engine::particles::ParticleTrigger triggerInstance(static_cast<WorldObject*>(0), 0);
	triggerInstance.load(m_triggerFile);
	return triggerInstance.getAndLoadAllUsedTextures();
}


void ParticleSpawner::setFlipMask(u32 p_flipMask)
{
	if (m_useObjectFlip)
	{
		m_flipMask = p_flipMask;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ParticleSpawner::ParticleSpawner(const TriggerInfo&           p_triggerInfo,
                                 const std::string&           p_triggerFile,
                                 PositionType                 p_positionType,
                                 const PresentationValue&     p_positionOffsetX,
                                 const PresentationValue&     p_positionOffsetY,
                                 const PresentationValue&     p_positionOffsetZ,
                                 const PresentationValue&     p_scale,
                                 const PresentationObjectPtr& p_followObject,
                                 u32                          p_particleCategory,
                                 s32                          p_renderGroup,
                                 const std::string&           p_renderGroupName)
:
TriggerBase(p_triggerInfo),
m_triggerFile(p_triggerFile),
m_positionOffsetX(p_positionOffsetX),
m_positionOffsetY(p_positionOffsetY),
m_positionOffsetZ(p_positionOffsetZ),
m_positionType(p_positionType),
m_scale(p_scale),
m_particleEffect(),
m_particleCategory(p_particleCategory),
m_renderGroup(p_renderGroup),
m_renderGroupName(p_renderGroupName),
m_followObject(p_followObject),
m_triggerInfo(p_triggerInfo),
m_flipMask(0),
m_useObjectFlip(true)
{
	if (ms_trySubDir.empty() == false)
	{
		const std::string newPath(fs::utils::addSubdirToPath(m_triggerFile, ms_trySubDir));
		
		if (fs::fileExists(newPath))
		{
			m_triggerFile = newPath;
		}
	}
}


ParticleSpawner::ParticleSpawner(const ParticleSpawner& p_rhs)
:
TriggerBase(p_rhs),
engine::particles::WorldObject(),
m_triggerFile(p_rhs.m_triggerFile),
m_positionOffsetX(p_rhs.m_positionOffsetX),
m_positionOffsetY(p_rhs.m_positionOffsetY),
m_positionOffsetZ(p_rhs.m_positionOffsetZ),
m_positionType(p_rhs.m_positionType),
m_scale(p_rhs.m_scale),
m_particleEffect(p_rhs.m_particleEffect == 0 ? engine::particles::ParticleEffectPtr() :
                                               p_rhs.m_particleEffect->clone()),
m_particleCategory(p_rhs.m_particleCategory),
m_renderGroup(p_rhs.m_renderGroup),
m_renderGroupName(p_rhs.m_renderGroupName),
m_followObject(p_rhs.m_followObject),
m_triggerInfo(p_rhs.m_triggerInfo),
m_flipMask(p_rhs.m_flipMask),
m_useObjectFlip(p_rhs.m_useObjectFlip)
{
}


void ParticleSpawner::setRanges(PresentationObject* p_presObj)
{
	TriggerBase::setRanges(p_presObj);
	
	m_positionOffsetX.updateValue(p_presObj);
	m_positionOffsetY.updateValue(p_presObj);
	m_positionOffsetZ.updateValue(p_presObj);
	m_scale.updateValue(p_presObj);
}


math::Vector3 ParticleSpawner::getWorldPositionWithFollowObject() const
{
	math::Vector3 pos(getPositionOffset());
	
	PresentationObjectPtr followPtr(m_followObject.lock());
	TT_NULL_ASSERT(followPtr);
	if (followPtr != 0)
	{
		pos = pos * followPtr->getCombinedMatrix();
	}
	
	return pos;
}


void ParticleSpawner::setFollowObject(const PresentationObjectPtr& p_followObject)
{
	m_followObject = p_followObject;
	
	// Override new rendergroup if one could be found in the follow object's manager
	// This is mainly used for Swap This! since in that game sets all particle effect layers were set in the presentations
	// In other games, the layer is usually set in the effects themselves
	// although this can still be overrun by setting it in the presentation using the "layer" attribute.
	if (p_followObject != 0 && m_renderGroupName.empty() == false && p_followObject->getManager() != 0)
	{
		const s32 newGroup = p_followObject->getManager()->particleRenderGroupFromName(m_renderGroupName);
		if (newGroup != -1)
		{
			m_renderGroup = newGroup;
		}
	}
}


// Namespace end
}
}
