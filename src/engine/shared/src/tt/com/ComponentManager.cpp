#include <sstream>

#include <tt/com/ComponentBase.h>
#include <tt/com/ComponentIterators.h>
#include <tt/com/ComponentManager.h>
#include <tt/com/Message.h>
#include <tt/com/Object.h>
#include <tt/com/ObjectComponent.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace com {

/*! \brief Function object that removes an object from the manager. */
class ObjectRemover
{
public:
	ObjectRemover(ComponentManager* p_manager)
	:
	m_manager(p_manager)
	{
	}
	
	inline void operator()(Object* p_object)
	{
		m_manager->removeObject(p_object);
	}
	
private:
	ComponentManager* m_manager;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

ComponentManager::ComponentManager()
:
m_registrationLocked(false),
m_interfaceIDToComponentIDs(0),
m_componentIDToInterfaceIDs(0),
m_componentIDToComponent(0),
m_messageIDToComponentIDs(0),
m_livingIterators(0),
m_componentRemoveQueue(),
m_componentAddQueue(),
m_objects()
{
	// Lock the identifiers (do not allow any new registrations)
	ComponentID::lock();
	InterfaceID::lock();
	MessageID::lock();
	
	// Create the arrays
	m_interfaceIDToComponentIDs = new ComponentIDSet[InterfaceID::getCount()];
	m_componentIDToInterfaceIDs = new InterfaceIDSet[ComponentID::getCount()];
	m_componentIDToComponent    = new ComponentMap[ComponentID::getCount()];
	m_messageIDToComponentIDs   = new ComponentIDSet[MessageID::getCount()];
	
	// Register the ObjectComponent
	ObjectComponent::registerComponentID(this);
}


ComponentManager::~ComponentManager()
{
	update(); // make sure components are removed
	destroyAllComponents();
	
	// Destroy the arrays
	delete[] m_interfaceIDToComponentIDs;
	delete[] m_componentIDToInterfaceIDs;
	delete[] m_componentIDToComponent;
	delete[] m_messageIDToComponentIDs;
	
	m_interfaceIDToComponentIDs = 0;
	m_componentIDToInterfaceIDs = 0;
	m_componentIDToComponent    = 0;
	m_messageIDToComponentIDs   = 0;
}


ObjectID ComponentManager::getUniqueObjectID(math::Random& p_rng) const
{
	ObjectID id;
	
	do
	{
		std::ostringstream name;
		name << "RandomObject_" << p_rng.getNext();
		id = ObjectID(name.str());
	} while (objectExists(id));
	
	return id;
}


ComponentBasePtr ComponentManager::queryInterface(ObjectID    p_oid,
                                                  InterfaceID p_iid)
{
	// Invalid IDs are never present in the manager
	if (p_oid.isValid() == false || p_iid.isValid() == false)
	{
		return ComponentBasePtr();
	}
	
	ComponentIDSet& compIDSet = m_interfaceIDToComponentIDs[p_iid.getValue()];
	
	// Check the components already in the database
	for (ComponentIDSet::iterator compIDSetIter = compIDSet.begin();
	     compIDSetIter != compIDSet.end(); ++compIDSetIter)
	{
		ComponentID compID = (*compIDSetIter);
		ComponentMap& compMap =
			m_componentIDToComponent[compID.getValue()];
		
		ComponentMap::iterator compMapIter = compMap.find(p_oid);
		if (compMapIter != compMap.end())
		{
			// We found our component
			return (*compMapIter).second;
		}
	}
	
	// Also check the components waiting to be added
	for (ComponentSet::iterator it = m_componentAddQueue.begin();
	     it != m_componentAddQueue.end(); ++it)
	{
		if ((*it)->getObjectID() == p_oid && 
			implementsInterface((*it)->getComponentID(), p_iid))
		{
			return *it;
		}
	}
	
	return ComponentBasePtr();
}


ObjectPtr ComponentManager::createObject(ObjectID p_id)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot create objects until registration is locked.");
		return ObjectPtr();
	}
	
	// Check if the object that we're trying to create is scheduled to be removed
	for (ComponentSet::iterator it = m_componentRemoveQueue.begin();
	     it != m_componentRemoveQueue.end(); ++it)
	{
		if ((*it)->getObjectID() == p_id)
		{
			TT_PANIC("Cannot create an object with the same ID "
			         "as one scheduled to be removed."
			         " '%s'(%X) == '%s'(%X)",
			         (*it)->getObjectID().getName().c_str(),
			         (*it)->getObjectID().getValue(),
			         p_id.getName().c_str(), p_id.getValue());
			return ObjectPtr();
		}
	}
	
	// Object cannot already exist
	if (objectExists(p_id))
	{
		TT_PANIC("Cannot create an object with ID '%s' (%X)."
		         " An object with the same ID already exists.",
		         p_id.getName().c_str(), p_id.getValue());
		return ObjectPtr();
	}
	
	// Create a new object
	ObjectPtr object(new Object(this, p_id), ObjectRemover(this));
	
	// Store the object pointer
	m_objects.insert(ObjectContainer::value_type(p_id, object));
	
	// Return the newly created object
	return object;
}


