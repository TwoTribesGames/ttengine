#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandChangeEntityProperties.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandChangeEntityPropertiesPtr CommandChangeEntityProperties::create(
		const level::entity::EntityInstanceSet& p_entitiesToChange)
{
	TT_ASSERTMSG(p_entitiesToChange.empty() == false, "No entities specified to change.");
	if (p_entitiesToChange.empty())
	{
		return CommandChangeEntityPropertiesPtr();
	}
	
	return CommandChangeEntityPropertiesPtr(new CommandChangeEntityProperties(p_entitiesToChange));
}


CommandChangeEntityProperties::~CommandChangeEntityProperties()
{
}


void CommandChangeEntityProperties::redo()
{
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
	
	for (Properties::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		(*it).first->setProperties((*it).second.replacementProperties);
	}
}


void CommandChangeEntityProperties::undo()
{
	for (Properties::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		(*it).first->setProperties((*it).second.originalProperties);
	}
}


void CommandChangeEntityProperties::setPropertyValue(const std::string& p_name,
                                                     const std::string& p_value)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call setPropertyValue after this command has already been added to an undo stack.");
		return;
	}
	
	if (p_name.empty())
	{
		TT_PANIC("Properties must have a name (trying to set value '%s' for empty property name).",
		         p_value.c_str());
		return;
	}
	
	for (Properties::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		(*it).second.replacementProperties[p_name] = p_value;
	}
}


void CommandChangeEntityProperties::removeProperty(const std::string& p_name)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call removeProperty after this command has already been added to an undo stack.");
		return;
	}
	
	for (Properties::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		level::entity::EntityInstance::Properties::iterator propIt =
			(*it).second.replacementProperties.find(p_name);
		if (propIt != (*it).second.replacementProperties.end())
		{
			(*it).second.replacementProperties.erase(propIt);
		}
	}
}


void CommandChangeEntityProperties::setPropertyValue(const level::entity::EntityInstancePtr& p_entity,
                                                     const std::string&                      p_name,
                                                     const std::string&                      p_value)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call setPropertyValue after this command has already been added to an undo stack.");
		return;
	}
	
	TT_NULL_ASSERT(p_entity);
	if (p_entity == 0)
	{
		return;
	}
	
	if (p_name.empty())
	{
		TT_PANIC("Properties must have a name (trying to set value '%s' for empty property name).",
		         p_value.c_str());
		return;
	}
	
	Properties::iterator it = m_properties.find(p_entity);
	if (it == m_properties.end())
	{
		TT_PANIC("Entity 0x%08X is not known to this undo command. Cannot set property value.",
		         p_entity.get());
		return;
	}
	
	(*it).second.replacementProperties[p_name] = p_value;
}


void CommandChangeEntityProperties::removeProperty(const level::entity::EntityInstancePtr& p_entity,
                                                   const std::string&                      p_name)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call removeProperty after this command has already been added to an undo stack.");
		return;
	}
	
	TT_NULL_ASSERT(p_entity);
	if (p_entity == 0)
	{
		return;
	}
	
	Properties::iterator it = m_properties.find(p_entity);
	if (it == m_properties.end())
	{
		TT_PANIC("Entity 0x%08X is not known to this undo command. Cannot remove property.",
		         p_entity.get());
		return;
	}
	
	level::entity::EntityInstance::Properties::iterator propIt =
		(*it).second.replacementProperties.find(p_name);
	if (propIt != (*it).second.replacementProperties.end())
	{
		(*it).second.replacementProperties.erase(propIt);
	}
}


bool CommandChangeEntityProperties::hasPropertyChanges() const
{
	for (Properties::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
	{
		if ((*it).second.replacementProperties != (*it).second.originalProperties)
		{
			return true;
		}
	}
	
	return false;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandChangeEntityProperties::CommandChangeEntityProperties(const level::entity::EntityInstanceSet& p_entities)
:
tt::undo::UndoCommand(L"Change Entity Properties"),
m_addedToStack(false),
m_properties()
{
	for (level::entity::EntityInstanceSet::const_iterator it = p_entities.begin();
	     it != p_entities.end(); ++it)
	{
		Props& props(m_properties[*it]);
		props.originalProperties    = (*it)->getProperties();
		props.replacementProperties = props.originalProperties;
	}
}

// Namespace end
}
}
}
}
