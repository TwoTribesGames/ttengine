#include <tt/code/ErrorStatus.h>
#include <tt/menu/elements/ShowOneValueChild.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/ValueDecorator.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

ShowOneValueChild::ShowOneValueChild(const std::string& p_name,
                                     const MenuLayout&  p_layout)
:
ContainerBase<ValueDecorator>(p_name, p_layout),
m_resourcesLoaded(false),
m_forceSelectionRefresh(false)
{
	MENU_CREATION_Printf("ShowOneValueChild::ShowOneValueChild: "
	                     "Element '%s': New ShowOneValueChild.\n",
	                     getName().c_str());
	
	// ShowOneValueChild should *always* have container looping enabled
	m_containerLoopEnabled = true;
}


ShowOneValueChild::~ShowOneValueChild()
{
	MENU_CREATION_Printf("ShowOneValueChild::~ShowOneValueChild: "
	                     "Element '%s': Freeing resources.\n",
	                     getName().c_str());
}


void ShowOneValueChild::loadResources()
{
	m_resourcesLoaded = true;
	updateChildrenResources();
}


void ShowOneValueChild::unloadResources()
{
	ContainerBase<ValueDecorator>::unloadResources();
	m_resourcesLoaded = false;
}


std::string ShowOneValueChild::getValueShowChild() const
{
	const value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->getValue();
	}
	return "";
}


std::string ShowOneValueChild::getChildValue(int p_index) const
{
	TT_ASSERTMSG(p_index >= 0 && p_index < getChildCount(),
	             "Child index %d out of range [0 - %d).",
	             p_index, getChildCount());
	ChildVector::size_type idx = static_cast<ChildVector::size_type>(p_index);
	return m_children.at(idx)->getValue();
}


void ShowOneValueChild::render(const math::PointRect& p_rect, s32 p_z)
{
	// Do not render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		showChild->render(p_rect, p_z);
	}
}


void ShowOneValueChild::doLayout(const math::PointRect& p_rect)
{
	bool force = m_forceSelectionRefresh;
	m_forceSelectionRefresh = false;
	
	// Lay out each child as if it has the entire rectangle to itself
	for (ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		MenuLayoutManager::Elements elem;
		elem.push_back(*it);
		MenuLayoutManager::doLayout(getLayout(), elem, p_rect);
	}
	
	
	// Select an initial child
	setInitialSelection();
	
	m_forceSelectionRefresh = force;
}


s32 ShowOneValueChild::getMinimumWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		// Return the largest minimum width of the children
		s32 width = 0;
		
		for (ChildVector::const_iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			width = std::max(width, (*it)->getMinimumWidth());
		}
		
		return width;
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("Element '%s' has undefined width type!", getName().c_str());
		return 0;
	}
}


s32 ShowOneValueChild::getMinimumHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		// Return the largest minimum height of the children
		s32 height = 0;
		
		for (ChildVector::const_iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			height = std::max(height, (*it)->getMinimumHeight());
		}
		
		return height;
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("Element '%s' has undefined height type!", getName().c_str());
		return 0;
	}
}


s32 ShowOneValueChild::getRequestedWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		// Return the largest requested width of the children
		s32 width = 0;
		
		for (ChildVector::const_iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			width = std::max(width, (*it)->getRequestedWidth());
		}
		
		return width;
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("Element '%s' has undefined width type!", getName().c_str());
		return 0;
	}
}


s32 ShowOneValueChild::getRequestedHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		// Return the largest requested height of the children
		s32 height = 0;
		
		for (ChildVector::const_iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			height = std::max(height, (*it)->getRequestedHeight());
		}
		
		return height;
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("Element '%s' has undefined height type!", getName().c_str());
		return 0;
	}
}


bool ShowOneValueChild::canHaveFocus() const
{
	const value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->canHaveFocus();
	}
	return false;
}


void ShowOneValueChild::setSelected(bool p_selected)
{
	value_type* showChild = getShowChild();
	
	// Make sure all children but the shown child are deselected
	for (ChildVector::iterator it = m_children.begin();
		it != m_children.end(); ++it)
	{
		if (*it != showChild)
		{
			(*it)->setSelected(false);
		}
	}
	
	
	if (showChild != 0)
	{
		showChild->setSelected(p_selected);
	}
	
	MenuElement::setSelected(p_selected);
}


