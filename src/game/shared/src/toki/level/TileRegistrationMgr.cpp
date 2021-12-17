#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/Game.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace level {


//--------------------------------------------------------------------------------------------------
// Public member functions

TileRegistrationMgr::TileRegistrationMgr()
:
m_registeredEntityHandles(0),
m_registeredEntityTiles(0),
m_collisionTypes(0),
m_hasEntities(0),
m_changedTiles(),
m_levelLayer(),
m_levelBounds(0,0),
m_tilesCount(0),
m_cellBounds(0,0),
m_cellCount(0)
{
#if defined(USE_STD_VECTOR)
	const u32 reserveSize = 200 * 200;
	m_registeredEntityHandles.reserve(reserveSize);
	m_registeredEntityTiles	 .reserve(reserveSize);
	m_collisionTypes		 .reserve(reserveSize);
	m_hasEntities			 .reserve(reserveSize);
#endif
}


TileRegistrationMgr::~TileRegistrationMgr()
{
#if !defined(USE_STD_VECTOR)
	delete[] m_registeredEntityHandles;
	delete[] m_registeredEntityTiles;
	delete[] m_collisionTypes;
	delete[] m_hasEntities;
#endif
}


void TileRegistrationMgr::reset()
{
#if !defined(USE_STD_VECTOR)
	if (m_registeredEntityTiles != 0)
#endif
	{
		for (s32 i = 0; i < m_tilesCount; ++i)
		{
			m_registeredEntityTiles[i].clear();
		}
		
		for (s32 i = 0; i < m_cellCount; ++i)
		{
			m_registeredEntityHandles[i].clear();
			m_hasEntities[i] = false;
		}
	}
	m_changedTiles.clear();
	getCollisionTypesFromLevel();
}


