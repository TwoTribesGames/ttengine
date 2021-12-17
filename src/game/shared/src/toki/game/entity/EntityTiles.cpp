#include <tt/code/bufferutils.h>
#include <tt/str/str.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/AttributeDebugView.h>
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {

EntityTilesWeakPtrs EntityTiles::ms_activeInstances;


//--------------------------------------------------------------------------------------------------
// Public member functions

EntityTilesPtr EntityTiles::create(const std::string&      p_tiles,
                                   const tt::math::Point2& p_pos,
                                   bool                    p_applyTilesAsActive,
                                   const EntityHandle&     p_ownerHandle)
{
	level::AttributeLayerSectionPtr entityTiles = level::AttributeLayerSection::createFromText(
		tt::str::explode(p_tiles, "\n"), p_pos);
	if (entityTiles == 0)
	{
		return EntityTilesPtr();
	}
	
	return create(entityTiles, p_applyTilesAsActive, p_ownerHandle);
}


EntityTilesPtr EntityTiles::create(const tt::math::PointRect& p_tileRect,
                                   level::CollisionType       p_type,
                                   bool                       p_applyTilesAsActive,
                                   const EntityHandle&        p_ownerHandle)
{
	level::AttributeLayerSectionPtr entityTiles = level::AttributeLayerSection::createFromRect(p_tileRect, p_type);
	if (entityTiles == 0)
	{
		return EntityTilesPtr();
	}
	
	return create(entityTiles, p_applyTilesAsActive, p_ownerHandle);
}


EntityTiles::~EntityTiles()
{
	if (AppGlobal::hasGame())
	{
		pathfinding::PathMgr& pathMgr = AppGlobal::getGame()->getPathMgr();
		pathMgr.removeTempObstacle(m_pathFindingObstacle);
	}
}


level::CollisionType EntityTiles::getCollisionType(const tt::math::Point2& p_worldPos) const
{
	return contains(p_worldPos) ?
			getCollisionTypeLocal(p_worldPos - m_entityTiles->getPosition()) :
			level::CollisionType_Invalid;
}


void EntityTiles::setActive(bool p_active)
{
	if (p_active != m_applyTilesAsActive)
	{
		m_applyTilesAsActive = p_active;
		updateTileGraphicsVertexColor();
	}
}


void EntityTiles::onNewTilePosition(const tt::math::Point2& p_newTilePos)
{
	// Move the tiles to the new position
	m_entityTiles->setPosition(p_newTilePos);
	
	createPathFindingObstacle();
}


void EntityTiles::debugRenderAllInstances()
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	for (EntityTilesWeakPtrs::iterator it = ms_activeInstances.begin();
	     it != ms_activeInstances.end(); ++it)
	{
		EntityTilesPtr instance((*it).second.lock());
		if (instance != 0)
		{
			TT_NULL_ASSERT(instance->m_tileGraphics);
			instance->m_tileGraphics->setPosition(tt::math::Vector3(
					level::tileToWorld(instance->m_entityTiles->getPosition().x),
					level::tileToWorld(instance->m_entityTiles->getPosition().y),
					0.0f));
			instance->m_tileGraphics->render();
		}
	}
#endif
}


void EntityTiles::handlePathMgrReset()
{
	for (EntityTilesWeakPtrs::iterator it = ms_activeInstances.begin();
	     it != ms_activeInstances.end(); ++it)
	{
		EntityTilesPtr instance((*it).second.lock());
		if (instance != 0)
		{
			instance->m_pathFindingObstacle = pathfinding::g_invalidObstacleIndex; // path manager was reset: reference is no longer valid
			instance->createPathFindingObstacle();
		}
	}
}


void EntityTiles::serialize(const EntityTilesPtr& p_value, tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	bu::put(p_value != 0, p_context);  // save whether there is an EntityTiles instance
	
	if (p_value == 0)
	{
		return;
	}
	
	bu::put(p_value->m_entityTiles->getPosition(), p_context);
	
	level::AttributeLayerPtr tileData = p_value->m_entityTiles->getAttributeLayer();
	bu::put(tileData->getWidth(),   p_context);
	bu::put(tileData->getHeight(),  p_context);
	bu::put(tileData->getRawData(), static_cast<size_t>(tileData->getLength()), p_context);
	
	bu::put(p_value->m_applyTilesAsActive, p_context);
	
	bu::putHandle(p_value->m_ownerHandle, p_context);
}


