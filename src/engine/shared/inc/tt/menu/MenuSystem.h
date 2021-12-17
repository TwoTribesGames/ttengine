#if !defined(INC_TT_MENU_MENUSYSTEM_H)
#define INC_TT_MENU_MENUSYSTEM_H


#include <string>
#include <vector>
#include <map>
#include <memory>

#include <tt/platform/tt_types.h>
#include <tt/math/Rect.h>
#include <tt/math/Point2.h>
//#include <tt/memory/SafeAllocator.h>
//#include <tt/memory/SafeString.h>
#include <tt/xml/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/menu/MenuSound.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/MenuSkin.h>


// If this is defined, a debug cursor is rendered at the stylus position
//#define MENU_DEBUG_CURSOR

// If this is defined, the selection cursor is disabled
//#define MENU_NO_SELECTION_CURSOR


namespace tt {

namespace engine {
namespace glyph {
	class GlyphSet;
}
}
namespace loc {
	class LocStr;
}

namespace menu {

namespace elements
{
	class Menu;
	class MenuElement;
	class MenuElementInterface;
	class SelectionCursor;
}
class MenuElementAction;
class MenuActionListener;
class MenuTreeNode;
class MenuFactory;
class MenuSoundPlayer;
class MenuTransition;


/*! \brief The menu system: manages all menus and menu resources. */
class MenuSystem
{
public:
	/*! \brief Returns the only instance of the menu system. */
	static MenuSystem* getInstance();
	
	/*! \brief Creates the only instance of the menu system. */
	static void createInstance(const std::string& p_menuTreeFilename);
	
	/*! \brief Destroys the only instance of the menu system. */
	static void releaseInstance();
	
	/*! \brief Goes to the menu with the specified name.
	    \param p_name The menu to switch to. */
	void gotoMenu(const std::string& p_name);
	
	/*! \brief Goes back to the parent of the current tree menu */
	void goBackMenu();
	
	/*! \brief Opens a new pop-up menu. */
	void openPopupMenu(const std::string& p_menu);
	
	/*! \brief Closes the top-most pop-up menu. */
	void closePopupMenu();
	
	/*! \brief reset the tree menu and set the root as active menu. */
	void resetTreeMenu();
	
	/*! \brief Opens the currently active menu in the tree. */
	void openTreeMenu();
	
	/*! \brief Closes the tree-menu immediately. */
	void closeTreeMenu();
	
	/*! \brief Closes the tree-menu in the next frame. */
	void closeTreeMenuDelayed();
	
	/*! \brief Indicates whether the tree-menu is open. */
	bool isTreeMenuOpen() const;
	
	/*! \brief Indicates whether currently transitioning
	           from one tree-menu to another. */
	inline bool isInMenuTransition() const { return m_transition != 0; }
	
	/*! \brief Cancels a currently active transition. */
	void cancelMenuTransition();
	
	/*! \brief Sets the menu to open when the tree-menu is opened.
	    \param p_menuName The name of the menu to set as start-up menu.
	    \param p_parentMenu Optional name of the parent menu
	                         (to facilitate look-ups of names that
	                          appear in the tree more than once). */
	void setStartupTreeMenu(const std::string& p_menuName,
	                        const std::string& p_parentMenu = "");
	
	/*! \brief Jumps directly to a tree menu, without traversing the tree. */
	void jumpToMenu(const std::string& p_menuName);
	
	/*! \brief Clears the selection path of the current tree-menu;
	           only has an effect if the tree-menu isn't open yet. */
	void clearSelectionPath();
	
	/*! \brief Clears the selection path of the current
	           tree-menu after it has been destroyed. */
	void clearSelectionPathAfterDestroy();
	
	/*! \brief Returns the name of the active menu. Empty string if no menu active. */
	std::string getActiveMenuName() const;
	
	/*! \brief Indicates whether a menu with the specified name is open.
	           Can be a pop-up menu or a tree-menu. */
	bool isMenuOpen(const std::string& p_menu) const;
	
	/*! \brief Indicates whether a pop-up menu is open. */
	bool isPopupMenuOpen() const;
	
	/*! \brief Registers a default action listener.
	           Ownership is transferred to the menu system. */
	void registerDefaultActionListener(MenuActionListener* p_listener);
	
	/*! \brief Registers a non-library action listener.
	           Ownership is transferred to the menu system. */
	void registerCustomActionListener(MenuActionListener* p_listener);
	
