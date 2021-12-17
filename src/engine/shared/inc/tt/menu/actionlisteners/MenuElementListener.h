#if !defined(INC_TT_MENU_ACTIONLISTENERS_MENUELEMENTLISTENER_H)
#define INC_TT_MENU_ACTIONLISTENERS_MENUELEMENTLISTENER_H


#include <tt/menu/MenuActionListener.h>


namespace tt {
namespace menu {
namespace actionlisteners {

/*! \brief Handles menu element commands. */
class MenuElementListener : public MenuActionListener
{
public:
	virtual ~MenuElementListener();
	
	/*! \brief Creates an action listener and registers it with the menu system. */
	static void createListener();
	
	virtual bool doAction(const MenuAction& p_action);
	
	virtual std::string getName() const { return "menu_element_commands"; }
	
protected:
	MenuElementListener();
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ACTIONLISTENERS_MENUELEMENTLISTENER_H)