void TileRegistrationMgr::registerEntityHandle(const tt::math::PointRect&        p_tiles,
                                               const game::entity::EntityHandle& p_entityHandle)
{
	const game::entity::Entity* entity = p_entityHandle.getPtr();
	TT_NULL_ASSERT(entity);
	if (entity == 0 || entity->isTileRegistrationEnabled() == false)
	{
		return;
	}
	TT_ASSERT(entity->isInitialized());
	
	const tt::math::Point2 minPos = tt::math::pointMax(p_tiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 maxPos = tt::math::pointMin(p_tiles.getMaxEdge(), m_levelBounds);
	
	const s32 minx = (minPos.x / cellSize) * cellSize;
	const s32 miny = (minPos.y / cellSize) * cellSize;
	const s32 maxx = static_cast<s32>(tt::math::ceil(maxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 maxy = static_cast<s32>(tt::math::ceil(maxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	for (s32 x = minx; x < maxx; x += cellSize)
	{
		for (s32 y = miny; y < maxy; y += cellSize)
		{
			const s32 cellIndex = getCellIndex(x, y);
			m_registeredEntityHandles[cellIndex].insert(p_entityHandle);
			m_hasEntities[cellIndex] = true;
		}
	}
}


void TileRegistrationMgr::unregisterEntityHandle(const tt::math::PointRect&        p_tiles,
                                                 const game::entity::EntityHandle& p_entityHandle)
{
	const game::entity::Entity* entity = p_entityHandle.getPtr();
	TT_NULL_ASSERT(entity);
	if (entity == 0 || entity->isTileRegistrationEnabled() == false)
	{
		return;
	}
	
	const tt::math::Point2 minPos = tt::math::pointMax(p_tiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 maxPos = tt::math::pointMin(p_tiles.getMaxEdge(), m_levelBounds);
	
	const s32 minx = (minPos.x / cellSize) * cellSize;
	const s32 miny = (minPos.y / cellSize) * cellSize;
	const s32 maxx = static_cast<s32>(tt::math::ceil(maxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 maxy = static_cast<s32>(tt::math::ceil(maxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	for (s32 x = minx; x < maxx; x += cellSize)
	{
		for (s32 y = miny; y < maxy; y += cellSize)
		{
			const s32 cellIndex = getCellIndex(x, y);
			
			EntityHandleSet::iterator it = m_registeredEntityHandles[cellIndex].find(p_entityHandle);
			
			if (it != m_registeredEntityHandles[cellIndex].end())
			{
				// Remove this entry from the set
				m_registeredEntityHandles[cellIndex].erase(it);
			}
			else
			{
				TT_PANIC("Trying to unregister EntityHandle '%d' on cell location (%d, %d) without that entity", p_entityHandle.getValue(), x, y);
			}

			if(m_registeredEntityHandles[cellIndex].empty())
			{
				m_hasEntities[cellIndex] = false;
			}
		}
	}
}


void TileRegistrationMgr::moveRegisterEntityHandle(const tt::math::PointRect&        p_prevTiles,
                                                   const tt::math::PointRect&        p_newTiles,
                                                   const game::entity::EntityHandle& p_entityHandle)
{
	const game::entity::Entity* entity = p_entityHandle.getPtr();
	TT_NULL_ASSERT(entity);
	if (entity == 0 || entity->isTileRegistrationEnabled() == false)
	{
		return;
	}
	TT_ASSERT(entity->isInitialized());
	
	const tt::math::Point2 prevMinPos = tt::math::pointMax(p_prevTiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 prevMaxPos = tt::math::pointMin(p_prevTiles.getMaxEdge(), m_levelBounds);
	
	const tt::math::Point2 newMinPos = tt::math::pointMax(p_newTiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 newMaxPos = tt::math::pointMin(p_newTiles.getMaxEdge(), m_levelBounds);
	
	const s32 pminx = (prevMinPos.x / cellSize) * cellSize;
	const s32 pminy = (prevMinPos.y / cellSize) * cellSize;
	const s32 pmaxx = static_cast<s32>(tt::math::ceil(prevMaxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 pmaxy = static_cast<s32>(tt::math::ceil(prevMaxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	const s32 nminx = (newMinPos.x / cellSize) * cellSize;
	const s32 nminy = (newMinPos.y / cellSize) * cellSize;
	const s32 nmaxx = static_cast<s32>(tt::math::ceil(newMaxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 nmaxy = static_cast<s32>(tt::math::ceil(newMaxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	// Unregister
	for (s32 x = pminx; x < pmaxx; x += cellSize)
	{
		for (s32 y = pminy; y < pmaxy; y += cellSize)
		{
			if (x >= nminx && x < nmaxx &&
			    y >= nminy && y < nmaxy)
			{
				continue; // Inside new rect, ignore.
			}
			
			const s32 cellIndex = getCellIndex(x, y);
			EntityHandleSet::iterator it = m_registeredEntityHandles[cellIndex].find(p_entityHandle);
			
			if (it != m_registeredEntityHandles[cellIndex].end())
			{
				// Remove this entry from the set
				m_registeredEntityHandles[cellIndex].erase(it);
			}
			else
			{
				TT_PANIC("Trying to unregister EntityHandle '%d' on location (%d, %d) without that entity", p_entityHandle.getValue(), x, y);
			}
			
			if(m_registeredEntityHandles[cellIndex].empty())
			{
				m_hasEntities[cellIndex] = false;
			}
		}
	}
	
	// Register
	for (s32 x = nminx; x < nmaxx; x += cellSize)
	{
		for (s32 y = nminy; y < nmaxy; y += cellSize)
		{
			if (x >= pminx && x < pmaxx &&
			    y >= pminy && y < pmaxy)
			{
				continue; // Inside prev rect, ignore.
			}
			const s32 cellIndex = getCellIndex(x, y);
			m_registeredEntityHandles[cellIndex].insert(p_entityHandle);
			m_hasEntities[cellIndex] = true;
		}
	}
}


bool TileRegistrationMgr::isEntityAtPosition(const tt::math::Point2&           p_position,
                                             const game::entity::EntityHandle& p_entityHandle) const
{
	const EntityHandleSet* entities = getEntitiesAtPosition(p_position);
	if (entities != 0)
	{
		return entities->find(p_entityHandle) != entities->end();
	}
	
	return false;
}


void TileRegistrationMgr::findRegisteredEntityHandles(const tt::math::Point2&        p_position,
                                                      game::entity::EntityHandleSet& p_result) const
{
	if(hasEntityAtPosition(p_position.x, p_position.y))
	{
		const EntityHandleSet* entities = getEntitiesAtPosition(p_position);
		if (entities != 0)
		{
			return p_result.insert(entities->begin(), entities->end()); 
		}
	}
}


void TileRegistrationMgr::findRegisteredEntityHandles(const tt::math::PointRect&     p_tiles,
                                                      game::entity::EntityHandleSet& p_result) const
{
	const tt::math::Point2 minPos = tt::math::pointMax(p_tiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 maxPos = tt::math::pointMin(p_tiles.getMaxEdge(), m_levelBounds);
	
	const s32 minx = (minPos.x / cellSize) * cellSize;
	const s32 miny = (minPos.y / cellSize) * cellSize;
	const s32 maxx = static_cast<s32>(tt::math::ceil(maxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 maxy = static_cast<s32>(tt::math::ceil(maxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	for (s32 x = minx; x < maxx; x += cellSize)
	{
		for (s32 y = miny; y < maxy; y += cellSize)
		{
			if (hasEntityAtPosition(x,y))
			{
				const EntityHandleSet& setAtPosition = m_registeredEntityHandles[getCellIndex(x, y)];
				p_result.insert(setAtPosition.begin(), setAtPosition.end());
			}
		}
	}
}


const game::entity::EntityHandleSet& TileRegistrationMgr::getRegisteredEntityHandles(const tt::math::Point2& p_position) const
{
	const s32 cellIndex = getCellIndex(p_position.x, p_position.y);
	
	TT_ASSERT(cellIndex >= 0);
	TT_ASSERT(cellIndex < m_cellCount);
	
	return m_registeredEntityHandles[cellIndex];
}


void TileRegistrationMgr::registerEntityTiles(const tt::math::PointRect&          p_tiles,
                                              const game::entity::EntityTilesPtr& p_entityTiles,
                                              bool                                p_doCallbacks)
{
#if !defined(USE_STD_VECTOR)
	TT_NULL_ASSERT(m_registeredEntityTiles);
#endif
	TT_NULL_ASSERT(p_entityTiles);
	if (p_entityTiles == 0)
	{
		return;
	}
	
	const tt::math::Point2 minPos = tt::math::pointMax(p_tiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 maxPos = tt::math::pointMin(p_tiles.getMaxEdge(), m_levelBounds);
	
	for (s32 x = minPos.x; x < maxPos.x; ++x)
	{
		for (s32 y = minPos.y; y < maxPos.y; ++y)
		{
			tt::math::Point2 pos(x,y);
			m_registeredEntityTiles[x + y * m_levelBounds.x][p_entityTiles.get()] = p_entityTiles;

			const bool typeChanged = updateCollisionType(pos);

			if(typeChanged)
			{
				m_entityTilesForFluids.push_back(pos);
			}
		}
	}
	
	if (p_doCallbacks)
	{
		// Registering these EntityTiles these tile locations were 'changed'
		addChangedTiles(p_tiles);
	}
}


void TileRegistrationMgr::unregisterEntityTiles(const tt::math::PointRect&          p_tiles,
                                                const game::entity::EntityTilesPtr& p_entityTiles)
{
	TT_NULL_ASSERT(p_entityTiles);
	if (p_entityTiles == 0)
	{
		return;
	}
	
	const tt::math::Point2 minPos = tt::math::pointMax(p_tiles.getMin(), tt::math::Point2::zero);
	const tt::math::Point2 maxPos = tt::math::pointMin(p_tiles.getMaxEdge(), m_levelBounds);
	
	for (s32 x = minPos.x; x < maxPos.x; ++x)
	{
		for (s32 y = minPos.y; y < maxPos.y; ++y)
		{
			s32 tileIndex = x + y * m_levelLayer->getWidth();
			
			game::entity::EntityTilesWeakPtrs::iterator entityTilesIt =
				m_registeredEntityTiles[tileIndex].find(p_entityTiles.get());
			
			if (entityTilesIt != m_registeredEntityTiles[tileIndex].end())
			{
				// Remove this entry from the set
				tt::math::Point2 pos(x,y);
				m_registeredEntityTiles[tileIndex].erase(entityTilesIt);
				
				const bool typeChanged = updateCollisionType(pos);

				if(typeChanged)
				{
					m_entityTilesForFluids.push_back(pos);
				}
			}
			else
			{
				TT_PANIC("Trying to unregister EntityTiles on location (%d, %d) without that entity", x, y);
			}
		}
	}
	
	// Unregistering these EntityTiles these tile locations were 'changed'
	addChangedTiles(p_tiles);
}


CollisionTypes TileRegistrationMgr::getCollisionTypesFromRegisteredTiles(
		const tt::math::Point2&           p_tilePos,
		const AttributeLayerPtr&          p_levelLayer,
		const game::entity::EntityTiles*  p_entityTilesToIgnore,
		const game::entity::EntityHandle& p_collisionAncestorToIgnore,
		bool                              p_ignoreActiveTiles,
		bool                              p_checkRectForCollisionEntities) const
{
	// FIXME: Lots of code duplication between this function and getCollisionTypesFromRegisteredTiles
	
	CollisionTypes typesFound;
	
	if (p_levelLayer->contains(p_tilePos))
	{
		typesFound.setFlag(p_levelLayer->getCollisionType(p_tilePos));
	}
	
	const game::entity::EntityTilesWeakPtrs* entityTiles = getTilesAtPosition(p_tilePos);
	if (entityTiles == 0)
	{
		return typesFound;
	}
	
	for (game::entity::EntityTilesWeakPtrs::const_iterator it = entityTiles->begin();
	     it != entityTiles->end(); ++it)
	{
		game::entity::EntityTilesPtr tiles((*it).second.lock());
		if (tiles == 0)
		{
			TT_PANIC("An EntityTiles pointer at location (%d, %d) is no longer valid.\n"
			         "This probably means that this location wasn't unregistered when it should have been.",
			         p_tilePos.x, p_tilePos.y);
			continue;
		}
		
		// If these tiles should be ignored, or if these tiles are active and active tiles should be ignored:
		// do not continue checking this EntityTiles instance
		if (tiles.get() == p_entityTilesToIgnore ||
		   (p_ignoreActiveTiles && tiles->isActive()))
		{
			continue;
		}
		
		if (p_collisionAncestorToIgnore.isEmpty() == false)
		{
			const game::entity::Entity* entity = tiles->getOwner().getPtr();
			if (entity != 0 && entity->getCachedCollisionAncestor() == p_collisionAncestorToIgnore)
			{
				continue;
			}
		}
		
		const CollisionType currentType = tiles->getCollisionType(p_tilePos);
		if (isValidCollisionType(currentType) == false ||
		    currentType == CollisionType_Air)  // Never report "Air" tiles: in the original "apply to layer" setup, these weren't applied either
		{
			continue;
		}
		
		if (p_checkRectForCollisionEntities && level::isSolid(currentType))
		{
			const game::entity::Entity* owner = tiles->getOwner().getPtr();
			if (owner != 0)
			{
				const tt::math::Vector2 centerOfTile(level::tileToWorld(p_tilePos) + tt::math::Vector2(0.5f, 0.5f));
				const tt::math::Vector2 halfSmallRectSize(0.4f, 0.4f);
				const tt::math::VectorRect tileCenterRect(centerOfTile - halfSmallRectSize, centerOfTile + halfSmallRectSize);
				const tt::math::VectorRect entityRect(owner->calcEntityTileRect());
				if (entityRect.contains(tileCenterRect) == false)
				{
					// Not inside the vector rect for entities tiles
					continue;
				}
			}
		}
		
		typesFound.setFlag(currentType);
	}
	
	return typesFound;
}


CollisionType TileRegistrationMgr::getCollisionTypeFromRegisteredTiles(
		const tt::math::Point2&           p_tilePos,
		const AttributeLayerPtr&          p_levelLayer,
		const game::entity::EntityTiles*  p_entityTilesToIgnore,
		const game::entity::EntityHandle& p_collisionAncestorToIgnore,
		bool                              p_ignoreActiveTiles) const
{
	// FIXME: It would be best if all this pointer passing and checking was moved to a single static
	//        "checking manager" (or whatever), which should be the sole reference point for all tile checking client code.
	
	CollisionType typeToReturn = CollisionType_Invalid;
	
	if (p_levelLayer->contains(p_tilePos))
	{
		typeToReturn = p_levelLayer->getCollisionType(p_tilePos);
		
		// Solid types have priority over all others
		if (level::isSolid(typeToReturn))
		{
			return typeToReturn;
		}
	}
	//*
	const game::entity::EntityTilesWeakPtrs* entityTiles = getTilesAtPosition(p_tilePos);
	if (entityTiles == 0)
	{
		return typeToReturn;
	}
	
	for (game::entity::EntityTilesWeakPtrs::const_iterator it = entityTiles->begin();
	     it != entityTiles->end(); ++it)
	{
		game::entity::EntityTilesPtr tiles((*it).second.lock());
		if (tiles == 0)
		{
			TT_PANIC("An EntityTiles pointer at location (%d, %d) is no longer valid.\n"
			         "This probably means that this location wasn't unregistered when it should have been.",
			         p_tilePos.x, p_tilePos.y);
			continue;
		}
		
		// If these tiles should be ignored, or if these tiles are active and active tiles should be ignored:
		// do not continue checking this EntityTiles instance
		if (tiles.get() == p_entityTilesToIgnore ||
		   (p_ignoreActiveTiles && tiles->isActive()))
		{
			continue;
		}
		
		if (p_collisionAncestorToIgnore.isEmpty() == false)
		{
			const game::entity::Entity* entity = tiles->getOwner().getPtr();
			if (entity != 0 && entity->getCachedCollisionAncestor() == p_collisionAncestorToIgnore)
			{
				continue;
			}
		}
		
		const CollisionType currentType = tiles->getCollisionType(p_tilePos);
		if (isValidCollisionType(currentType) == false ||
		    currentType == CollisionType_Air)  // Never report "Air" tiles: in the original "apply to layer" setup, these weren't applied either
		{
			continue;
		}
		
		// Solid types have priority over all others
		if (level::isSolid(currentType))
		{
			typeToReturn = currentType;
			break;
		}
	}
	// */
	
	return typeToReturn;
}


bool TileRegistrationMgr::isSolid(const tt::math::Point2&           p_tilePos,
                                  const AttributeLayerPtr&          p_levelLayer,
                                  const game::entity::EntityTiles*  p_entityTilesToIgnore,
                                  const game::entity::EntityHandle& p_collisionAncestorToIgnore,
                                  bool                              p_ignoreActiveTiles,
                                  game::entity::EntityTilesPtr*     p_entityTilesWhichReportedSolid_OUT) const
{
	// FIXME: Calling code that doesn't need the extra parameters should call the simpler versions directly
	if (p_entityTilesToIgnore               == 0     &&
	    p_collisionAncestorToIgnore.isEmpty()        &&
	    p_ignoreActiveTiles                 == false &&
	    p_entityTilesWhichReportedSolid_OUT == 0)
	{
		TT_ASSERT(m_levelLayer == p_levelLayer);
		return isSolid(p_tilePos);
	}
	
	const CollisionTypes& solidCheckMask(g_collisionTypesSolid);
	
	// First check all EntityTiles instances at this position
	
	const game::entity::EntityTilesWeakPtrs* entityTiles = getTilesAtPosition(p_tilePos);
	if (entityTiles != 0)
	{
		for (game::entity::EntityTilesWeakPtrs::const_iterator it = entityTiles->begin();
		     it != entityTiles->end(); ++it)
		{
			game::entity::EntityTilesPtr tiles((*it).second.lock());
			if (tiles == 0)
			{
				TT_PANIC("An EntityTiles pointer at location (%d, %d) is no longer valid.\n"
				         "This probably means that this location wasn't unregistered when it should have been.",
				         p_tilePos.x, p_tilePos.y);
				continue;
			}
			
			// If these tiles should be ignored, or if these tiles are active and active tiles should be ignored:
			// do not continue checking this EntityTiles instance
			if (tiles.get() == p_entityTilesToIgnore ||
			   (p_ignoreActiveTiles && tiles->isActive()))
			{
				continue;
			}
			
			if (p_collisionAncestorToIgnore.isEmpty() == false)
			{
				const game::entity::Entity* entity = tiles->getOwner().getPtr();
				if (entity != 0 && entity->getCachedCollisionAncestor() == p_collisionAncestorToIgnore)
				{
					continue;
				}
			}
			
			const CollisionType currentType = tiles->getCollisionType(p_tilePos);
			if (isValidCollisionType(currentType) && solidCheckMask.checkFlag(currentType))
			{
				// Found an EntityTiles instance which has solid here!
				if (p_entityTilesWhichReportedSolid_OUT != 0)
				{
					*p_entityTilesWhichReportedSolid_OUT = tiles;
				}
				return true;
			}
		}
	}
	
	// Lastly: check if the level reports this location as solid
	if (p_levelLayer->contains(p_tilePos) == false ||  // report anything outside the level as solid
	    solidCheckMask.checkFlag(p_levelLayer->getCollisionType(p_tilePos)))
	{
		if (p_entityTilesWhichReportedSolid_OUT != 0)
		{
			// No EntityTiles instance was responsible for the "solid" report, but the level was
			p_entityTilesWhichReportedSolid_OUT->reset();
		}
		return true;
	}
	
	return false;
}


bool TileRegistrationMgr::isSolid(const tt::math::Point2& p_tilePos) const
{
	const s32 tileIndex = p_tilePos.x + p_tilePos.y * m_levelBounds.x;
	
	if (m_levelLayer->contains(p_tilePos))
	{
		TT_ASSERT(tileIndex >=0 && tileIndex < m_tilesCount);
		return toki::level::isSolid(m_collisionTypes[tileIndex]);
	}
	return true;
}


bool TileRegistrationMgr::isLightBlocking(const tt::math::Point2& p_tilePos) const
{
	const s32 tileIndex = p_tilePos.x + p_tilePos.y * m_levelBounds.x;
	
	if (m_levelLayer->contains(p_tilePos))
	{
		TT_ASSERT(tileIndex >=0 && tileIndex < m_tilesCount);
		return toki::level::isLightBlocking(m_collisionTypes[tileIndex]);
	}
	return true;
}


bool TileRegistrationMgr::isSoundBlocking(const tt::math::Point2& p_tilePos) const
{
	const s32 tileIndex = p_tilePos.x + p_tilePos.y * m_levelBounds.x;
	
	if (m_levelLayer->contains(p_tilePos))
	{
		TT_ASSERT(tileIndex >=0 && tileIndex < m_tilesCount);
		return toki::level::isSoundBlocking(m_collisionTypes[tileIndex]);
	}
	return true;
}


CollisionType TileRegistrationMgr::getCollisionType(const tt::math::Point2& p_tilePos) const
{
	const s32 tileIndex = p_tilePos.x + p_tilePos.y * m_levelBounds.x;
	
	if (m_levelLayer->contains(p_tilePos))
	{
		TT_ASSERT(tileIndex >=0 && tileIndex < m_tilesCount);
		return m_collisionTypes[tileIndex];
	}
	return CollisionType_Solid;
}


void TileRegistrationMgr::onTileChange(const tt::math::Point2& p_position)
{
	// Store the position itself
	if (contains(p_position))
	{
		m_changedTiles.push_back(p_position);
	}
	else
	{
		TT_PANIC("onTileChange position (%d, %d) is out of bounds (%d, %d)", p_position.x, p_position.y, m_levelBounds.x, m_levelBounds.y);
	}
	
	// Store 4 the locations around it
	using namespace tt::math;
	Point2 up    = p_position + Point2( 0,  1);
	Point2 down  = p_position + Point2( 0, -1);
	Point2 left  = p_position + Point2(-1,  0);
	Point2 right = p_position + Point2( 1,  0);
	
	if (contains(up))    m_changedTiles.push_back(up);
	if (contains(down))  m_changedTiles.push_back(down);
	if (contains(left))  m_changedTiles.push_back(left);
	if (contains(right)) m_changedTiles.push_back(right);
	
	updateCollisionType(p_position);
}


void TileRegistrationMgr::handleLevelResized()
{
	// Create arrays of level size
	m_levelBounds.x = m_levelLayer->getWidth();
	m_levelBounds.y = m_levelLayer->getHeight();
	m_tilesCount    = m_levelBounds.x * m_levelBounds.y;
	
	// Use +1 to account for 1 tile overflow (used in Shape::getEntitiesInRange())
	m_cellBounds.x = static_cast<s32>(tt::math::ceil(m_levelLayer->getWidth() / static_cast<real>(cellSize))) + 1;
	m_cellBounds.y = static_cast<s32>(tt::math::ceil(m_levelLayer->getHeight() / static_cast<real>(cellSize))) + 1;
	m_cellCount     = m_cellBounds.x * m_cellBounds.y;
	
	m_changedTiles.clear();
	
#if defined(USE_STD_VECTOR)
	m_registeredEntityTiles  .resize(m_tilesCount);
	m_collisionTypes         .resize(m_tilesCount);
	m_registeredEntityHandles.resize(m_cellCount);
	m_hasEntities            .resize(m_cellCount);
#else
	delete[] m_registeredEntityHandles;
	delete[] m_registeredEntityTiles;
	delete[] m_collisionTypes;
	delete[] m_hasEntities;
	
	m_registeredEntityTiles   = new game::entity::EntityTilesWeakPtrs[m_tilesCount];
	m_collisionTypes          = new CollisionType[m_tilesCount];
	m_registeredEntityHandles = new EntityHandleSet[m_cellCount];
	m_hasEntities             = new bool[m_cellCount];
#endif
	getCollisionTypesFromLevel();
}


void TileRegistrationMgr::setLevelLayer(const AttributeLayerPtr& p_layer)
{
	m_levelLayer = p_layer;
	handleLevelResized();
}


void TileRegistrationMgr::update(real /*p_elapsedTime*/)
{
	// Sort fluid tiles
	{
		std::sort(m_entityTilesForFluids.begin(), m_entityTilesForFluids.end(), tt::math::Point2Less());
		TilePositions::iterator it(std::unique(m_entityTilesForFluids.begin(), m_entityTilesForFluids.end()));
		m_entityTilesForFluids.resize(std::distance(m_entityTilesForFluids.begin(), it));
	}
	
	if (m_changedTiles.empty() == false)
	{
		// FIXME: Get rid of stl sets and replace with a set that uses a fixed pool allocator or something similar
		static EntityHandleSet affectedEntities;
		static TilePositions copy;
		std::swap(copy, m_changedTiles);
		
		// Sort changed tiles
		{
			std::sort(copy.begin(), copy.end(), tt::math::Point2Less());
			TilePositions::iterator it(std::unique(copy.begin(), copy.end()));
			copy.resize(std::distance(copy.begin(), it));
		}
		
		for (TilePositions::const_iterator it = copy.begin(); it != copy.end(); ++it)
		{
			findRegisteredEntityHandles((*it), affectedEntities);
		}
		copy.clear();
		
		using namespace game::entity;
		EntityMgr& mgr = AppGlobal::getGame()->getEntityMgr();
		for (EntityHandleSet::const_iterator it = affectedEntities.begin(); it != affectedEntities.end(); ++it)
		{
			Entity* entity = mgr.getEntity(*it);
			TT_NULL_ASSERT(entity);	// should not happen
			if (entity != 0 && entity->isInitialized())
			{
				entity->onTileChange();
			}
		}
		affectedEntities.clear();
	}
}


void TileRegistrationMgr::render()
{
#if !defined(TT_BUILD_FINAL)
	if (AppGlobal::getDebugRenderMask().checkFlag(DebugRender_EntityRegisteredTiles))
	{
		static const tt::engine::renderer::ColorRGBA colorEntityHandles(tt::engine::renderer::ColorRGB::white);
		static const tt::engine::renderer::ColorRGBA colorEntityTiles  (tt::engine::renderer::ColorRGB::cyan);
		
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		const tt::engine::debug::DebugRendererPtr debug(renderer->getDebug());
		
		tt::math::VectorRect entityRect    (tt::math::Vector2::zero, 0.8f, 0.8f);
		tt::math::VectorRect entityTileRect(tt::math::Vector2::zero, 0.7f, 0.7f);
		
		for (s32 y = 0; y < m_levelBounds.y; ++y)
		{
			entityRect.alignTop    (y + 0.10f);
			entityTileRect.alignTop(y + 0.15f);
			
			for (s32 x = 0; x < m_levelBounds.x; ++x)
			{
				entityRect.alignLeft    (x + 0.10f);
				entityTileRect.alignLeft(x + 0.15f);
				
				if (m_registeredEntityHandles[x + y * m_levelBounds.x].empty() == false)
				{
					debug->renderRect(colorEntityHandles, entityRect);
				}
				if (m_registeredEntityTiles[x + y * m_levelBounds.x].empty() == false)
				{
					debug->renderRect(colorEntityTiles, entityTileRect);
				}
			}
		}
	}
#endif
}


bool TileRegistrationMgr::hasSolidEntityTilesAtPosition(const tt::math::Point2& p_position) const
{
	const game::entity::EntityTilesWeakPtrs* tiles = getTilesAtPosition(p_position);
	if (tiles == 0 || tiles->empty())
	{
		return false;
	}
	for (game::entity::EntityTilesWeakPtrs::const_iterator it = tiles->begin(); it != tiles->end(); ++it)
	{
		game::entity::EntityTilesPtr ptr = it->second.lock();
		if (ptr != 0 && ptr->isSolid())
		{
			return true;
		}
	}
	
	return false;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TileRegistrationMgr::addChangedTiles(const tt::math::PointRect& p_tileRect)
{
	tt::math::Point2 affectedMin(p_tileRect.getMin()       - tt::math::Point2::allOne);
	tt::math::Point2 affectedMax(p_tileRect.getMaxInside() + tt::math::Point2::allOne);
	
	if (affectedMin.x >= m_levelBounds.x || affectedMin.y >= m_levelBounds.y || affectedMax.x < 0 || affectedMax.y < 0)
	{
		return;
	}
	
	affectedMin.x = std::max(0, static_cast<int>(affectedMin.x));
	affectedMin.y = std::max(0, static_cast<int>(affectedMin.y));
	affectedMax.x = std::min(m_levelBounds.x - 1, affectedMax.x);
	affectedMax.y = std::min(m_levelBounds.y - 1, affectedMax.y);
	
	for (tt::math::Point2 pos(affectedMin); pos.y <= affectedMax.y; ++pos.y)
	{
		for (pos.x = affectedMin.x; pos.x <= affectedMax.x; ++pos.x)
		{
			m_changedTiles.push_back(pos);
		}
	}
}


const TileRegistrationMgr::EntityHandleSet* TileRegistrationMgr::getEntitiesAtPosition(
		const tt::math::Point2& p_position) const
{
#if !defined(USE_STD_VECTOR)
	TT_NULL_ASSERT(m_registeredEntityHandles);
#endif
	
	if (m_levelLayer->contains(p_position))
	{
		const s32 cellIndex = getCellIndex(p_position.x, p_position.y);
		TT_ASSERT(cellIndex >= 0 && cellIndex < m_cellCount);
		return &m_registeredEntityHandles[cellIndex];
	}
	return 0;
}


const game::entity::EntityTilesWeakPtrs* TileRegistrationMgr::getTilesAtPosition(
		const tt::math::Point2& p_position) const
{
#if !defined(USE_STD_VECTOR)
	TT_NULL_ASSERT(m_registeredEntityTiles);
#endif
	
	const s32 tileIndex = p_position.x + p_position.y * m_levelBounds.x;
	
	if (m_levelLayer->contains(p_position))
	{
		TT_ASSERT(tileIndex >= 0 && tileIndex < m_tilesCount);
		return &m_registeredEntityTiles[tileIndex];
	}
	return 0;
}


void TileRegistrationMgr::getCollisionTypesFromLevel()
{
	TT_NULL_ASSERT(m_levelLayer);
	
	tt::math::Point2 pos;
	for (pos.y = 0; pos.y < m_levelBounds.y; ++pos.y)
	{
		for (pos.x = 0; pos.x < m_levelBounds.x; ++pos.x)
		{
			m_collisionTypes[pos.x + pos.y * m_levelBounds.x] = m_levelLayer->getCollisionType(pos);
		}
	}
}


bool TileRegistrationMgr::updateCollisionType(const tt::math::Point2& p_position)
{
	const s32 tileIndex = p_position.x + p_position.y * m_levelBounds.x;
	
	TT_ASSERT(tileIndex >= 0 && tileIndex < m_tilesCount);

	const bool wasSolid = toki::level::isSolid(m_collisionTypes[tileIndex]);
	
	m_collisionTypes[tileIndex] = getCollisionTypeFromRegisteredTiles(p_position, m_levelLayer);

	return wasSolid != toki::level::isSolid(m_collisionTypes[tileIndex]);
}

// Namespace end
}
}
