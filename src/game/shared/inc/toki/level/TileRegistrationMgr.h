#if !defined(INC_TOKI_LEVEL_TILEREGISTRATIONMGR_H)
#define INC_TOKI_LEVEL_TILEREGISTRATIONMGR_H


#include <set>
#include <map>
#include <vector>

#include <tt/math/Rect.h>
#include <tt/platform/tt_types.h>

#include <toki/game/entity/fwd.h>
#include <toki/level/TileChangedObserver.h>
#include <toki/level/types.h>


#define USE_STD_VECTOR

namespace toki {
namespace level {

class TileRegistrationMgr : public TileChangedObserver
{
public:
	static const s32 cellSize = 4;
	
	TileRegistrationMgr();
	virtual ~TileRegistrationMgr();
	
	void reset();
	
	// EntityHandle on tile positions:
	
	inline bool contains(const tt::math::Point2& p_tilePos) const
	{
		return p_tilePos.x >= 0               && p_tilePos.y >= 0               &&
		       p_tilePos.x <  m_levelBounds.x && p_tilePos.y <  m_levelBounds.y;
	}
	
	void registerEntityHandle(const tt::math::PointRect&        p_tiles,
	                          const game::entity::EntityHandle& p_entityHandle);
	
	void unregisterEntityHandle(const tt::math::PointRect&        p_tiles,
	                            const game::entity::EntityHandle& p_entityHandle);
	
	void moveRegisterEntityHandle(const tt::math::PointRect&        p_prevTiles,
	                              const tt::math::PointRect&        p_newTiles,
	                              const game::entity::EntityHandle& p_entityHandle);
	
	// NOTE: Caller is responsible for range checking the arguments
	inline bool hasEntityAtPosition(u32 p_x, u32 p_y) const { return m_hasEntities[getCellIndex(p_x, p_y)]; }

	bool isEntityAtPosition(const tt::math::Point2& p_position, const game::entity::EntityHandle& p_entityHandle) const;
	void findRegisteredEntityHandles(const tt::math::Point2&    p_position, game::entity::EntityHandleSet& p_result) const;
	void findRegisteredEntityHandles(const tt::math::PointRect& p_tiles,    game::entity::EntityHandleSet& p_result) const;
	
	const game::entity::EntityHandleSet& getRegisteredEntityHandles(const tt::math::Point2& p_position) const;
	
	// EntityTiles on tile positions:
	
	void registerEntityTiles(const tt::math::PointRect&          p_tiles,
	                         const game::entity::EntityTilesPtr& p_entityTiles,
	                         bool                                p_doCallbacks = true);
	
	void unregisterEntityTiles(const tt::math::PointRect&          p_tiles,
	                           const game::entity::EntityTilesPtr& p_entityTiles);
	
	/*! \brief Returns all CollisionType values at the specified tile position. */
	CollisionTypes getCollisionTypesFromRegisteredTiles(
			const tt::math::Point2&           p_tilePos,
			const AttributeLayerPtr&          p_levelLayer,
			const game::entity::EntityTiles*  p_entityTilesToIgnore       = 0,
			const game::entity::EntityHandle& p_collisionAncestorToIgnore = game::entity::EntityHandle(),
			bool                              p_ignoreActiveTiles         = false,
			bool                              p_checkRectForCollisionEntities = false) const;
	
	CollisionType getCollisionTypeFromRegisteredTiles(
			const tt::math::Point2&           p_tilePos,
			const AttributeLayerPtr&          p_levelLayer,
			const game::entity::EntityTiles*  p_entityTilesToIgnore       = 0,
			const game::entity::EntityHandle& p_collisionAncestorToIgnore = game::entity::EntityHandle(),
			bool                              p_ignoreActiveTiles         = false) const;
	
