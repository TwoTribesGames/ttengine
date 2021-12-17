#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/game/Game.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace movement {


const TileCollisionHelper::CollisionResultMask TileCollisionHelper::cornersMask(
		CollisionResultMask(CollisionResult_TopLeft)    |
		CollisionResultMask(CollisionResult_TopRight)   |
		CollisionResultMask(CollisionResult_BottomLeft) |
		CollisionResultMask(CollisionResult_BottomRight));

const TileCollisionHelper::CollisionResultMask TileCollisionHelper::verticalMask(
		CollisionResultMask(CollisionResult_Top)    |
		CollisionResultMask(CollisionResult_Bottom) |
		cornersMask);

const TileCollisionHelper::CollisionResultMask TileCollisionHelper::horizontalMask(
		CollisionResultMask(CollisionResult_Left)  |
		CollisionResultMask(CollisionResult_Right) |
		cornersMask);


//--------------------------------------------------------------------------------------------------
// Public member functions

TileCollisionHelper::CollisionResultMask TileCollisionHelper::hasTileCollision(
		const tt::math::PointRect& p_previousTiles,
		const tt::math::PointRect& p_newTiles,
		const entity::Entity&      p_entity,
		bool                       p_ignoreParentOffsetInTileChecks,
		bool                       p_ignoreSameAncestorCollision)
{
	CollisionResultMask result;
	
	// This can be refactored into a function which gets the surrounding colllision for a rect (now p_previousTiles)
	// for a specific area (now p_newTiles).
	
	/********************************
	 *  Diagram explaining how to get the different rects.
	 *  
	 *  +-----+----+----+----+----+----+
	 *  |          |         |     new |
	 *  |     |    |    |    |    |max | <-- New max Y
	 *  |          |         |         |
	 *  + - - + - -+ - -+ - -+ - -+ - -+
	 *  |          |     prev|prev     |
	 *  |     |    |    |maxY|max |    | <-- Prev max Y + 1
	 *  |          |      +1 | +1      | Above
	 *  +-----+----+----+----+----+----+ -------
	 *  |          |     prev|prev     | Inside
	 *  |     |    |    |max |maxX|    | <-- Prev Max Y
	 *  |          |         | +1      |
	 *  + - - + - -+ - -+ - -+ - -+ - -+
	 *  |      prev|prev     |         |
	 *  |     |minX|min |    |    |    | <-- Prev Min Y
	 *  |       -1 |         |         | Inside
	 *  +-----+----+----+----+----+----+ -------
	 *  |      prev|prev     |         | Below
	 *  |     |min |minY|    |    |    | <-- Prev Min Y - 1
	 *  |       -1 | -1      |         |
	 *  + - - + - -+ - -+ - -+ - -+ - -+
	 *  | new      |         |         |
	 *  | min |    |    |    |    |    | <-- New min Y
	 *  |          |         |         |
	 *  +-----+----+----+----+----+----+
	 *                                  
	 *    /\    /\ | /\   /\ | /\   /\. 
	 *   new   prev|prev prev|prev new  
	 *   minX  minX|minX maxX|maxX maxX 
	 *          -1 |         | +1       
	 *             |         |          
	 *        left | inside  | right    
	 *
	 **********************************/
	
	const tt::math::Point2 prevMin = p_previousTiles.getMin();
	const tt::math::Point2 prevMax = p_previousTiles.getMaxInside();
	const tt::math::Point2 newMin  = p_newTiles.getMin();
	const tt::math::Point2 newMax  = p_newTiles.getMaxInside();
	
	const s32 leftX  = prevMin.x - 1;
	const s32 rightX = prevMax.x + 1;
	const s32 belowY = prevMin.y - 1;
	const s32 aboveY = prevMax.y + 1;
	
	const bool checkLeft  = newMin.x <= leftX;
	const bool checkRight = newMax.x >= rightX;
	const bool checkBelow = newMin.y <= belowY;
	const bool checkAbove = newMax.y >= aboveY;
	
	if (checkBelow)
	{
		if (checkLeft && // Bottom Left
		    hasTileCollision(newMin,
		                     tt::math::Point2(leftX, belowY),
		                     p_entity,
		                     p_ignoreParentOffsetInTileChecks,
		                     p_ignoreSameAncestorCollision))
		{
			result.setFlag(CollisionResult_BottomLeft);
		}
		if (checkRight && // Bottom Right
		    hasTileCollision(tt::math::Point2(rightX,   newMin.y),
		                     tt::math::Point2(newMax.x, belowY),
		                     p_entity,
		                     p_ignoreParentOffsetInTileChecks,
		                     p_ignoreSameAncestorCollision))
		{
			result.setFlag(CollisionResult_BottomRight);
		}
		// Bottom (inside)
		if (hasTileCollision(tt::math::Point2(prevMin.x, newMin.y),
		                     tt::math::Point2(prevMax.x, belowY),
		                     p_entity,
		                     p_ignoreParentOffsetInTileChecks,
		                     p_ignoreSameAncestorCollision))
		{
			result.setFlag(CollisionResult_Bottom);
		}
	}
	if (checkAbove)
	{
		if (checkLeft && // Top Left
		    hasTileCollision(tt::math::Point2(newMin.x, aboveY),
		                     tt::math::Point2(leftX,    newMax.y),
		                     p_entity,
		                     p_ignoreParentOffsetInTileChecks,
		                     p_ignoreSameAncestorCollision))
		{
			result.setFlag(CollisionResult_TopLeft);
		}
		if (checkRight && // Top Right
		    hasTileCollision(tt::math::Point2(rightX,   aboveY),
		                     newMax,
		                     p_entity,
		                     p_ignoreParentOffsetInTileChecks,
		                     p_ignoreSameAncestorCollision))
		{
			result.setFlag(CollisionResult_TopRight);
		}
		// Top (inside)
		if (hasTileCollision(tt::math::Point2(prevMin.x, aboveY),
		                     tt::math::Point2(prevMax.x, newMax.y),
		                     p_entity,
		                     p_ignoreParentOffsetInTileChecks,
		                     p_ignoreSameAncestorCollision))
		{
			result.setFlag(CollisionResult_Top);
		}
	}
	if (checkLeft &&
	    hasTileCollision(tt::math::Point2(newMin.x, prevMin.y),
	                     tt::math::Point2(leftX,    prevMax.y),
	                     p_entity,
	                     p_ignoreParentOffsetInTileChecks,
	                     p_ignoreSameAncestorCollision))
	{
		result.setFlag(CollisionResult_Left);
	}
	if (checkRight &&
	    hasTileCollision(tt::math::Point2(rightX,   prevMin.y),
	                     tt::math::Point2(newMax.x, prevMax.y),
	                     p_entity,
	                     p_ignoreParentOffsetInTileChecks,
	                     p_ignoreSameAncestorCollision))
	{
		result.setFlag(CollisionResult_Right);
	}
	
	level::CollisionTypes collisionTypes = getCollisionTypes(
			p_entity, AppGlobal::getGame()->getAttributeLayer(), prevMin, prevMax, 0,
			p_ignoreParentOffsetInTileChecks, p_ignoreSameAncestorCollision);
	
	if (collisionTypes.checkAnyFlags(level::g_collisionTypesSolid))
	{
		result.setFlag(CollisionResult_Inside);
	}
	
	return result;
}