EntityTilesPtr EntityTiles::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const bool haveValue = bu::get<bool>(p_context);
	if (haveValue == false)
	{
		// No EntityTiles instance was saved: done unserializing this object
		return EntityTilesPtr();
	}
	
	const tt::math::Point2 pos    = bu::get<tt::math::Point2>(p_context);
	const s32              width  = bu::get<s32             >(p_context);
	const s32              height = bu::get<s32             >(p_context);
	
	level::AttributeLayerSectionPtr section = level::AttributeLayerSection::createEmpty(
			tt::math::PointRect(pos, width, height));
	
	level::AttributeLayerPtr tileData = section->getAttributeLayer();
	bu::get(tileData->getRawData(), tileData->getLength(), p_context);
	
	const bool         applyTilesAsActive = bu::get<bool>(p_context);
	const EntityHandle ownerHandle        = bu::getHandle<Entity>(p_context);
	
	return create(section, applyTilesAsActive, ownerHandle);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

EntityTilesPtr EntityTiles::create(const level::AttributeLayerSectionPtr& p_entityTiles,
                                   bool                                   p_applyTilesAsActive,
                                   const EntityHandle&                    p_ownerHandle)
{
	EntityTilesPtr instance(new EntityTiles(p_entityTiles, p_applyTilesAsActive, p_ownerHandle),
	                        removeInstance);
	ms_activeInstances[instance.get()] = instance;
	
	instance->createPathFindingObstacle();
	
	return instance;
}


EntityTiles::EntityTiles(const level::AttributeLayerSectionPtr& p_entityTiles,
                         bool                                   p_applyTilesAsActive,
                         const EntityHandle&                    p_ownerHandle)
:
m_entityTiles(p_entityTiles),
m_applyTilesAsActive(p_applyTilesAsActive),
m_ownerHandle(p_ownerHandle),
m_containedCollisionTypes(),
m_pathFindingObstacle(pathfinding::g_invalidObstacleIndex)
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
,
m_tileGraphics(AttributeDebugView::create(p_entityTiles->getAttributeLayer(),
                                          AttributeDebugView::ViewMode_CollisionType))
#endif
{
	TT_NULL_ASSERT(m_entityTiles);
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	if (m_tileGraphics != 0)
	{
		updateTileGraphicsVertexColor();
		m_tileGraphics->update();
	}
#endif
	
	// Compose a bitmask of all the collision types contained in these tiles
	const s32 layerLength = m_entityTiles->getAttributeLayer()->getLength();
	const u8* layerData   = m_entityTiles->getAttributeLayer()->getRawData();
	for (s32 i = 0; i < layerLength; ++i, ++layerData)
	{
		m_containedCollisionTypes.setFlag(level::getCollisionType(*layerData));
	}
}


void EntityTiles::updateTileGraphicsVertexColor()
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	TT_NULL_ASSERT(m_tileGraphics);
	m_tileGraphics->setVertexColor(m_applyTilesAsActive ?
		tt::engine::renderer::ColorRGB::cyan :
		tt::engine::renderer::ColorRGB::white);
#endif
}


void EntityTiles::createPathFindingObstacle()
{
	if (isSolidForPathFinding())
	{
		// Has collision
		
		if (AppGlobal::hasGame())
		{
			pathfinding::PathMgr& pathMgr = AppGlobal::getGame()->getPathMgr();
			
			pathMgr.removeTempObstacle(m_pathFindingObstacle);
			m_pathFindingObstacle = pathMgr.addTempObstacle(level::tileToWorld(m_entityTiles->getRect()));
			
			// Martijn: removed this assert, since there might not be any pathfinding objects in the level
			//TT_ASSERTMSG(m_pathFindingObstacle != pathfinding::g_invalidObstacleIndex,
			//             "Creating a path finding obstacle for entity tiles failed.");
		}
	}
}


void EntityTiles::removeInstance(EntityTiles* p_instance)
{
	if (p_instance == 0)
	{
		return;
	}
	
	EntityTilesWeakPtrs::iterator it = ms_activeInstances.find(p_instance);
	if (it != ms_activeInstances.end())
	{
		ms_activeInstances.erase(it);
	}
	
	delete p_instance;
}

// Namespace end
}
}
}