ObjectPtr ComponentManager::createObject(math::Random& p_rng)
{
	return createObject(getUniqueObjectID(p_rng));
}


ObjectPtr ComponentManager::getObject(ObjectID p_id)
{
	ObjectContainer::iterator it = m_objects.find(p_id);
	if (it == m_objects.end())
	{
		// No smart pointer exists; make a new Object
		if (objectExists(p_id) == false)
		{
			// Object with specified ID does not exist; return null pointer
			return ObjectPtr();
		}
		
		// Create a new Object and store the pointer
		ObjectPtr object(new Object(this, p_id), ObjectRemover(this));
		m_objects.insert(ObjectContainer::value_type(p_id, object));
		return object;
	}
	
	return (*it).second.lock();
}


bool ComponentManager::createObject(ObjectID                p_object,
                                    const ComponentBasePtr& p_component)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot create objects until registration is locked.");
		return false;
	}
	
	// Check if the object that we're trying to create is scheduled to be removed
	for (ComponentSet::iterator it = m_componentRemoveQueue.begin();
	     it != m_componentRemoveQueue.end(); ++it)
	{
		if ((*it)->getObjectID() == p_object)
		{
			TT_PANIC("Cannot create an object with the same ID "
			         "as one scheduled to be removed."
			         " '%s'(%X) == '%s'(%X)",
			         (*it)->getObjectID().getName().c_str(),
			         (*it)->getObjectID().getValue(),
			         p_object.getName().c_str(), p_object.getValue());
			return false;
		}
	}
	
	// Object cannot already exist
	if (objectExists(p_object))
	{
		TT_PANIC("Cannot create an object with ID '%s' (%X)."
		         " An object with the same ID already exists.",
		         p_object.getName().c_str(), p_object.getValue());
		return false;
	}
	
	// Add the object to the manager
	addComponentToDB(p_object, p_component);
	
	return true;
}


bool ComponentManager::addComponentToObject(ObjectID                p_object,
                                            const ComponentBasePtr& p_component)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot add components to objects "
		         "until registration is locked.");
		return false;
	}
	
	// Make sure object exists
	if (objectExists(p_object) == false)
	{
		return false;
	}
	
	ComponentID componentID = p_component->getComponentID();
	
	// Make sure the interfaces of the specified component
	// aren't already added for the specified object
	InterfaceIDSet& componentInterfaceIDs = 
		m_componentIDToInterfaceIDs[componentID.getValue()];
	
	for (InterfaceIDSet::iterator it = componentInterfaceIDs.begin();
	     it != componentInterfaceIDs.end(); ++it)
	{
		if (queryInterface(p_object, (*it)) != 0)
		{
			return false;
		}
	}
	
	// Make sure the component can be added to the object
	if (p_component->handleAddToObject(p_object) == false)
	{
		return false;
	}
	
	// Add this component to the object
	addComponentToDB(p_object, p_component);
	
	return true;
}


bool ComponentManager::removeComponentFromObject(ObjectID p_object, InterfaceID p_interfaceID)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot remove components from objects "
		         "until registration is locked.");
		return false;
	}
	
	// Make sure object exists
	if (objectExists(p_object) == false)
	{
		return false;
	}
	
	// Find the component that implements the specified interface
	ComponentBasePtr component = queryInterface(p_object, p_interfaceID);
	if (component == 0)
	{
		return false;
	}
	
	// Let removeComponent handle the rest
	return removeComponent(component);
}


