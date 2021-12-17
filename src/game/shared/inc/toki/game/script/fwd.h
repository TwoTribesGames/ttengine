#if !defined(INC_TOKI_GAME_SCRIPT_FWD_H)
#define INC_TOKI_GAME_SCRIPT_FWD_H


#include <vector>
#include <tt/math/hash/Hash.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace script {

class Callback;
typedef tt_ptr<Callback>::shared CallbackPtr;

class EntityBase;
typedef tt_ptr<EntityBase>::shared EntityBasePtr;
typedef tt_ptr<EntityBase>::weak EntityBaseWeakPtr;
typedef std::vector<EntityBase*> EntityBaseCollection;

class EntityScriptClass;
typedef tt_ptr<EntityScriptClass>::shared EntityScriptClassPtr;

class EntityScriptMgr;

class EntityState;

class Registry;

class Timer;
typedef tt_ptr<Timer>::shared TimerPtr;

typedef tt::math::hash::Hash<32> TimerHash;


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_FWD_H)
