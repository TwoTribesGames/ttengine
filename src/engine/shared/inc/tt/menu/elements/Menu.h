#if !defined(INC_TT_MENU_ELEMENTS_MENU_H)
#define INC_TT_MENU_ELEMENTS_MENU_H


#include <vector>
#include <map>

#include <tt/menu/elements/Container.h>
#include <tt/menu/elements/element_traits.h>


namespace tt {
namespace menu {
namespace elements {

class SelectionCursor;


/*! \brief Menu. */
class Menu : public Container
{
public:
	typedef menu_element_tag element_category;
	
	
	Menu(const std::string& p_name,
	     const MenuLayout&  p_layout);
	virtual ~Menu();
	
	void doTopLevelLayout();
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyHold(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	virtual bool onKeyRepeat(const MenuKeyboard& p_keys);
	
	inline void setCanGoBack(bool p_canGoBack) { m_canGoBack = p_canGoBack; }
	
	/*! \brief Indicates whether this menu allows going back using the back button. */
	inline bool canGoBack() const { return m_canGoBack; }
	
	void        addVar(const std::string& p_varName,
	                   const std::string& p_varValue);
	std::string getVar(const std::string& p_varName);
	void        setVar(const std::string& p_varName,
	                   const std::string& p_varValue);
	
	/*! \brief Sets the queued actions for this menu. */
	void setQueuedActions(const MenuActions& p_actions);
	
	/*! \brief Returns the queued actions for this menu and clears its queue. */
	MenuActions getQueuedActions();
	
	/*! \brief Indicates whether this menu has any queued actions. */
	bool hasQueuedActions() const;
	
	/*! \brief Binds a key to a menu element
	    \param p_key The trigger key
	    \param p_element The element which has to receive the key input */
	void bindKeyToMenuElement(MenuKeyboard::MenuKey p_key,
	                          MenuElementInterface* p_element);
	
	/*! \brief Adds an action set with the specified name. */
	void addActionSet(const std::string& p_name);
	
	/*! \brief Adds an action to the specified set. Set must already exist. */
	void addActionToSet(const std::string& p_setName,
	                    const MenuAction&  p_action);
	
	/*! \brief Indicates whether the menu has the specified action set. */
	bool hasActionSet(const std::string& p_name) const;
	
	/*! \brief Executes the specified action set. */
	void executeActionSet(const std::string& p_name);
	
	virtual Menu* clone() const;
	
	void setRenderBackground(bool p_renderBackground);
	
	bool getRenderBackground() const;
	
private:
	typedef std::map<std::string, std::string>                          Variables;
	typedef std::multimap<MenuKeyboard::MenuKey, MenuElementInterface*> KeyElementBinding;
	typedef std::map<std::string, Actions>                              ActionSets;
	
	
	math::PointRect   m_rect;
	bool              m_canGoBack;
	bool              m_renderBackground;
	Variables         m_vars;              //!< Menu variables by name
	KeyElementBinding m_keyElementBinding; //!< Input key * -> 1 menu element mapping.
	MenuActions       m_queuedActions;
	ActionSets        m_actionSets;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_MENU_H)
