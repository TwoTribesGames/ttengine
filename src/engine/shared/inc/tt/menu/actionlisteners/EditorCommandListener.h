#if !defined(INC_TT_MENU_ACTIONLISTENERS_EDITORCOMMANDLISTENER_H)
#define INC_TT_MENU_ACTIONLISTENERS_EDITORCOMMANDLISTENER_H


#include <tt/menu/MenuActionListener.h>


namespace tt {
namespace menu {
namespace actionlisteners {

/*! \brief Handles default menu commands. */
class EditorCommandListener : public MenuActionListener
{
public:
	virtual ~EditorCommandListener();
	
	/*! \brief Creates an action listener and registers it with the menu system. */
	static void createListener();
	
	virtual bool doAction(const MenuAction& p_action);
	
	virtual std::string getName() const { return "default_editor_commands"; }
	
protected:
	EditorCommandListener();
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ACTIONLISTENERS_EDITORCOMMANDLISTENER_H)
