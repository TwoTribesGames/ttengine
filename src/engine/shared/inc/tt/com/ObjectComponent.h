#if !defined(INC_TT_COM_OBJECTCOMPONENT_H)
#define INC_TT_COM_OBJECTCOMPONENT_H


#include <tt/com/ComponentBase.h>


namespace tt {
namespace com {

class ComponentManager;
class Object;

class ObjectComponent;
typedef tt_ptr<ObjectComponent>::shared ObjectComponentPtr;


/*! \brief .... For internal use by the component system only. */
class ObjectComponent : public ComponentBase
{
public:
	static const InterfaceID interfaceID;
	
	
	virtual ~ObjectComponent();
	
	virtual ComponentID getComponentID() const;
	
	static void registerComponentID(ComponentManager* p_manager);
	
private:
	static const ComponentID componentID;
	
	
	static ObjectComponentPtr create();
	
	ObjectComponent();
	
	friend class Object;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_COM_OBJECTCOMPONENT_H)
