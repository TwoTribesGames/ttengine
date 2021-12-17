#include <sstream>

#include <tt/code/ErrorStatus.h>
#include <tt/menu/elements/ShowOneChild.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

ShowOneChild::ShowOneChild(const std::string& p_name,
                           const MenuLayout&  p_layout)
:
ContainerBase<>(p_name, p_layout),
m_resourcesLoaded(false),
m_forceSelectionRefresh(false)
{
	MENU_CREATION_Printf("ShowOneChild::ShowOneChild: Element '%s': "
	                     "New ShowOneChild.\n", getName().c_str());
	
	// ShowOneChild should *always* have container looping enabled
	m_containerLoopEnabled = true;
}


ShowOneChild::~ShowOneChild()
{
	MENU_CREATION_Printf("ShowOneChild::~ShowOneChild: Element '%s': "
	                     "Freeing resources.\n", getName().c_str());
}


void ShowOneChild::loadResources()
{
	m_resourcesLoaded = true;
	updateChildrenResources();
}


void ShowOneChild::unloadResources()
{
	m_resourcesLoaded = false;
	ContainerBase<>::unloadResources();
}


std::string ShowOneChild::getValueShowChild() const
{
	// Return the index of the selected child
	std::ostringstream oss;
	oss << m_focusChildIndex;
	return oss.str();
}


void ShowOneChild::render(const math::PointRect& p_rect, s32 p_z)
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


void ShowOneChild::update()
{
	value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		showChild->update();
	}
}


void ShowOneChild::doLayout(const math::PointRect& p_rect)
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


s32 ShowOneChild::getMinimumWidth() const
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


s32 ShowOneChild::getMinimumHeight() const
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


s32 ShowOneChild::getRequestedWidth()  const
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


s32 ShowOneChild::getRequestedHeight() const
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


bool ShowOneChild::canHaveFocus() const
{
	const value_type* showChild = getShowChild();
	if (showChild != 0)
	{
		return showChild->canHaveFocus();
	}
	return false;
}


void ShowOneChild::setSelected(bool p_selected)
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
	
	// NOTE: Shouldn't this set selection for all children,
	//       or remember selection and update it for
	//       children upon child change?
	if (showChild != 0)
	{
		showChild->setSelected(p_selected);
	}
	
	MenuElement::setSelected(p_selected);
}


bool ShowOneChild::onStylusPressed(s32 p_x, s32 p_y)
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


bool ShowOneChild::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
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


bool ShowOneChild::onStylusReleased(s32 p_x, s32 p_y)
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


bool ShowOneChild::onKeyPressed(const MenuKeyboard& p_keys)
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


bool ShowOneChild::onKeyHold(const MenuKeyboard& p_keys)
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


bool ShowOneChild::onKeyReleased(const MenuKeyboard& p_keys)
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


bool ShowOneChild::doAction(const MenuElementAction& p_action)
{
	if (ContainerBase<>::doAction(p_action))
	{
		return true;
	}
	
	TT_ASSERTMSG(p_action.getTargetElement() == getName(),
	             "Menu element action '%s' wasn't meant for this element "
	             "('%s'), but for '%s'.", p_action.getCommand().c_str(),
	             getName().c_str(), p_action.getTargetElement().c_str());
	
	if (p_action.getCommand() == "prev_tab")
	{
		selectPreviousChild();
		return true;
	}
	else if (p_action.getCommand() == "next_tab")
	{
		selectNextChild();
		return true;
	}
	else if (p_action.getCommand() == "select_tab")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'select_tab' takes 1 parameter: "
		             "the index of the tab to select.");
		
		TT_ERR_CREATE(p_action.getCommand());
		int tab = static_cast<int>(str::parseS32(p_action.getParameter(0), &errStatus));
		TT_ASSERTMSG(errStatus.hasError() == false, "Invalid tab index specified: '%s'",
		             p_action.getParameter(0).c_str());
		
		setShowChild(tab);
		
		return true;
	}
	else if (p_action.getCommand() == "set_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "ShowOneChild '%s': Command 'set_value' "
		             "takes 1 parameter.", getName().c_str());
		
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(0),
		                                      getValueShowChild());
		return true;
	}
	
	// Return action NOT handled
	return false;
}


ShowOneChild* ShowOneChild::clone() const
{
	return new ShowOneChild(*this);
}


void ShowOneChild::setShowChild(int p_index)
{
	TT_ASSERTMSG(p_index >= 0 && p_index < getChildCount(),
	             "Child index %d out of range [0 - %d).",
	             p_index, getChildCount());
	
	selectChildByIndex(p_index);
}


void ShowOneChild::setForceSelectionRefresh(bool p_force)
{
	m_forceSelectionRefresh = p_force;
}


void ShowOneChild::selectChildByIndex(int p_index, bool p_forceSelected)
{
	int currentSelection = getSelectedChildIndex();
	
	ContainerBase<>::selectChildByIndex(p_index, p_forceSelected);
	
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


void ShowOneChild::setContainerLoopEnable(bool                  p_enabled,
                                          MenuLayout::OrderType p_parentOrder)
{
	// Pass the flag on to all children (so they can decide what to do with it)
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->setContainerLoopEnable(p_enabled, p_parentOrder);
	}
}


//------------------------------------------------------------------------------
// Protected member functions

ShowOneChild::ShowOneChild(const ShowOneChild& p_rhs)
:
ContainerBase<>(p_rhs),
m_resourcesLoaded(p_rhs.m_resourcesLoaded),
m_forceSelectionRefresh(p_rhs.m_forceSelectionRefresh)
{
}


bool ShowOneChild::isSelectableElement(const value_type* p_element) const
{
	// Default behavior: Element is selectable if it is visible
	return p_element->isVisible();
}


//------------------------------------------------------------------------------
// Private member functions

const ShowOneChild::value_type* ShowOneChild::getShowChild() const
{
	return m_focusChild;
}


ShowOneChild::value_type* ShowOneChild::getShowChild()
{
	return m_focusChild;
}


void ShowOneChild::updateChildrenResources()
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
