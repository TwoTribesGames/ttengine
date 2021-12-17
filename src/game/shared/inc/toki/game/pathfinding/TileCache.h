#if !defined(INC_TOKI_GAME_PATHFINDING_TILECACHE_H)
#define INC_TOKI_GAME_PATHFINDING_TILECACHE_H

#include <recastnavigation/Detour/DetourNavMesh.h>
#include <recastnavigation/Detour/DetourNavMeshQuery.h>
#include <recastnavigation/DetourCrowd/DetourCrowd.h>
#include <recastnavigation/DetourTileCache/DetourTileCache.h>
#include <recastnavigation/Recast/Recast.h>

#include <tt/code/fwd.h>
#include <tt/fs/types.h>

#include <toki/level/fwd.h>
#include <toki/serialization/fwd.h>



/* TODO:
- Check what can be shared between TileCaches so we don't do double work or use a lot more memory than needed.
*/

namespace toki {
namespace game {
namespace pathfinding {


class TileCache
{
public:
	explicit TileCache(real p_agentRadius);
	~TileCache();
	
	void cleanup();
	void load(tt::code::BufferReadContext* p_context);
	void save(const level::AttributeLayerPtr& p_layer, tt::code::BufferWriteContext* p_context) const;
	void build(const level::AttributeLayerPtr& p_layer);
	
	inline s32 addAgent(const tt::math::Vector2& p_pos, const tt::math::Vector2& p_targetPos, bool p_separation) { return addAgent(p_pos, &p_targetPos, p_separation); } 
	       s32 addAgent(const tt::math::Vector2& p_pos, const tt::math::Vector2* p_targetPos, bool p_separation);
	void removeAgent(const s32 p_idx);
	void removeAllAgents();
	void setMoveTarget(const tt::math::Vector2& p_pos);
	s32 hitTestAgents(const tt::math::Vector2& p_pos) const;
	
	AgentState getAgentState(s32 p_idx) const;
	tt::math::Vector2 getAgentEndPoint(s32 p_idx) const;
	tt::math::Vector2 getAgentVelocity(s32 p_idx) const;
	void updateAgentPosition( s32 p_idx, const tt::math::Vector2& p_pos);
	void updateTargetPosition(s32 p_idx, const tt::math::Vector2& p_targetPos);
	// Crowd -- end
	
	// Obstacles -- start
	ObstacleIndex addTempObstacle(const tt::math::VectorRect& p_rectangle);
	void          removeTempObstacle(ObstacleIndex p_index);
	void          clearAllTempObstacles();
	ObstacleIndex hitTestObstacle(const tt::math::Vector2& p_pos);
	// Obstacles -- end
	
	bool hasTileCacheUpdatesPending() const;
	void update(real p_deltaTime);
	void render() const;
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	enum
	{
		MaxObstacles = 1024
	};
	
	void initConfig(s32 p_levelWidth, s32 p_levelHeight,
	                s32& p_tileWidth_OUT, s32& p_tileHeight_OUT, rcConfig& p_config_OUT,
	                dtTileCacheParams* p_tileCacheParams_OUT = 0,
	                dtNavMeshParams* p_navMeshParams_OUT = 0) const;
	
	bool initMemory(const rcConfig& p_config, const dtTileCacheParams& p_tileCacheParams,
	                const dtNavMeshParams& p_navMeshParams);
	
	void initCrowd();
	
	s32 preprocess(const level::AttributeLayerPtr& p_layer,
	               s32 p_tileWidth, s32 p_tileHeight, const rcConfig& p_cfg,
	               TileCacheData*& p_tileCache_OUT) const;
	
	void buildMeshes(s32 p_tileWidth, s32 p_tileHeight);
	
	dtNavMesh*               m_navMesh;
	dtNavMeshQuery*          m_navQuery;
	dtCrowd*                 m_crowd;
	
	real                     m_agentRadius;
	
	struct LinearAllocator*  m_talloc;
	struct FastLZCompressor* m_tcomp;
	struct MeshProcess*      m_tmproc;
	
	dtTileCache*             m_tileCache;
	dtObstacleRef            m_obstacles[MaxObstacles];
	
	//  Crowd
	dtPolyRef                m_targetRef;
	float                    m_targetPos[3];
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_PATHFINDING_TILECACHE_H)