	/*! \brief Registers a default action listener for blocking actions.
	           Ownership is transferred to the menu system. */
	void registerDefaultBlockingActionListener(MenuActionListener* p_listener);
	
	/*! \brief Registers a non-library action listener for blocking actions.
	           Ownership is transferred to the menu system. */
	void registerCustomBlockingActionListener(MenuActionListener* p_listener);
	
	/*! \brief Unregisters a non-library action listener.
	           Ownership is transferred to calling code. */
	void unregisterCustomActionListener(MenuActionListener* p_listener);
	
	/*! \brief Unregisters a non-library action listener.
	           Ownership is transferred to calling code. */
	MenuActionListener* unregisterCustomActionListener(const std::string& p_name);
	
	/*! \brief Unregisters a non-library blocking action listener.
	           Ownership is transferred to calling code. */
	void unregisterCustomBlockingActionListener(MenuActionListener* p_listener);
	
	/*! \brief Unregisters a non-library blocking action listener.
	           Ownership is transferred to calling code. */
	MenuActionListener* unregisterCustomBlockingActionListener(const std::string& p_name);
	
	/*! \brief Allows the menu action listeners to perform the action.
	           Asserts if none of the action listeners handled the action.
	    \param p_action The action to perform. */
	void doActions(const MenuActions& p_actions);
	
	/*! \brief Allows client code to perform actions on menu elements.
	           Asserts if not an menu_element_action
	    \param p_action The action to perform. */
	void doMenuElementAction(const MenuElementAction& p_action);
	
	/*! \brief Executes the specified action set for the currently active menu.
	    \param p_setName The name of the action set to execute. */
	void executeActionSet(const std::string& p_setName);
	
	/*! \brief Attempts to set the focus of the active menu to the specified element. */
	void setFocus(const std::string& p_elementName);
	
	/*! \brief Allows the menu system to perform logic and handle input. */
	void update();
	
	/*! \brief Renders the currently active menu. */
	void render();
	
	/*! \brief Sets the offset to use when rendering. */
	void setRenderOffset(const math::Point2& p_offset);
	
	/*! \brief Enables or disables the shading behind pop-up menus. */
	inline void setShowPopupShade(bool p_showShade) { m_showPopupShade = p_showShade; }
	
	/*! \brief Indicates whether the shading behind pop-up menus is displayed. */
	inline bool getShowPopupShade() const { return m_showPopupShade; }
	
	/*! \brief Returns the menu system glyph set. */
	engine::glyph::GlyphSet* getGlyphSet();
	
	/*! \brief Translate string from menu.
	    \return Translated wstring */
	std::wstring translateString(const std::string& p_string);
	
	/*! \brief Needs to be called when entering a state. */
	void enterState(engine::glyph::GlyphSet* p_glyphSet,
	                const std::string&       p_language);
	
	/*! \brief Needs to be called when exiting a state. */
	void exitState();
	
	/*! \brief Sets a custom menu factory. The current menu factory will be destroyed.
	           Ownership is transferred to MenuSystem. */
	void setMenuFactory(MenuFactory* p_factory);
	
	/*! \brief Sets the sound player to use for menu sounds.
	           The current sound player will be destroyed.
	           Ownership is transferred to MenuSystem. */
	void setSoundPlayer(MenuSoundPlayer* p_player);
	
	/*! \brief Plays the specified menu sound. */
	void playSound(MenuSound p_sound, bool p_looping = false);
	
	/*! \brief Stops all looping sounds. */
	void stopLoopingSounds();
	
	/*! \brief Plays the menu music. */
	void playMenuMusic();
	
	/*! \brief Stops the menu music. */
	void stopMenuMusic();
	
	/*! \brief Set menu variables. Used by MenuElements.
	    \param p_name var name
	    \param p_value var value */
	void setMenuVar(const std::string& p_name, const std::string& p_value);
	
	/*! \brief Set system variables.
	    \param p_name var name
	    \param p_value var value */
	void setSystemVar(const std::string& p_name, const std::string& p_value);
	
	/*! \brief Retrieves the value of a system variable.
	    \param p_name The name of the system variable to retrieve.
	    \return The value of the system variable. */
	std::string getSystemVar(const std::string& p_name) const;
	
	/*! \brief Removes a system variable.
	    \param p_name The name of the system variable to remove. */
	void removeSystemVar(const std::string& p_name);
	
