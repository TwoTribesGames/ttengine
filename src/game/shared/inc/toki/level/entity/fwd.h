#if !defined(INC_TOKI_LEVEL_ENTITY_FWD_H)
#define INC_TOKI_LEVEL_ENTITY_FWD_H


#include <set>
#include <vector>

#include <tt/platform/tt_types.h>


namespace toki {
namespace level {
namespace entity {

class EntityProperty;

class EntityInstance;
typedef tt_ptr<EntityInstance>::shared EntityInstancePtr;

typedef std::vector<EntityInstancePtr> EntityInstances;
typedef std::set<   EntityInstancePtr> EntityInstanceSet;
typedef std::set<   s32>               EntityIDSet;

class EntityInstanceObserver;
typedef tt_ptr<EntityInstanceObserver>::shared EntityInstanceObserverPtr;
typedef tt_ptr<EntityInstanceObserver>::weak   EntityInstanceObserverWeakPtr;

class EntityInfo;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_FWD_H)
