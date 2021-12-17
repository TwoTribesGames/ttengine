#include <tt/com/ObjectComponent.h>
#include <tt/com/ComponentManager.h>


namespace tt {
namespace com {

const InterfaceID ObjectComponent::interfaceID;
const ComponentID ObjectComponent::componentID;


//------------------------------------------------------------------------------
// Public member functions

ObjectComponent::~ObjectComponent()
{
}


ComponentID ObjectComponent::getComponentID() const
{
	return componentID;
}


void ObjectComponent::registerComponentID(ComponentManager* p_manager)
{
	p_manager->registerInterface(interfaceID, componentID);
}


//------------------------------------------------------------------------------
// Private member functions

ObjectComponentPtr ObjectComponent::create()
{
	return ObjectComponentPtr(new ObjectComponent);
}


ObjectComponent::ObjectComponent()
{
}

// Namespace end
}
}