bool ComponentManager::destroyObject(ObjectID p_object)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot destroy objects until registration is locked.");
		return false;
	}
	
	// Cannot destroy object by ID if it is managed by a smart pointer
	if (m_objects.find(p_object) != m_objects.end())
	{
		TT_PANIC("Cannot destroy object by ID if it is "
		         "managed by a smart pointer."
		         "ID '%s'(%X)", 
		         p_object.getName().c_str(), p_object.getValue());
		return false;
	}
	
	/*
	// Do not allow removal when iterators are alive
	if (m_livingIterators > 0)
	{
		TT_PANIC("Cannot destroy object when iterators are still alive.");
		return false;
	}
	// */
	
	// Object must exist
	if (objectExists(p_object) == false)
	{
		return false;
	}
	
	// Remove all components that belong to this object
	for (ComponentID id = ComponentID::first(); id <= ComponentID::last(); ++id)
	{
		// Check if a component of this type is registered for the object
		ComponentMap& cm(m_componentIDToComponent[id.getValue()]);
		ComponentMap::iterator it = cm.find(p_object);
		if (it != cm.end())
		{
			// Do not remove this component if it is already queued for removal
			if (m_componentRemoveQueue.find((*it).second) ==
			    m_componentRemoveQueue.end())
			{
				// Remove the component from the object
				if (removeComponent((*it).second) == false)
				{
					return false;
				}
			}
		}
	}
	
	// Object has been removed
	return true;
}


bool ComponentManager::removeComponent(const ComponentBasePtr& p_component)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot remove components from objects "
		         "until registration is locked.");
		return false;
	}
	
	// Ensure component isn't already scheduled to be removed
	if (m_componentRemoveQueue.find(p_component) !=
	    m_componentRemoveQueue.end())
	{
		TT_WARN("Attempting to remove the same component more than once.");
		return true;
	}
	
	if (m_livingIterators == 0)
	{
		// No iterators are alive; safe to modify the containers directly
		
		// First remove all the queued components
		processComponentRemoveQueue();
		
		// Then remove this component
		ComponentID id(p_component->getComponentID());
		ComponentMap::iterator mapIt =
			m_componentIDToComponent[id.getValue()].find(
				p_component->getObjectID());
		if (mapIt != m_componentIDToComponent[id.getValue()].end())
		{
			p_component->handleRemoveFromObject();
			m_componentIDToComponent[id.getValue()].erase(mapIt);
		}
		
		// Lastly, process the queued components to be added
		processComponentAddQueue();
	}
	else
	{
		// Schedule the component for removal
		m_componentRemoveQueue.insert(p_component);
	}
	
	return true;
}


void ComponentManager::destroyAllComponents()
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot destroy components until registration is locked.");
		return;
	}
	
	for (ComponentID id = ComponentID::first(); id <= ComponentID::last(); ++id)
	{
		// Remove all components of this component ID
		m_componentIDToComponent[id.getValue()].clear();
	}
}


void ComponentManager::update()
{
	// Remove all components that are waiting to be removed
	processComponentRemoveQueue();
	
	// Add all components that are waiting to be added
	processComponentAddQueue();
}


ComponentByInterfaceIterator ComponentManager::getComponentsByInterfaceID(InterfaceID p_id)
{
	TT_ASSERTMSG(m_registrationLocked,
	             "Cannot iterate over components until registration is locked.");
	return ComponentByInterfaceIterator(this, p_id);
}


ComponentByTypeIterator ComponentManager::getComponentsByComponentID(ComponentID p_id)
{
	TT_ASSERTMSG(m_registrationLocked,
	             "Cannot iterate over components until registration is locked.");
	return ComponentByTypeIterator(this, p_id);
}


ComponentByObjectIterator ComponentManager::getComponentsByObjectID(ObjectID p_id)
{
	TT_ASSERTMSG(m_registrationLocked,
	             "Cannot iterate over components until registration is locked.");
	return ComponentByObjectIterator(this, p_id);
}


void ComponentManager::registerInterface(InterfaceID p_iid, ComponentID p_cid)
{
	if (m_registrationLocked)
	{
		TT_PANIC("Cannot register new interfaces after "
		         "registration has been locked.");
		return;
	}
	
	// FIXME: Check double insert for both.
	m_interfaceIDToComponentIDs[p_iid.getValue()].insert(p_cid);
	m_componentIDToInterfaceIDs[p_cid.getValue()].insert(p_iid);
}