	/*! \brief Reports whether the specified tile location is solid (either because of the level layer
	           or any EntityTiles present at that location).
	    \param p_tilePos                           The position to check.
	    \param p_levelLayer                        The level's attribute layer which can be checked for solidity.
	    \param p_entityTilesToIgnore               Optional: ignore a specific EntityTiles instance in the checks.
	    \param p_ignoreActiveTiles                 Optional: whether to ignore EntityTiles marked as "active".
	    \param p_entityTilesWhichReportedSolid_OUT Optional: if passed, this will be set to the EntityTiles instance which caused this function to report solid. */
	bool isSolid(const tt::math::Point2&           p_tilePos,
	             const AttributeLayerPtr&          p_levelLayer,
	             const game::entity::EntityTiles*  p_entityTilesToIgnore               = 0,
	             const game::entity::EntityHandle& p_collisionAncestorToIgnore         = game::entity::EntityHandle(),
	             bool                              p_ignoreActiveTiles                 = false,
	             game::entity::EntityTilesPtr*     p_entityTilesWhichReportedSolid_OUT = 0) const;
	
	// Faster functions that use a cached collision type lookup table
	// Use these when the extra parameters are not needed
	bool isSolid            (const tt::math::Point2& p_tilePos) const;
	bool isLightBlocking    (const tt::math::Point2& p_tilePos) const;
	bool isSoundBlocking    (const tt::math::Point2& p_tilePos) const;
	
	CollisionType getCollisionType(const tt::math::Point2& p_tilePos) const;
	
	
	virtual void onTileChange(const tt::math::Point2& p_position);
	
	
	
	void handleLevelResized();
	void setLevelLayer(const AttributeLayerPtr& p_layer);
	
	// For fluids
	typedef std::vector<tt::math::Point2> TilePositions;
	const TilePositions& getEntityTilesForFluids() const { return m_entityTilesForFluids; }
	void clearEntityTilesForFluids() { m_entityTilesForFluids.clear(); }
	
	void update(real p_elapsedTime);
	void render();
	
	inline bool hasTilesAtPosition(const tt::math::Point2& p_position) const
	{
		const game::entity::EntityTilesWeakPtrs* tiles = getTilesAtPosition(p_position);
		return tiles != 0 && tiles->empty() == false;
	}
	
	bool hasSolidEntityTilesAtPosition(const tt::math::Point2& p_position) const;
	
private:
	typedef game::entity::EntityHandleSet EntityHandleSet;
	inline s32 getCellIndex(s32 p_x, s32 p_y) const
	{
		const s32 cx = (p_x / cellSize);
		const s32 cy = (p_y / cellSize);
		
		TT_ASSERT(cx >= 0 && cx < m_cellBounds.x);
		TT_ASSERT(cy >= 0 && cy < m_cellBounds.y);
		
		return cx + cy * m_cellBounds.x;
	}
	
	void                                     addChangedTiles(const tt::math::PointRect& p_tileRect);
	const EntityHandleSet*                   getEntitiesAtPosition(const tt::math::Point2& p_position) const;
	const game::entity::EntityTilesWeakPtrs* getTilesAtPosition   (const tt::math::Point2& p_position) const;
	void                                     getCollisionTypesFromLevel();
	bool                                     updateCollisionType(const tt::math::Point2& p_position);
	
	TileRegistrationMgr(const TileRegistrationMgr&);                  // Disabled
	const TileRegistrationMgr& operator=(const TileRegistrationMgr&); // Disabled
	
#if defined(USE_STD_VECTOR)
	std::vector<EntityHandleSet>                   m_registeredEntityHandles;
	std::vector<game::entity::EntityTilesWeakPtrs> m_registeredEntityTiles;
	std::vector<CollisionType>                     m_collisionTypes;
	std::vector<bool>                              m_hasEntities;
#else
	EntityHandleSet*                   m_registeredEntityHandles;
	game::entity::EntityTilesWeakPtrs* m_registeredEntityTiles;
	CollisionType*                     m_collisionTypes;
	bool*                              m_hasEntities;
#endif
	
	TilePositions                      m_changedTiles;
	TilePositions                      m_entityTilesForFluids;
	
	AttributeLayerPtr m_levelLayer;
	tt::math::Point2  m_levelBounds;
	s32               m_tilesCount;
	tt::math::Point2  m_cellBounds;
	s32               m_cellCount;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_TILEREGISTRATIONMGR_H)