bool ShowOneValueChild::onStylusPressed(s32 p_x, s32 p_y)
{
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->onStylusPressed(p_x, p_y);
	}
	return false;
}


bool ShowOneValueChild::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->onStylusDragging(p_x, p_y, p_isInside);
	}
	return false;
}


bool ShowOneValueChild::onStylusReleased(s32 p_x, s32 p_y)
{
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->onStylusReleased(p_x, p_y);
	}
	return false;
}


bool ShowOneValueChild::onKeyPressed(const MenuKeyboard& p_keys)
{
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->onKeyPressed(p_keys);
	}
	return false;
}


bool ShowOneValueChild::onKeyHold(const MenuKeyboard& p_keys)
{
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->onKeyHold(p_keys);
	}
	return false;
}


bool ShowOneValueChild::onKeyReleased(const MenuKeyboard& p_keys)
{
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->onKeyReleased(p_keys);
	}
	return false;
}


bool ShowOneValueChild::doAction(const MenuElementAction& p_action)
{
	if (ContainerBase<ValueDecorator>::doAction(p_action))
	{
		return true;
	}
	
	TT_ASSERT(p_action.getTargetElement() == getName());
	
	std::string command(p_action.getCommand());
	if (command == "prev_tab")
	{
		selectPreviousChild();
		return true;
	}
	else if (command == "next_tab")
	{
		selectNextChild();
		return true;
	}
	else if (command == "select_tab")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes 1 parameter: "
		             "the index of the tab to select.",
		             command.c_str());
		
		TT_ERR_CREATE(command);
		int tab = static_cast<int>(str::parseS32(p_action.getParameter(0), &errStatus));
		TT_ASSERTMSG(errStatus.hasError() == false, "Invalid tab specified: '%s'",
		             p_action.getParameter(0).c_str());
		
		selectChildByIndex(tab);
		return true;
	}
	else if (command == "set_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'set_value' command takes 1 parameter.");
		
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(0),
		                                      getValueShowChild());
		return true;
	}
	else if (command == "show_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "ShowOneValueChild '%s': Command 'show_value' takes 1 "
		             "parameter.", getName().c_str());
		selectChildByValue(p_action.getParameter(0));
		return true;
	}
	else if (command == "set_value_visible")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "ShowOneValueChild '%s': Command 'set_value_visible' "
		             "takes two parameters: "
		             "the value to show or hide, and whether the value should "
		             "be visible.", getName().c_str());
		
		std::string value(p_action.getParameter(0));
		
		// Parse the visibility parameter
		TT_ERR_CREATE(command);
		bool visible = str::parseBool(p_action.getParameter(1), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid visible value: '%s'", p_action.getParameter(1).c_str());
		}
		
		// Find the child with the specified value
		for (ChildVector::iterator it = m_children.begin(); it != m_children.end(); ++it)
		{
			if ((*it)->getValue() == value)
			{
				(*it)->setVisible(visible);
				return true;
			}
		}
		
		TT_PANIC("ShowOneValueChild '%s': Command '%s': No child found with value '%s'!",
		         getName().c_str(), command.c_str(), value.c_str());
		return true;
	}
	else if (command == "get_name_from_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "ShowOneValueChild '%s': Command '%s' command takes 2 "
		             "parameters: "
		             "the value for which to find the child and the menu "
		             "variable in which to store the name.",
		             getName().c_str(), command.c_str());
		
		// Find the child with the specified value
		std::string value(p_action.getParameter(0));
		for (ChildVector::iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			if ((*it)->getValue() == value)
			{
				MenuSystem::getInstance()->setMenuVar(p_action.getParameter(1),
				                                      (*it)->getName());
				return true;
			}
		}
		
		TT_PANIC("ShowOneValueChild '%s': Command '%s': No child found with "
		         "value '%s'!",
		         getName().c_str(), command.c_str(), value.c_str());
		return true;
	}
	else if (command == "get_name_from_value_sysvar")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "ShowOneValueChild '%s': Command '%s' command takes 2 "
		             "parameters: "
		             "the value for which to find the child and the system "
		             "variable in which to store the name.",
		             getName().c_str(), command.c_str());
		
		// Find the child with the specified value
		std::string value(p_action.getParameter(0));
		for (ChildVector::iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			if ((*it)->getValue() == value)
			{
				MenuSystem::getInstance()->setSystemVar(p_action.getParameter(1),
				                                        (*it)->getName());
				return true;
			}
		}
		
		TT_PANIC("ShowOneValueChild '%s': Command '%s': No child found with "
		         "value '%s'!",
		         getName().c_str(), command.c_str(), value.c_str());
		return true;
	}
	
	// Return action NOT handled
	return false;
}


