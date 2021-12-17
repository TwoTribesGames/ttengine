#include <tt/platform/tt_printf.h>
#include <tt/com/ComponentIterators.h>
#include <tt/com/Object.h>
#include <tt/com/ObjectComponent.h>


namespace tt {
namespace com {

//------------------------------------------------------------------------------
// Public member functions

Object::~Object()
{
	// Remove ObjectComponent from the object
	bool result = m_manager->removeComponentFromObject(m_objectID, ObjectComponent::interfaceID);
	TT_WARNING(result,
	           "Can't remove ObjectComponent from object with ID: '%s'", m_objectID.getName().c_str());
	
	if (m_destroyWholeObject)
	{
		result = m_manager->destroyObject(m_objectID);
		TT_ASSERTMSG(result, "Can't destroy object with ID: '%s'", m_objectID.getName().c_str());
	}
}


bool Object::addComponent(ComponentBasePtr p_component)
{
	return m_manager->addComponentToObject(m_objectID, p_component);
}


bool Object::removeComponent(InterfaceID p_interfaceID)
{
	return m_manager->removeComponentFromObject(m_objectID, p_interfaceID);
}


void Object::postMessage(const Message& p_message)
{
	m_manager->postMessage(m_objectID, p_message);
}

#ifndef TT_BUILD_FINAL
void Object::printComponents() const
{
	ComponentByObjectIterator it(m_manager->getComponentsByObjectID(m_objectID));
	
	TT_Printf("Printing Components for ObjectID 0x%X (%s)\n", 
		m_objectID.getValue(), m_objectID.getName().c_str());
	
	for (int count = 0; it.isDone() == false; ++it, ++count)
	{
		TT_Printf("%2d> %s\n", count, it.getComponent()->getComponentName().c_str());
	}
}
#endif

//------------------------------------------------------------------------------
// Private member functions

Object::Object(ComponentManager* p_manager, ObjectID p_id)
:
m_manager(p_manager),
m_objectID(p_id),
m_destroyWholeObject(false)
{
	// Create a new object with an ObjectComponent
	m_manager->addComponentToDB(m_objectID, ObjectComponent::create());
}

// Namespace end
}
}
