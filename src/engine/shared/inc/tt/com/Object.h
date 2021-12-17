#if !defined(INC_TT_COM_OBJECT_H)
#define INC_TT_COM_OBJECT_H


#include <tt/com/InterfaceTrait.h>
#include <tt/com/ComponentManager.h>


namespace tt {
namespace com {

class Object
{
public:
	~Object();
	
	inline ObjectID getID() const { return m_objectID; }
	
	bool addComponent(ComponentBasePtr p_component);
	bool removeComponent(InterfaceID p_interfaceID);
	void postMessage(const Message& p_message);
	
	template<class IfaceType>
	typename InterfaceTrait<IfaceType>::Ptr getComponent(
		InterfaceTrait<IfaceType> p_interface)
	{
		return m_manager->queryInterface(m_objectID, p_interface);
	}
	
	template<class IfaceType>
	bool hasComponent(InterfaceTrait<IfaceType> p_interface)
	{
		return m_manager->queryInterface(m_objectID, p_interface.id) != 0;
	}
	
	template<class IfaceType>
	bool removeComponent(const InterfaceTrait<IfaceType>& p_interface)
	{
		return removeComponent(p_interface.id);
	}
	
	//! \brief Set whether the whole (tt::com) object should be destroyed when this object is dies.
	inline void setDestroyWholeObject(bool p_whole) { m_destroyWholeObject = p_whole; }
	inline bool doesDestroyWholeObject() const      { return m_destroyWholeObject; }

#if defined(TT_BUILD_FINAL)
	inline void printComponents() const {}
#else
	void printComponents() const;
#endif
	
private:
	Object(ComponentManager* p_manager, ObjectID p_id);
	
	// No copying
	Object(const Object&);
	const Object& operator=(const Object&);
	
	
	ComponentManager* m_manager;
	ObjectID          m_objectID;
	
	//! \brief If true then no just the object component, but the _whole_ com object is destroyed.
	bool              m_destroyWholeObject;
	
	friend class ComponentManager;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_COM_OBJECT_H)
