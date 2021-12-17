#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/toStr.h>

#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

MenuElement::MenuElement(const std::string& p_name,
                         const MenuLayout&  p_layout)
:
m_name(p_name),
m_layout(p_layout),
m_canHaveFocus(false),
m_isSelected(false),
m_isDefaultSelected(false),
m_enabled(true),
m_visible(true),
m_stylusOnly(false),
m_minimumWidth(0),
m_minimumHeight(0),
m_requestedWidth(0),
m_requestedHeight(0),
m_requestedX(0),
m_requestedY(0),
m_depth(0),
m_wantCursor(SelectionCursorType_Both),
m_rectangle(math::Point2(0, 0), 1, 1),
m_parent(0)
{
	MENU_CREATION_Printf("MenuElement::MenuElement: Element '%s': "
	                     "Constructed.\n", m_name.c_str());
}


MenuElement::~MenuElement()
{
	MENU_CREATION_Printf("MenuElement::~MenuElement: Element '%s': "
	                     "Destructing.\n", m_name.c_str());
}


void MenuElement::loadResources()
{
	// Do nothing by default
}


void MenuElement::unloadResources()
{
	// Do nothing by default
}


std::string MenuElement::getName() const
{
	return m_name;
}


void MenuElement::doLayout(const PointRect& /* p_rect */)
{
	// Do nothing by default
}


void MenuElement::dumpLayout() const
{
	MENU_Printf("MenuElement::dumpLayout: Element '%s': Width: ",
	            m_name.c_str());
	
	switch (m_layout.getWidthType())
	{
	case MenuLayout::Size_Absolute:
		MENU_Printf("%d\n", m_layout.getWidth());
		break;
		
	case MenuLayout::Size_Auto:
		MENU_Printf("auto\n");
		break;
		
	case MenuLayout::Size_Max:
		MENU_Printf("max\n");
		break;
		
	default:
		MENU_Printf("undefined\n");
		break;
	}
	
	MENU_Printf("MenuElement::dumpLayout: Element '%s': Height: ",
	            m_name.c_str());
	switch (m_layout.getHeightType())
	{
	case MenuLayout::Size_Absolute:
		MENU_Printf("%d\n", m_layout.getHeight());
		break;
		
	case MenuLayout::Size_Auto:
		MENU_Printf("auto\n");
		break;
		
	case MenuLayout::Size_Max:
		MENU_Printf("max\n");
		break;
		
	default:
		MENU_Printf("undefined\n");
		break;
	}
	
	MENU_Printf("MenuElement::dumpLayout: Element '%s': Horizontal position type: ",
	            m_name.c_str());
	switch (m_layout.getHorizontalPositionType())
	{
	case MenuLayout::Position_Left:
		MENU_Printf("left\n");
		break;
		
	case MenuLayout::Position_Center:
		MENU_Printf("center\n");
		break;
		
	case MenuLayout::Position_Right:
		MENU_Printf("right\n");
		break;
		
	default:
		MENU_Printf("undefined\n");
		break;
	}
	
	MENU_Printf("MenuElement::dumpLayout: Element '%s': Vertical position type: ",
	            m_name.c_str());
	switch (m_layout.getVerticalPositionType())
	{
	case MenuLayout::Position_Top:
		MENU_Printf("top\n");
		break;
		
	case MenuLayout::Position_Center:
		MENU_Printf("center\n");
		break;
		
	case MenuLayout::Position_Bottom:
		MENU_Printf("bottom\n");
		break;
		
	default:
		MENU_Printf("undefined\n");
		break;
	}
}


void MenuElement::render(const PointRect& /* p_rect */, s32 /* p_z */)
{
	// Do nothing by default
}


void MenuElement::update()
{
	// Do nothing by default
}


void MenuElement::addAction(const MenuAction& p_action)
{
	m_actions.push_back(p_action);
}


int MenuElement::getActionCount() const
{
	return static_cast<int>(m_actions.size());
}


MenuAction MenuElement::getAction(int p_index) const
{
	TT_ASSERTMSG(p_index >= 0 && p_index < static_cast<int>(m_actions.size()),
	             "Action index out of range: %d", p_index);
	return m_actions.at(static_cast<Actions::size_type>(p_index));
}


void MenuElement::clearActions()
{
	m_actions.clear();
}


