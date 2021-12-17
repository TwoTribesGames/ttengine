#include <tt/platform/tt_error.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuAction.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

MenuElementAction::MenuElementAction(const MenuAction& p_action)
{
	TT_ASSERTMSG(p_action.getCommand() == "menu_element_action",
	             "MenuSystem::doMenuElementAction can only handle "
	             "'menu_element_action' commands");
	
	TT_ASSERTMSG(p_action.getParameterCount() >= 2,
	             "menu_element_action should have at least 2 parameters: "
	             "target element name, command name.");
	
	m_targetElement = p_action.getParameter(0);
	m_command       = p_action.getParameter(1);
	for (int idx = 2; idx < p_action.getParameterCount(); ++idx)
	{
		m_parameters.push_back(p_action.getParameter(idx));
	}
}


MenuElementAction::MenuElementAction(const std::string& p_targetElement,
                                     const std::string& p_command)
:
m_targetElement(p_targetElement),
m_command(p_command)
{
}


std::string MenuElementAction::getParameter(int p_index) const
{
	TT_ASSERTMSG(p_index >= 0 && p_index < getParameterCount(),
	             "MenuElementAction parameter index %d out of range [0 - %d)",
	             p_index, getParameterCount());
	return m_parameters.at(static_cast<Parameters::size_type>(p_index));
}

// Namespace end
}
}
