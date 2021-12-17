#include <tt/platform/tt_error.h>

#include <tt/menu/actionlisteners/EditorCommandListener.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {
namespace actionlisteners {

//------------------------------------------------------------------------------
// Public member functions

EditorCommandListener::~EditorCommandListener()
{
}


void EditorCommandListener::createListener()
{
	// Create a listener for each action supported by this listener
	MenuSystem* sys = MenuSystem::getInstance();
	sys->registerDefaultActionListener(new EditorCommandListener);
}


bool EditorCommandListener::doAction(const MenuAction& p_action)
{
	std::string command(p_action.getCommand());
	
	MENU_Printf("EditorCommandListener::doAction: Received command '%s'.\n",
	            command.c_str());
	
	if (command == "set_editor_color")
	{
		// set the color of the available image editor
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'set_editor_color' requires one parameter "
		             "(the color index).");
		
		MENU_Printf("EditorCommandListener::doAction: Setting color '%s'.\n",
		            p_action.getParameter(0).c_str());
		//MenuSystem::getInstance()->gotoMenu(p_action.getParameter(0));
		// todo: handle action
		
		return true;
	}
	
	// Listener cannot handle this command
	return false;
}


//------------------------------------------------------------------------------
// Protected member functions

EditorCommandListener::EditorCommandListener()
{
}

// Namespace end
}
}
}