void ComponentManager::subscribeToMessageID(ComponentID p_cid, MessageID p_mid)
{
	if (m_registrationLocked)
	{
		TT_PANIC("Cannot subscribe to message IDs after "
		         "registration has been locked.");
		return;
	}
	
	m_messageIDToComponentIDs[p_mid.getValue()].insert(p_cid);
}


void ComponentManager::subscribeToAllMessageIDs(ComponentID p_componentID)
{
	if (m_registrationLocked)
	{
		TT_PANIC("Cannot subscribe to message IDs after "
		         "registration has been locked.");
		return;
	}
	
	for (MessageID msgID = MessageID::first(); msgID <= MessageID::last(); ++msgID)
	{
		subscribeToMessageID(p_componentID, msgID);
	}
}


void ComponentManager::postMessage(ObjectID p_oid, const Message& p_msg)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot post messages until registration is locked.");
		return;
	}
	
	// Iterate through all components for the object ID
	
	/*
#if !defined(TT_BUILD_FINAL)
	bool messageHandled = false;
#endif
	// */
	
	for (ComponentByObjectIterator it = getComponentsByObjectID(p_oid); it.isDone() == false; ++it)
	{
		/*
#if !defined(TT_BUILD_FINAL)
		MessageResult result = it.getComponent()->handleMessage(p_msg);
		
		if (result != MessageResult_Unknown)
		{
			messageHandled = true;
		}
#else
		// */
		it.getComponent()->handleMessage(p_msg);
//#endif
	}
	
	/*
#if !defined(TT_BUILD_FINAL)
	// check for unhandled messages
	//TT_ASSERTMSG(messageHandled, "Message %d was not handled by any component of object %d",
	//	p_msg.getID().getValue(), p_oid.getValue());
#endif
	// */
}


void ComponentManager::broadcastMessage(const Message& p_msg)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot broadcast messages until registration is locked.");
		return;
	}
	
	ComponentIDSet& compSet = m_messageIDToComponentIDs[p_msg.getID().getValue()];
	
	// Find the component types that subscribe to this message
	for (ComponentIDSet::iterator it = compSet.begin(); it != compSet.end(); ++it)
	{
		ComponentID cid = (*it); // Found one
		
		// Go through the components of this component type and send message
		for (ComponentByTypeIterator ci = getComponentsByComponentID(cid);
		     ci.isDone() == false; ++ci)
		{
			ComponentBasePtr component(ci.getComponent());
			if (component != 0)
			{
#ifndef TT_BUILD_FINAL
				MessageResult result = component->handleMessage(p_msg);
				
				// check for unhandled messages
				TT_ASSERTMSG(result != MessageResult_Unknown, 
					"Component %s subscribed to unhandled message %d",
					component->getComponentName().c_str(), p_msg.getID().getValue());
#else
				component->handleMessage(p_msg);
#endif
			}
		}
	}
}


bool ComponentManager::isInterfaceImplementedByComponent(
		InterfaceID             p_iid,
		const ComponentBasePtr& p_component) const
{
	if (m_interfaceIDToComponentIDs[p_iid.getValue()].find(p_component->getComponentID()) ==
	    m_interfaceIDToComponentIDs[p_iid.getValue()].end())
	{
		return false;
	}
	
	return true;
}