	/*! \brief Indicates whether a system variable with the specified name exists.
	    \param p_name The name of the system variable to query. */
	bool hasSystemVar(const std::string& p_name) const;
	
	/*! \brief Outputs all current system variables and values to the debugger. */
	void dumpSystemVars() const;
	
	
	/*! \brief Returns the menu skinning information. */
	const MenuSkin* getSkin() const;
	
	/*! \brief Sets the menu skin. Ownership is transferred to MenuSystem. */
	void setSkin(MenuSkin* p_skin);
	
	/*! \brief Sets the selected element to 0,
	           causes the selected element to be recalculated. */
	void resetSelectedElement();
	
	/*! \brief Sets the delay before keys start repeating, in milliseconds. */
	void setKeyRepeatDelay(u32 p_delay);
	
	/*! \brief Sets the delay between key repeats, in milliseconds. */
	void setKeyRepeatFrequency(u32 p_frequency);
	
	// MenuSystem needs to be on the safe heap
	/*
	static void* operator new(std::size_t p_blockSize);
	static void operator delete(void* p_block);
	//*/
	
private:
	//---------------------------------
	// Safe heap containers
	
	// Container that maps action listeners by name
	/*
	typedef std::map<memory::safestring, MenuActionListener*,
	                 std::less<memory::safestring>,
	                 memory::SafeAllocator<std::pair<const memory::safestring,
	                                             MenuActionListener*> > > ActionListeners;
	//*/
	typedef std::map<std::string, MenuActionListener*> ActionListeners;
	
	
	//---------------------------------
	// Non-safe heap containers
	
	typedef std::vector<elements::Menu*>       PopupMenus;
	typedef std::vector<elements::Menu*>       MenuDeathRow;
	typedef std::map<std::string, std::string> Variables;
	
	
	// Prevent direct instantiation
	MenuSystem(const std::string& p_menuTreeFilename);
	~MenuSystem();
	
	/*! \brief Creates the menu tree from the specified filename. */
	void buildMenuTree(const std::string& p_filename);
	
	/*! \brief Used by buildMenuTree to recursively parse the tree XML nodes. */
	void parseMenuTreeXML(MenuTreeNode* p_treeNode,
	                      xml::XmlNode* p_xmlNode);
	
	/*! \brief Outputs the menu tree, starting at the specified node. */
	void dumpMenuTree(MenuTreeNode* p_node, const std::string& p_indent = "");
	
	/*! \brief Resets the input state for the menu system. */
	void resetInput();
	
	/*! \brief Creates a menu with the specified name based on an XML definition. */
	elements::Menu* getMenu(const std::string& p_menuName);
	
	/*! \brief Returns a pointer to the currently active menu,
	           or a null pointer if no menu is active. */
	elements::Menu* getActiveMenu();
	
	/*! \brief Returns a constant pointer to the currently active menu,
	           or a null pointer if no menu is active. */
	const elements::Menu* getActiveMenu() const;
	
	/*! \brief Handle the input for the active menu. */
	void doInput();
	
	/*! \brief Check a string for % and replace with menu variable if needed.
	    \param p_string string to be parsed/check
	    \return parsed string with menu variables between '%' changed. */
	std::string parseMenuVar(const std::string& p_string);
	
	/*! \brief Create the back button to be displayed in the lower left corner
	           of the screen when in the tree menu. */
	void createBackButton();
	
	/*! \brief Creates the selection cursor, if one should be created. */
	void createSelectionCursor();
	
	/*! \brief Destroys the selection cursor, if required. */
	void destroySelectionCursor();
	
	/*! \brief Updates the selection cursor position. */
	void updateSelectionCursor();
	
	/*! \brief Allows the menu action listeners to perform the action.
	           Asserts if none of the action listeners handled the action.
	    \param p_action The action to perform.
	    \return True if the action is blocking, false if it is non-blocking. */
	bool doAction(const MenuAction& p_action);
	
	/*! \brief Performs a system action. Needs no active menu to function. */
	void doSystemAction(const MenuAction& p_action);
	
	bool sendToDefaultListeners(const MenuAction& p_action);
	bool sendToCustomListeners(const MenuAction& p_action);
	bool sendToDefaultBlockingListeners(const MenuAction& p_action);
	bool sendToCustomBlockingListeners(const MenuAction& p_action);
	
