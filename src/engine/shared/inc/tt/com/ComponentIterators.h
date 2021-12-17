#if !defined(INC_TT_COM_COMPONENTITERATORS_H)
#define INC_TT_COM_COMPONENTITERATORS_H


#include <set>

#include <tt/com/types.h>
#include <tt/com/ComponentManager.h>


namespace tt {
namespace com {

class ComponentManager;

/*! \brief Base class wich stores a pointer to the component manager and
           makes sure said manager knows how many iterators are alive. */
class ComponentIteratorBase
{
protected:
	explicit inline ComponentIteratorBase(ComponentManager* p_manager)
	:
	m_manager(p_manager)
	{
		TT_NULL_ASSERT(m_manager);
		m_manager->iteratorLives();
	}
	
	inline ComponentIteratorBase(const ComponentIteratorBase& p_copy)
	:
	m_manager(p_copy.m_manager)
	{
		m_manager->iteratorLives();
	}
	
	inline ~ComponentIteratorBase()
	{
		m_manager->iteratorDied();
	}
	
	inline const ComponentIteratorBase& operator=(const ComponentIteratorBase& p_rhs)
	{
		m_manager = p_rhs.m_manager;
		return (*this);
	}
	
	inline const ComponentManager* getManager() const { return m_manager; }
	inline       ComponentManager* getManager()       { return m_manager; }
	
private:
	ComponentManager* m_manager;
};



/*! \brief Iterates through all component instances of a given component ID. */
class ComponentByTypeIterator : private ComponentIteratorBase
{
public:
	ComponentByTypeIterator& operator++();
	ComponentByTypeIterator operator++(int);
	
	bool             isDone() const;
	const ComponentBasePtr& getComponent() const;
	
	void reset(ComponentID p_type);
	
private:
	ComponentByTypeIterator(ComponentManager* p_manager, ComponentID p_type);
	
	
	ComponentMap::iterator m_iter;
	ComponentID            m_type;
	
	friend class ComponentManager;
	friend class ComponentByInterfaceIterator;
};


/*! \brief Iterates through all component instances of
           components which implement a given interface. */
class ComponentByInterfaceIterator : private ComponentIteratorBase
{
public:
	ComponentByInterfaceIterator& operator++();
	ComponentByInterfaceIterator operator++(int);
	
	bool isDone() const;
	//template <class InterfaceType> // TODO: Make templated, and return correct InterfacePtr
	const ComponentBasePtr& getComponent() const;
	
private:
	ComponentByInterfaceIterator(ComponentManager* p_manager,
	                             InterfaceID       p_interfaceID);
	void setToFirstValidComponent();
	
	
	ComponentIDSet::iterator m_compIDIter;  //!< Iterator for all components of this interface.
	ComponentByTypeIterator  m_compIter;    //!< Iterator for all objects of a component
	InterfaceID              m_interfaceID; //!< InterfaceID which is being iterated.
	
	friend class ComponentManager;
};


/*! \brief Iterates through all component instances
           that have been added to a given object. */
class ComponentByObjectIterator : private ComponentIteratorBase
{
public:
	ComponentByObjectIterator& operator++();
	ComponentByObjectIterator operator++(int);
	
	bool isDone() const;
	const ComponentBasePtr& getComponent() const;
	
private:
	ComponentByObjectIterator(ComponentManager* p_manager, ObjectID p_objectID);
	void setToFirstValidComponent();
	
	ObjectID          m_objectID;
	int               m_index;
	
	friend class ComponentManager;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_COM_COMPONENTITERATORS_H)
