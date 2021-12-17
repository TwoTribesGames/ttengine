#if !defined(INC_TOKI_GAME_ENTITY_FWD_H)
#define INC_TOKI_GAME_ENTITY_FWD_H


#include <map>
#include <set>
#include <vector>

#include <tt/code/Handle.h>

#include <toki/game/entity/types.h>
#include <toki/game/movement/fwd.h>


namespace toki {
namespace game {
namespace entity {

class Entity;
typedef tt::code::Handle<Entity> EntityHandle;
typedef std::vector<EntityHandle> EntityHandles;
typedef std::set<EntityHandle> EntityHandleSet;

class EntityMgr;
typedef tt_ptr<EntityMgr>::shared EntityMgrPtr;

class EntityTiles;
typedef tt_ptr<EntityTiles>::shared EntityTilesPtr;
typedef tt_ptr<EntityTiles>::weak   EntityTilesWeakPtr;

// Map raw pointer to weak pointer, so that lookups of pointers are faster
// (checking weak pointers requires each to be locked in order to be compared)
// Do not actually use the raw pointer! It can become invalid. It's meant to be used for easy comparison.
typedef std::map<EntityTiles*, EntityTilesWeakPtr> EntityTilesWeakPtrs;

typedef std::map<std::string, std::string> EntityProperties;

namespace movementcontroller
{
	class DirectionalMovementController;
	typedef tt::code::Handle<DirectionalMovementController> MovementControllerHandle;
	class MovementControllerMgr;
}

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_FWD_H)
