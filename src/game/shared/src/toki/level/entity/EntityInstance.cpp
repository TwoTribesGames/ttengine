#include <tt/platform/tt_error.h>

#include <toki/game/script/EntityScriptMgr.h>
#include <toki/level/entity/editor/EntityInstanceEditorRepresentation.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/entity/EntityInstanceObserver.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace level {
namespace entity {

static const std::string g_emptyString;


//--------------------------------------------------------------------------------------------------
// Public member functions

EntityInstancePtr EntityInstance::create(const std::string&       p_type,
                                         s32                      p_id,
                                         const tt::math::Vector2& p_position)
{
	if (p_type.empty())
	{
		TT_PANIC("Entity type name must not be empty.");
		return EntityInstancePtr();
	}
	
	if (p_id < 0)
	{
		TT_PANIC("Invalid entity ID specified: %d", p_id);
		return EntityInstancePtr();
	}
	
	EntityInstancePtr instance(new EntityInstance(p_type, p_id, p_position));
	instance->m_this = instance;
	return instance;
}


EntityInstance::~EntityInstance()
{
	delete m_editorRepresentation;
}


EntityInstancePtr EntityInstance::clone() const
{
	EntityInstancePtr copy(new EntityInstance(*this));
	copy->m_this = copy;
	return copy;
}


EntityInstancePtr EntityInstance::cloneWithNewID(s32 p_newID) const
{
	EntityInstancePtr copy(new EntityInstance(*this));
	copy->m_this = copy;
	copy->m_id   = p_newID;
	return copy;
}


void EntityInstance::setPosition(const tt::math::Vector2& p_pos)
{
	if (p_pos != m_position)
	{
		m_position = p_pos;
		notifyPositionChanged();
	}
}


void EntityInstance::setProperties(const Properties& p_properties)
{
	if (p_properties != m_properties)
	{
		m_properties = p_properties;
		notifyPropertiesChanged();
	}
}


bool EntityInstance::hasProperty(const std::string& p_name) const
{
	return m_properties.find(p_name) != m_properties.end();
}


const std::string& EntityInstance::getPropertyValue(const std::string& p_name) const
{
	Properties::const_iterator it = m_properties.find(p_name);
	if (it == m_properties.end())
	{
		TT_PANIC("Entity with ID %d (of type '%s') does not have a property named '%s'.",
		         m_id, m_type.c_str(), p_name.c_str());
		return g_emptyString;
	}
	
	return (*it).second;
}


void EntityInstance::setPropertyValue(const std::string& p_name, const std::string& p_value)
{
	if (p_name.empty())
	{
		TT_PANIC("Properties must have a name (trying to set value '%s' for empty property name).",
		         p_value.c_str());
		return;
	}
	
	bool propertyChanged = false;
	Properties::iterator it = m_properties.find(p_name);
	if (it == m_properties.end())
	{
		// Adding new property
		m_properties[p_name] = p_value;
		propertyChanged = true;
	}
	else
	{
		// Changing existing property
		if (p_value != (*it).second)
		{
			(*it).second = p_value;
			propertyChanged = true;
		}
	}
	
	if (propertyChanged)
	{
		notifyPropertiesChanged();
	}
}


void EntityInstance::removeProperty(const std::string& p_name)
{
	Properties::iterator it = m_properties.find(p_name);
	if (it != m_properties.end())
	{
		m_properties.erase(it);
		
		notifyPropertiesChanged();
	}
}


bool EntityInstance::isPropertyVisible(const std::string& p_name) const
{
	const EntityInfo* typeInfo = AppGlobal::getEntityLibrary().getEntityInfo(getType());
	if (typeInfo == 0)
	{
		return false;
	}
	
	if (typeInfo->hasProperty(p_name) == false)
	{
		return false;
	}
	
	const EntityProperty& prop(typeInfo->getProperty(p_name));
	if (prop.hasConditional() == false)
	{
		// Early exit
		return true;
	}
	
	const EntityProperty::Conditional& conditional(prop.getConditional());
	const std::string& targetPropertyName(conditional.getTargetPropertyName());
	
	if (hasProperty(targetPropertyName))
	{
		return conditional.test(getPropertyValue(targetPropertyName));
	}
	
	// Not set in instance; look further in entity for default
	if (typeInfo->hasProperty(targetPropertyName) == false)
	{
		TT_PANIC("EntityInstance '%s' doesn't have target property '%s'",
			getType().c_str(), targetPropertyName.c_str());
		return false;
	}
	
	const EntityProperty& targetProp(typeInfo->getProperty(targetPropertyName));
	
	return conditional.test(targetProp.getDefault().getAsString());
}


void EntityInstance::registerObserver(const EntityInstanceObserverWeakPtr& p_observer)
{
	EntityInstanceObserverPtr observer(p_observer.lock());
	TT_NULL_ASSERT(observer);
	if (observer == 0)
	{
		return;
	}
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); ++it)
	{
		if ((*it).lock() == observer)
		{
			TT_PANIC("Observer 0x%08X is already registered.", observer.get());
			return;
		}
	}
	
	m_observers.push_back(p_observer);
}


