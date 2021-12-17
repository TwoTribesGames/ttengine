#include <tt/platform/tt_error.h>

#include <tt/menu/actionlisteners/MenuElementListener.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {
namespace actionlisteners {

//------------------------------------------------------------------------------
// Public member functions

MenuElementListener::~MenuElementListener()
{
}


void MenuElementListener::createListener()
{
	// Create a listener for each action supported by this listener
	MenuSystem* sys = MenuSystem::getInstance();
	sys->registerDefaultActionListener(new MenuElementListener);
}


bool MenuElementListener::doAction(const MenuAction& p_action)
{
	std::string command(p_action.getCommand());
	
	MENU_Printf("MenuElementListener::doAction: Received command '%s'.\n",
	            command.c_str());
	
	if (command == "menu_element_action")
	{
		MenuElementAction meAction(p_action);
		
		// Pass the action to menusystem to get it to the target element.
		MENU_Printf("MenuElementListener::doAction: Command '%s' for '%s'.\n",
		            meAction.getCommand().c_str(),
		            meAction.getTargetElement().c_str());
		MenuSystem::getInstance()->doMenuElementAction(meAction);
		
		return true;
	}
	
	// Listener cannot handle this command
	return false;
}


//------------------------------------------------------------------------------
// Protected member functions

MenuElementListener::MenuElementListener()
{
}

// Namespace end
}
}
}