void ShowOneValueChild::setForceSelectionRefresh(bool p_force)
{
	m_forceSelectionRefresh = p_force;
}


void ShowOneValueChild::setInitialChildByValue(const std::string& p_value)
{
	m_initialChildValue = p_value;
}


void ShowOneValueChild::selectChildByIndex(int p_index, bool p_forceSelected)
{
	int currentSelection = getSelectedChildIndex();
	
	ContainerBase<ValueDecorator>::selectChildByIndex(p_index, p_forceSelected);
	
	if (p_index != currentSelection)
	{
		updateChildrenResources();
		
		if (m_forceSelectionRefresh)
		{
			if (getParent() != 0)
			{
				getParent()->recalculateChildSelection();
			}
		}
	}
	
	// Update selection
	setSelected(isSelected());
}


void ShowOneValueChild::selectChildByValue(const std::string& p_value)
{
	// Select the child with the specified value
	int index = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		if ((*it)->getValue() == p_value)
		{
			if (isSelectableElement(*it))
			{
				selectChildByIndex(index);
			}
			else
			{
				// Cannot select child; select the first selectable one instead
				int first_child = getFirstSelectableChildIndex();
				if (first_child != -1)
				{
					selectChildByIndex(first_child);
				}
			}
			return;
		}
	}
	
	// Child not found
	TT_PANIC("ShowOneValueChild '%s' does not have a child with value '%s'.",
	         getName().c_str(), p_value.c_str());
}


void ShowOneValueChild::setContainerLoopEnable(
		bool p_enabled,
		MenuLayout::OrderType p_parentOrder)
{
	// Pass the flag on to all children (so they can decide what to do with it)
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->setContainerLoopEnable(p_enabled, p_parentOrder);
	}
}


ShowOneValueChild* ShowOneValueChild::clone() const
{
	return new ShowOneValueChild(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

ShowOneValueChild::ShowOneValueChild(const ShowOneValueChild& p_rhs)
:
ContainerBase<ValueDecorator>(p_rhs),
m_initialChildValue(p_rhs.m_initialChildValue),
m_resourcesLoaded(p_rhs.m_resourcesLoaded),
m_forceSelectionRefresh(p_rhs.m_forceSelectionRefresh)
{
}


//------------------------------------------------------------------------------
// Protected member functions

bool ShowOneValueChild::setInitialSelection()
{
	// Select the first child, child by value or child by name
	TT_ASSERTMSG(m_initialChildName.empty() || m_initialChildValue.empty(),
	             "ShowOneValueChild '%s': Cannot specify both an "
	             "initial child by name ('%s') AND by value ('%s').",
	             getName().c_str(),
	             m_initialChildName.c_str(),
	             m_initialChildValue.c_str());
	
	if (m_initialChildValue.empty() == false)
	{
		selectChildByValue(m_initialChildValue);
		if (getSelectedChildIndex() != -1)
		{
			// Selection succeeded
			return true;
		}
	}
	
	// Use default behavior
	return ContainerBase<ValueDecorator>::setInitialSelection();
}


bool ShowOneValueChild::isSelectableElement(const value_type* p_element) const
{
	// Default behavior: Element is selectable if it is visible
	return p_element->isVisible();
}


//------------------------------------------------------------------------------
// Private member functions

const ShowOneValueChild::value_type* ShowOneValueChild::getShowChild() const
{
	return m_focusChild;
}


ShowOneValueChild::value_type* ShowOneValueChild::getShowChild()
{
	return m_focusChild;
}


void ShowOneValueChild::updateChildrenResources()
{
	if (m_resourcesLoaded == false)
	{
		return;
	}
	
	value_type* shown = getShowChild();
	
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		if ((*it) == shown)
		{
			(*it)->loadResources();
		}
		else
		{
			(*it)->unloadResources();
		}
	}
}

// Namespace end
}
}
}
