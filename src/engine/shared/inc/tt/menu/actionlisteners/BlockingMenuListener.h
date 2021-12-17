#if !defined(INC_TT_MENU_ACTIONLISTENERS_BLOCKINGMENULISTENER_H)
#define INC_TT_MENU_ACTIONLISTENERS_BLOCKINGMENULISTENER_H


#include <tt/menu/MenuActionListener.h>


namespace tt {
namespace menu {
namespace actionlisteners {

/*! \brief Handles menu commands that cause the action chain to block.

Commands supported by this listener:

- open_softwarekeyboard (opens the software keyboard; takes the string to display as caption and the name of the system variable to store the result in as well as the maximum length of the string)
- open_random_name (opens the random name generator; takes the string to display as caption and the name of the system variable to store the result in as well as the maximum length of the string)
- open_menu_and_wait (opens a pop-up menu and blocks further actions until it is closed; takes the name of the menu to open as parameter)

- confirm:
  Displays a confirmation dialog. Action takes the following parameters:
  Required:
  - display text (will be localized)
  - variable to replace in text (must be a hex string)
  - name of the action set to execute when the 'yes' button is pressed
  - name of the action set to execute when the 'no' button is pressed

  Optional:
  - default button ('yes' or 'no')
  - window title (will be localized)
  - caption for 'yes' button (will be localized)
  - caption for 'no' button (will be localized)
  - whether to render the underlying menus
 */
class BlockingMenuListener : public MenuActionListener
{
public:
	virtual ~BlockingMenuListener();
	
	/*! \brief Creates an action listener and registers it with the menu system. */
	static void createListener();
	
	virtual bool doAction(const MenuAction& p_action);
	
	virtual std::string getName() const;
	
protected:
	BlockingMenuListener();
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ACTIONLISTENERS_BLOCKINGMENULISTENER_H)
