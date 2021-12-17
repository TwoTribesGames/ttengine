#include <algorithm>

//#include <tt/system/System.h>
//#include <tt/system/Pad.h>
//#include <tt/memory/HeapMgr.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/fs/fs.h>
#include <tt/loc/LocStr.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>

#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuTreeNode.h>
#include <tt/menu/MenuActionListener.h>
#include <tt/menu/MenuFactory.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuTransition.h>
#include <tt/menu/MenuSoundPlayer.h>
#include <tt/menu/MenuUtils.h>
#include <tt/menu/MenuDebug.h>

#include <tt/menu/actionlisteners/MenuCommandListener.h>
#include <tt/menu/actionlisteners/EditorCommandListener.h>
#include <tt/menu/actionlisteners/MenuElementListener.h>
#include <tt/menu/actionlisteners/BlockingMenuListener.h>

#include <tt/menu/elements/Menu.h>
#include <tt/menu/elements/Button.h>
#include <tt/menu/elements/SelectionCursor.h>
#include <tt/menu/elements/SkinElementIDs.h>


namespace tt {
namespace menu {

//using memory::safestring;
//using memory::toSafeString;
//using memory::fromSafeString;

MenuSystem* MenuSystem::ms_singleton = 0;


// Define this to get profiling information about menu creation and destruction
//#define MENUSYS_MENU_MEMORY_PROFILING


//------------------------------------------------------------------------------
// Public member functions

MenuSystem* MenuSystem::getInstance()
{
	TT_ASSERTMSG(ms_singleton != 0, "Menu system instance does not exist.");
	
	// Return the singleton instance
	return ms_singleton;
}


void MenuSystem::createInstance(const std::string& p_menuTreeFilename)
{
	TT_ASSERTMSG(ms_singleton == 0, "Menu system instance already exists.");
	
	// Create the singleton instance
	ms_singleton = new MenuSystem(p_menuTreeFilename);
	
	// Create the default action listeners
	using namespace menu::actionlisteners;
	MenuCommandListener::createListener();
	EditorCommandListener::createListener();
	MenuElementListener::createListener();
	BlockingMenuListener::createListener();
}


void MenuSystem::releaseInstance()
{
	// Destroy the singleton instance
	delete ms_singleton;
	ms_singleton = 0;
}


void MenuSystem::gotoMenu(const std::string& p_name)
{
	TT_ASSERTMSG(m_transition == 0, "Can't go to menu '%s', because a menu "
	             "transition is still in progress.", p_name.c_str());
	TT_ASSERTMSG(m_currentMenu != 0, "There is no current menu, so cannot "
	             "move to a different menu.");
	TT_ASSERTMSG(m_currentMenu->isDirectChild(p_name),
	             "Cannot reach menu '%s' directly from current menu '%s'. "
	             "Check the menu tree XML.", p_name.c_str(),
	             m_currentMenu->getName().c_str());
	
	// Send old menu the deactivation event
	if (m_currentMenu->hasMenu())
	{
		m_currentMenu->getMenu()->onMenuDeactivated();
	}
	
	// Create a transition to the new menu
	m_transition = new MenuTransition(MenuTransition::Direction_Right,
	                                  m_currentMenu,
	                                  m_currentMenu->findNode(p_name));
	
	if (m_selectionCursor != 0)
	{
		m_selectionCursor->setSelectionRect(m_noSelectionRect);
	}
}


void MenuSystem::goBackMenu()
{
	TT_ASSERTMSG(m_transition == 0,
	             "Can't go back to parent menu, because a "
	             "menu transition is still in progress.");
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	
	if (m_popupMenus->empty() == false)
	{
		// Close the current pop-up menu
		closePopupMenu();
	}
	else
	{
		// Move up in the menu tree
		TT_ASSERTMSG(m_currentMenu != 0, "Cannot move up the menu tree "
		             "when there is no current menu.");
		
		if (m_currentMenu->getParent() != 0)
		{
			// Send old menu the deactivation event
			if (m_currentMenu->hasMenu())
			{
				m_currentMenu->getMenu()->onMenuDeactivated();
			}
			
			// Create a transition to the parent menu
			m_transition = new MenuTransition(MenuTransition::Direction_Left,
			                                  m_currentMenu,
			                                  m_currentMenu->getParent());
			
			if (m_selectionCursor != 0)
			{
				m_selectionCursor->setSelectionRect(m_noSelectionRect);
			}
		}
	}
}


void MenuSystem::openPopupMenu(const std::string& p_menu)
{
	// Create a pop-up menu
	elements::Menu* menu = getMenu(p_menu);
	
	// Push it onto the stack
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	m_popupMenus->push_back(menu);
	
	// Send it the menu activation event
	menu->onMenuActivated();
	
	if (m_selectionCursor != 0)
	{
		m_selectionCursor->forceSelectionRect(m_noSelectionRect);
	}
}


void MenuSystem::closePopupMenu()
{
	MENU_Printf("MenuSystem::closePopupMenu: Closing pop-up menu.\n");
	
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	TT_ASSERTMSG(m_popupMenus->empty() == false,
	             "Cannot close pop-up menu: no pop-up menus remain.");
	
	// Get the top-most pop-up menu
	elements::Menu* popup = m_popupMenus->back();
	
	MENU_Printf("MenuSystem::closePopupMenu: Closing pop-up menu '%s' (%p).\n",
	            popup->getName().c_str(), popup);
	
	// Pop it from the stack
	m_popupMenus->pop_back();
	
	// Send it the menu deactivation event
	popup->onMenuDeactivated();
	
	// Schedule the menu for deletion
	TT_ASSERTMSG(m_menuDeathRow != 0, "MenuSystem::enterState has not been "
	             "called (menu death-row vector is null).");
	m_menuDeathRow->push_back(popup);
	
	// Reset the input
	resetInput();
	
	// Send menu activation event to the new active menu
	elements::Menu* activeMenu = getActiveMenu();
	if (activeMenu != 0)
	{
		activeMenu->onMenuActivated();
		
		// Check if the active menu has any pending actions
		if (activeMenu->hasQueuedActions())
		{
			// Perform the pending menu actions
			doActions(activeMenu->getQueuedActions());
		}
	}
	
	if (m_selectionCursor != 0)
	{
		m_selectionCursor->forceSelectionRect(m_noSelectionRect);
	}
}


void MenuSystem::resetTreeMenu()
{
	TT_ASSERTMSG(m_menuTreeRoot != 0,
	             "Cannot reset the tree-menu if the root node is null "
	             "(has the menu tree been created at all?).");
	
	// Destroy the menu transition (if there is one)
	delete m_transition;
	m_transition = 0;
	
	// Destroy all menus in the tree
	m_menuTreeRoot->destroyMenuRecursive();
	
	// Create the first menu in the tree
	m_currentMenu = m_menuTreeRoot;
	m_currentMenu->createMenu();
	
	// Send it the menu activation event
	m_currentMenu->getMenu()->onMenuActivated();
}


void MenuSystem::openTreeMenu()
{
	TT_ASSERTMSG(m_menuTreeRoot != 0,
	             "Cannot open the tree-menu if the root node is null "
	             "(has the menu tree been created at all?).");
	
	// Send a notification that the tree-menu is about to be opened
	sendTreeMenuPreOpenAction();
	
	if (m_currentMenu == 0)
	{
		resetTreeMenu();
	}
	else
	{
		m_currentMenu->createMenu();
		
		// Send menu activation event
		m_currentMenu->getMenu()->onMenuActivated();
	}
}


void MenuSystem::closeTreeMenu()
{
	// Reset the "close tree menu" flag
	m_closeTreeMenu = false;
	
	// Notify the current tree-menu (if any) that it is being deactivated
	if (m_currentMenu != 0)
	{
		elements::Menu* tree_menu = m_currentMenu->getMenu();
		if (tree_menu != 0)
		{
			// Send menu deactivation event
			tree_menu->onMenuDeactivated();
		}
	}
	
	// Destroy the menu transition (if there is one)
	delete m_transition;
	m_transition = 0;
	
	// Destroy all open tree-menus
	m_menuTreeRoot->destroyMenuRecursive();
}


void MenuSystem::closeTreeMenuDelayed()
{
	TT_ASSERTMSG(m_transition == 0, "Cannot close tree menu while a menu "
	             "transition is still in progress.");
	
	m_closeTreeMenu = true;
}


bool MenuSystem::isTreeMenuOpen() const
{
	return m_transition != 0 ||
	       (m_closeTreeMenu == false &&
	        m_currentMenu   != 0     &&
	        m_currentMenu->hasMenu());
}


void MenuSystem::cancelMenuTransition()
{
	if (m_transition != 0)
	{
		// Handle cancellation gracefully
		if (m_transition->getTargetMenu()->hasMenu() == false)
		{
			// Transition hasn't reached target menu yet;
			// set previous menu as active again
			if (m_currentMenu->hasMenu() == false)
			{
				m_currentMenu->createMenu();
			}
			
			// Send menu activation event
			m_currentMenu->getMenu()->onMenuActivated();
		}
		else
		{
			// Transition has reached target menu;
			// kill previous menu and set target menu as current
			if (m_currentMenu->hasMenu())
			{
				m_currentMenu->getMenu()->onMenuDeactivated();
				m_currentMenu->destroyMenu();
			}
			
			m_currentMenu = m_transition->getTargetMenu();
		}
		
		// Kill the menu transition
		delete m_transition;
		m_transition = 0;
	}
}


void MenuSystem::setStartupTreeMenu(const std::string& p_menuName,
                                    const std::string& p_parentMenu)
{
	// Make sure the tree-menu is not open
	if (isTreeMenuOpen())
	{
		TT_PANIC("Cannot set startup menu when tree-menu is open.");
		return;
	}
	
	// Determine the parent menu first
	MenuTreeNode* parent = m_menuTreeRoot;
	if (p_parentMenu.empty() == false)
	{
		parent = m_menuTreeRoot->findNode(p_parentMenu);
		TT_ASSERTMSG(parent != 0, "Parent menu '%s' for menu '%s' "
		             "is not part of the menu tree.",
		             p_parentMenu.c_str(), p_menuName.c_str());
		if (parent == 0)
		{
			parent = m_menuTreeRoot;
		}
	}
	
	
	MenuTreeNode* node = parent->findNode(p_menuName);
	TT_ASSERTMSG(node != 0, "Menu '%s' could not be found "
	             "in the tree starting at node '%s'.",
	             p_menuName.c_str(), parent->getName().c_str());
	if (node != 0)
	{
		m_currentMenu = node;
	}
}


void MenuSystem::jumpToMenu(const std::string& p_menuName)
{
	TT_ASSERTMSG(m_transition == 0, "Can't jump to menu '%s', because a menu "
	             "transition is still in progress.", p_menuName.c_str());
	TT_ASSERTMSG(m_currentMenu != 0, "There is no current menu, "
	             "so cannot move to a different menu.");
	TT_ASSERTMSG(m_menuTreeRoot->findNode(p_menuName) != 0,
	             "Menu '%s' is not a part of the menu tree.",
	             p_menuName.c_str());
	
	// Send old menu the deactivation event
	if (m_currentMenu->hasMenu())
	{
		m_currentMenu->getMenu()->onMenuDeactivated();
	}
	
	// Create a transition to the new menu
	MenuTransition::Direction dir = MenuTransition::Direction_Left;
	if (m_currentMenu->findNode(p_menuName) != 0)
	{
		// Menu we're jumping to is a child of the current menu;
		// transition right
		dir = MenuTransition::Direction_Right;
	}
	
	m_transition = new MenuTransition(dir,
	                                  m_currentMenu,
	                                  m_menuTreeRoot->findNode(p_menuName));
	
	if (m_selectionCursor != 0)
	{
		m_selectionCursor->setSelectionRect(m_noSelectionRect);
	}
}


void MenuSystem::clearSelectionPath()
{
	MenuTreeNode* node = 0;
	if (m_currentMenu != 0)
	{
		node = m_currentMenu;
	}
	else
	{
		node = m_menuTreeRoot;
	}
	
	if (node == 0)
	{
		return;
	}
	
	
	TT_WARNING(node->getMenu() == 0, "Current tree-menu is already open; "
	           "clearing selection path has no effect.");
	node->clearSelectionPath();
}


void MenuSystem::clearSelectionPathAfterDestroy()
{
	MenuTreeNode* node = 0;
	if (m_currentMenu != 0)
	{
		node = m_currentMenu;
	}
	else
	{
		node = m_menuTreeRoot;
	}
	
	if (node == 0)
	{
		return;
	}
	
	
	node->clearSelectionPathAfterDestroy();
}


std::string MenuSystem::getActiveMenuName() const
{
	// If there are pop-up menus, return the name of the top-most menu
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	if (m_popupMenus->empty() == false)
	{
		return m_popupMenus->back()->getName();
	}
	
	// If there is an active tree-menu, return its name
	if (m_currentMenu != 0)
	{
		return m_currentMenu->getName();
	}
	
	// No menu is active -- return empty string
	return "";
}


bool MenuSystem::isMenuOpen(const std::string& p_menu) const
{
	// Check the currently active tree-menu
	if (m_currentMenu != 0 && m_currentMenu->getName() == p_menu)
	{
		// A tree-menu with the specified name is open
		return true;
	}
	
	// Check the pop-up menus
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	for (PopupMenus::const_iterator it = m_popupMenus->begin();
	     it != m_popupMenus->end(); ++it)
	{
		if ((*it)->getName() == p_menu)
		{
			// A pop-up menu with the specified name is open
			return true;
		}
	}
	
	// No menu with the specified name is open
	return false;
}


bool MenuSystem::isPopupMenuOpen() const
{
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	return m_popupMenus->empty() == false;
}


void MenuSystem::registerDefaultActionListener(MenuActionListener* p_listener)
{
	// Listener can never be null, nor can it already be registered
	TT_ASSERTMSG(p_listener != 0,
	             "Action listener pointers can never be null.");
	
	// Make sure the listener wasn't already registered
	//safestring name(toSafeString(p_listener->getName()));
	std::string name(p_listener->getName());
	TT_ASSERTMSG(m_defaultListeners.find(name) == m_defaultListeners.end(),
	             "A default action listener with name '%s' was already "
	             "registered.", p_listener->getName().c_str());
	
	// Register the listener for the specified command
	m_defaultListeners.insert(ActionListeners::value_type(name, p_listener));
}


void MenuSystem::registerCustomActionListener(MenuActionListener* p_listener)
{
	// Listener can never be null, nor can it already be registered
	TT_ASSERTMSG(p_listener != 0,
	             "Action listener pointers can never be null.");
	
	//safestring name(toSafeString(p_listener->getName()));
	std::string name(p_listener->getName());
	TT_ASSERTMSG(m_customListeners.find(name) == m_customListeners.end(),
	             "A custom action listener with name '%s' was already "
	             "registered.", p_listener->getName().c_str());
	
	// Register the listener by name
	m_customListeners.insert(ActionListeners::value_type(name,
	                                                     p_listener));
}


void MenuSystem::registerDefaultBlockingActionListener(
		MenuActionListener* p_listener)
{
	// Listener can never be null, nor can it already be registered
	TT_ASSERTMSG(p_listener != 0,
	             "Action listener pointers can never be null.");
	
	// Make sure the listener wasn't already registered
	//safestring name(toSafeString(p_listener->getName()));
	std::string name(p_listener->getName());
	TT_ASSERTMSG(m_defaultBlockingListeners.find(name) ==
	             m_defaultBlockingListeners.end(),
	             "A default blocking action listener with name '%s' was "
	             "already registered.", p_listener->getName().c_str());
	
	// Register the listener for the specified command
	m_defaultBlockingListeners.insert(ActionListeners::value_type(name,
	                                                              p_listener));
}


void MenuSystem::registerCustomBlockingActionListener(
		MenuActionListener* p_listener)
{
	// Listener can never be null, nor can it already be registered
	TT_ASSERTMSG(p_listener != 0,
	             "Action listener pointers can never be null.");
	
	//safestring name(toSafeString(p_listener->getName()));
	std::string name(p_listener->getName());
	TT_ASSERTMSG(m_customBlockingListeners.find(name) ==
	             m_customBlockingListeners.end(),
	             "A custom blocking action listener with name '%s' was already "
	             "registered.", p_listener->getName().c_str());
	
	// Register the listener by name
	m_customBlockingListeners.insert(ActionListeners::value_type(name,
	                                                             p_listener));
}


void MenuSystem::unregisterCustomActionListener(MenuActionListener* p_listener)
{
	// Listener can never be null
	TT_ASSERTMSG(p_listener != 0,
	             "Action listener pointers can never be null.");
	
	// Find the action listener
	//safestring name(toSafeString(p_listener->getName()));
	std::string name(p_listener->getName());
	ActionListeners::iterator it = m_customListeners.find(name);
	
	TT_ASSERTMSG(it != m_customListeners.end(),
	             "Custom action listener %p (name '%s') was never registered.",
	             p_listener, p_listener->getName().c_str());
	
	// Remove the listener from the container
	m_customListeners.erase(it);
}


MenuActionListener* MenuSystem::unregisterCustomActionListener(
		const std::string& p_name)
{
	// Find the action listener
	//safestring name(toSafeString(p_name));
	std::string name(p_name);
	ActionListeners::iterator it = m_customListeners.find(name);
	
	TT_ASSERTMSG(it != m_customListeners.end(),
	             "Custom action listener with name '%s' was never registered.",
	             p_name.c_str());
	
	// Remove the listener from the container
	MenuActionListener* listener = (*it).second;
	m_customListeners.erase(it);
	
	// Return the pointer to the listener
	return listener;
}


void MenuSystem::unregisterCustomBlockingActionListener(
		MenuActionListener* p_listener)
{
	// Listener can never be null
	TT_ASSERTMSG(p_listener != 0,
	             "Action listener pointers can never be null.");
	
	// Find the action listener
	//safestring name(toSafeString(p_listener->getName()));
	std::string name(p_listener->getName());
	ActionListeners::iterator it = m_customBlockingListeners.find(name);
	
	TT_ASSERTMSG(it != m_customBlockingListeners.end(),
	             "Custom blocking action listener %p (name '%s') was never "
	             "registered.", p_listener, p_listener->getName().c_str());
	
	if (it != m_customBlockingListeners.end())
	{
		// Remove the listener from the container
		m_customBlockingListeners.erase(it);
	}
}


MenuActionListener* MenuSystem::unregisterCustomBlockingActionListener(
		const std::string& p_name)
{
	// Find the action listener
	//safestring name(toSafeString(p_name));
	std::string name(p_name);
	ActionListeners::iterator it = m_customBlockingListeners.find(name);
	
	TT_ASSERTMSG(it != m_customBlockingListeners.end(),
	             "Custom blocking action listener with name '%s' was never "
	             "registered.", p_name.c_str());
	
	if (it != m_customBlockingListeners.end())
	{
		// Remove the listener from the container
		MenuActionListener* listener = (*it).second;
		m_customBlockingListeners.erase(it);
		
		// Return the pointer to the listener
		return listener;
	}
	
	return 0;
}


void MenuSystem::doActions(const MenuActions& p_actions)
{
	for (MenuActions::const_iterator it = p_actions.begin();
	     it != p_actions.end(); ++it)
	{
		// Get the active menu
		elements::Menu* activeMenu = getActiveMenu();
		
		if (doAction(*it))
		{
			// This action is blocking;
			// save the remainder of the actions in the menu.
			std::string command((*it).getCommand());
			
			++it;
			if (it != p_actions.end())
			{
				TT_ASSERTMSG(activeMenu != 0,
				             "Cannot queue actions after blocking action '%s', "
				             "because there is no active menu.",
				             command.c_str());
				
				MenuActions queuedActions(it, p_actions.end());
				activeMenu->setQueuedActions(queuedActions);
			}
			break;
		}
	}
}


void MenuSystem::doMenuElementAction(const MenuElementAction& p_action)
{
	using namespace elements;
	Menu* activeMenu = getActiveMenu();
	TT_ASSERTMSG(activeMenu != 0,
	             "Cannot perform menu element action '%s' on element '%s' "
	             "without an active menu.", p_action.getCommand().c_str(),
	             p_action.getTargetElement().c_str());
	
	if (activeMenu != 0)
	{
		MenuElementInterface* target = activeMenu->getMenuElement(
			p_action.getTargetElement());
		if (target != 0)
		{
			bool ret = target->doAction(p_action);
			TT_ASSERTMSG(ret,
			             "Menu element '%s' did not handle action '%s'.",
			             p_action.getTargetElement().c_str(),
			             p_action.getCommand().c_str());
		}
		else
		{
			TT_PANIC("Menu element '%s' not found, can't perform action '%s'.",
			         p_action.getTargetElement().c_str(),
			         p_action.getCommand().c_str());
		}
	}
}


void MenuSystem::executeActionSet(const std::string& p_setName)
{
	elements::Menu* activeMenu = getActiveMenu();
	TT_ASSERTMSG(activeMenu != 0,
	             "Cannot execute action set '%s' without an active menu!",
	             p_setName.c_str());
	if (activeMenu != 0)
	{
		activeMenu->executeActionSet(p_setName);
	}
}


void MenuSystem::setFocus(const std::string& p_elementName)
{
	elements::Menu* menu = getActiveMenu();
	if (menu != 0)
	{
		elements::MenuElementInterface::SelectionPath path;
		if (menu->getSelectionPathForElement(path, p_elementName))
		{
			menu->setSelectionPath(path);
			resetSelectedElement();
		}
	}
}


void MenuSystem::update()
{
	TT_ASSERTMSG(m_menuDeathRow != 0, "MenuSystem::enterState has not been "
	             "called (menu death-row vector is null).");
	
	// Remove all menus on death row
	for (MenuDeathRow::iterator it = m_menuDeathRow->begin();
	     it != m_menuDeathRow->end(); ++it)
	{
		destroyMenu(*it);
	}
	m_menuDeathRow->clear();
	
	
	// Close the tree menu, if so specified
	if (m_closeTreeMenu)
	{
		closeTreeMenu();
	}
	
	
	// Only handle input and animation in the bottom frame
	//if (engine::renderer::Renderer::getInstance()->isRenderingTopScreen())
	{
		if (m_transition == 0)
		{
			// Update the current tree menu
			if (m_currentMenu != 0 && m_currentMenu->getMenu() != 0)
			{
				m_currentMenu->getMenu()->update();
			}
		}
		else
		{
			if (m_transition->isDone())
			{
				// Done with transition; set new menu
				m_currentMenu = m_transition->getTargetMenu();
				delete m_transition;
				m_transition = 0;
				
				// Send menu activation event
				if (m_currentMenu->getMenu() != 0)
				{
					m_currentMenu->getMenu()->onMenuActivated();
				}
				
				// Update the selection cursor
				updateSelectionCursor();
				
				// Reset input
				resetInput();
			}
			else
			{
				// Update the menu transition animation
				m_transition->update();
			}
		}
		
		
		// Update all pop-up menus, back to front
		TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
		             "called (pop-up menu vector is null).");
		if (m_popupMenus->empty() == false)
		{
			// - Make a copy of the current pop-up vector contents,
			//   so that the member vector can be safely altered
			//   during the update.
			PopupMenus popups(*m_popupMenus);
			for (PopupMenus::iterator it = popups.begin();
				 it != popups.end(); ++it)
			{
				(*it)->update();
			}
		}
		
		// Handle input if there is a pop-up menu or tree-menu open
		// (but not if there is solely a menu transition active)
		if (m_popupMenus->empty() == false || m_transition == 0)
		{
			doInput();
		}
	}
	