s32 MenuElement::getMinimumWidth() const
{
	switch (m_layout.getWidthType())
	{
	case MenuLayout::Size_Auto:
	case MenuLayout::Size_Max:
		return m_minimumWidth;
		
	case MenuLayout::Size_Absolute:
		return m_layout.getWidth();
		
	default:
		TT_PANIC("Menu element '%s' has undefined width type!",
		         getName().c_str());
		break;
	}
	
	return 0;
}


s32 MenuElement::getMinimumHeight() const
{
	switch (m_layout.getHeightType())
	{
	case MenuLayout::Size_Auto:
	case MenuLayout::Size_Max:
		return m_minimumHeight;
		
	case MenuLayout::Size_Absolute:
		return m_layout.getHeight();
		
	default:
		TT_PANIC("Menu element '%s' has undefined height type!",
		         getName().c_str());
		break;
	}
	
	return 0;
}


s32 MenuElement::getRequestedWidth() const
{
	switch (m_layout.getWidthType())
	{
	case MenuLayout::Size_Auto:
	case MenuLayout::Size_Max:
		return m_requestedWidth;
		
	case MenuLayout::Size_Absolute:
		return m_layout.getWidth();
		
	default:
		TT_PANIC("Menu element '%s' has undefined width type!",
		         getName().c_str());
		break;
	}
	
	return 0;
}


s32 MenuElement::getRequestedHeight() const
{
	switch (m_layout.getHeightType())
	{
	case MenuLayout::Size_Auto:
	case MenuLayout::Size_Max:
		return m_requestedHeight;
		
	case MenuLayout::Size_Absolute:
		return m_layout.getHeight();
		
	default:
		TT_PANIC("Menu element '%s' has undefined height type!",
		         getName().c_str());
		break;
	}
	
	return 0;
}


s32 MenuElement::getRequestedHorizontalPosition() const
{
	return m_requestedX;
}


s32 MenuElement::getRequestedVerticalPosition() const
{
	return m_requestedY;
}


MenuLayout& MenuElement::getLayout()
{
	return m_layout;
}


const MenuLayout& MenuElement::getLayout() const
{
	return m_layout;
}


bool MenuElement::canHaveFocus() const
{
	// Element can receive focus if the element indicates it can,
	// it's enabled and it's visible
	return m_canHaveFocus && isEnabled() && isVisible();
}


bool MenuElement::isSelected() const
{
	return m_isSelected;
}


bool MenuElement::isDefaultSelected() const
{
	return m_isDefaultSelected;
}


bool MenuElement::isEnabled() const
{
	return m_enabled;
}


bool MenuElement::isVisible() const
{
	return m_visible;
}


SelectionCursorType MenuElement::wantCursor() const
{
	return m_wantCursor;
}

bool MenuElement::isStylusOnly() const
{
	return m_stylusOnly;
}


void MenuElement::setSelected(bool p_selected)
{
	m_isSelected = p_selected;
}


void MenuElement::setDefaultSelected(bool p_selected)
{
	m_isDefaultSelected = p_selected;
}


void MenuElement::setEnabled(bool p_enabled)
{
	m_enabled = p_enabled;
	
	if (getParent() != 0)
	{
		getParent()->recalculateChildSelection();
	}
}


void MenuElement::setVisible(bool p_visible)
{
	m_visible = p_visible;
	
	if (getParent() != 0)
	{
		getParent()->recalculateChildSelection();
	}
}


void MenuElement::setWantCursor(SelectionCursorType p_want_cursor)
{
	m_wantCursor = p_want_cursor;
}


void MenuElement::setStylusOnly(bool p_stylusOnly)
{
	m_stylusOnly = p_stylusOnly;
}


void MenuElement::setUserLoopEnable(bool /* p_enabled */)
{
}


void MenuElement::setContainerLoopEnable(bool /* p_enabled */,
                                         MenuLayout::OrderType /* p_parentOrder */)
{
}


bool MenuElement::isUserLoopEnabled() const
{
	return true;
}


bool MenuElement::isContainerLoopEnabled() const
{
	return true;
}


bool MenuElement::onStylusPressed(s32 /* p_x */, s32 /* p_y */)
{
	// Input is not handled by default
	return false;
}


bool MenuElement::onStylusDragging(s32 /* p_x */, s32 /* p_y */, bool /* p_isInside */)
{
	// Input is not handled by default
	return false;
}