void ComponentManager::lockRegistration()
{
	TT_ASSERTMSG(m_registrationLocked == false, "Cannot lock registration more than once.");
	m_registrationLocked = true;
	
	// Check all component IDs to make sure they have an interface registered
	for (ComponentID cid = ComponentID::first(); cid <= ComponentID::last(); ++cid)
	{
		int val = cid.getValue();
		TT_ASSERTMSG(m_componentIDToInterfaceIDs[val].empty() == false,
		             "Component with ID %d has not been registered yet.",
		             val);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool ComponentManager::addComponentToDB(ObjectID                p_oid,
                                        const ComponentBasePtr& p_component)
{
	if (m_registrationLocked == false)
	{
		TT_PANIC("Cannot add components to the database "
		         "until registration is locked.");
		return false;
	}
	
	// Pointer cannot be null
	if (p_component == 0)
	{
		TT_PANIC("Cannot add invalid component pointers.");
		return false;
	}
	
	// Check if the component is already in the database
	// (or waiting to be added)
	ComponentID id(p_component->getComponentID());
	if ((m_componentIDToComponent[id.getValue()].find(p_oid) !=
	     m_componentIDToComponent[id.getValue()].end()) ||
	    m_componentAddQueue.find(p_component) != m_componentAddQueue.end())
	{
		return false;
	}
	
	// Set the object ID for the component
	p_component->setObjectID(p_oid);
	
	if (m_livingIterators == 0)
	{
		// No iterators are alive; safe to modify the containers directly
		
		// First add all the queued components
		processComponentRemoveQueue();
		processComponentAddQueue();
		
		// Then add this component
		m_componentIDToComponent[p_component->getComponentID().getValue()].insert(
			ComponentMap::value_type(p_oid, p_component));
	}
	else
	{
		// Queue the component for addition
		m_componentAddQueue.insert(p_component);
	}
	
	return true;
}


bool ComponentManager::objectExists(ObjectID p_oid) const
{
	// Check the database for the object ID
	for (ComponentID id = ComponentID::first(); id <= ComponentID::last(); ++id)
	{
		const ComponentMap& compMap = m_componentIDToComponent[id.getValue()];
		ComponentMap::const_iterator compMapIter = compMap.find(p_oid);
		if (compMapIter != compMap.end())
		{
			// We found a component belonging to this object, so the object exists
			return true;
		}
	}
	
	// Also check the components waiting to be added
	for (ComponentSet::const_iterator it = m_componentAddQueue.begin();
	     it != m_componentAddQueue.end(); ++it)
	{
		if ((*it)->getObjectID() == p_oid)
		{
			return true;
		}
	}
	
	return false;
}


bool ComponentManager::removeComponent(ComponentBase* p_component)
{
	// Find the component in the database
	for (ComponentID id = ComponentID::first(); id <= ComponentID::last(); ++id)
	{
		ComponentMap& cm(m_componentIDToComponent[id.getValue()]);
		for (ComponentMap::iterator it = cm.begin(); it != cm.end(); ++it)
		{
			if ((*it).second.get() == p_component)
			{
				return removeComponent((*it).second);
			}
		}
	}
	
	// Component not found in the database
	return false;
}


bool ComponentManager::implementsInterface(ComponentID p_componentID, 
                                           InterfaceID p_interfaceID) const
{
	InterfaceIDSet& component = m_componentIDToInterfaceIDs[p_componentID.getValue()];
	return component.find(p_interfaceID) != component.end();
	// ComponentIDSet& interface = m_interfaceIDToComponentIDs[p_interfaceID.getValue()];
	// return interface.find(p_componentID) != interface.end();
}


void ComponentManager::removeObject(Object* p_object)
{
	if (p_object == 0)
	{
		TT_PANIC("Null pointer passed!");
		return;
	}
	
	ObjectContainer::iterator it = m_objects.find(p_object->getID());
	if (it != m_objects.end())
	{
		TT_ASSERT((*it).second.expired());
		
		// Remove object pointer from list
		m_objects.erase(it);
		
		// Destroy object
		delete p_object;
	}
}


void ComponentManager::iteratorLives()
{
	++m_livingIterators;
}


void ComponentManager::iteratorDied()
{
	TT_ASSERTMSG(m_livingIterators > 0, "More iterators died than were alive!");
	if (m_livingIterators > 0)
	{
		--m_livingIterators;
	}
}


void ComponentManager::processComponentAddQueue()
{
	for (ComponentSet::iterator it = m_componentAddQueue.begin();
	     it != m_componentAddQueue.end(); ++it)
	{
		ComponentID id((*it)->getComponentID());
		m_componentIDToComponent[id.getValue()].insert(
			ComponentMap::value_type((*it)->getObjectID(), *it));
	}
	m_componentAddQueue.clear();
}


void ComponentManager::processComponentRemoveQueue()
{
	for (ComponentSet::iterator it = m_componentRemoveQueue.begin();
	     it != m_componentRemoveQueue.end(); ++it)
	{
		ComponentID id((*it)->getComponentID());
		ComponentMap::iterator mapIt =
			m_componentIDToComponent[id.getValue()].find((*it)->getObjectID());
		if (mapIt != m_componentIDToComponent[id.getValue()].end())
		{
			(*it)->handleRemoveFromObject();
			m_componentIDToComponent[id.getValue()].erase(mapIt);
		}
	}
	m_componentRemoveQueue.clear();
}

// Namespace end
}
}
