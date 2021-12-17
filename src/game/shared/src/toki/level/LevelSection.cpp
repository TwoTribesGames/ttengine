#include <algorithm>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/LevelSection.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace level {

//--------------------------------------------------------------------------------------------------
// Public member functions

LevelSectionPtr LevelSection::createFromLevelRect(
		const LevelDataPtr&        p_levelToCopyFrom,
		const tt::math::PointRect& p_rectToCopy,
		Type                       p_type)
{
	if (p_levelToCopyFrom == 0)
	{
		TT_PANIC("Invalid source level specified. Use createEmpty if an empty section is needed.");
		return LevelSectionPtr();
	}
	
	LevelSectionPtr section = createEmpty(p_rectToCopy, p_type);
	if (section == 0)
	{
		return LevelSectionPtr();
	}
	
	section->copyFromLevel(p_levelToCopyFrom);
	
	return section;
}


LevelSectionPtr LevelSection::createEmpty(const tt::math::PointRect& p_rect, Type p_type)
{
	if (p_rect.getWidth() == 0 || p_rect.getHeight() == 0)
	{
		TT_PANIC("Attribute layer sections must be at least 1 x 1 tile (requested %d x %d).",
		         p_rect.getWidth(), p_rect.getHeight());
		return LevelSectionPtr();
	}
	
	AttributeLayerPtr attribs = AttributeLayer::create(p_rect.getWidth(), p_rect.getHeight());
	if (attribs == 0)
	{
		return LevelSectionPtr();
	}
	
	attribs->clear();
	
	return LevelSectionPtr(new LevelSection(p_rect.getPosition(), attribs, p_type));
}


LevelSection::~LevelSection()
{
}


void LevelSection::applyToLevel(const LevelDataPtr& p_targetLevel) const
{
	TT_NULL_ASSERT(p_targetLevel);
	if (p_targetLevel == 0)
	{
		return;
	}
	
	// Apply attributes
	if ((m_type & Type_Attributes) != 0)
	{
		AttributeLayerPtr targetLayer(p_targetLevel->getAttributeLayer());
		
		tt::math::Point2 pos(0, 0);
		for (pos.y = 0; pos.y < m_attributes->getHeight(); ++pos.y)
		{
			for (pos.x = 0; pos.x < m_attributes->getWidth(); ++pos.x)
			{
				const tt::math::Point2 targetPos(m_position + pos);
			
				if (targetLayer->contains(targetPos))
				{
					targetLayer->setCollisionType(targetPos, m_attributes->getCollisionType(pos));
					targetLayer->setThemeType    (targetPos, m_attributes->getThemeType(pos));
				}
			}
		}
	}
	
	// Make sure only mission specific entities are affected
	// FIXME: Ideally these have been filtered out earlier
	const std::string& missionID(AppGlobal::getGame()->getMissionID());
	level::entity::EntityInstances missionSpecificEntities;
	game::entity::EntityMgr::appendMissionSpecificEntities(p_targetLevel->getAllEntities(), missionID, missionSpecificEntities);
	
	// Apply entities
	if ((m_type & Type_Entities) != 0)
	{
		// Erase all entities in the target rect
		const tt::math::VectorRect sectionWorldRect(makeEntitySelectionWorldRect(getRect()));
		for (entity::EntityInstances::const_iterator it = missionSpecificEntities.begin();
			it != missionSpecificEntities.end(); ++it)
		{
			if (sectionWorldRect.contains((*it)->getPosition()))
			{
				p_targetLevel->removeEntity(*it);
			}
		}
		
		// Add all entities from this section to the target level
		entity::OldNewIDs       oldNewIDs;
		entity::EntityInstances newInstances;
		
		const tt::math::VectorRect targetWorldRect(tileToWorld(p_targetLevel->getLevelRect()));
		const tt::math::Vector2    entityOffset(tileToWorld(m_position));
		for (entity::EntityInstances::const_iterator it = m_entities.begin();
		     it != m_entities.end(); ++it)
		{
			const tt::math::Vector2 newPos((*it)->getPosition() + entityOffset);
			if (targetWorldRect.contains(newPos))
			{
				entity::EntityInstancePtr instance;
				const s32 oldID = (*it)->getID();
				if (p_targetLevel->hasEntity(oldID))
				{
					instance = (*it)->cloneWithNewID(p_targetLevel->createEntityID());
				}
				else
				{
					instance = (*it)->clone();
				}
				instance->setPosition(newPos);
				
				if (p_targetLevel->addEntity(instance))
				{
					oldNewIDs.push_back(entity::OldNewID(oldID, instance->getID()));
					newInstances.push_back(instance);
				}
			}
		}
		
		// Fix the entity-to-entity references (so they remain valid)
		entity::updateInternalEntityReferences(newInstances, oldNewIDs, p_targetLevel);
	}
}


tt::math::PointRect LevelSection::getRect() const
{
	return tt::math::PointRect(m_position, m_attributes->getWidth(), m_attributes->getHeight());
}


LevelSectionPtr LevelSection::clone() const
{
	return LevelSectionPtr(new LevelSection(*this));
}