void EntityInstance::unregisterObserver(const EntityInstanceObserverWeakPtr& p_observer)
{
	EntityInstanceObserverPtr observer(p_observer.lock());
	TT_NULL_ASSERT(observer);
	if (observer == 0)
	{
		return;
	}
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); ++it)
	{
		if ((*it).lock() == observer)
		{
			m_observers.erase(it);
			return;
		}
	}
	
	TT_PANIC("Observer 0x%08X was not registered.", observer.get());
}


editor::EntityInstanceEditorRepresentation* EntityInstance::getOrCreateEditorRepresentation()
{
	if (m_editorRepresentation == 0)
	{
		m_editorRepresentation = new editor::EntityInstanceEditorRepresentation(this);
	}
	return m_editorRepresentation;
}


void EntityInstance::destroyEditorRepresentation()
{
	delete m_editorRepresentation;
	m_editorRepresentation = 0;
}


bool EntityInstance::matchesFilter(const tt::str::StringSet& p_filter) const
{
	// Early exit; if no filter is set, all EntityInstances are accepted
	if (p_filter.empty()) return true;
	
	const game::script::EntityScriptMgr& scriptMgr(AppGlobal::getEntityScriptMgr());
	
	// Now check if entities are of filtered class (or are derived from it)
	for (tt::str::StringSet::const_iterator filterIt = p_filter.begin();
			filterIt != p_filter.end(); ++filterIt)
	{
		if (scriptMgr.classHasBaseClass(getType(), *filterIt))
		{
			return true;
		}
	}
	
	return false;
}


bool EntityInstance::sortOrder(const EntityInstancePtr& p_left, const EntityInstancePtr& p_right)
{
	const level::entity::EntityInfo* entityLeftInfo  = AppGlobal::getEntityLibrary().getEntityInfo(p_left->getType());
	const level::entity::EntityInfo* entityRightInfo = AppGlobal::getEntityLibrary().getEntityInfo(p_right->getType());
	if (entityLeftInfo == 0 || entityRightInfo == 0)
	{
		return false;
	}
	
	if (entityLeftInfo->getOrder() == entityRightInfo->getOrder())
	{
		return p_left->getID() < p_right->getID();
	}
	return entityLeftInfo->getOrder() < entityRightInfo->getOrder();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

EntityInstance::EntityInstance(const std::string&       p_type,
                               s32                      p_id,
                               const tt::math::Vector2& p_position)
:
m_this(),
m_type(p_type),
m_id(p_id),
m_spawnSectionID(-1),
m_position(p_position),
m_properties(),
m_observers(),
m_propertiesUpdatedByScript(false),
m_editorRepresentation(0)
{
}


EntityInstance::EntityInstance(const EntityInstance& p_rhs)
:
m_this(),  // NOTE: Calling code must set m_this to a valid pointer
m_type(p_rhs.m_type),
m_id(p_rhs.m_id),
m_spawnSectionID(p_rhs.m_spawnSectionID),
m_position(p_rhs.m_position),
m_properties(p_rhs.m_properties),
m_observers(),  // NOTE: Copying an EntityInstance does not copy the observers: these should be explicitly registered
m_propertiesUpdatedByScript(p_rhs.m_propertiesUpdatedByScript),
m_editorRepresentation(0)  // NOTE: Editor representation will need to be recreated for copies
{
}


void EntityInstance::notifyPositionChanged()
{
	if (m_observers.empty())
	{
		return;
	}
	
	EntityInstancePtr instance(m_this.lock());
	TT_NULL_ASSERT(instance);
	if (instance == 0)
	{
		// Somehow this function was called when no more smart pointers point to this object
		return;
	}
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); )
	{
		EntityInstanceObserverPtr observer((*it).lock());
		if (observer == 0)
		{
			it = m_observers.erase(it);
		}
		else
		{
			observer->onEntityInstancePositionChanged(instance);
			++it;
		}
	}
}


void EntityInstance::notifyPropertiesChanged()
{
	if (m_editorRepresentation != 0) m_editorRepresentation->onPropertiesChanged();
	
	if (m_observers.empty())
	{
		return;
	}
	
	EntityInstancePtr instance(m_this.lock());
	TT_NULL_ASSERT(instance);
	if (instance == 0)
	{
		// Somehow this function was called when no more smart pointers point to this object
		return;
	}
	
	for (Observers::iterator it = m_observers.begin(); it != m_observers.end(); )
	{
		EntityInstanceObserverPtr observer((*it).lock());
		if (observer == 0)
		{
			it = m_observers.erase(it);
		}
		else
		{
			observer->onEntityInstancePropertiesChanged(instance);
			++it;
		}
	}
}

// Namespace end
}
}
}