bool MenuElement::onStylusReleased(s32 /* p_x */, s32 /* p_y */)
{
	// Input is not handled by default
	return false;
}


bool MenuElement::onStylusRepeat(s32 p_x, s32 p_y)
{
	// Input is not handled by default
	bool ret = onStylusReleased(p_x, p_y);
	if (onStylusPressed(p_x, p_y))
	{
		if (ret == false)
		{
			ret = true;
		}
	}
	
	return ret;
}


bool MenuElement::onKeyPressed(const MenuKeyboard& /* p_keys */)
{
	// Input is not handled by default
	return false;
}


bool MenuElement::onKeyHold(const MenuKeyboard& /* p_keys */)
{
	// Input is not handled by default
	return false;
}


bool MenuElement::onKeyReleased(const MenuKeyboard& /* p_keys */)
{
	// Input is not handled by default
	return false;
}


bool MenuElement::onKeyRepeat(const MenuKeyboard& p_keys)
{
	// Input is not handled by default
	bool ret = onKeyReleased(p_keys);
	if (onKeyPressed(p_keys))
	{
		if (ret == false)
		{
			ret = true;
		}
	}
	
	return ret;
}


bool MenuElement::doAction(const MenuElementAction& p_action)
{
	// Actions that apply to all menu elements are handled here
	std::string command(p_action.getCommand());
	
	if (command == "set_enabled")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "whether to enable the element.",
		             command.c_str());
		
		bool        enabled = true;
		std::string enabledParam(p_action.getParameter(0));
		
		if (enabledParam == "true")
		{
			enabled = true;
		}
		else if (enabledParam == "false")
		{
			enabled = false;
		}
		else
		{
			TT_PANIC("Element '%s': Action '%s': Invalid enabled Boolean value: %s",
			         getName().c_str(), command.c_str(), enabledParam.c_str());
		}
		
		setEnabled(enabled);
		
		return true;
	}
	else if (command == "set_visible")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "whether to show the element.",
		             command.c_str());
		
		bool        visible = true;
		std::string visibleParam(p_action.getParameter(0));
		
		if (visibleParam == "true")
		{
			visible = true;
		}
		else if (visibleParam == "false")
		{
			visible = false;
		}
		else
		{
			TT_PANIC("Element '%s': Action '%s': Invalid visible Boolean value: %s",
			         getName().c_str(), command.c_str(), visibleParam.c_str());
		}
		
		setVisible(visible);
		
		return true;
	}
	else if (command == "get_enabled")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the menu variable to store the enabled state in.",
		             command.c_str());
		
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(0),
		                                      str::toStr(isEnabled()));
		
		return true;
	}
	else if (command == "get_visible")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the menu variable to store the visible state in.",
		             command.c_str());
		
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(0),
		                                      str::toStr(isVisible()));
		
		return true;
	}
	else if (command == "perform_actions")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Element command '%s' takes no parameters.",
		             command.c_str());
		
		performActions();
		
		return true;
	}
	
	return false;
}


MenuElementInterface* MenuElement::getMenuElement(const std::string& p_name)
{
	if (getName() == p_name)
	{
		return this;
	}
	
	return 0;
}


MenuElementInterface* MenuElement::getSelectedElement()
{
	if (isSelected())
	{
		return this;
	}
	return 0;
}


const MenuElementInterface* MenuElement::getSelectedElement() const
{
	if (isSelected())
	{
		return this;
	}
	return 0;
}


bool MenuElement::getSelectedElementRect(PointRect& p_rect) const
{
	// If we're not selected, it is not our rectangle that is requested
	if (isSelected() == false)
	{
		return false;
	}
	
	// Set the rectangle to our rectangle, offset by the given rectangle
	const PointRect& r(getRectangle());
	p_rect.translate(r.getPosition());
	p_rect.setWidth(r.getWidth());
	p_rect.setHeight(r.getHeight());
	
	return true;
}


const PointRect& MenuElement::getRectangle() const
{
	return m_rectangle;
}


void MenuElement::setRectangle(const PointRect& p_rect)
{
	m_rectangle = p_rect;
}


s32 MenuElement::getDepth() const
{
	return m_depth;
}


void MenuElement::onLayoutDone()
{
}


void MenuElement::onMenuActivated()
{
}


void MenuElement::onMenuDeactivated()
{
}


void MenuElement::setSelectionPath(SelectionPath& /* p_path */)
{
	// By default, menu elements do not form part of the selection path
	// (so do nothing here)
}