void LevelSection::addEntity(const entity::EntityInstancePtr& p_entity)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity == 0 || ((m_type & Type_Entities) == 0))
	{
		return;
	}
	
	TT_ASSERTMSG(std::find(m_entities.begin(), m_entities.end(), p_entity) == m_entities.end(),
	             "EntityInstance %p (type '%s', ID %d) was already added to level section %p.",
	             p_entity.get(), p_entity->getType().c_str(), p_entity->getID(), this);
	
	m_entities.push_back(p_entity);
}


void LevelSection::addEntities(const entity::EntityInstances& p_entities)
{
	if (((m_type & Type_Entities) == 0))
	{
		return;
	}
	
	for (entity::EntityInstances::const_iterator it = p_entities.begin();
	     it != p_entities.end(); ++it)
	{
		addEntity(*it);
	}
}


#if !defined(TT_BUILD_FINAL)
void LevelSection::debugPrint() const
{
	TT_Printf("LevelSection::debugPrint: Section at pos (%d, %d), size %d x %d, containing %u entities:\n",
	          m_position.x, m_position.y, m_attributes->getWidth(), m_attributes->getHeight(),
	          u32(m_entities.size()));
	
	tt::math::Point2 pos;
	TT_Printf("LevelSection::debugPrint: +%s+\n", std::string(m_attributes->getWidth(), '-').c_str());
	for (pos.y = m_attributes->getHeight() - 1; pos.y >= 0; --pos.y)
	{
		TT_Printf("LevelSection::debugPrint: |");
		for (pos.x = 0; pos.x < m_attributes->getWidth(); ++pos.x)
		{
			const CollisionType type = m_attributes->getCollisionType(pos);
			if (isValidCollisionType(type))
			{
				TT_Printf("%c", getCollisionTypeAsChar(type));
			}
			else
			{
				TT_Printf("?");
			}
		}
		TT_Printf("|\n");
	}
	TT_Printf("LevelSection::debugPrint: +%s+\n", std::string(m_attributes->getWidth(), '-').c_str());
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

LevelSection::LevelSection(const tt::math::Point2&  p_position,
                           const AttributeLayerPtr& p_attributes,
                           Type                     p_type)
:
m_position(p_position),
m_attributes(p_attributes),
m_entities(),
m_type(p_type)
{
}


LevelSection::LevelSection(const LevelSection& p_rhs)
:
m_position(p_rhs.m_position),
m_attributes(p_rhs.m_attributes->clone()),
m_entities(),
m_type(p_rhs.m_type)
{
	m_entities.reserve(p_rhs.m_entities.size());
	for (entity::EntityInstances::const_iterator it = p_rhs.m_entities.begin();
	     it != p_rhs.m_entities.end(); ++it)
	{
		m_entities.push_back((*it)->clone());
	}
}


void LevelSection::copyFromLevel(const LevelDataPtr& p_sourceLevel)
{
	TT_NULL_ASSERT(p_sourceLevel);
	if (p_sourceLevel == 0)
	{
		return;
	}
	
	// Copy attributes
	if ((m_type & Type_Attributes) != 0)
	{
		AttributeLayerPtr sourceLayer(p_sourceLevel->getAttributeLayer());
		
		tt::math::Point2 pos(0, 0);
		for (pos.y = 0; pos.y < m_attributes->getHeight(); ++pos.y)
		{
			for (pos.x = 0; pos.x < m_attributes->getWidth(); ++pos.x)
			{
				const tt::math::Point2 sourcePos(m_position + pos);
				const bool inSourceBounds = sourceLayer->contains(sourcePos);
				
				m_attributes->setCollisionType(
					pos,
					inSourceBounds ?
						sourceLayer->getCollisionType(sourcePos) :
						CollisionType_Air);
				
				m_attributes->setThemeType(
					pos,
					inSourceBounds ?
						sourceLayer->getThemeType(sourcePos) :
						ThemeType_UseLevelDefault);
			}
		}
	}
	
	// Copy entities
	if ((m_type & Type_Entities) != 0)
	{
		// Copy entities whose position is inside this section's rect
		// FIXME: Take size of entities into account (how? data layer has no knowledge of these sizes)
		// Make sure only mission specific entities are affected
		// FIXME: Ideally these have been filtered out earlier
		const std::string& missionID(AppGlobal::getGame()->getMissionID());
		level::entity::EntityInstances missionSpecificEntities;
		game::entity::EntityMgr::appendMissionSpecificEntities(p_sourceLevel->getAllEntities(), missionID, missionSpecificEntities);
		
		const tt::math::VectorRect worldRect(makeEntitySelectionWorldRect(getRect()));
		const tt::math::Vector2    entityOffset(tileToWorld(m_position));
		for (entity::EntityInstances::const_iterator it = missionSpecificEntities.begin();
			it != missionSpecificEntities.end(); ++it)
		{
			if (worldRect.contains((*it)->getPosition()))
			{
				entity::EntityInstancePtr sectionEntity((*it)->clone());
				sectionEntity->setPosition(sectionEntity->getPosition() - entityOffset);
				m_entities.push_back(sectionEntity);
			}
		}
	}
}

// Namespace end
}
}
