#include <utility>

#include <tt/algorithms/set_helpers.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/pres/GroupFactory.h>
#include <tt/pres/ParticleLayer.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationCache.h>
#include <tt/pres/RenderableInterface.h>
#include <tt/pres/PresentationGroup.h>


namespace tt {
namespace pres {


Tags PresentationMgr::stringsToTags(const str::Strings& p_strings )
{
	Tags tags;
	
	for(str::Strings::const_iterator it(p_strings.begin()) ; it != p_strings.end() ; ++it)
	{
		std::pair<Tags::iterator,bool> returnPair(tags.insert(Tag(*it)));
		if(returnPair.second == false)
		{
			TT_PANIC("Hashing error: 2 Tags have the same Hash result,"
				" please choose something different for one of them. Tag 1: '%s', Tag 2: '%s'",
				it->c_str(), returnPair.first->getName().c_str());
			tags.clear();
			return tags;
		}
	}
	
	return tags;
}


Cues PresentationMgr::stringsToCues( const str::Strings& p_strings )
{
	return stringsToTags(p_strings);
}


void PresentationMgr::checkRequiredTags( const Tags &p_requiredTags, Tags p_objectTags, 
                                         const std::string& p_file )
{
	if(p_requiredTags != p_objectTags)
	{
		algorithms::SetIntersectResult<Tag> intersect = 
			algorithms::intersectSet(p_requiredTags, p_objectTags);
		std::stringstream msg;
		msg << "Required Tags not met in file: '" << p_file << "'.";
		if(intersect.onlyInFirst.empty() == false)
		{
			msg << std::endl << std::endl << "Missing Tags: ";
			for(Tags::const_iterator it(intersect.onlyInFirst.begin()) ; 
				it != intersect.onlyInFirst.end() ; ++it)
			{
				msg << it->getName().c_str() << ", ";
			}
		}
		if(intersect.onlyInSecond.empty() == false)
		{
			msg << std::endl << std::endl << "Found but do not need: ";
			for(Tags::const_iterator it(intersect.onlyInSecond.begin()) ; 
				it != intersect.onlyInSecond.end() ; ++it)
			{
				msg << it->getName().c_str() << ", ";
			}
		}
		msg << std::endl << std::endl << "Required tags are: ";
		for(Tags::const_iterator it(p_requiredTags.begin()) ; 
			it != p_requiredTags.end() ; ++it)
		{
			msg << it->getName().c_str() << ", ";
		}
		TT_PANIC(msg.str().c_str());
	}
}


GroupFactoryInterfacePtr PresentationMgr::getDefaultGroupFactory()
{
	return GroupFactoryInterfacePtr(new GroupFactory<PresentationObjectLess>);
}


PresentationMgrPtr PresentationMgr::create( 
		const Tags& p_acceptedTags, 
		const TriggerFactoryInterfacePtr& p_triggerFactory, 
		u32 p_particleCategory /*= particles::Category_All*/,
		const GroupFactoryInterfacePtr& p_groupFactory )
{
	PresentationMgrPtr presMgr(new PresentationMgr(p_acceptedTags, p_triggerFactory, 
	                                               p_particleCategory, p_groupFactory));
	
	// create a new group that spans over the whole range
	math::Range zRange(-std::numeric_limits<real>::max(),
	                    std::numeric_limits<real>::max());
	
	GroupInterfacePtr group(p_groupFactory->create(presMgr.get(), zRange));
	presMgr->m_presentationGroups.insert(group);
	
	return presMgr;
}


void PresentationMgr::addCustomRenderLayer(const RenderableInterfacePtr& p_renderable, 
                                           real p_depth, s32 p_priority)
{
	m_customLayers.insert(CustomLayer(p_renderable, p_depth, p_priority));
	manageGroupsForLayerAdd(p_depth);
}


void PresentationMgr::createParticleLayer(s32                p_renderGroup,
                                          const std::string& p_layerName,
                                          real               p_depth,
                                          s32                p_priority)
{
	// Create the name/rendergroup mapping
	addParticleLayerMapping(p_layerName, p_renderGroup);
	
	// Create the custom render layer
	RenderableInterfacePtr renderable(new ParticleLayer(p_renderGroup));
	addCustomRenderLayer(renderable, p_depth, p_priority);
	m_particleRenderables.push_back(renderable);
}


void PresentationMgr::addParticleLayerMapping(const std::string& p_layerName,
                                              s32                p_renderGroup)
{
	std::pair<ParticleLayerGroupName::iterator, bool> retVal =
			m_particleLayers.insert(std::make_pair(p_layerName, p_renderGroup));
	
	(void)retVal; // suppress warning in final builds
	TT_ASSERTMSG(retVal.second,
	             "Adding presentation particle layer: a layer with name '%s' already exists "
	             "(with render group %d).",
	             p_layerName.c_str(), (*retVal.first).second);
}


void PresentationMgr::update( real p_deltaTime )
{
	for(PresentationObjects::iterator it(m_updateObjects.begin()) ; it != m_updateObjects.end(); ++it )
	{
		(*it)->update(p_deltaTime);
	}
	
	m_updateForRenderWasCalled = false;
}


void PresentationMgr::updateForRender(real p_deltaTime)
{
	m_updateForRenderWasCalled = true;
	
	(void)p_deltaTime;
	
	// update the groups for possible resorting
	for(Groups::iterator it(m_presentationGroups.begin()) ; it != m_presentationGroups.end() ; ++it)
	{
		(*it)->update();
	}
	
	//remove expired weak pointers
	//FIXME: this doesn't have to be called each frame
	for(CustomLayers::iterator it(m_customLayers.begin()) ; it != m_customLayers.end() ;)
	{
		if(it->renderable.expired())
		{
			// NOTE: Taking a copy of the iterator here, because set::erase() only invalidates
			//       iterators to elements being removed,
			//       and set::erase() does not return an iterator to the next element.
			CustomLayers::iterator eraseIt(it);
			++it;
			m_customLayers.erase(eraseIt);
		}
		else
		{
			++it;
		}
	}
}


void PresentationMgr::render() const
{
	engine::renderer::Renderer* renderer(engine::renderer::Renderer::getInstance());
	renderer->getDebug()->startRenderGroup("Presentation");
	
#if defined(TT_PLATFORM_WIN)
	TT_ASSERTMSG(m_updateForRenderWasCalled,
	             "PresentationMgr::render was called without first calling PresentationMgr::updateForRender.\n"
	             "Or you're rendering multiple viewports. (Just ignore this assert in that case.)");
#endif
	
	Groups::const_iterator groupIt  = m_presentationGroups.begin();
	CustomLayers::const_iterator customLayerIt = m_customLayers.begin();
	
	while (groupIt != m_presentationGroups.end() && customLayerIt != m_customLayers.end())
	{
		RenderableInterfacePtr renderable(customLayerIt->renderable.lock());
		if (renderable == 0)
		{
			++customLayerIt;
		}
		else if ((*groupIt)->getZRange().getMax() <= customLayerIt->depth)
		{
			(*groupIt)->render();
			++groupIt;
		}
		else if (customLayerIt->depth <= (*groupIt)->getZRange().getMin())
		{
			renderable->render();
			++customLayerIt;
		}
		else
		{
			TT_PANIC("Layer is within group range");
		}
	}
	// render any remaining presObj/renderables
	while (groupIt != m_presentationGroups.end())
	{
		(*groupIt)->render();
		++groupIt;
	}
	while (customLayerIt != m_customLayers.end())
	{
		RenderableInterfacePtr renderable(customLayerIt->renderable.lock());
		if(renderable != 0)
		{
			renderable->render();
		}
		++customLayerIt;
	}
	
	renderer->setBlendMode();

	renderer->getDebug()->endRenderGroup();
}


void PresentationMgr::renderPass(const std::string& p_passName) const
{
	engine::renderer::Renderer* renderer(engine::renderer::Renderer::getInstance());

	const std::string groupName = std::string("Presentation: ") + p_passName;
	renderer->getDebug()->startRenderGroup(groupName.c_str());

	// render all groups, but not the renderables
	for (Groups::const_iterator it = m_presentationGroups.begin(); it != m_presentationGroups.end(); ++it)
	{
		(*it)->renderPass(p_passName);
	}
	
	renderer->setBlendMode();

	renderer->getDebug()->endRenderGroup();
}


PresentationObjectPtr PresentationMgr::createPresentationObject(const std::string& p_file, 
                                                                const Tags& p_requiredTags)
{
	return PresentationCache::get(p_file, p_requiredTags, this);
}


void PresentationMgr::registerPresentationObject(PresentationObject* p_object)
{
	m_updateObjects.push_back(p_object);
	addToGroup(p_object);
}


void PresentationMgr::unregisterPresentationObject(PresentationObject* p_object)
{
	TT_NULL_ASSERT(p_object);
	
	if (p_object->isInGroup())
	{
		p_object->removeFromGroup();
	}
	
	for (PresentationObjects::iterator it(m_updateObjects.begin()) ; it != m_updateObjects.end(); ++it )
	{
		if (*it == p_object)
		{
			tt::code::helpers::unorderedErase(m_updateObjects, it);
			return;
		}
	}
	
	TT_PANIC("Cannot find presentation object '%p'", p_object);
}


void PresentationMgr::reinsertPresentationObject(PresentationObject* p_object)
{
	if(p_object == 0)
	{
		return;
	}
	
	removeFromGroup(p_object);
	addToGroup(p_object);
}


s32 PresentationMgr::particleRenderGroupFromName( const std::string& p_name ) const
{
	ParticleLayerGroupName::const_iterator it(m_particleLayers.find(p_name));
	if(it == m_particleLayers.end())
	{
		TT_ASSERTMSG(p_name == "nolayer" || m_particleLayers.empty(), 
		             "Unknown ParticleLayer name: %s\nAllowed layer names are: %s", 
		             p_name.c_str(), getPossibleParticleLayerNames().c_str());
		return -1;
	}
	else
	{
		return it->second;
	}
}


PresentationMgr::PresentationMgr(const Tags& p_acceptedTags,
                                 const TriggerFactoryInterfacePtr& p_triggerFactory,
                                 u32 p_particleCategory,
                                 const GroupFactoryInterfacePtr& p_groupFactory)
:
m_updateObjects(),
m_presentationGroups(),
m_customLayers(),
m_particleLayers(),
m_particleRenderables(),
m_acceptedTags(p_acceptedTags),
m_loader(p_acceptedTags, p_particleCategory),
m_triggerFactory(p_triggerFactory),
m_groupFactory(p_groupFactory),
m_usedForPrecache(false),
m_updateForRenderWasCalled(false)
{
}


std::string PresentationMgr::getPossibleParticleLayerNames()const
{
	if(m_particleLayers.empty()) return "No particle Layers Specified";
	std::string retStr;
	for(ParticleLayerGroupName::const_iterator it(m_particleLayers.begin()) ; 
	    it != m_particleLayers.end() ; ++it)
	{
		retStr += it->first + ", ";
	}
	return retStr;
}


void PresentationMgr::addToGroup(PresentationObject* p_object)
{
	if (p_object == 0)
	{
		return;
	}
	
	TT_ASSERTMSG(p_object->isInGroup() == false, "PresentationMgr::addToGroup - Object is still in a group");
	
	if(p_object->isInGroup())
	{
		p_object->removeFromGroup();
	}
	
	const real depth(p_object->getWorldPosition().z);
	for(Groups::const_iterator it(m_presentationGroups.begin()); it != m_presentationGroups.end() ; ++it)
	{
		const math::Range& range((*it)->getZRange());
		if (range.getMin() <= depth && range.getMax() > depth)
		{
			// add to the new group
			p_object->addToGroup(*it);
			return;
		}
	}
	
	TT_PANIC("Did not find a group to add the presentation Object to");
}


void PresentationMgr::removeFromGroup(PresentationObject* p_object)
{
	TT_ASSERTMSG(p_object != 0, "PresentationMgr::removeFromGroup - Object expired, Cannot remove from group");
	if (p_object != 0)
	{
		p_object->removeFromGroup();
	}
}


void PresentationMgr::manageGroupsForLayerAdd( real p_depth )
{
	// find the group where the layer is going to split
	for(Groups::iterator it(m_presentationGroups.begin()) ; it != m_presentationGroups.end() ; ++it)
	{
		if((*it)->getZRange().getMin() < p_depth && (*it)->getZRange().getMax() > p_depth)
		{
			// create a new group
			math::Range zRange(p_depth, (*it)->getZRange().getMax());
			GroupInterfacePtr newGroup(m_groupFactory->create(this, zRange));
			
			// now split the found group in 2
			GroupInterface::PresentationObjects objects((*it)->split(p_depth));
			
			// add all objects to the new group
			for(GroupInterface::PresentationObjects::iterator objIt(objects.begin());
			    objIt != objects.end(); ++objIt)
			{
				newGroup->add(*objIt);
			}
			
			// add the new group
			m_presentationGroups.insert(newGroup);
			return;
		}
	}
}


//namespace end
}
}
