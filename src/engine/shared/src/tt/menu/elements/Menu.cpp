#include <tt/code/ErrorStatus.h>
#include <tt/menu/elements/Menu.h>
#include <tt/menu/elements/SelectionCursor.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/parse.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

Menu::Menu(const std::string& p_name,
           const MenuLayout&  p_layout)
:
Container(p_name, p_layout),
m_rect(math::Point2(0, 0), 256, 192), // FIXME: Remove hard-coded dimensions!
m_canGoBack(true),
m_renderBackground(true)
{
	MENU_CREATION_Printf("Menu::Menu: Element '%s': New Menu.\n",
	                     getName().c_str());
}


Menu::~Menu()
{
	MENU_CREATION_Printf("Menu::~Menu: Element '%s': Destructing menu.\n",
	                     getName().c_str());
}


void Menu::doTopLevelLayout()
{
	Container::doLayout(m_rect);
	Container::loadResources();
	
	/*
	TT_Printf("Menu::doTopLevelLayout: Selection tree after initial selection:\n");
	dumpSelectionTree(0);
	//*/
}


bool Menu::doAction(const MenuElementAction& p_action)
{
	// Allow base to handle action first
	if (Container::doAction(p_action))
	{
		return true;
	}
	
	std::string command(p_action.getCommand());
	if (command == "set_can_go_back")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "whether to enable going back for the menu.",
		             command.c_str());
		
		TT_ERR_CREATE(command);
		bool newCanGoBack = str::parseBool(p_action.getParameter(0), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid 'can_go_back' Boolean specified: '%s'",
			         p_action.getParameter(0).c_str());
			newCanGoBack = canGoBack();
		}
		
		setCanGoBack(newCanGoBack);
		
		return true;
	}
	
	return false;
}


bool Menu::onKeyPressed(const MenuKeyboard& p_keys)
{
	// Allow the children to process the input first
	if (Container::onKeyPressed(p_keys))
	{
		return true;
	}
	
	
	bool handled = false;
	
	// Send input to all elements that are bound to this key input
	for (KeyElementBinding::iterator it = m_keyElementBinding.find(p_keys.getKeyMask());
	     it != m_keyElementBinding.end() && (*it).first == p_keys.getKeyMask();
	     ++it)
	{
		TT_NULL_ASSERT((*it).second);
		if ((*it).second->onKeyPressed(p_keys))
		{
			handled = true;
		}
	}
	
	// Menu itself doesn't handle key input
	return handled;
}


bool Menu::onKeyHold(const MenuKeyboard& p_keys)
{
	// Allow the children to process the input first
	if (Container::onKeyHold(p_keys))
	{
		return true;
	}
	
	
	bool handled = false;
	
	// Send input to all elements that are bound to this key input
	for (KeyElementBinding::iterator it = m_keyElementBinding.find(p_keys.getKeyMask());
	     it != m_keyElementBinding.end() && (*it).first == p_keys.getKeyMask();
	     ++it)
	{
		TT_NULL_ASSERT((*it).second);
		if ((*it).second->onKeyHold(p_keys))
		{
			handled = true;
		}
	}
	
	// Menu itself doesn't handle key input
	return handled;
}


bool Menu::onKeyReleased(const MenuKeyboard& p_keys)
{
	// Allow the children to process the input first
	if (Container::onKeyReleased(p_keys))
	{
		return true;
	}
	
	
	bool handled = false;
	
	// Send input to all elements that are bound to this key input
	for (KeyElementBinding::iterator it = m_keyElementBinding.find(p_keys.getKeyMask());
	     it != m_keyElementBinding.end() && (*it).first == p_keys.getKeyMask();
	     ++it)
	{
		TT_NULL_ASSERT((*it).second);
		if ((*it).second->onKeyReleased(p_keys))
		{
			handled = true;
		}
	}
	
	// Menu itself doesn't handle key input
	return handled;
}


bool Menu::onKeyRepeat(const MenuKeyboard& p_keys)
{
	// Allow the children to process the input first
	if (Container::onKeyRepeat(p_keys))
	{
		return true;
	}
	
	
	bool handled = false;
	
	// Send input to all elements that are bound to this key input
	for (KeyElementBinding::iterator it = m_keyElementBinding.find(p_keys.getKeyMask());
	     it != m_keyElementBinding.end() && (*it).first == p_keys.getKeyMask();
	     ++it)
	{
		TT_NULL_ASSERT((*it).second);
		if ((*it).second->onKeyRepeat(p_keys))
		{
			handled = true;
		}
	}
	
	// Menu itself doesn't handle key input
	return handled;
}