bool TileCollisionHelper::hasTileCollision(const tt::math::PointRect& p_tiles,
                                           const entity::Entity&      p_entity,
                                           bool                       p_ignoreParentOffsetInTileChecks,
                                           bool                       p_ignoreSameAncestorCollision)
{
	return hasTileCollision(p_tiles.getMin(), p_tiles.getMaxInside(), p_entity,
	                        p_ignoreParentOffsetInTileChecks, p_ignoreSameAncestorCollision);
}


bool TileCollisionHelper::hasTileCollision(const tt::math::Point2& p_minPos,
                                           const tt::math::Point2& p_maxPos,
                                           const entity::Entity&   p_entity,
                                           bool                    p_ignoreParentOffsetInTileChecks,
                                           bool                    p_ignoreSameAncestorCollision)
{
	const level::AttributeLayerPtr& layer = AppGlobal::getGame()->getAttributeLayer();
	level::TileRegistrationMgr&     tileMgr  (AppGlobal::getGame()->getTileRegistrationMgr());
	entity::EntityMgr&              entityMgr(AppGlobal::getGame()->getEntityMgr());
	
	const entity::EntityTiles*  entityTiles = p_entity.getCollisionTiles();
	
	const entity::EntityHandle collisionParentHandle(p_ignoreParentOffsetInTileChecks ?
			entity::EntityHandle() : p_entity.getCollisionParentEntity());
	const entity::Entity*      collisionParent = entityMgr.getEntity(collisionParentHandle);
	const entity::EntityTiles* parentTiles     =  (collisionParent != 0) ? collisionParent->getCollisionTiles()          : 0;
	const tt::math::Vector2    parentWorldTilePos((collisionParent != 0) ? collisionParent->calcWorldTileRect().getMin() : tt::math::Vector2::zero);
	
	const entity::EntityHandle ancestorIgnoreHandle(p_ignoreSameAncestorCollision ?
			p_entity.getCachedCollisionAncestor() : entity::EntityHandle());
	
	for (tt::math::Point2 tilePos = p_minPos; tilePos.y <= p_maxPos.y; ++tilePos.y)
	{
		for (tilePos.x = p_minPos.x; tilePos.x <= p_maxPos.x; ++tilePos.x)
		{
			// If this tile position has our collision parent as a registered tile,
			// and this position is within the parent's tiles, check with the parent instead of the layer
			if (collisionParent != 0 &&
			    parentTiles     != 0 &&
			    tileMgr.isEntityAtPosition(tilePos, collisionParentHandle))
			{
				const tt::math::Point2 parentLocalCheckPos(level::worldToTile(
					level::tileToWorld(tilePos) + tt::math::Vector2(0.5f, 0.5f) - parentWorldTilePos));
				
				if (parentTiles->containsLocal(parentLocalCheckPos))
				{
					const level::CollisionType colType = parentTiles->getCollisionTypeLocal(parentLocalCheckPos);
					if (level::isSolid(colType))
					{
						return true;
					}
					
					// Whether this tile is solid or not, do not perform the other checking below,
					// because we should only look at the parent tiles in this case
					continue;
				}
			}
			
			{
				if (tileMgr.isSolid(tilePos, layer, entityTiles, ancestorIgnoreHandle, false))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}


level::CollisionTypes TileCollisionHelper::getCollisionTypes(
		const entity::Entity&           p_entity,
		const level::AttributeLayerPtr& p_layer,
		const tt::math::Point2&         p_minPos,
		const tt::math::Point2&         p_maxPos,
		level::CollisionType*           p_resultAllTiles,
		bool                            p_ignoreParentOffsetInTileChecks,
		bool                            p_ignoreSameAncestorCollision)
{
	TT_NULL_ASSERT(p_layer);
	TT_ASSERT(p_minPos.x <= p_maxPos.x);
	TT_ASSERT(p_minPos.y <= p_maxPos.y);
	
	const s32 width = p_layer->getWidth();
	const tt::math::Point2 levelMax(width - 1, p_layer->getHeight() - 1);
	
	// Check if whole rect is outside of level. (Outside of level is collision.)
	if (p_minPos.x > levelMax.x || p_minPos.y > levelMax.y ||
	    p_maxPos.x < 0          || p_maxPos.y < 0)
	{
		if (p_resultAllTiles != 0)
		{
			*p_resultAllTiles = level::CollisionType_Solid;
		}
		return level::CollisionTypes(level::CollisionType_Solid);
	}
	
	bool                  resultAllTilesSet = false;
	level::CollisionType  resultAllTiles    = level::CollisionType_Invalid;
	level::CollisionTypes result;
	
	// Clamp rect to level size. (Outside of level is collision.)
	tt::math::Point2 minPos(p_minPos);
	tt::math::Point2 maxPos(p_maxPos);
	
	bool clamped = tt::math::clamp(minPos.x, s32(0), levelMax.x);
	clamped      = tt::math::clamp(minPos.y, s32(0), levelMax.y) || clamped;
	clamped      = tt::math::clamp(maxPos.x, s32(0), levelMax.x) || clamped;
	clamped      = tt::math::clamp(maxPos.y, s32(0), levelMax.y) || clamped;
	if (clamped)
	{
		resultAllTilesSet = true;
		resultAllTiles    = level::CollisionType_Solid;
		result.setFlag(level::CollisionType_Solid);
	}
	
	// Step through each tile and check collision type.
	level::TileRegistrationMgr& tileMgr  (AppGlobal::getGame()->getTileRegistrationMgr());
	entity::EntityMgr&          entityMgr(AppGlobal::getGame()->getEntityMgr());
	
	const entity::EntityTiles*  entityTiles = p_entity.getCollisionTiles();
	
	const entity::EntityHandle  collisionParentHandle(p_ignoreParentOffsetInTileChecks ?
			entity::EntityHandle() : p_entity.getCollisionParentEntity());
	const entity::Entity*       collisionParent = entityMgr.getEntity(collisionParentHandle);
	const entity::EntityTiles*  parentTiles = (collisionParent != 0) ? collisionParent->getCollisionTiles() : 0;
	const tt::math::Vector2     parentWorldTilePos((collisionParent != 0) ? collisionParent->calcWorldTileRect().getMin() : tt::math::Vector2::zero);
	
	const entity::EntityHandle ancestorIgnoreHandle(p_ignoreSameAncestorCollision ?
			p_entity.getCachedCollisionAncestor() : entity::EntityHandle());
	
	const u8* attributes = p_layer->getRawData();
	const u8* columnPtr  = &attributes[(minPos.y * width) + minPos.x];
	for (tt::math::Point2 tilePos = minPos; tilePos.y <= maxPos.y; ++tilePos.y, columnPtr += width)
	{
		const u8* rowPtr = columnPtr;
		for (tilePos.x = minPos.x; tilePos.x <= maxPos.x; ++tilePos.x, ++rowPtr)
		{
			level::CollisionType colType = level::CollisionType_Invalid;
			
			// If this tile position has our collision parent as a registered tile,
			// and this position is within the parent's tiles, check with the parent instead of the layer
			if (collisionParent != 0 &&
			    parentTiles     != 0 &&
			    tileMgr.isEntityAtPosition(tilePos, collisionParentHandle))
			{
				const tt::math::Point2 parentLocalCheckPos(level::worldToTile(
					level::tileToWorld(tilePos) + tt::math::Vector2(0.5f, 0.5f) - parentWorldTilePos));
				
				if (parentTiles->containsLocal(parentLocalCheckPos))
				{
					colType = parentTiles->getCollisionTypeLocal(parentLocalCheckPos);
				}
				else
				{
					colType = level::getCollisionType(*rowPtr);
					// FIXME: The line above should actually be this: colType = tileMgr.getCollisionTypeFromRegisteredTiles(tilePos, p_layer, entityTiles, ancestorIgnoreHandle);
				}
			}
			else
			{
				colType = tileMgr.getCollisionTypeFromRegisteredTiles(
						tilePos, p_layer, entityTiles, ancestorIgnoreHandle);
				
#if defined(TT_BUILD_DEV)
				TT_ASSERTMSG(  level::getCollisionType(*rowPtr) == p_layer->getCollisionType(tilePos),
				             "getCollisionTypes get's different result from getCollisionType!."
				             "type from rowPtr(%p): %u != %u for tilePos x: %d, y: %d",
				             rowPtr, level::getCollisionType(*rowPtr), p_layer->getCollisionType(tilePos),
				             tilePos.x, tilePos.y);
				TT_ASSERTMSG(rowPtr == &attributes[(tilePos.y * width) + tilePos.x],
				             "rowPtr(%p) isn't where it should be %p for tile x: %d, y: %d",
				             rowPtr, &attributes[(tilePos.y * width) + tilePos.x], tilePos.x, tilePos.y);
#endif
			}
			
			if (level::isValidCollisionType(colType)) result.setFlag(colType);
			
			if (resultAllTilesSet == false) // Is this the first type found?
			{
				resultAllTilesSet = true;
				resultAllTiles    = colType;
			}
			else if (resultAllTiles != colType) // Is this type different from the previous types?
			{
				// Not all tiles are the same type
				resultAllTiles = level::CollisionType_Invalid;
			}
		}
	}
	
	if (p_resultAllTiles != 0)
	{
		*p_resultAllTiles = resultAllTiles;
	}
	return result;
}

// Namespace end
}
}
}
