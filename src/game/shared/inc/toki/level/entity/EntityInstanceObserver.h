#if !defined(INC_TOKI_LEVEL_ENTITY_ENTITYINSTANCEOBSERVER_H)
#define INC_TOKI_LEVEL_ENTITY_ENTITYINSTANCEOBSERVER_H


#include <toki/level/entity/fwd.h>


namespace toki {
namespace level {
namespace entity {

/*! \brief Receives notifications about EntityInstance changes. */
class EntityInstanceObserver
{
public:
	EntityInstanceObserver() { }
	virtual ~EntityInstanceObserver() { }
	
	virtual void onEntityInstancePositionChanged  (const EntityInstancePtr& /*p_instance*/) { }
	virtual void onEntityInstancePropertiesChanged(const EntityInstancePtr& /*p_instance*/) { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_ENTITYINSTANCEOBSERVER_H)
