#if !defined(INC_TT_COM_COMPONENTMANAGER_H)
#define INC_TT_COM_COMPONENTMANAGER_H


#include <tt/com/InterfaceTrait.h>
#include <tt/com/types.h>
#include <tt/math/Random.h>


namespace tt {
namespace com {

class Message;
class ComponentIteratorBase;
class ComponentByInterfaceIterator;
class ComponentByTypeIterator;
class ComponentByObjectIterator;
class ObjectRemover;


class ComponentManager
{
public:
	ComponentManager();
	~ComponentManager();
	
	/*! \brief Generates an object ID that is unique to this manager
	           (has not been created yet). */
	ObjectID getUniqueObjectID(
			math::Random& p_rng = math::Random::getStatic()) const;
	
	/*! \brief If an interface is registered with an object,
	           returns a pointer to the component that implements it. */
	ComponentBasePtr queryInterface(ObjectID    p_objectID,
	                                InterfaceID p_interfaceID);
	
	/*! \brief If an interface is registered with an object,
	           returns a pointer to the component that implements it. */
	template<class IfaceType>
	typename InterfaceTrait<IfaceType>::Ptr queryInterface(
		ObjectID p_objectID, InterfaceTrait<IfaceType> p_interface)
	{
		ComponentBasePtr ptr = queryInterface(p_objectID, p_interface.id);
		if (ptr == 0)
		{
			return typename InterfaceTrait<IfaceType>::Ptr();
		}
		
#if defined(TT_BUILD_FINAL)
		return tt_ptr_static_cast<IfaceType>(ptr);
#else
		TT_NULL_ASSERT(ptr);
		typename InterfaceTrait<IfaceType>::Ptr returnPtr = tt_ptr_dynamic_cast<IfaceType>(ptr);
		TT_NULL_ASSERT(returnPtr);
		return returnPtr;
#endif
	}
	
	/*! \brief Creates a new object.
	    \return Smart pointer to the newly created object; null pointer
	            if the object could not be created */
	ObjectPtr createObject(ObjectID p_id);
	
	/*! \brief Creates a new object with a randomly generated identifier.
	    \param p_rng The random number generator to use.
	    \return Smart pointer to the newly created object; null pointer
	            if the object could not be created */
	ObjectPtr createObject(math::Random& p_rng = math::Random::getStatic());
	
	/*! \brief Retrieves an object with the specified ID.
	    \return Smart pointer to the object; null pointer if
	            the object doesn't exist. */
	ObjectPtr getObject(ObjectID p_id);
	
	/*! \brief Creates a new object.
	    \return True if new object was created, false if not. */
	bool createObject(ObjectID p_object, const ComponentBasePtr& p_component);
	
	/*! \brief Adds a component to an existing object.
	    \return True if the component was added, false if not. */
	bool addComponentToObject(ObjectID p_object, const ComponentBasePtr& p_component);
	
	/*! \brief Removes a component from an existing object.
	           Note: If the last component is removed from an object,
	           the object also ceases to exist.
	    \return True if the component was removed, false if not. */
	bool removeComponentFromObject(ObjectID p_object, InterfaceID p_interfaceID);
	
	/*! \brief Destroys the specified object, removing all components.
	    \return True if destroying the object was successful, false if not. */
	bool destroyObject(ObjectID p_object);
	
	/*! \brief Removes a component from an existing object.
	           Note: If the last component is removed from an object,
	           the object also ceases to exist.
	    \return True if removing the component was successful, false if not. */
	bool removeComponent(const ComponentBasePtr& p_component);
	
	/*! \brief Destroys all registered components. */
	void destroyAllComponents();
	
	/*! \brief Destroys components that are waiting to be destroyed and
	           creates components that are waiting to be created. */
	void update();
	
	// The following method get iterators that iterate over all the components
	// that fulfill the criteria passed into the method.
	ComponentByInterfaceIterator getComponentsByInterfaceID(InterfaceID p_id);
	ComponentByTypeIterator      getComponentsByComponentID(ComponentID p_id);
	ComponentByObjectIterator    getComponentsByObjectID(ObjectID p_id);
	
	/*! \brief This lets the object manager know that the given component type
	           implements the interface of the given interface type. */
	void registerInterface(InterfaceID p_interfaceID,
	                       ComponentID p_componentID);
	
	/*! \brief Subscribes a component ID to receive a specific type of message. */
	void subscribeToMessageID(ComponentID p_componentID,
	                          MessageID   p_messageID);
	
	/*! \brief Subscribes a component ID to receive all available messages. */
	void subscribeToAllMessageIDs(ComponentID p_componentID);
	
	/*! \brief Sends a message to all subscribing components of one object
	           (as defined by the first parameter).
	    \note This function does not honor the individual components' message subscriptions! */
	void postMessage(ObjectID p_objectID, const Message& p_msg);
	
	/*! \brief Sends a message to all subscribing components of all objects. */
	void broadcastMessage(const Message& p_msg);
	
	// Goes through all the interfaces implemented by the component and
	// finds out if the interface id passed in is one of them.
	bool isInterfaceImplementedByComponent(InterfaceID             p_interface,
	                                       const ComponentBasePtr& p_component) const;
	
	/*! \brief Locks the manager from any new component, interface or
	           message registration. Must be called before the
	           manager can be used. */
	void lockRegistration();
	
private:
	typedef std::map<ObjectID, tt_ptr<Object>::weak> ObjectContainer;
	
	
	ComponentManager(const ComponentManager&);            //!< No copy. (Not implemented.)
	ComponentManager& operator=(const ComponentManager&); //!< No assignment. (Not implemented.)
	
	bool addComponentToDB(ObjectID p_id, const ComponentBasePtr& p_component);
	
	// Checks for the existence of an object (one or more component
	// instances with the given object id)
	bool objectExists(ObjectID p_id) const;
	
	/*! \brief Version of removeComponent that accepts a raw pointer, so that
	           components can remove themselves. */
	bool removeComponent(ComponentBase* p_component);
	
	bool implementsInterface(ComponentID, InterfaceID) const;
	
	void removeObject(Object* p_object);
	
	void iteratorLives();
	void iteratorDied();
	
	void processComponentAddQueue();
	void processComponentRemoveQueue();
	
	
	bool m_registrationLocked;
	
	// Static component type data
	ComponentIDSet* m_interfaceIDToComponentIDs; // for each InterfaceID
	
	// Static interface type data
	InterfaceIDSet* m_componentIDToInterfaceIDs; // for each ComponentID
	
	// Dynamic component data
	ComponentMap* m_componentIDToComponent; // for each ComponentID
	
	// Message data
	ComponentIDSet* m_messageIDToComponentIDs; // for each MessageID
	
	int             m_livingIterators; //!< The number of iterators that are alive.
	ComponentSet    m_componentRemoveQueue;
	ComponentSet    m_componentAddQueue;
	ObjectContainer m_objects;
	
	friend class ComponentIteratorBase;
	friend class ComponentByTypeIterator;
	friend class ComponentByInterfaceIterator;
	friend class ComponentByObjectIterator;
	friend class ComponentBase;
	friend class Object;
	friend class ObjectRemover;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_COM_COMPONENTMANAGER_H)
