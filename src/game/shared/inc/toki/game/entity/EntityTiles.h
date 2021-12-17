#if !defined(INC_TOKI_GAME_ENTITY_ENTITYTILES_H)
#define INC_TOKI_GAME_ENTITY_ENTITYTILES_H


#include <map>

#include <tt/code/fwd.h>
#include <tt/math/Point2.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_types.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/pathfinding/fwd.h>
#include <toki/game/fwd.h>
#include <toki/level/AttributeLayerSection.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace entity {

class EntityTiles
{
public:
	static EntityTilesPtr create(const std::string&      p_tiles,
	                             const tt::math::Point2& p_pos,
	                             bool                    p_applyTilesAsActive,
	                             const EntityHandle&     p_ownerHandle);
	static EntityTilesPtr create(const tt::math::PointRect& p_tileRect,
	                             level::CollisionType       p_type,
	                             bool                       p_applyTilesAsActive,
	                             const EntityHandle&        p_ownerHandle);
	~EntityTiles();
	
	inline const tt::math::Point2& getPos() const { TT_NULL_ASSERT(m_entityTiles); return m_entityTiles->getPosition(); }
	inline s32 getWidth() const                   { TT_NULL_ASSERT(m_entityTiles); return m_entityTiles->getAttributeLayer()->getWidth(); }
	inline s32 getHeight() const                  { TT_NULL_ASSERT(m_entityTiles); return m_entityTiles->getAttributeLayer()->getHeight(); }
	inline tt::math::PointRect getRect() const    { TT_NULL_ASSERT(m_entityTiles); return m_entityTiles->getRect(); }
	inline bool containsLocal(const tt::math::Point2& p_localPos) const
	{
		TT_NULL_ASSERT(m_entityTiles);
		return m_entityTiles->getAttributeLayer()->contains(p_localPos);
	}
	inline bool contains(const tt::math::Point2& p_worldPos) const
	{
		TT_NULL_ASSERT(m_entityTiles);
		return m_entityTiles->contains(p_worldPos);
	}
	inline level::CollisionType getCollisionTypeLocal(const tt::math::Point2& p_localPos) const
	{
		TT_NULL_ASSERT(m_entityTiles);
		return m_entityTiles->getAttributeLayer()->getCollisionType(p_localPos);
	}
	
	level::CollisionType getCollisionType( const tt::math::Point2& p_worldPos) const;
	void setActive(bool p_active);
	inline bool isActive() const         { return m_applyTilesAsActive;     }
	
	void onNewTilePosition(const tt::math::Point2& p_newTilePos);
	
	inline const EntityHandle& getOwner() const { return m_ownerHandle; }
	
	inline bool isSolid()               const { return level::g_collisionTypesSolid              .checkFlags(m_containedCollisionTypes); }
	inline bool isSolidForPathFinding() const { return level::g_collisionTypesSolidForPathfinding.checkFlags(m_containedCollisionTypes); }
	inline bool isLightBlocking()       const { return level::g_collisionTypesLightBlocking.      checkFlags(m_containedCollisionTypes); }
	
	static void debugRenderAllInstances();
	
	static void handlePathMgrReset();
	
	inline static const EntityTilesWeakPtrs& getActiveInstances() { return ms_activeInstances; }
	
	static void serialize(const EntityTilesPtr& p_value, tt::code::BufferWriteContext* p_context);
	static EntityTilesPtr unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	static EntityTilesPtr create(const level::AttributeLayerSectionPtr& p_entityTiles,
	                             bool                                   p_applyTilesAsActive,
	                             const EntityHandle&                    p_ownerHandle);
	EntityTiles(const level::AttributeLayerSectionPtr& p_entityTiles,
	            bool                                   p_applyTilesAsActive,
	            const EntityHandle&                    p_ownerHandle);
	
	void updateTileGraphicsVertexColor();
	void createPathFindingObstacle();
	
	static void removeInstance(EntityTiles* p_instance);
	
	
	level::AttributeLayerSectionPtr m_entityTiles;
	bool                            m_applyTilesAsActive;
	
	EntityHandle    m_ownerHandle;
	
	level::CollisionTypes      m_containedCollisionTypes;
	pathfinding::ObstacleIndex m_pathFindingObstacle;
	
#if !defined(TT_BUILD_FINAL)
	AttributeDebugViewPtr m_tileGraphics;
#endif
	
	static EntityTilesWeakPtrs ms_activeInstances;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_ENTITYTILES_H)
