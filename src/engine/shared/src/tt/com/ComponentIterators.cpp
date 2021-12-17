#include <tt/platform/tt_printf.h>
#include <tt/com/ComponentIterators.h>
#include <tt/com/ComponentManager.h>


namespace tt {
namespace com {


ComponentBasePtr g_emptyComponentBasePtr;

//------------------------------------------------------------------------------
// ComponentByTypeIterator implementation


ComponentByTypeIterator& ComponentByTypeIterator::operator++()
{
	if (isDone() == false)
	{
		++m_iter;
	}
	return *this;
}


ComponentByTypeIterator ComponentByTypeIterator::operator++(int)
{
	ComponentByTypeIterator retVal(*this);
	operator++();
	return retVal;
}


bool ComponentByTypeIterator::isDone() const
{
	// Always done if type is invalid
	if (m_type.isValid() == false)
	{
		return true;
	}
	return (m_iter == getManager()->m_componentIDToComponent[m_type.getValue()].end());
}


const ComponentBasePtr& ComponentByTypeIterator::getComponent() const
{
	if (isDone())
	{
		return g_emptyComponentBasePtr;
	}
	return (*m_iter).second;
}


void ComponentByTypeIterator::reset(ComponentID p_type)
{
	m_type = p_type;
	if (m_type.isValid() == false)
	{
		return;
	}
	m_iter = getManager()->m_componentIDToComponent[m_type.getValue()].begin();
}


ComponentByTypeIterator::ComponentByTypeIterator(ComponentManager* p_manager, ComponentID p_type)
:
ComponentIteratorBase(p_manager),
m_iter(),
m_type(ComponentID::invalid)
{
	reset(p_type);
}


//------------------------------------------------------------------------------
// ComponentByInterfaceIterator implementation


ComponentByInterfaceIterator& ComponentByInterfaceIterator::operator++()
{
	// Is both outer and inner iter done?
	if (isDone())
	{
		return *this;
	}
	
	// Is the inner iter done?
	if (m_compIter.isDone())
	{
		// Move outer iter
		++m_compIDIter;
		// Is the outer iter done?
		if (m_compIDIter == getManager()->m_interfaceIDToComponentIDs[m_interfaceID.getValue()].end())
		{
			// Done
			return *this;
		}
		// Reset inner iter with new outer iter.
		m_compIter.reset(*m_compIDIter);
		// If the new inner is done -> ++ again..
		if (m_compIter.isDone())
		{
			operator++();
		}
		return *this;
	}
	else
	{
		// ++ inner iter.
		++m_compIter;
		// Check if inner is done -> ++ again.
		if (m_compIter.isDone())
		{
			operator++();
		}
		return *this;
	}
}


ComponentByInterfaceIterator ComponentByInterfaceIterator::operator++(int)
{
	ComponentByInterfaceIterator retVal(*this);
	operator++();
	return retVal;
}


bool ComponentByInterfaceIterator::isDone() const
{
	// is outer and inner done?
	return m_compIDIter == getManager()->m_interfaceIDToComponentIDs[m_interfaceID.getValue()].end() &&
	       m_compIter.isDone();
}


const ComponentBasePtr& ComponentByInterfaceIterator::getComponent() const
{
	// This call to isDone() needs to be here, otherwise it could
	// potentially try to read from bad addresses
	if (isDone())
	{
		return g_emptyComponentBasePtr;
	}
	return m_compIter.getComponent();
}


ComponentByInterfaceIterator::ComponentByInterfaceIterator(
	ComponentManager* p_manager,
	InterfaceID       p_interfaceID)
:
ComponentIteratorBase(p_manager),
m_compIDIter(),
m_compIter(p_manager, ComponentID::invalid), // We're going to reset it before it's used
m_interfaceID(p_interfaceID)
{
	setToFirstValidComponent();
}


void ComponentByInterfaceIterator::setToFirstValidComponent()
{
	// Set to the first possible component type
	m_compIDIter = getManager()->m_interfaceIDToComponentIDs[m_interfaceID.getValue()].begin();
	
	// Check if m_compIDIter != .end()
	if (m_compIDIter == getManager()->m_interfaceIDToComponentIDs[m_interfaceID.getValue()].end())
	{
		// There can't find any valid component.
		// No component types implement this interface.
		return;
	}
	
	m_compIter.reset(*m_compIDIter);
	
	if (m_compIter.isDone())
	{
		// The first component type gave us nothing
		// Now that we've got both iterators going,
		// we simply need to call the increment operator
		// to get a valid component pointer (if there is one)
		operator++();
	}
}


//------------------------------------------------------------------------------
// ComponentByObjectIterator implementation


ComponentByObjectIterator& ComponentByObjectIterator::operator++()
{
	// There can be only one component implementing an interface in each object,
	// so we don't have to check the rest of the map m_index is pointing at.
	// Move on until we have found another.
	
	while (++m_index < ComponentID::getCount())
	{
		ComponentMap& compMap = getManager()->m_componentIDToComponent[m_index];

		if (compMap.find(m_objectID) != compMap.end())
		{
			// We found a component pointer, so we'll stop here
			break;
		}
	}
	
	return *this;
}


ComponentByObjectIterator ComponentByObjectIterator::operator++(int)
{
	ComponentByObjectIterator retVal(*this);
	operator++();
	return retVal;
}


bool ComponentByObjectIterator::isDone() const
{
	return m_index >= ComponentID::getCount();
}


const ComponentBasePtr& ComponentByObjectIterator::getComponent() const
{
	if (isDone())
	{
		return g_emptyComponentBasePtr;
	}
	
	return (getManager()->m_componentIDToComponent[m_index].find(m_objectID))->second;
}


ComponentByObjectIterator::ComponentByObjectIterator(
	ComponentManager* p_manager,
	ObjectID          p_objectID)
:
ComponentIteratorBase(p_manager),
m_objectID(p_objectID),
m_index(0)
{
	setToFirstValidComponent();
}


void ComponentByObjectIterator::setToFirstValidComponent()
{
	TT_ASSERTMSG(ComponentID::getCount() > 0,
	             "No component IDs have been registered.");
	if (getManager()->m_componentIDToComponent[m_index].find(m_objectID) ==
	    getManager()->m_componentIDToComponent[m_index].end())
	{
		// Couldn't find a component belonging to m_objectID in the first list,
		// now a simple increment will do
		operator++();
	}
}

// Namespace end
}
}
