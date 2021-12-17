#include <sstream>

#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/level/entity/EntityInfo.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/entity/EntityProperty.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace level {
namespace entity {

void removeUnreferencedEntityReferences(const LevelDataPtr& p_targetLevel)
{
	const EntityInstances& entities(p_targetLevel->getAllEntities());
	EntityIDSet validEntityIDs;
	
	for (EntityInstances::const_iterator instanceIt = entities.begin();
	     instanceIt != entities.end(); ++instanceIt)
	{
		validEntityIDs.insert((*instanceIt)->getID());
	}
	
	TT_NULL_ASSERT(p_targetLevel);
	
	// Go through all entities and update any internal references to other entities
	const game::entity::EntityLibrary& entityLib(AppGlobal::getEntityLibrary());
	for (EntityInstances::const_iterator instanceIt = entities.begin();
	     instanceIt != entities.end(); ++instanceIt)
	{
		const EntityInstancePtr& instance(*instanceIt);
		if (instance == 0)
		{
			// Ignore null pointers (shouldn't be in the container, but shouldn't crash either)
			TT_PANIC("Entity instance container contains a null entity instance pointer.");
			continue;
		}
		
		const EntityInfo* typeInfo = entityLib.getEntityInfo(instance->getType());
		if (typeInfo == 0)
		{
			// Unsupported/unknown entity type: ignore
			continue;
		}
		
		const EntityInstance::Properties props(instance->getProperties());
		for (EntityInstance::Properties::const_iterator propIt = props.begin();
		     propIt != props.end(); ++propIt)
		{
			const std::string& propName((*propIt).first);
			if (typeInfo->hasProperty(propName) == false)
			{
				// Entity type does not (any longer) have this property: ignore
				continue;
			}
			
			const EntityProperty& typeProp(typeInfo->getProperty(propName));
			
			if (typeProp.getType() == EntityProperty::Type_Entity ||
			    typeProp.getType() == EntityProperty::Type_EntityID ||
			    typeProp.getType() == EntityProperty::Type_DelayedEntityID)
			{
				// Found an entity-to-entity reference
				TT_ERR_CREATE("");
				const s32 entityID = tt::str::parseS32((*propIt).second, &errStatus);
				if (errStatus.hasError())
				{
					// Not a valid entity ID: remove and continue
					instance->removeProperty(propName);
					continue;
				}
				
				// Now find out if it points to one of the existing entities
				if (validEntityIDs.find(entityID) == validEntityIDs.end())
				{
					// Entity does not exist in the level or points to an entity of the wrong type:
					// remove the reference
					instance->removeProperty(propName);
				}
			}
			else if (typeProp.getType() == EntityProperty::Type_EntityArray ||
			         typeProp.getType() == EntityProperty::Type_EntityIDArray ||
			         typeProp.getType() == EntityProperty::Type_DelayedEntityIDArray)
			{
				// Found an entity-to-entity reference
				tt::str::Strings ids(tt::str::explode((*propIt).second, ","));
				tt::str::Strings updatedIds;
				bool             removedEntities = false;
				for (tt::str::Strings::iterator valIt = ids.begin(); valIt != ids.end(); ++valIt)
				{
					TT_ERR_CREATE("");
					const s32 entityID = tt::str::parseS32(*valIt, &errStatus);
					if (errStatus.hasError())
					{
						// Not a valid entity ID: don't save it
						removedEntities = true;
						continue;
					}
					
					// Now find out if it points to one of the existing entities
					if (validEntityIDs.find(entityID) != validEntityIDs.end())
					{
						updatedIds.push_back(*valIt);
					}
					else
					{
						removedEntities = true;
					}
				}
				
				if (removedEntities)
				{
					// Changes to this property value were made: set the new value
					if (updatedIds.empty())
					{
						instance->removeProperty(propName);
					}
					else
					{
						instance->setPropertyValue(propName, tt::str::implode(updatedIds, ","));
					}
				}
			}
		}
	}
}


void updateInternalEntityReferences(const EntityInstances& p_instances,
                                    const OldNewIDs&       p_oldAndNewIDs,
                                    const LevelDataPtr&    p_targetLevel)
{
	TT_NULL_ASSERT(p_targetLevel);
	
	// Go through all entities and update any internal references to other entities
	const game::entity::EntityLibrary& entityLib(AppGlobal::getEntityLibrary());
	for (EntityInstances::const_iterator instanceIt = p_instances.begin();
	     instanceIt != p_instances.end(); ++instanceIt)
	{
		const EntityInstancePtr& instance(*instanceIt);
		if (instance == 0)
		{
			// Ignore null pointers (shouldn't be in the container, but shouldn't crash either)
			TT_PANIC("Entity instance container contains a null entity instance pointer.");
			continue;
		}
		
		const EntityInfo* typeInfo = entityLib.getEntityInfo(instance->getType());
		if (typeInfo == 0)
		{
			// Unsupported/unknown entity type: ignore
			continue;
		}
		
		const EntityInstance::Properties props(instance->getProperties());
		for (EntityInstance::Properties::const_iterator propIt = props.begin();
		     propIt != props.end(); ++propIt)
		{
			const std::string& propName((*propIt).first);
			if (typeInfo->hasProperty(propName) == false)
			{
				// Entity type does not (any longer) have this property: ignore
				continue;
			}
			
			const EntityProperty& typeProp(typeInfo->getProperty(propName));
			if (typeProp.hasChoice())
			{
				// Not interested in combo boxes
				continue;
			}
			
			if (typeProp.getType() == EntityProperty::Type_Entity ||
			    typeProp.getType() == EntityProperty::Type_EntityID ||
			    typeProp.getType() == EntityProperty::Type_DelayedEntityID)
			{
				// Found an entity-to-entity reference
				TT_ERR_CREATE("");
				const s32 entityID = tt::str::parseS32((*propIt).second, &errStatus);
				if (errStatus.hasError())
				{
					// Not a valid entity ID: ignore
					continue;
				}
				
				// Now find out if it points to one of the entities in the collection that was passed
				bool idRefersToCopiedEntity = false;
				for (OldNewIDs::const_iterator idIt = p_oldAndNewIDs.begin();
				     idIt != p_oldAndNewIDs.end(); ++idIt)
				{
					if ((*idIt).first == entityID)
					{
						// It does: update the property value to the new ID
						idRefersToCopiedEntity = true;
						
						// New ID is different; make sure instance knows this
						if ((*idIt).first != (*idIt).second)
						{
							instance->setPropertyValue(propName, tt::str::toStr((*idIt).second));
						}
						break;
					}
				}
				
				if (idRefersToCopiedEntity == false)
				{
					// Could not find the entity ID in the entities that were passed:
					// ensure it points to a valid entity in the level data
					const tt::str::StringSet& entityTypeFilter(typeProp.getFilter());
					EntityInstancePtr referencedEntity(p_targetLevel->getEntityByID(entityID));
					
					if (referencedEntity == 0 || referencedEntity->matchesFilter(entityTypeFilter) == false)
					{
						// Entity does not exist in the level or points to an entity of the wrong type:
						// remove the reference
						instance->removeProperty(propName);
					}
				}
			}
			else if (typeProp.getType() == EntityProperty::Type_EntityArray ||
			         typeProp.getType() == EntityProperty::Type_EntityIDArray ||
			         typeProp.getType() == EntityProperty::Type_DelayedEntityIDArray)
			{
				// Found an entity-to-entity reference
				tt::str::Strings ids(tt::str::explode((*propIt).second, ","));
				bool             madeChanges = false;
				for (tt::str::Strings::iterator valIt = ids.begin(); valIt != ids.end(); )
				{
					TT_ERR_CREATE("");
					const s32 entityID = tt::str::parseS32(*valIt, &errStatus);
					if (errStatus.hasError())
					{
						// Not a valid entity ID: ignore
						++valIt;
						continue;
					}
					
					// Now find out if it points to one of the entities in the collection that was passed
					bool idRefersToCopiedEntity = false;
					for (OldNewIDs::const_iterator idIt = p_oldAndNewIDs.begin();
					     idIt != p_oldAndNewIDs.end(); ++idIt)
					{
						if ((*idIt).first == entityID)
						{
							// It does: update the property value to the new ID
							idRefersToCopiedEntity = true;
							
							// New ID is different; make sure instance knows this
							if ((*idIt).first != (*idIt).second)
							{
								*valIt      = tt::str::toStr((*idIt).second);
								madeChanges = true;
							}
							break;
						}
					}
					
					bool eraseEntry = false;
					if (idRefersToCopiedEntity == false)
					{
						// Could not find the entity ID in the entities that were passed:
						// ensure it points to a valid entity in the level data
						const tt::str::StringSet& entityTypeFilter(typeProp.getFilter());
						EntityInstancePtr referencedEntity(p_targetLevel->getEntityByID(entityID));
						
						if (referencedEntity == 0 || referencedEntity->matchesFilter(entityTypeFilter) == false)
						{
							// Entity does not exist in the level or points to an entity of the wrong type:
							// remove the reference
							eraseEntry = true;
						}
					}
					
					if (eraseEntry)
					{
						valIt = ids.erase(valIt);
						madeChanges = true;
					}
					else
					{
						++valIt;
					}
				}
				
				if (madeChanges)
				{
					// Changes to this property value were made: set the new value
					if (ids.empty())
					{
						instance->removeProperty(propName);
					}
					else
					{
						instance->setPropertyValue(propName, tt::str::implode(ids, ","));
					}
				}
			}
		}
	}
}


tt::engine::renderer::ColorRGB parseColorRGBProperty(const std::string&     p_propertyValue,
                                                     tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(tt::engine::renderer::ColorRGB, tt::engine::renderer::ColorRGB::black,
	             "Parsing entity property value as ColorRGB.");
	
	const tt::str::Strings rgbComponents(tt::str::explode(p_propertyValue, ","));
	
	TT_ERR_ASSERTMSG(rgbComponents.size() == 3,
	                 "Can't parse ColorRGB property value '" << p_propertyValue << "': "
	                 "incorrect number of color components specified. Expected 3 components "
	                 "('r,g,b'), got " << rgbComponents.size() << ".");
	
	tt::engine::renderer::ColorRGB color(tt::engine::renderer::ColorRGB::black);
	if (rgbComponents.size() == 3)
	{
		color.r = tt::str::parseU8(rgbComponents[0], &errStatus);
		color.g = tt::str::parseU8(rgbComponents[1], &errStatus);
		color.b = tt::str::parseU8(rgbComponents[2], &errStatus);
	}
	
	return color;
}


tt::engine::renderer::ColorRGBA parseColorRGBAProperty(const std::string&     p_propertyValue,
                                                       tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(tt::engine::renderer::ColorRGBA, tt::engine::renderer::ColorRGBA(0, 0, 0, 0),
	             "Parsing entity property value as ColorRGBA.");
	
	const tt::str::Strings rgbComponents(tt::str::explode(p_propertyValue, ","));
	
	const tt::str::Strings::size_type componentCount = rgbComponents.size();
	TT_ERR_ASSERTMSG(componentCount == 3 || componentCount == 4,  // allow implicit alpha component
	                 "Can't parse ColorRGBA property value '" << p_propertyValue << "': "
	                 "incorrect number of color components specified. Expected 3 or 4 components "
	                 "('r,g,b[,a]'), got " << componentCount << ".");
	
	tt::engine::renderer::ColorRGBA color(0, 0, 0, 0);
	if (componentCount == 3 || componentCount == 4)
	{
		color.r = tt::str::parseU8(rgbComponents[0], &errStatus);
		color.g = tt::str::parseU8(rgbComponents[1], &errStatus);
		color.b = tt::str::parseU8(rgbComponents[2], &errStatus);
		if (componentCount == 4)
		{
			color.a = tt::str::parseU8(rgbComponents[3], &errStatus);
		}
		else
		{
			color.a = 255;  // implicitly add an alpha value of 255, for upgrades from color_rgb to color_rgba
		}
	}
	
	return color;
}


std::string makePropertyString(const tt::engine::renderer::ColorRGB& p_color)
{
	std::ostringstream oss;
	oss << static_cast<u32>(p_color.r) << ","
	    << static_cast<u32>(p_color.g) << ","
	    << static_cast<u32>(p_color.b);
	return oss.str();
}


std::string makePropertyString(const tt::engine::renderer::ColorRGBA& p_color)
{
	std::ostringstream oss;
	oss << static_cast<u32>(p_color.r) << ","
	    << static_cast<u32>(p_color.g) << ","
	    << static_cast<u32>(p_color.b) << ","
	    << static_cast<u32>(p_color.a);
	return oss.str();
}

// Namespace end
}
}
}
