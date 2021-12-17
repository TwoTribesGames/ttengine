#if !defined(INC_TOKI_GAME_PATHFINDING_PATHMGR_H)
#define INC_TOKI_GAME_PATHFINDING_PATHMGR_H


#include <map>

#include <tt/code/fwd.h>
#include <tt/math/Rect.h>

#include <toki/game/pathfinding/fwd.h>
#include <toki/level/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace pathfinding {

class PathMgr
{
public:
	typedef std::set<real> AgentRadii;
	
	PathMgr();
	~PathMgr();
	
	void cleanup();
	void buildTileCaches(const level::AttributeLayerPtr& p_layer);
	
	void reset();
	void recreateTileCaches(const AgentRadii& p_agentRadii);
	void recreateTileCaches(const level::LevelDataPtr& p_levelData);
	void addAndBuildMissingTileCaches(const level::LevelDataPtr& p_levelData);
	void loadTileCachesFromLevelData(const level::LevelDataPtr& p_levelData);
	void saveTileCachesToLevelData  (const level::LevelDataPtr& p_levelData);
	void removeAllAgents();
	
	AgentRadii getUniqueAgentRadiiForLevel(const level::LevelDataPtr& p_levelData) const;
	TileCache* getTileCacheForAgentRadius(real p_agentRadius);
	
	// Obstacles -- start
	ObstacleIndex addTempObstacle(const tt::math::VectorRect& p_rectangle);
	void          removeTempObstacle(ObstacleIndex p_index);
	void          clearAllTempObstacles();
	// Obstacles -- end
	
	void update(real p_deltaTime);
	void render() const;
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	void loadTileCaches(tt::code::BufferReadContext* p_context);
	void saveTileCaches(const level::AttributeLayerPtr& p_layer,
	                    tt::code::BufferWriteContext* p_context) const;
	
	// Maps agent radius to a tile cache created for that radius
	typedef std::map<real, TileCache*> TileCaches;
	TileCaches m_tileCaches;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_PATHFINDING_PATHMGR_H)