	if (m_soundPlayer != 0)
	{
		m_soundPlayer->update();
	}
}


void MenuSystem::render()
{
	/*
	if (engine::renderer::Renderer::getInstance()->isRenderingTopScreen() == false)
	{
		// Only render the menu on the bottom screen (sub frame)
		return;
	}
	//*/
	
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	
	bool renderBackground = true;
	
	if (m_popupMenus->empty() == false)
	{
		renderBackground = m_popupMenus->back()->getRenderBackground();
	}
	
	s32       zDepth = 100;
	const s32 zStep  = -30;
	
	math::PointRect renderRect(m_menuRect);
	renderRect.translate(m_renderOffset);
	
	// If in tree menu
	if (renderBackground && m_currentMenu != 0)
	{
		// Render the currently active tree-menu
		if (m_transition == 0)
		{
			// Not in a transition; render the tree-menu
			if (m_currentMenu->getMenu() != 0)
			{
				m_currentMenu->getMenu()->render(renderRect, zDepth);
				//zDepth -= m_currentMenu->getMenu()->getDepth();
			}
		}
		else
		{
			// In a transition; render the transition
			m_transition->render(zDepth);
		}
	}
	
	// Render all pop-up menus, back to front
	for (PopupMenus::iterator it = m_popupMenus->begin();
	     it != m_popupMenus->end(); ++it)
	{
		zDepth += zStep;
		
		// Check if this is the last pop-up menu
		if (m_showPopupShade && (it + 1) == m_popupMenus->end())
		{
			++zDepth;
			
			// Render the shade behind the pop-up menu
			m_popupShade->setPositionZ(static_cast<real>(zDepth));
			m_popupShade->update();
			m_popupShade->render();
			
			++zDepth;
		}
		
		if (renderBackground || (it + 1) == m_popupMenus->end())
		{
			(*it)->render(renderRect, zDepth);
			//zDepth -= (*it)->getDepth();
		}
	}
	
	
	// Check if the Back button needs rendering
	elements::Menu* activeMenu = getActiveMenu();
	if (m_backButton != 0)
	{
		if (activeMenu != 0 && activeMenu->canGoBack())
		{
			// Render Back button
			m_backButton->update();
			m_backButton->render();
			
			m_backButtonOverlay->update();
			m_backButtonOverlay->render();
		}
	}
	
	
#ifndef MENU_NO_SELECTION_CURSOR
	// Render the selection cursor, if we have one
	// and we're not in a transition
	if (m_selectionCursor != 0 && activeMenu != 0)
	{
		m_selectionCursor->render(renderRect, 0);
	}
#endif  // !defined(MENU_NO_SELECTION_CURSOR)
	
	
#if defined(MENU_DEBUG_CURSOR)
	if (m_cursorQuad != 0)
	{
		// Render the debug cursor quad at the last stylus position
		m_cursorQuad->setPosition(math::Vector3(
			static_cast<real>(m_stylusPreviousX),
			static_cast<real>(m_stylusPreviousY),
			1.0f));
		m_cursorQuad->update();
		m_cursorQuad->render();
	}
#endif
}


void MenuSystem::setRenderOffset(const math::Point2& p_offset)
{
	m_renderOffset = p_offset;
}


engine::glyph::GlyphSet* MenuSystem::getGlyphSet()
{
	TT_ASSERTMSG(m_glyphSet != 0, "No glyphset was created.");
	return m_glyphSet;
}


std::wstring MenuSystem::translateString(const std::string& p_string)
{
	TT_ASSERTMSG(m_translation != 0,
	             "No translation instance available.");
	
	std::wstring translation(m_translation->getString(p_string.c_str()));
	
	TT_ASSERTMSG(translation != m_translation->getErrorString(),
	             "Translating string '%s' failed.", p_string.c_str());
	return translation;
}


void MenuSystem::enterState(
		engine::glyph::GlyphSet* p_glyphSet,
		const std::string&       p_language)
{
	TT_ASSERTMSG(p_glyphSet != 0, "Invalid glyph set pointer passed.");
	
	//memory::HeapMgr::startmeasure("menu system::enter state");
	
	// Reset render offset
	m_renderOffset.setValues(0, 0);
	
	// Clear the 'close tree-menu' flag
	m_closeTreeMenu = false;
	
	// Set the central menu glyph set from param
	m_glyphSet = p_glyphSet;
	//memory::HeapMgr::measure("new m_glyphSet");
	
	// Create the central string translation
	m_translation = new loc::LocStr("strings/menu.bin");
	m_translation->selectLanguage(p_language);
	
	//memory::HeapMgr::measure("new m_translation");
	
	// Open data file for the menu system
	/*
	m_menuDataFile = g_Ocean3D.OpenDataFile("ingamegui.dat");
	TT_ASSERTMSG(m_menuDataFile != 0, "Loading menu data file failed.");
	//memory::HeapMgr::measure("new m_menuDataFile");
	//*/
	
	using engine::renderer::QuadSprite;
	using engine::renderer::ColorRGBA;
	
#if defined(MENU_DEBUG_CURSOR)
	// Create a debug cursor quad sprite
	m_cursorQuad = QuadSprite::createQuad(4.0f, 4.0f, ColorRGBA(0, 0, 255, 255));
#endif
	
	// Create the containers for the menus
	TT_ASSERTMSG(m_popupMenus == 0, "MenuSystem::exitState has not been called "
	             "(pop-up menu vector already exists).");
	m_popupMenus = new PopupMenus;
	//memory::HeapMgr::measure("new m_popupMenus");
	
	TT_ASSERTMSG(m_menuDeathRow == 0, "MenuSystem::exitState has not been "
	             "called (menu death-row vector already exists).");
	m_menuDeathRow = new MenuDeathRow;
	//memory::HeapMgr::measure("new m_menuDeathRow");
	
	// Create a shade quad for behind the top-most pop-up menu
	m_popupShade = QuadSprite::createQuad(
		static_cast<real>(m_menuRect.getWidth()),
		static_cast<real>(m_menuRect.getHeight()),
		ColorRGBA(0, 0, 0, 127));
	//memory::HeapMgr::measure("new m_popupShade");
	
	m_popupShade->setPosition(math::Vector3(
		static_cast<real>(m_menuRect.getCenterPosition().x),
		static_cast<real>(m_menuRect.getCenterPosition().y),
		0.0f));
	
	// Create the back button
	createBackButton();
	
	// Create the selection cursor
	createSelectionCursor();
	
	//memory::HeapMgr::stopmeasure("Done (MenuSystem::enterState)");
}


void MenuSystem::exitState()
{
	stopMenuMusic();
	
	// Destroy all tree-menus
	delete m_transition;
	m_transition = 0;
	closeTreeMenu();
	
	
	// Destroy all menus on death row
	TT_ASSERTMSG(m_menuDeathRow != 0, "MenuSystem::enterState has not been "
	             "called (menu death-row vector is null).");
	for (MenuDeathRow::iterator it = m_menuDeathRow->begin();
	     it != m_menuDeathRow->end(); ++it)
	{
		destroyMenu(*it);
	}
	delete m_menuDeathRow;
	m_menuDeathRow = 0;
	
	// Destroy all open pop-up menus
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	TT_WARNING(m_popupMenus->empty(),
	           "Pop-up menus were still open upon state exit!");
	
	for (PopupMenus::iterator it = m_popupMenus->begin();
	     it != m_popupMenus->end(); ++it)
	{
		destroyMenu(*it);
	}
	delete m_popupMenus;
	m_popupMenus = 0;
	
	// Destroy the back button
	m_backButton.reset();
	m_backButtonTexture.reset();
	
	m_backButtonOverlay.reset();
	m_backButtonOverlayTexture.reset();
	
#ifndef MENU_NO_SELECTION_CURSOR
	// Destroy the menu item selection cursor
	delete m_selectionCursor;
	m_selectionCursor = 0;
	
	// No element is selected
	m_selectedElement = 0;
#endif
	
	// Kill reference to glyph set
	m_glyphSet = 0;
	
	// Destroy the central string translation
	delete m_translation;
	m_translation = 0;
	
#if defined(MENU_DEBUG_CURSOR)
	// Free the debug cursor quad sprite
	m_cursorQuad.reset();
#endif
	
	// Close the menu data file
	/*
	if (m_menuDataFile != 0)
	{
		g_Ocean3D.CloseDataFile(m_menuDataFile);
		m_menuDataFile = 0;
	}
	//*/
	
	// Clean up the pop-up shade quad
	m_popupShade.reset();
	
	// Clean up the system variables
	m_systemVars.clear();
}


void MenuSystem::setMenuFactory(MenuFactory* p_factory)
{
	// Destroy the current factory and assign the new one
	delete m_menuFactory;
	m_menuFactory = p_factory;
}


void MenuSystem::setSoundPlayer(MenuSoundPlayer* p_player)
{
	// Replace the sound player instance
	delete m_soundPlayer;
	m_soundPlayer = p_player;
}


void MenuSystem::playSound(MenuSound p_sound, bool p_looping)
{
	if (m_soundPlayer != 0)
	{
		m_soundPlayer->playSound(p_sound, p_looping);
	}
}


void MenuSystem::stopLoopingSounds()
{
	if (m_soundPlayer != 0)
	{
		m_soundPlayer->stopLoopingSounds();
	}
}


void MenuSystem::playMenuMusic()
{
	if (m_soundPlayer != 0)
	{
		m_soundPlayer->startSong();
	}
}


void MenuSystem::stopMenuMusic()
{
	if (m_soundPlayer != 0)
	{
		m_soundPlayer->stopSong();
	}
}


void MenuSystem::setMenuVar(const std::string& p_name,
                            const std::string& p_value)
{
	elements::Menu* activeMenu = getActiveMenu();
	TT_ASSERTMSG(activeMenu != 0,
	             "Cannot set menu variable when no menu is active.");
	activeMenu->setVar(p_name, p_value);
}


void MenuSystem::setSystemVar(const std::string& p_name,
                              const std::string& p_value)
{
	MENU_Printf("MenuSystem::setSystemVar: Setting variable '%s' to '%s'.\n",
	            p_name.c_str(), p_value.c_str());
	
	Variables::iterator it = m_systemVars.find(p_name);
	
	/* No longer warn about non-existing variables.
	TT_WARNING(it != m_systemVars.end(),
	           "System variable '%s' does not exist (init value: '%s').",
	           p_name.c_str(), p_value.c_str());
	//*/
	
	if (it == m_systemVars.end())
	{
		m_systemVars.insert(Variables::value_type(p_name, p_value));
	}
	else
	{
		(*it).second = p_value;
	}
}


std::string MenuSystem::getSystemVar(const std::string& p_name) const
{
	MENU_Printf("MenuSystem::getSystemVar: Variable name: '%s'.\n",
	            p_name.c_str());
	
	Variables::const_iterator it = m_systemVars.find(p_name);
	if (it != m_systemVars.end())
	{
		return (*it).second;
	}
	
	TT_PANIC("System variable '%s' was never set; can't get value.",
	         p_name.c_str());
	return "";
}


void MenuSystem::removeSystemVar(const std::string& p_name)
{
	MENU_Printf("MenuSystem::removeSystemVar: Removing system variable '%s'.\n",
	            p_name.c_str());
	
	Variables::iterator it = m_systemVars.find(p_name);
	TT_ASSERTMSG(it != m_systemVars.end(),
	             "System variable '%s' does not exist; cannot remove it.",
	             p_name.c_str());
	
	if (it != m_systemVars.end())
	{
		m_systemVars.erase(it);
	}
}


bool MenuSystem::hasSystemVar(const std::string& p_name) const
{
	return (m_systemVars.find(p_name) != m_systemVars.end());
}


void MenuSystem::dumpSystemVars() const
{
	if (m_systemVars.empty() == false)
	{
		TT_Printf("MenuSystem::dumpSystemVars: Current system variables:\n");
		for (Variables::const_iterator it = m_systemVars.begin();
		     it != m_systemVars.end(); ++it)
		{
			std::string value;
			if ((*it).second.length() > 40)
			{
				// Truncate value
				value = (*it).second.substr(0, 40);
				value += " ...";
			}
			else
			{
				// Use entire value
				value = (*it).second;
			}
			
			TT_Printf("MenuSystem::dumpSystemVars: '%s' = '%s'\n",
			          (*it).first.c_str(), value.c_str());
		}
	}
	else
	{
		TT_Printf("MenuSystem::dumpSystemVars: No system variables set.\n");
	}
}


const MenuSkin* MenuSystem::getSkin() const
{
	return m_menuSkin;
}


void MenuSystem::setSkin(MenuSkin* p_skin)
{
	delete m_menuSkin;
	m_menuSkin = p_skin;
	
	// Update the back button, if necessary
	if (m_backButton != 0)
	{
		m_backButton.reset();
		m_backButtonTexture.reset();
		
		m_backButtonOverlay.reset();
		m_backButtonOverlayTexture.reset();
		
		createBackButton();
	}
}


void MenuSystem::resetSelectedElement()
{
	m_selectedElement = 0;
}


void MenuSystem::setKeyRepeatDelay(u32 p_delay)
{
	m_keyDelayTime = p_delay;
}


void MenuSystem::setKeyRepeatFrequency(u32 p_frequency)
{
	m_keyRepeatTime = p_frequency;
}


/*
void* MenuSystem::operator new(std::size_t p_blockSize)
{
	using memory::HeapMgr;
#ifndef TT_BUILD_FINAL
	u32 foo = 0;
	asm {    mov     foo, lr}
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize, 4, (void*)foo);
#else
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize);
#endif
}


void MenuSystem::operator delete(void* p_block)
{
	memory::HeapMgr::freeToHeap(p_block);
}
//*/


//------------------------------------------------------------------------------
// Private member functions

MenuSystem::MenuSystem(const std::string& p_menuTreeFilename)
:
m_menuTreeRoot(0),
m_menuSkin(0),
m_menuFactory(0),
m_soundPlayer(0),
m_menuRect(math::Point2(0, 0), 256, 192), // FIXME: Remove hard-coded dimensions!
m_closeTreeMenu(false),
m_currentMenu(0),
m_popupMenus(0),
m_menuDeathRow(0),
m_glyphSet(0),
m_transition(0),
m_backButtonRect(math::Point2(0, 0), 1, 1),
m_backButtonHandled(false),
m_renderOffset(0, 0),
m_showPopupShade(true),
#ifndef MENU_NO_SELECTION_CURSOR
m_selectionCursor(0),
m_selectedElement(0),
m_noSelectionRect(math::Point2(-10, 0), 276, 192), // FIXME: Remove hard-coded dimensions!
#endif
m_translation(0),
m_stylusMask(false),
m_stylusPressedPreviousFrame(false),
m_stylusPreviousX(0),
m_stylusPreviousY(0),
m_stylusDelayTime(500),
m_stylusRepeatTime(100),
m_stylusPressedTime(0),
m_stylusHandledTime(0),
m_keysPreviousPressed(0),
m_keysBlockedUntilReleased(0),
m_keyDelayTime(500),
m_keyRepeatTime(100),
m_keyRepeatKey(0),
m_keyPressedTime(0),
m_keyHandledTime(0)
{
	// NOTE: Only create safe-heap members here.
	
	// Create the default menu factory
	m_menuFactory = new MenuFactory;
	
	// Build the menu tree
	buildMenuTree(p_menuTreeFilename);
	
	// Load the key mapping for the virtual keyboard
	MenuKeyboard::loadVirtualKeyMapping("menu/keymapping.xml");
}


MenuSystem::~MenuSystem()
{
	// Use exitState to destroy all non-safe heap resources
	exitState();
	
	// Destroy the entire menu tree
	delete m_menuTreeRoot;
	
	// Destroy all registered action listeners
	for (ActionListeners::iterator it = m_defaultBlockingListeners.begin();
	     it != m_defaultBlockingListeners.end(); ++it)
	{
		delete (*it).second;
	}
	
	for (ActionListeners::iterator it = m_customBlockingListeners.begin();
	     it != m_customBlockingListeners.end(); ++it)
	{
		delete (*it).second;
	}
	
	for (ActionListeners::iterator it = m_defaultListeners.begin();
	     it != m_defaultListeners.end(); ++it)
	{
		delete (*it).second;
	}
	
	for (ActionListeners::iterator it = m_customListeners.begin();
	     it != m_customListeners.end(); ++it)
	{
		delete (*it).second;
	}
	
	delete m_menuFactory;
	delete m_menuSkin;
	delete m_soundPlayer;
}


void MenuSystem::buildMenuTree(const std::string& p_filename)
{
	TT_ASSERTMSG(m_menuTreeRoot == 0,
	             "Cannot build menu tree: a menu tree already exists!");
	
	// Load the menu tree XML file
	using xml::XmlNode;
	
	// Prepare a pointer to the root node
	xml::XmlDocument doc(p_filename);
	XmlNode* menuRoot = doc.getRootNode();
	if (menuRoot == 0)
	{
		TT_PANIC("Failed to load XML file '%s'.", p_filename.c_str());
		return;
	}
	
	/*
	{
		// Load the xml file
		xml::XmlFileReader xml;
		
		if (xml.loadFile(p_filename) == false)
		{
			TT_PANIC("Failed to load XML file '%s'.", p_filename.c_str());
		}
		
		// Create a hierarchy
		menuRoot = XmlNode::createTree(xml);
		TT_ASSERT(menuRoot != 0);
	} // Release memory of XmlFileReader
	*/
	
	// Make sure a valid menu XML file is loaded
	TT_ASSERT(menuRoot->getName() == "menu"); // Root elem. should be "menu"
	TT_ASSERT(menuRoot->getSibling() == 0);   // No siblings of root allowed
	
	// Get name attribute
	const std::string rootName = menuRoot->getAttribute("name");
	TT_ASSERTMSG(rootName.empty() == false,
	             "Root menu tree node is missing required 'name' attribute.");
	
#if !defined(TT_BUILD_FINAL)
	// Make sure the menu file exists
	std::string menuFilename("menu/");
	menuFilename += rootName;
	menuFilename += ".xml";
	TT_ASSERTMSG(fs::fileExists(menuFilename),
	             "The menu tree XML (%s) specifies non-existent '%s' menu "
	             "(as root node)!", p_filename.c_str(), rootName.c_str());
#endif
	
	m_menuTreeRoot = new MenuTreeNode(rootName);
	
	// Parse the child nodes
	for (XmlNode* child = menuRoot->getChild(); child != 0; child = child->getSibling())
	{
		parseMenuTreeXML(m_menuTreeRoot, child);
	}
	
	// Free memory used by xml hierarchy
	/*
	delete menuRoot;
	menuRoot = 0;
	*/
	
	// Output the tree that was created
	MENU_Printf("MenuSystem::buildMenuTree: Menu tree that was built:\n");
	dumpMenuTree(m_menuTreeRoot);
}


void MenuSystem::parseMenuTreeXML(MenuTreeNode* p_treeNode,
                                  xml::XmlNode* p_xmlNode)
{
	TT_ASSERTMSG(p_treeNode != 0,
	             "Parent menu tree node cannot be null.");
	
	// Add the current node to the tree
	const std::string nodeName = p_xmlNode->getAttribute("name");
	TT_ASSERTMSG(nodeName.empty() == false,
	             "Menu tree node (child of node '%s') is missing required "
	             "'name' attribute.", p_treeNode->getName().c_str());
	
#if !defined(TT_BUILD_FINAL)
	// Make sure the menu file exists
	std::string menuFilename("menu/");
	menuFilename += nodeName;
	menuFilename += ".xml";
	TT_ASSERTMSG(fs::fileExists(menuFilename),
	             "The menu tree XML specifies non-existent '%s' menu "
	             "(child of '%s')!", nodeName.c_str(),
	             p_treeNode->getName().c_str());
#endif
	
	MenuTreeNode* node = p_treeNode->createChild(nodeName);
	
	// Add any children
	for (xml::XmlNode* child = p_xmlNode->getChild();
	     child != 0; child = child->getSibling())
	{
		parseMenuTreeXML(node, child);
	}
}


void MenuSystem::dumpMenuTree(MenuTreeNode* p_node, const std::string& p_indent)
{
	TT_ASSERTMSG(p_node != 0,
	             "Cannot dump menu tree information for null nodes.");
	
	MENU_Printf("MenuSystem::dumpMenuTree: %s- %s\n",
	            p_indent.c_str(), p_node->getName().c_str());
	
	for (int i = 0; i < p_node->getChildCount(); ++i)
	{
		dumpMenuTree(p_node->getChild(i), p_indent + "  ");
	}
}


void MenuSystem::resetInput()
{
	// No current menu; reset input
	m_stylusMask                 = true;
	m_stylusPressedPreviousFrame = false;
	m_keysBlockedUntilReleased   = 0xFFFFFFFF;
	m_keysPreviousPressed        = 0;
	m_stylusPressedTime          = 0;
	m_stylusHandledTime          = 0;
	m_keyRepeatKey               = 0;
	m_keyPressedTime             = 0;
	m_keyHandledTime             = 0;
}


elements::Menu* MenuSystem::getMenu(const std::string& p_menuName)
{
	// Menu factory is required
	TT_ASSERTMSG(m_menuFactory != 0,
	             "No menu factory is available for creating a menu.");
	
	// Menu skin is required
	TT_ASSERTMSG(m_menuSkin != 0,
	             "Cannot create a menu without having a skin set.");
	
	//using memory::HeapMgr;
	//HeapMgr::startPeakMeasure();
	
	// Reset the input
	resetInput();
	
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
	using memory::HeapMgr;
	TT_Printf("MenuSystem::getMenu: Menu '%s': Heap stats before menu "
	          "pre-create action:\n", p_menuName.c_str());
	HeapMgr::dumpHeapStats();
#endif
	
	// Send an action that a menu is being created
	sendMenuPreCreateAction(p_menuName);
	
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
	TT_Printf("MenuSystem::getMenu: Menu '%s': Heap stats after menu "
	          "pre-create action (right before creating menu):\n",
	          p_menuName.c_str());
	HeapMgr::dumpHeapStats();
#endif
	
	// Create a filename for the menu name
	std::string filename("menu/" + p_menuName + ".xml");
	
	// Attempt to load the menu
	elements::Menu* menu = m_menuFactory->createMenuFromXML(filename);
	
	// Verify that the menu name in the XML file is the same as what was requested
	TT_ASSERTMSG(menu->getName() == p_menuName,
	             "Loaded menu '%s' (filename '%s'), but the XML specified a "
	             "menu named '%s' (the names must match)!",
	             p_menuName.c_str(), filename.c_str(), menu->getName().c_str());
	
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
	TT_Printf("MenuSystem::getMenu: Menu '%s': Heap stats after creating menu "
	          "(right before menu post-create action):\n", p_menuName.c_str());
	HeapMgr::dumpHeapStats();
#endif
	
	// Send an action that a menu has been created
	sendMenuPostCreateAction(menu);
	
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
	TT_Printf("MenuSystem::getMenu: Menu '%s': Heap stats after menu "
	          "post-create action:\n", p_menuName.c_str());
	HeapMgr::dumpHeapStats();
#endif
	
	/*
	HeapMgr::stopPeakMeasure();
	TT_Printf("\nMenuSystem::getMenu: Peaks in building menu '%s':\n",
	          p_menuName.c_str());
	HeapMgr::dumpPeaks();
	//*/
	
	// Return a pointer to the menu
	return menu;
}


elements::Menu* MenuSystem::getActiveMenu()
{
	elements::Menu* activeMenu = 0;
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	if (m_popupMenus->empty() == false)
	{
		activeMenu = m_popupMenus->back();
	}
	else if (m_currentMenu != 0)
	{
		activeMenu = m_currentMenu->getMenu();
	}
	return activeMenu;
}


const elements::Menu* MenuSystem::getActiveMenu() const
{
	const elements::Menu* activeMenu = 0;
	TT_ASSERTMSG(m_popupMenus != 0, "MenuSystem::enterState has not been "
	             "called (pop-up menu vector is null).");
	if (m_popupMenus->empty() == false)
	{
		activeMenu = m_popupMenus->back();
	}
	else if (m_currentMenu != 0)
	{
		activeMenu = m_currentMenu->getMenu();
	}
	return activeMenu;
}


void MenuSystem::doInput()
{
	/*
	if (PAD_DetectFold())
	{
		return;
	}
	
	elements::Menu* activeMenu = getActiveMenu();
	if (activeMenu == 0)
	{
		// No active menu; nothing to do here
		return;
	}
	
	
	using system::System;
	
	//------------------------------------------------------------
	// Handle stylus input
	
	bool   stylusPressed = System::isStylusTouching();
	TPData stylus(System::getCalibratedStylusSample());
	bool   inputHandled = false;
	
	if (m_stylusMask)
	{
		if (stylusPressed)
		{
			stylusPressed = false;
		}
		else
		{
			m_stylusMask = false;
		}
	}
	
	u64 currentTime = System::getTimeMs();
	
	if (stylusPressed && m_stylusPressedPreviousFrame == false)
	{
		// Stylus is depressed
		if (stylus.validity == TP_VALIDITY_VALID)
		{
			inputHandled = activeMenu->onStylusPressed((s32)stylus.x,
			                                           (s32)stylus.y);
			m_stylusPressedTime = currentTime;
			m_stylusHandledTime = 0;
		}
	}
	else if (stylusPressed && m_stylusPressedPreviousFrame)
	{
		// Stylus is dragging
		if (stylus.validity == TP_VALIDITY_VALID)
		{
			inputHandled = activeMenu->onStylusDragging((s32)stylus.x,
			                                            (s32)stylus.y, true);
			
			// Check if "stylus repeat" messages need to be sent
			if (m_stylusHandledTime == 0)
			{
				if (currentTime - m_stylusPressedTime >= m_stylusDelayTime)
				{
					m_stylusHandledTime = currentTime;
					if (activeMenu->onStylusRepeat((s32)stylus.x,
					                               (s32)stylus.y) == false)
					{
						m_stylusHandledTime = 0;
						m_stylusPressedTime = 0;
					}
				}
			}
			else
			{
				if (currentTime - m_stylusHandledTime >= m_stylusRepeatTime)
				{
					m_stylusHandledTime = currentTime;
					if (activeMenu->onStylusRepeat((s32)stylus.x,
					                               (s32)stylus.y) == false)
					{
						m_stylusHandledTime = 0;
						m_stylusPressedTime = 0;
					}
				}
			}
		}
	}
	else if (stylusPressed == false && m_stylusPressedPreviousFrame)
	{
		// Stylus was released
		inputHandled = activeMenu->onStylusReleased((s32)m_stylusPreviousX,
		                                            (s32)m_stylusPreviousY);
		m_stylusHandledTime = 0;
		m_stylusPressedTime = 0;
	}
	
	
	// Check if the Back button was hit
	if (m_backButtonHandled == false &&
	    inputHandled        == false &&
	    m_backButtonRect.contains(math::Point2(stylus.x, stylus.y)) &&
	    activeMenu->canGoBack())
	{
		m_backButtonHandled = true;
		
		// Execute the actions for the back button
		if (activeMenu->hasActionSet("system_back_button"))
		{
			activeMenu->executeActionSet("system_back_button");
		}
		else
		{
			// Menu doesn't specify actions; use the default
			goBackMenu();
		}
	}
	
	// Request "stylus touching" directly from system,
	// because stylusPressed flag is not accurate at this point
	if (System::isStylusTouching() == false)
	{
		m_backButtonHandled = false;
	}
	
	m_stylusPressedPreviousFrame = stylusPressed;
	m_stylusPreviousX            = stylus.x;
	m_stylusPreviousY            = stylus.y;
	
	
	//------------------------------------------------------------
	// Handle key input
	
	// - Get raw key input
	u32 keys = system::Pad::keyRead();
	
	// - Filter out the keys that were blocked until released
	m_keysBlockedUntilReleased &= keys;
	keys &= ~m_keysBlockedUntilReleased;
	
	// Determine which keys remained down, were pressed or were released
	u32 down     = keys & m_keysPreviousPressed;
	u32 pressed  = ~down & keys;
	u32 released = ~down & m_keysPreviousPressed;
	
	// Save the raw input
	m_keysPreviousPressed = keys;
	
	if (pressed != 0)
	{
		// Send key pressed event to the menu
		MenuKeyboard keyboard(pressed);
		bool         handled = activeMenu->onKeyPressed(keyboard);
		
		if (handled == false)
		{
			// Menu didn't handle the key input; check if back button was pressed
			if (keyboard.isKeySet(MenuKeyboard::MENU_BACK))
			{
				if (activeMenu->canGoBack())
				{
					// Execute the actions for the back button
					if (activeMenu->hasActionSet("system_back_button"))
					{
						activeMenu->executeActionSet("system_back_button");
					}
					else
					{
						// Menu doesn't specify actions; use the default
						goBackMenu();
					}
				}
			}
		}
	}
	
	if (down != 0)
	{
		activeMenu->onKeyHold(MenuKeyboard(down));
		
		if ((down & m_keyRepeatKey) != 0)
		{
			u64 current_time = System::getTimeMs();
			
			// If key repeat has not been triggered before
			if (m_keyHandledTime == 0)
			{
				// And it's time to trigger it
				if (current_time >= m_keyPressedTime + m_keyDelayTime)
				{
					m_keyHandledTime = current_time;
					activeMenu->onKeyRepeat(MenuKeyboard(m_keyRepeatKey));
				}
			}
			else if (current_time >= m_keyHandledTime + m_keyRepeatTime)
			{
				m_keyHandledTime = current_time;
				activeMenu->onKeyRepeat(MenuKeyboard(m_keyRepeatKey));
			}
		}
	}
	
	if (released != 0)
	{
		activeMenu->onKeyReleased(MenuKeyboard(released));
		
		// Disable keyrepeat for released key(s)
		m_keyRepeatKey &= ~released;
	}
	
	// Check for new keyrepeat
	if (pressed != 0)
	{
		// When multiple keys are pressed at the same time, grab the lowest
		m_keyRepeatKey   = MATH_GetLeastSignificantBit(pressed);
		m_keyPressedTime = System::getTimeMs();
		m_keyHandledTime = 0;
	}
	//*/
	
	// Update the selection cursor, if there is one
	updateSelectionCursor();
}


std::string MenuSystem::parseMenuVar(const std::string& p_string)
{
	std::string::const_iterator it = p_string.begin();
	if (it != p_string.end())
	{
		if ((*it) == '%')
		{
			++it;
			if (it != p_string.end())
			{
				elements::Menu* activeMenu = getActiveMenu();
				TT_ASSERTMSG(activeMenu != 0, "Cannot parse menu variable "
				             "when no menu is active.");
				return activeMenu->getVar(std::string(it, p_string.end()));
			}
		}
		else if ((*it) == '#')
		{
			++it;
			if (it != p_string.end())
			{
				return getSystemVar(std::string(it, p_string.end()));
			}
		}
	}
	
	return p_string;
}


void MenuSystem::createBackButton()
{
	// Do nothing if back button already created
	if (m_backButton != 0)
	{
		return;
	}
	
	// Cannot create back button without skinning information
	TT_ASSERTMSG(m_menuSkin != 0,
	             "Cannot create back button without a menu skin.");
	
	// Get skinning information for the back button
	using namespace elements;
	TT_ASSERTMSG(m_menuSkin->hasElementSkin(SkinElement_BackButton),
	             "Skin does not provide skinning information "
	             "for system back button.");
	
	const MenuSkin::ElementSkin& element(
		m_menuSkin->getElementSkin(SkinElement_BackButton));
	const MenuSkin::SkinTexture& skinTexture(
		element.getTexture(BackButtonSkin_Texture));
	const MenuSkin::SkinTexture& overlayTexture(
		element.getTexture(BackButtonSkin_OverlayTexture));
	
	// Get back button textures
	// FIXME: MUST SUPPORT NAMESPACES
	m_backButtonTexture = engine::renderer::TextureCache::get(skinTexture.getFilename(), "");
	if(m_backButtonTexture == 0)
	{
		TT_PANIC("Loading system back button texture '%s' failed.",
		         skinTexture.getFilename().c_str());
		return;
	}
	m_backButtonOverlayTexture = engine::renderer::TextureCache::get(overlayTexture.getFilename(), "");
	if(m_backButtonOverlayTexture == 0)
	{
		TT_PANIC("Loading system back button overlay texture '%s' failed.",
		         overlayTexture.getFilename().c_str());
		m_backButtonTexture.reset();
		return;
	}
	
	// Create a quad for rendering the image
	using engine::renderer::QuadSprite;
	m_backButton = QuadSprite::createQuad(m_backButtonTexture,
		element.getVertexColor(BackButtonSkin_Color));
	m_backButtonOverlay = QuadSprite::createQuad(m_backButtonOverlayTexture,
		element.getVertexColor(BackButtonSkin_OverlayColor));
	
	// Position the back button in the lower left corner of the screen
	m_backButtonRect.setWidth (static_cast<s32>(skinTexture.getWidth()));
	m_backButtonRect.setHeight(static_cast<s32>(skinTexture.getHeight()));
	m_backButtonRect.setPosition(math::Point2(
		0,
		m_menuRect.getHeight() - m_backButtonRect.getHeight()));
	
	// Quad's position is the center of the image
	math::PointRect quadRect(m_backButtonRect);
	quadRect.setWidth (m_backButtonTexture->getWidth());
	quadRect.setHeight(m_backButtonTexture->getHeight());
	
	// Set the position for the quad.
	m_backButton->setPosition(math::Vector3(
		static_cast<real>(quadRect.getCenterPosition().x),
		static_cast<real>(quadRect.getCenterPosition().y),
		10.0f));
	
	m_backButtonOverlay->setPosition(math::Vector3(
		static_cast<real>(quadRect.getCenterPosition().x),
		static_cast<real>(quadRect.getCenterPosition().y),
		0.0f));
}


void MenuSystem::createSelectionCursor()
{
#ifndef MENU_NO_SELECTION_CURSOR
	// If the cursor already exists, there is nothing to do
	if (m_selectionCursor != 0)
	{
		return;
	}
	
	m_selectionCursor = new elements::SelectionCursor("", MenuLayout());
#endif  // !defined(MENU_NO_SELECTION_CURSOR)
}


void MenuSystem::destroySelectionCursor()
{
#ifndef MENU_NO_SELECTION_CURSOR
	delete m_selectionCursor;
	m_selectionCursor = 0;
#endif
}


void MenuSystem::updateSelectionCursor()
{
#ifndef MENU_NO_SELECTION_CURSOR
	// Update the selection cursor, if there is one
	elements::Menu* activeMenu = getActiveMenu();
	if (activeMenu != 0 && m_selectionCursor != 0)
	{
		elements::MenuElementInterface* newSelection =
			activeMenu->getSelectedElement();
		if (newSelection != m_selectedElement)
		{
			if (newSelection != 0 &&
			    newSelection->wantCursor() != elements::SelectionCursorType_None)
			{
				// Update the selection rectangle
				math::PointRect selectRect(math::Point2(0, 0), 1, 1);
				if (activeMenu->getSelectedElementRect(selectRect))
				{
					m_selectionCursor->setSelectionRect(selectRect);
					m_selectionCursor->setSelectionType(
						newSelection->wantCursor());
					m_selectionCursor->setVisible(true);
				}
				else
				{
					// Hide the selection cursor
					m_selectionCursor->setVisible(false);
				}
			}
			else
			{
				// Hide the selection cursor
				m_selectionCursor->setVisible(false);
			}
			
			m_selectedElement = newSelection;
		}
	}
#endif  // !defined(MENU_NO_SELECTION_CURSOR)
}


bool MenuSystem::doAction(const MenuAction& p_action)
{
	TT_ASSERTMSG(p_action.getCommand().empty() == false,
	             "Action command cannot be empty (action %p).",
	             &p_action);
	
	// Get the active menu
	//elements::Menu* activeMenu = getActiveMenu();
	//TT_WARNING(activeMenu != 0,
	//           "Performing menu actions without an active menu! Action: %s",
	//           p_action.getCommand().c_str());
	
	// Check for menu variables (var) in the action.
	MenuAction action(parseMenuVar(p_action.getCommand()));
	for (int idx = 0; idx < p_action.getParameterCount(); ++idx)
	{
		action.addParameter(parseMenuVar(p_action.getParameter(idx)));
	}
	
	// NOTE: After this line only action should be used and NOT p_action!
	
	
	// Check if this command is blocking somehow
	if (sendToDefaultBlockingListeners(action))
	{
		// Action blocked by default listener
		return true;
	}
	
	if (sendToCustomBlockingListeners(action))
	{
		// Action blocked by custom listener
		return true;
	}
	
	
	// Check if any of the default non-blocking listeners handled the command
	if (sendToDefaultListeners(action))
	{
		// Action handled but not blocked by default listener
		return false;
	}
	
	// Check if any of the custom non-blocking listeners handled the command
	if (sendToCustomListeners(action))
	{
		// Action handled but not blocked by custom listener
		return false;
	}
	
	// No listener handled the action: assert
	TT_PANIC("Menu action '%s' (before parsing: '%s') was not handled "
	         "by any action listener.",
	         action.getCommand().c_str(), p_action.getCommand().c_str());
	return false;
}


void MenuSystem::doSystemAction(const MenuAction& p_action)
{
	TT_ASSERTMSG(p_action.getCommand().empty() == false,
	             "Action command cannot be empty.");
	
	// Check for menu variables (var) in the action.
	MenuAction action(parseMenuVar(p_action.getCommand()));
	for (int idx = 0; idx < p_action.getParameterCount(); ++idx)
	{
		action.addParameter(parseMenuVar(p_action.getParameter(idx)));
	}
	
	// NOTE: After this line only action should be used and NOT p_action!
	
	
	// Check if this command is blocking somehow
	if (sendToDefaultBlockingListeners(action))
	{
		// Action blocked by default listener
		return;
	}
	
	if (sendToCustomBlockingListeners(action))
	{
		// Action blocked by custom listener
		return;
	}
	
	
	// Check if any of the default non-blocking listeners handled the command
	if (sendToDefaultListeners(action))
	{
		// Action handled but not blocked by default listener
		return;
	}
	
	// Check if any of the custom non-blocking listeners handled the command
	if (sendToCustomListeners(action))
	{
		// Action handled but not blocked by custom listener
		return;
	}
	
	// No listener handled the action: assert
	TT_PANIC("Menu system action '%s' (before parsing: '%s') was not handled "
	         "by any action listener.",
	         action.getCommand().c_str(), p_action.getCommand().c_str());
}


bool MenuSystem::sendToDefaultListeners(const MenuAction& p_action)
{
	for (ActionListeners::iterator it = m_defaultListeners.begin();
	     it != m_defaultListeners.end(); ++it)
	{
		if ((*it).second->doAction(p_action))
		{
			MENU_Printf("MenuSystem::sendToDefaultListeners: Action '%s' was "
			            "handled by default action listener '%s'.\n",
			            p_action.getCommand().c_str(), (*it).first.c_str());
			return true;
		}
	}
	
	return false;
}


bool MenuSystem::sendToCustomListeners(const MenuAction& p_action)
{
	for (ActionListeners::iterator it = m_customListeners.begin();
	     it != m_customListeners.end(); ++it)
	{
		if ((*it).second->doAction(p_action))
		{
			MENU_Printf("MenuSystem::sendToCustomListeners: Action '%s' was "
			            "handled by custom action listener '%s'.\n",
			            p_action.getCommand().c_str(), (*it).first.c_str());
			return true;
		}
	}
	
	return false;
}


bool MenuSystem::sendToDefaultBlockingListeners(const MenuAction& p_action)
{
	for (ActionListeners::iterator it = m_defaultBlockingListeners.begin();
	     it != m_defaultBlockingListeners.end(); ++it)
	{
		if ((*it).second->doAction(p_action))
		{
			MENU_Printf("MenuSystem::sendToDefaultBlockingListeners: Action "
			            "'%s' was handled by default blocking action listener "
			            "'%s'.\n",
			            p_action.getCommand().c_str(), (*it).first.c_str());
			return true;
		}
	}
	
	return false;
}


bool MenuSystem::sendToCustomBlockingListeners(const MenuAction& p_action)
{
	for (ActionListeners::iterator it = m_customBlockingListeners.begin();
	     it != m_customBlockingListeners.end(); ++it)
	{
		if ((*it).second->doAction(p_action))
		{
			MENU_Printf("MenuSystem::sendToCustomBlockingListeners: Action "
			            "'%s' was handled by custom blocking action listener "
			            "'%s'.\n",
			            p_action.getCommand().c_str(), (*it).first.c_str());
			return true;
		}
	}
	
	return false;
}


void MenuSystem::sendTreeMenuPreOpenAction()
{
	// Send an action that the tree-menu is about to be opened
	MenuAction action("menusystem_treemenu_pre_open");
	doSystemAction(action);
}


void MenuSystem::sendMenuPreCreateAction(const std::string& p_menuName)
{
	// Send an action that a menu is about to be created
	MenuAction action("menusystem_menu_pre_create");
	action.addParameter(p_menuName);
	doSystemAction(action);
}


void MenuSystem::sendMenuPostCreateAction(elements::Menu* p_menu)
{
	TT_ASSERTMSG(p_menu != 0, "Invalid menu pointer passed.");
	
	// Send an action that a menu has been created
	MenuAction action("menusystem_menu_post_create");
	action.addParameter(p_menu->getName());
	doSystemAction(action);
	
	// Execute the "on created" action set, if the menu has one
	if (p_menu->hasActionSet("system_on_created"))
	{
		p_menu->executeActionSet("system_on_created");
	}
}


void MenuSystem::sendMenuPreDestroyAction(elements::Menu* p_menu)
{
	// Send an action that a menu is about to be destroyed
	MenuAction action("menusystem_menu_pre_destroy");
	action.addParameter(p_menu->getName());
	doSystemAction(action);
	
	// Execute the "on destroy" action set, if the menu has one
	if (p_menu->hasActionSet("system_on_destroy"))
	{
		p_menu->executeActionSet("system_on_destroy");
	}
}


void MenuSystem::destroyMenu(elements::Menu* p_menu)
{
	//TT_Printf("MenuSystem::destroyMenu: Got menu pointer %p\n", p_menu);
	
	if (p_menu != 0)
	{
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
		const std::string menuName(p_menu->getName());
		
		using memory::HeapMgr;
		TT_Printf("MenuSystem::destroyMenu: Menu '%s': Heap stats before menu "
		          "pre-destroy action:\n", menuName.c_str());
		HeapMgr::dumpHeapStats();
#endif
		
		sendMenuPreDestroyAction(p_menu);
		
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
		TT_Printf("MenuSystem::destroyMenu: Menu '%s': Heap stats after menu "
		          "pre-destroy action (right before menu destruction):\n",
		          menuName.c_str());
		HeapMgr::dumpHeapStats();
#endif
		
		delete p_menu;
		p_menu = 0;
		
#if defined(MENUSYS_MENU_MEMORY_PROFILING)
		TT_Printf("MenuSystem::destroyMenu: Menu '%s': Heap stats after menu "
		          "destruction:\n", menuName.c_str());
		HeapMgr::dumpHeapStats();
#endif
		
		/*
		TT_Printf("\n\nMenuSystem::destroyMenu: MEM USAGE POST DESTROY '%s':\n",
		          menuName.c_str());
		HeapMgr::dumpHeapStats();
		TT_Printf("\n\n");
		//*/
	}
}

// Namespace end
}
}