void MenuElement::getSelectionPath(SelectionPath& /* p_path */) const
{
	// By default, menu elements do not form part of the selection path
	// (so do nothing here)
}


bool MenuElement::getSelectionPathForElement(
		SelectionPath&     /* p_path */,
		const std::string& /* p_name */) const
{
	return false;
}


void MenuElement::dumpSelectionTree(int p_treeLevel) const
{
	// Nothing to do here
	std::string indent(static_cast<std::string::size_type>(p_treeLevel), '-');
	TT_Printf("%s < Leaf '%s'. Selected: %s >\n",
	          indent.c_str(), getName().c_str(),
	          isSelected() ? "yes" : "no");
}


MenuElement* MenuElement::clone() const
{
	return new MenuElement(*this);
}


void MenuElement::setParent(MenuElementInterface* p_parent)
{
	m_parent = p_parent;
}


MenuElementInterface* MenuElement::getParent()
{
	return m_parent;
}


const MenuElementInterface* MenuElement::getParent() const
{
	return m_parent;
}


MenuElementInterface* MenuElement::getRoot()
{
	// If we have no parent, we are the root
	if (getParent() == 0)
	{
		return this;
	}
	
	// Move up the tree to find the top-most element
	MenuElementInterface* root = getParent();
	while (root->getParent() != 0)
	{
		root = root->getParent();
	}
	
	return root;
}


const MenuElementInterface* MenuElement::getRoot() const
{
	// If we have no parent, we are the root
	if (getParent() == 0)
	{
		return this;
	}
	
	// Move up the tree to find the top-most element
	const MenuElementInterface* root = getParent();
	while (root->getParent() != 0)
	{
		root = root->getParent();
	}
	
	return root;
}


void MenuElement::recalculateChildSelection()
{
	// By default, menu elements cannot have any children,
	// so there is nothing to do here.
}


//------------------------------------------------------------------------------
// Protected member functions

MenuElement::MenuElement(const MenuElement& p_rhs)
:
m_name(p_rhs.m_name),
m_layout(p_rhs.m_layout),
m_canHaveFocus(p_rhs.m_canHaveFocus),
m_isSelected(p_rhs.m_isSelected),
m_isDefaultSelected(p_rhs.m_isDefaultSelected),
m_enabled(p_rhs.m_enabled),
m_visible(p_rhs.m_visible),
m_stylusOnly(p_rhs.m_stylusOnly),
m_actions(p_rhs.m_actions),
m_minimumWidth(p_rhs.m_minimumWidth),
m_minimumHeight(p_rhs.m_minimumHeight),
m_requestedWidth(p_rhs.m_requestedWidth),
m_requestedHeight(p_rhs.m_requestedHeight),
m_requestedX(p_rhs.m_requestedX),
m_requestedY(p_rhs.m_requestedY),
m_depth(p_rhs.m_depth),
m_wantCursor(p_rhs.m_wantCursor),
m_rectangle(p_rhs.m_rectangle),
m_parent(p_rhs.m_parent)
{
}


void MenuElement::setMinimumWidth(s32 p_minimumWidth)
{
	m_minimumWidth = p_minimumWidth;
}


void MenuElement::setMinimumHeight(s32 p_minimumHeight)
{
	m_minimumHeight = p_minimumHeight;
}


void MenuElement::setRequestedWidth(s32 p_requestedWidth)
{
	m_requestedWidth = p_requestedWidth;
}


void MenuElement::setRequestedHeight(s32 p_requestedHeight)
{
	m_requestedHeight = p_requestedHeight;
}


void MenuElement::setRequestedPositionX(s32 p_requestedX)
{
	m_requestedX = p_requestedX;
}


void MenuElement::setRequestedPositionY(s32 p_requestedY)
{
	m_requestedY = p_requestedY;
}


void MenuElement::setCanHaveFocus(bool p_canHaveFocus)
{
	m_canHaveFocus = p_canHaveFocus;
}


void MenuElement::setDepth(s32 p_depth)
{
	m_depth = p_depth;
}


void MenuElement::performActions()
{
	MenuSystem* menuSystem = MenuSystem::getInstance();
	menuSystem->doActions(m_actions);
}


bool MenuElement::shouldHandleInput() const
{
	return isVisible() && isEnabled();
}

// Namespace end
}
}
}
