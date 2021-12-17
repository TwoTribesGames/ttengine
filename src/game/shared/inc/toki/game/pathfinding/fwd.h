#if !defined(INC_TOKI_GAME_PATHFINDING_FWD_H)
#define INC_TOKI_GAME_PATHFINDING_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace pathfinding {

class PathMgr;
class TileCache;
class TileRasterizationContext;
struct TileCacheData;

typedef s32 ObstacleIndex;
extern const ObstacleIndex g_invalidObstacleIndex;  // defined in PathMgr.cpp

enum PolyAreas
{
	PolyAreas_Nothing,        //  DT_TILECACHE_NULL_AREA
	PolyAreas_Air_LowCost,    // Air 'type' but agents will like to go here.
	PolyAreas_Air_NormalCost, // Air 'type'.
	PolyAreas_Air_HighCost,   // Air 'type' but agents will try to avoid this area.
	PolyAreas_Warp
};


enum PolyFlags
{
	PolyFlags_Fly      = 0x01,  // Ability to fly
	PolyFlags_Warp     = 0x02,  // Ability to warp.
	PolyFlags_Disabled = 0x04,  // Disabled polygon.
	PolyFlags_All      = 0xffff // All abilities.
};


enum AgentState
{
	AgentState_None,
	AgentState_Busy,
	AgentState_PathSucceded,
	AgentState_PathFailed,

	AgentState_Count,
	AgentState_Invalid
};


inline bool isValidAgentState(AgentState p_state) { return p_state >= 0 && p_state < AgentState_Count; }

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_PATHFINDING_FWD_H)
