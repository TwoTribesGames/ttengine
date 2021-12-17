#if !defined(INC_TT_MENU_ACTIONLISTENERS_MENUCOMMANDLISTENER_H)
#define INC_TT_MENU_ACTIONLISTENERS_MENUCOMMANDLISTENER_H


#include <tt/menu/MenuActionListener.h>


namespace tt {
namespace menu {
namespace actionlisteners {

/*! \brief Handles default menu commands.

Commands supported by this listener:

- goto_menu (moves to the specified tree-menu; needs to be a direct child of the current tree-menu)
- jump_to_menu (jumps directly to the specified tree-menu, without traversing the tree)
- open_menu (opens the specified pop-up menu)
- close_menu (closes the current pop-up menu; takes no parameters)
- go_back_menu (goes back to the previously active menu (pop-up or tree-menu); takes no parameters)
- close_all_popups (closes all open pop-up menus; takes no parameters)
- set_system_var (sets a system variable; takes the name of the variable and the value to set)
- remove_system_var (removes a system variable; takes the name of the variable to remove)
- open_softwarekeyboard (opens the software keyboard; takes the string to display as caption and the name of the system variable to store the result in and the maximum string length)
- open_treemenu (opens the tree-menu; takes no parameters)
- close_treemenu (closes the tree-menu; takes no parameters)
- set_startup_menu (sets the menu to open when the tree-menu is opened; takes the name of the menu, and optionally the name of the parent menu)
- execute_action_set (executes the actions in the specified action set; takes the name of the set)
- string_to_hex (converts an ASCII string to hex; takes the string to convert and the name of the menu variable to store the result in)
- delete_file_from_variable
- play_menu_sound (plays a menu sound effect; takes the name of the sound effect and an optional Boolean specifying whether the sound should loop)
- stop_menu_looping_sounds (stops all looping sound effects; takes no parameters)
- add_integer (adds two integers together and stores the result in a system variable)
- translate_string (translates a string and stores the result in a system variable)
- append_string (appends one string to another and stores the result in a system variable)
- clear_treemenu_selection_path (clears the tree-menu selection path -- only works if the tree-menu isn't open yet; takes no parameters)
- clear_treemenu_selection_path_after_destroy (clears the tree-menu selection path after the menu is destroyed; takes no parameters)
- set_focus (sets the focus to the specified menu element; takes the name of the menu element)

- memprofile_set_reference_point (sets the reference point for memory profiling; takes no parameters)
- memprofile_verify_memory (verifies the memory since the reference point; takes one parameter: whether to break if errors are detected)
- debug_output (prints a message to the debugger; takes variable number of parameters: the strings to output)
- debug_hex_output (prints a message to the debugger; takes variable number of parameters: the hex-encoded strings to output)

- if:
  Tests a given variable to a given value and executes one of two given action sets
  Required:
  - variable to test
  - value to test against
  - name of the action set to execute when the variable matches the value
  - name of the action set to execute when the variable does not exist or does not match the value

- if_in_range
  Checks if an integer value is in the specified range and executes one of two given action sets.
  Parameters:
  - integer to check
  - lower bound of range
  - upper bound of range
  - name of the action set to execute when the integer is in range
  - name of the action set to execute when the integer is not in range
*/
class MenuCommandListener : public MenuActionListener
{
public:
	virtual ~MenuCommandListener();
	
	/*! \brief Creates an action listener and registers it with the menu system. */
	static void createListener();
	
	virtual bool doAction(const MenuAction& p_action);
	
	virtual std::string getName() const;
	
protected:
	MenuCommandListener();
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ACTIONLISTENERS_MENUCOMMANDLISTENER_H)
