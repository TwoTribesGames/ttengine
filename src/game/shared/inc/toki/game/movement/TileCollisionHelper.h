#if !defined(INC_TOKI_GAME_MOVEMENT_TILECOLLISIONHELPER_H)
#define INC_TOKI_GAME_MOVEMENT_TILECOLLISIONHELPER_H


#include <tt/code/BitMask.h>
#include <tt/math/Rect.h>

#include <toki/game/entity/fwd.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace movement {

class TileCollisionHelper
{
public:
	enum CollisionResult
	{
		CollisionResult_TopLeft,
		CollisionResult_Top,
		CollisionResult_TopRight,
		CollisionResult_Left,
		CollisionResult_Inside,
		CollisionResult_Right, 
		CollisionResult_BottomLeft,
		CollisionResult_Bottom,
		CollisionResult_BottomRight,
		
		CollisionResult_Count
	};
	typedef tt::code::BitMask<CollisionResult, CollisionResult_Count> CollisionResultMask;
	
	static const CollisionResultMask cornersMask;
	static const CollisionResultMask verticalMask;
	static const CollisionResultMask horizontalMask;
	
	/*! \param p_ignoreParentOffsetInTileChecks  Whether to take the parent position relative to our own into account when checking tiles.
	    \param p_ignoreSameAncestorCollision     Whether to ignore collision from other entities with the same ancestor as p_entity. */
	static CollisionResultMask hasTileCollision(const tt::math::PointRect& p_previousTiles,
	                                            const tt::math::PointRect& p_newTiles,
	                                            const entity::Entity&      p_entity,
	                                            bool                       p_ignoreParentOffsetInTileChecks = false,
	                                            bool                       p_ignoreSameAncestorCollision    = false);
	static bool hasTileCollision(const tt::math::PointRect& p_tiles,
	                             const entity::Entity&      p_entity,
	                             bool                       p_ignoreParentOffsetInTileChecks = false,
	                             bool                       p_ignoreSameAncestorCollision        = false);
	static bool hasTileCollision(const tt::math::Point2& p_minPos,
	                             const tt::math::Point2& p_maxPos,
	                             const entity::Entity&   p_entity,
	                             bool                    p_ignoreParentOffsetInTileChecks = false,
	                             bool                    p_ignoreSameAncestorCollision    = false);
	
	/*! \param p_resultAllTiles If specified, receives the collision type that all tiles in the specified rect are (Invalid if all are not the same). */
	static level::CollisionTypes getCollisionTypes(const entity::Entity&           p_entity,
	                                               const level::AttributeLayerPtr& p_layer,
	                                               const tt::math::Point2&         p_minPos,
	                                               const tt::math::Point2&         p_maxPos,
	                                               level::CollisionType*           p_resultAllTiles                 = 0,
	                                               bool                            p_ignoreParentOffsetInTileChecks = false,
	                                               bool                            p_ignoreSameAncestorCollision        = false);
	
private:
	TileCollisionHelper(); // Not implemented, no construction, static class.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_TILECOLLISIONHELPER_H)