void Menu::addVar(const std::string& p_varName, const std::string& p_varValue)
{
	MENU_Printf("Menu::addVar: Name: '%s', value: '%s'\n",
	            p_varName.c_str(), p_varValue.c_str());
	m_vars.insert(Variables::value_type(p_varName, p_varValue));
}


std::string Menu::getVar(const std::string& p_varName)
{
	MENU_Printf("Menu::getVar: Name: '%s'\n", p_varName.c_str());
	
	Variables::iterator it = m_vars.find(p_varName);
	if (it != m_vars.end())
	{
		return (*it).second;
	}
	
	TT_PANIC("Menu var '%s' not found, can't get var value.",
	         p_varName.c_str());
	return "";
}


void Menu::setVar(const std::string& p_varName, const std::string& p_varValue)
{
	MENU_Printf("Menu::setVar: Name: '%s', value: '%s'.\n",
	            p_varName.c_str(), p_varValue.c_str());
	
	Variables::iterator it = m_vars.find(p_varName);
	
	TT_ASSERTMSG(it != m_vars.end(), 
	             "Menu var '%s' not found, can't set var to '%s'.",
	             p_varName.c_str(), p_varValue.c_str());
	
	(*it).second = p_varValue;
}


void Menu::setQueuedActions(const MenuActions& p_actions)
{
	m_queuedActions = p_actions;
}


MenuActions Menu::getQueuedActions()
{
	MenuActions qa(m_queuedActions);
	m_queuedActions.clear();
	return qa;
}


bool Menu::hasQueuedActions() const
{
	return m_queuedActions.empty() == false;
}


void Menu::bindKeyToMenuElement(MenuKeyboard::MenuKey p_key,
                                MenuElementInterface* p_element)
{
	m_keyElementBinding.insert(KeyElementBinding::value_type(p_key, p_element));
}


void Menu::addActionSet(const std::string& p_name)
{
	TT_ASSERTMSG(p_name.empty() == false,
	             "Action sets must have a name.");
	
	ActionSets::iterator it = m_actionSets.find(p_name);
	TT_ASSERTMSG(it == m_actionSets.end(),
	             "Menu '%s' already has an action set named '%s'.",
	             getName().c_str(), p_name.c_str());
	if (it == m_actionSets.end())
	{
		// Add an empty action set
		m_actionSets.insert(ActionSets::value_type(p_name, Actions()));
	}
}


void Menu::addActionToSet(const std::string& p_setName,
                          const MenuAction&  p_action)
{
	TT_ASSERTMSG(p_setName.empty() == false,
	             "Action sets must have a name.");
	
	ActionSets::iterator it = m_actionSets.find(p_setName);
	TT_ASSERTMSG(it != m_actionSets.end(),
	             "Menu '%s' does not have an action set named '%s'.",
	             getName().c_str(), p_setName.c_str());
	
	if (it != m_actionSets.end())
	{
		(*it).second.push_back(p_action);
	}
}


bool Menu::hasActionSet(const std::string& p_name) const
{
	TT_ASSERTMSG(p_name.empty() == false,
	             "Action sets must have a name.");
	return (m_actionSets.find(p_name) != m_actionSets.end());
}


void Menu::executeActionSet(const std::string& p_name)
{
	TT_ASSERTMSG(p_name.empty() == false,
	             "Action sets must have a name.");
	
	ActionSets::iterator it = m_actionSets.find(p_name);
	TT_ASSERTMSG(it != m_actionSets.end(),
	             "Menu '%s' does not have action set '%s'.",
	             getName().c_str(), p_name.c_str());
	
	if (it != m_actionSets.end())
	{
		MenuSystem::getInstance()->doActions((*it).second);
	}
}


Menu* Menu::clone() const
{
	TT_PANIC("Menus cannot be cloned.");
	return 0;
}


void Menu::setRenderBackground(bool p_renderBackground)
{
	m_renderBackground = p_renderBackground;
}


bool Menu::getRenderBackground() const
{
	return m_renderBackground;
}

// Namespace end
}
}
}