	void sendTreeMenuPreOpenAction();
	void sendMenuPreCreateAction(const std::string& p_menuName);
	void sendMenuPostCreateAction(elements::Menu* p_menu);
	void sendMenuPreDestroyAction(elements::Menu* p_menu);
	
	void destroyMenu(elements::Menu* p_menu);
	
	// Disallow copying (functions not implemented)
	MenuSystem(const MenuSystem&);
	const MenuSystem& operator=(const MenuSystem&);
	
	
	//---------------------------------
	// Safe heap member variables
	
	static MenuSystem* ms_singleton; //!< Singleton instance.
	
	MenuTreeNode*    m_menuTreeRoot;    //!< The root node of the menu name tree.
	
	ActionListeners  m_defaultListeners;
	ActionListeners  m_customListeners;
	ActionListeners  m_defaultBlockingListeners;
	ActionListeners  m_customBlockingListeners;
	
	MenuSkin*        m_menuSkin;         //!< Menu skinning information (no default provided).
	MenuFactory*     m_menuFactory;      //!< The menu factory used to create menus from XML.
	MenuSoundPlayer* m_soundPlayer;      //!< Plays menu sounds. No default implementation is provided.
	math::PointRect  m_menuRect;
	bool             m_closeTreeMenu; //!< Whether the tree-menu needs to be closed this frame.
	
	
	//---------------------------------
	// Non-safe heap member variables
	
	MenuTreeNode*    m_currentMenu;   //!< The currently active tree menu.
	PopupMenus*      m_popupMenus;    //!< Stack of pop-up menus.
	MenuDeathRow*    m_menuDeathRow; //!< Menus that need to be killed next update.
	engine::glyph::GlyphSet* m_glyphSet;
	MenuTransition*  m_transition;               //!< Menu transition animation for when switching tree menus.
	math::PointRect  m_backButtonRect;           //!< The rectangle for the position and size of the back button.
	bool             m_backButtonHandled;        //!< Whether or not the back button was active last frame.
	engine::renderer::QuadSpritePtr m_backButton;               //!< The quad with the back button.
	engine::renderer::TexturePtr    m_backButtonTexture;        //!< The texture used by the back button.
	engine::renderer::QuadSpritePtr m_backButtonOverlay;        //!< The quad with the back button overlay.
	engine::renderer::TexturePtr    m_backButtonOverlayTexture; //!< The texture used by the back button overlay.
	Variables        m_systemVars;               //!< A name=>value map of system variables
	
	math::Point2 m_renderOffset;
	
	engine::renderer::QuadSpritePtr m_popupShade;
	bool                            m_showPopupShade;
	
#ifndef MENU_NO_SELECTION_CURSOR
	elements::SelectionCursor*      m_selectionCursor;  //!< Menu item selection cursor.
	elements::MenuElementInterface* m_selectedElement;  //!< Currently selected menu element (only used for detecting selection changes)
	math::PointRect                 m_noSelectionRect; //!< PointRect to use when hands should move outside of the screen (for transitions and no selection).
#endif
	
	
#if defined(MENU_DEBUG_CURSOR)
	engine::renderer::QuadSpritePtr m_cursorQuad;
#endif
	
	//CO3DFile*             m_menuDataFile;
	loc::LocStr* m_translation;
	
	
	// Input handling variables
	bool m_stylusMask;
	bool m_stylusPressedPreviousFrame;
	u16  m_stylusPreviousX;
	u16  m_stylusPreviousY;
	
	u32  m_stylusDelayTime;   //!< Time until stylus starts repeating.
	u32  m_stylusRepeatTime;  //!< Time between every repeat.
	u64  m_stylusPressedTime; //!< Time at which stylus was pressed.
	u64  m_stylusHandledTime; //!< Time at which last repeat was generated.
	
	u32  m_keysPreviousPressed;
	u32  m_keysBlockedUntilReleased;
	
	u32  m_keyDelayTime;   //!< Time until key starts repeating.
	u32  m_keyRepeatTime;  //!< Time between every repeat.
	u32  m_keyRepeatKey;   //!< Key to which starts repeat.
	u64  m_keyPressedTime; //!< Time at which key was pressed.
	u64  m_keyHandledTime; //!< Time at which last keyrepeat was generated.
	
	
	// MenuTreeNode needs access to getMenu for the creation of the menu
	friend class MenuTreeNode;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUSYSTEM_H)
