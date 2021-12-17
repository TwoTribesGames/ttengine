#include <tt/code/ErrorStatus.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuLayoutManager.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
//#include <tt/system/Pad.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

template<typename ChildType>
ContainerBase<ChildType>::ContainerBase(const std::string& p_name,
                                        const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout),
m_focusChild(0),
m_stylusFocusChild(0),
m_focusChildIndex(-1),
m_userLoopEnabled(true),
m_containerLoopEnabled(true)
{
	MENU_CREATION_Printf("ContainerBase::ContainerBase: Element '%s': "
	                     "New ContainerBase.\n", getName().c_str());
}


template<typename ChildType>
ContainerBase<ChildType>::~ContainerBase()
{
	MENU_CREATION_Printf("ContainerBase::~ContainerBase: Element '%s': "
	                     "Freeing memory for all children.\n",
	                     getName().c_str());
	
	removeChildren();
}


template<typename ChildType>
void ContainerBase<ChildType>::loadResources()
{
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->loadResources();
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::unloadResources()
{
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->unloadResources();
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::render(const math::PointRect& p_rect, s32 p_z)
{
	if (isVisible())
	{
		renderChildren(m_children, p_rect, p_z);
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::update()
{
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->update();
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::doLayout(const math::PointRect& p_rect)
{
	// Lay out the children
	{
		MenuLayoutManager::Elements elements(
			MenuLayoutManager::getElements(m_children));
		MenuLayoutManager::doLayout(getLayout(), elements, p_rect);
	}
	
	// Select an initial child
	setInitialSelection();
}


template<typename ChildType>
void ContainerBase<ChildType>::dumpLayout() const
{
	// Dump the layout info for this element first
	MenuElement::dumpLayout();
	
	// Dump the layout info for all children
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		const math::PointRect& rect((*it)->getRectangle());
		MENU_Printf("\nContainerBase<ChildType>::dumpLayout: ---- Layout for element '%s' ----\n",
		            (*it)->getName().c_str());
		MENU_Printf("ContainerBase<ChildType>::dumpLayout:   Width  : %d\n", rect.getWidth());
		MENU_Printf("ContainerBase<ChildType>::dumpLayout:   Height : %d\n", rect.getHeight());
		MENU_Printf("ContainerBase<ChildType>::dumpLayout:   X      : %d\n", rect.getX());
		MENU_Printf("ContainerBase<ChildType>::dumpLayout:   Y      : %d\n", rect.getY());
		(*it)->dumpLayout();
		(void)rect;
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::addChild(typename ContainerBase<ChildType>::value_type* p_child)
{
	m_children.push_back(p_child);
	p_child->setParent(this);
}


template<typename ChildType>
void ContainerBase<ChildType>::removeChildren()
{
	// Clear the children vector
	ChildVector children(m_children);
	m_children.clear();
	
	// Free memory for all children
	for (typename ChildVector::iterator it = children.begin();
	     it != children.end(); ++it)
	{
		delete *it;
	}
	
	// Reset the focus child
	m_focusChild       = 0;
	m_focusChildIndex = -1;
	//selectChildByIndex(-1);
	MenuSystem::getInstance()->resetSelectedElement();
}


template<typename ChildType>
bool ContainerBase<ChildType>::doAction(const MenuElementAction& p_action)
{
	// Allow base to handle action first
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	
	std::string command(p_action.getCommand());
	if (command == "get_selected_child_index")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the system variable to store the selected child index in.",
		             command.c_str());
		
		// Get selected child index
		std::string selChild(str::toStr(getSelectedChildIndex()));
		MenuSystem::getInstance()->setSystemVar(p_action.getParameter(0), selChild);
		
		return true;
	}
	else if (command == "get_selected_child_name")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the system variable to store the selected child name in.",
		             command.c_str());
		
		// Get selected child name (empty string if no selection)
		std::string selChild;
		if (m_focusChild != 0)
		{
			selChild = m_focusChild->getName();
		}
		MenuSystem::getInstance()->setSystemVar(p_action.getParameter(0),
		                                        selChild);
		
		return true;
	}
	else if (command == "set_selected_child_index")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the index of the child to select.",
		             command.c_str());
		
		// Get specified index
		TT_ERR_CREATE(command);
		int childIndex = static_cast<int>(str::parseS32(p_action.getParameter(0), &errStatus));
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid child index specified: '%s'",
			         p_action.getParameter(0).c_str());
			childIndex = getSelectedChildIndex();
		}
		
		// Range check index
		if (childIndex < 0 || childIndex >= getChildCount())
		{
			TT_PANIC("Child index %d out of range [0 - %d).",
			         childIndex, getChildCount());
		}
		else
		{
			selectChildByIndex(childIndex);
		}
		
		return true;
	}
	else if (command == "set_selected_child_name")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the name of the child to select.",
		             command.c_str());
		
		// Select child by name
		selectChildByName(p_action.getParameter(0));
		
		return true;
	}
	else if (command == "select_first")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Element command '%s' takes no parameters.",
		             command.c_str());
		
		// Select first selectable child
		int index = getFirstSelectableChildIndex();
		if (index != -1)
		{
			selectChildByIndex(index);
		}
		
		return true;
	}
	
	return false;
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getMinimumWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		return getMinimumChildrenWidth(m_children);
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("Element '%s' has unsupported width type!",
		         getName().c_str());
		return 0;
	}
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getMinimumHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		return getMinimumChildrenHeight(m_children);
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("Element '%s' has unsupported height type!",
		         getName().c_str());
		return 0;
	}
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getRequestedWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		return getRequestedChildrenWidth(m_children);
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("Element '%s' has unsupported width type!",
		         getName().c_str());
		return 0;
	}
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getRequestedHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		return getRequestedChildrenHeight(m_children);
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("Element '%s' has unsupported height type!",
		         getName().c_str());
		return 0;
	}
}


template<typename ChildType>
bool ContainerBase<ChildType>::canHaveFocus() const
{
	// If the container can receive focus, no need to check further
	if (MenuElement::canHaveFocus())
	{
		return true;
	}
	
	// Check if any of the children can receive focus
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		//if ((*it)->canHaveFocus() && (*it)->isVisible())
		if (isSelectableElement(*it))
		{
			return true;
		}
	}
	
	// None of these elements can receive focus
	return false;
}


template<typename ChildType>
void ContainerBase<ChildType>::setUserLoopEnable(bool p_enabled)
{
	m_userLoopEnabled = p_enabled;
}


template<typename ChildType>
void ContainerBase<ChildType>::setContainerLoopEnable(
		bool                  p_enabled,
		MenuLayout::OrderType p_parentOrder)
{
	//*
	// Only do something if the parent's ordering matches our own
	if (getLayout().getOrder() != p_parentOrder)
	{
		return;
	}
	//*/
	
	// Only do something if the parent's ordering matches our own
	if (getLayout().getOrder() == p_parentOrder)
	{
		// Set our own flag
		m_containerLoopEnabled = p_enabled;
	}
	
	
	//TT_Printf("ContainerBase::setContainerLoopEnable: [%s] Container loop enabled: %s\n",
	//          getName().c_str(), p_enabled ? "yes" : "no");
	
	// Pass it down to all children /* with the same ordering */
	MenuLayout::OrderType ourOrder = getLayout().getOrder();
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		//if ((*it)->getLayout().getOrder() == ourOrder)
		{
			//TT_Printf("ContainerBase::setContainerLoopEnable: [%s] Setting container loop to %s for child '%s'.\n",
			//          getName().c_str(), p_enabled ? "enabled" : "disabled", (*it)->getName().c_str());
			(*it)->setContainerLoopEnable(p_enabled, ourOrder);
		}
	}
}


template<typename ChildType>
bool ContainerBase<ChildType>::isUserLoopEnabled() const
{
	return m_userLoopEnabled;
}


template<typename ChildType>
bool ContainerBase<ChildType>::isContainerLoopEnabled() const
{
	return m_containerLoopEnabled;
}


template<typename ChildType>
void ContainerBase<ChildType>::setSelected(bool p_selected)
{
	//bool newSelection = (isSelected() == false && p_selected);
	
	MenuElement::setSelected(p_selected);
	
	if (p_selected == false)
	{
		// Deselect all children
		for (typename ChildVector::iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			(*it)->setSelected(p_selected);
		}
		
		// Reset the focus child
		m_focusChild       = 0;
		m_focusChildIndex = -1;
		//selectChildByIndex(-1);
	}
	else
	{
		if (m_initialChildName.empty() == false)
		{
			// Select the child by name
			selectChildByName(m_initialChildName);
			if (getSelectedChildIndex() != -1)
			{
				return;
			}
		}
		
		
		{
			int index = 0;
			
			// See if there is a child that has the default focus
			for (typename ChildVector::iterator it = m_children.begin();
			     it != m_children.end(); ++it, ++index)
			{
				if ((*it)->isDefaultSelected())
				{
					/*
					if (m_focusChild != *it)
					{
						if (m_focusChild != 0)
						{
							m_focusChild->setSelected(false);
						}
						
						m_focusChild       = *it;
						m_focusChildIndex = index;
						
						m_focusChild->setSelected(true);
					}
					//*/
					selectChildByIndex(index, true);
					return;
				}
			}
			
			index = 0;
			
			/* FIXME: Pad::keyUp must be adapted to new input code!
			if ((system::Pad::keyUp() &&
			     getLayout().getOrder() == MenuLayout::Order_Vertical) &&
			    newSelection)
			{
				int lastChild = getLastSelectableChildIndex();
				if (lastChild != -1)
				{
					selectChildByIndex(lastChild, true);
				}
			}
			else
			//*/
			{
				// Set the focus to the first focusable child
				for (typename ChildVector::iterator it = m_children.begin();
				     it != m_children.end(); ++it, ++index)
				{
					if (isSelectableElement(*it))
					{
						selectChildByIndex(index, true);
						break;
					}
				}
			}
		}
	}
}


template<typename ChildType>
bool ContainerBase<ChildType>::onStylusPressed(s32 p_x, s32 p_y)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// Check if any of the children handled the input
	int childIndex = 0;
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++childIndex)
	{
		// Check if the input is inside the rectangle for this child
		const math::PointRect& rect((*it)->getRectangle());
		
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			// Check if this child can receive focus
			if (/*(*it)->canHaveFocus() && (*it)->isVisible()*/
			    isSelectableElement(*it))
			{
				if ((*it) != m_focusChild)
				{
					if ((*it)->isStylusOnly())
					{
						// Tapped element is stylus only,
						// so don't change the current selection
						if (m_stylusFocusChild != 0)
						{
							m_stylusFocusChild->setSelected(false);
						}
						m_stylusFocusChild = *it;
						m_stylusFocusChild->setSelected(true);
					}
					else
					{
						// Deselect the previous focus child
						if (m_focusChild != 0)
						{
							m_focusChild->setSelected(false);
						}
						
						// Set the focus to this child
						m_focusChild      = *it;
						m_focusChildIndex = childIndex;
						m_focusChild->setSelected(true);
					}
				}
			}
			
			// Check if the child handled the input
			//if ((*it)->isEnabled())
			{
				if ((*it)->onStylusPressed(p_x - rect.getPosition().x,
				                           p_y - rect.getPosition().y))
				{
					return true;
				}
			}
		}
	}
	
	// No children handled the input, and container doesn't either
	return false;
}


template<typename ChildType>
bool ContainerBase<ChildType>::onStylusDragging(
		s32 p_x, s32 p_y, bool /* p_isInside */)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// Send drag input directly to the child that has focus
	value_type* focus = 0;
	
	if (m_stylusFocusChild != 0)
	{
		focus = m_stylusFocusChild;
	}
	else if (m_focusChild != 0)
	{
		focus = m_focusChild;
	}
	
	if (focus != 0)
	{
		// Check whether the coordinates are inside the child
		const math::PointRect& rect(focus->getRectangle());
		bool                   isInside = false;
		
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			isInside = true;
		}
		
		// Send input to the child
		//if (focus->isEnabled())
		{
			return focus->onStylusDragging(p_x - rect.getPosition().x,
			                               p_y - rect.getPosition().y,
			                               isInside);
		}
	}
	
	// Otherwise ignore the input
	return false;
}


template<typename ChildType>
bool ContainerBase<ChildType>::onStylusReleased(s32 p_x, s32 p_y)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// Send drag input directly to the child that has focus
	value_type* focus = 0;
	
	if (m_stylusFocusChild != 0)
	{
		focus = m_stylusFocusChild;
	}
	else if (m_focusChild != 0)
	{
		focus = m_focusChild;
	}
	
	
	// Send stylus released input directly to the child that has focus
	if (focus != 0)
	{
		// Check whether the coordinates are inside the child
		const math::PointRect& rect(focus->getRectangle());
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			//if (focus->isEnabled())
			{
				// Send input to the child
				focus->onStylusReleased(p_x - rect.getPosition().x,
				                        p_y - rect.getPosition().y);
				
				if (m_stylusFocusChild != 0)
				{
					m_stylusFocusChild->setSelected(false);
					m_stylusFocusChild = 0;
				}
			}
		}
	}
	
	// Otherwise ignore the input
	return false;
}


template<typename ChildType>
bool ContainerBase<ChildType>::onStylusRepeat(s32 p_x, s32 p_y)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// Send drag input directly to the child that has focus
	value_type* focus = 0;
	
	if (m_stylusFocusChild != 0)
	{
		focus = m_stylusFocusChild;
	}
	else if (m_focusChild != 0)
	{
		focus = m_focusChild;
	}
	
	if (focus != 0)
	{
		// Check whether the coordinates are inside the child
		const math::PointRect& rect(focus->getRectangle());
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			// Send input to the child
			//if (focus->isEnabled())
			{
				return focus->onStylusRepeat(p_x - rect.getPosition().x,
				                             p_y - rect.getPosition().y);
			}
		}
		else
		{
			return false;
		}
	}
	
	// Otherwise ignore the input
	return false;
}



template<typename ChildType>
bool ContainerBase<ChildType>::onKeyPressed(const MenuKeyboard& p_keys)
{
	if (isStylusOnly())
	{
		return false;
	}
	
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// Give the selected child the chance to handle the input first
	if (m_focusChild != 0 && m_focusChild->onKeyPressed(p_keys))
	{
		return true;
	}
	
	// Selection can't change if we have no children
	if (m_children.empty())
	{
		return false;
	}
	
	// Check if the selection needs to change
	if (getLayout().getOrder() == MenuLayout::Order_Vertical)
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_DOWN))
		{
			bool handled = selectNextChild();
			if (handled)
			{
				MenuSystem::getInstance()->playSound(MenuSound_FE_HighlightShift);
			}
			return handled;
		}
		else if (p_keys.isKeySet(MenuKeyboard::MENU_UP))
		{
			bool handled = selectPreviousChild();
			if (handled)
			{
				MenuSystem::getInstance()->playSound(MenuSound_FE_HighlightShift);
			}
			return handled;
		}
	}
	else if (getLayout().getOrder() == MenuLayout::Order_Horizontal)
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_RIGHT))
		{
			bool handled = selectNextChild();
			if (handled)
			{
				MenuSystem::getInstance()->playSound(MenuSound_FE_HighlightShift);
			}
			return handled;
		}
		else if (p_keys.isKeySet(MenuKeyboard::MENU_LEFT))
		{
			bool handled = selectPreviousChild();
			if (handled)
			{
				MenuSystem::getInstance()->playSound(MenuSound_FE_HighlightShift);
			}
			return handled;
		}
	}
	
	return false;
}


template<typename ChildType>
bool ContainerBase<ChildType>::onKeyHold(const MenuKeyboard& p_keys)
{
	if (isStylusOnly())
	{
		return false;
	}
	
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_focusChild != 0/* && m_focusChild->isEnabled()*/)
	{
		return m_focusChild->onKeyHold(p_keys);
	}
	
	return false;
}


template<typename ChildType>
bool ContainerBase<ChildType>::onKeyReleased(const MenuKeyboard& p_keys)
{
	if (isStylusOnly())
	{
		return false;
	}
	
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_focusChild != 0 /* && m_focusChild->isEnabled()*/)
	{
		return m_focusChild->onKeyReleased(p_keys);
	}
	
	return false;
}


template<typename ChildType>
bool ContainerBase<ChildType>::onKeyRepeat(const MenuKeyboard& p_keys)
{
	if (isStylusOnly())
	{
		return false;
	}
	
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_focusChild != 0 /* && m_focusChild->isEnabled() */)
	{
		if (m_focusChild->onKeyRepeat(p_keys))
		{
			return true;
		}
	}
	
	return MenuElement::onKeyRepeat(p_keys);
}


template<typename ChildType>
MenuElementInterface* ContainerBase<ChildType>::getMenuElement(
		const std::string& p_name)
{
	MenuElementInterface* element = MenuElement::getMenuElement(p_name);
	if (element == 0)
	{
		for (typename ChildVector::const_iterator it = m_children.begin();
		     it != m_children.end() && element == 0; ++it)
		{
			element = (*it)->getMenuElement(p_name);
		}
	}
	
	return element;
}


template<typename ChildType>
MenuElementInterface* ContainerBase<ChildType>::getSelectedElement()
{
	if (isStylusOnly())
	{
		return 0;
	}
	
	MenuElementInterface* element = 0;
	
	// Begin with focus child
	if (m_focusChild != 0)
	{
		// Focus child should always have selected flag set
		if (m_focusChild->isSelected() == false)
		{
			//TT_Printf("ContainerBase::getSelectedElement: [%s] Focus child (%d, '%s') does not have selection flag set!\n",
			//          getName().c_str(), m_focusChildIndex, m_focusChild->getName().c_str());
		}
		
		element = m_focusChild->getSelectedElement();
		if (element != 0)
		{
			return element;
		}
		
		return m_focusChild;
	}
	
	// Check all children for the deepest selected element
	int index = 0;
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		element = (*it)->getSelectedElement();
		if (element != 0 && element->isStylusOnly() == false)
		{
			if (m_focusChild != element || m_focusChildIndex != index)
			{
				//TT_Printf("ContainerBase::getSelectedElement: [%s] Selection is in incorrect state! "
				//          "Focus child is '%s' (%d), selected is '%s' (%d).\n",
				//          getName().c_str(), m_focusChild != 0 ? m_focusChild->getName().c_str() : "{none}",
				//          m_focusChildIndex, element->getName().c_str(), index);
			}
			
			//TT_Printf("ContainerBase::getSelectedElement: [%s] Selected element is %d ('%s').\n",
			//          getName().c_str(), index, element->getName().c_str());
			return element;
		}
		
		/*
		if ((*it)->isStylusOnly() == false)
		{
			element = (*it)->getSelectedElement();
			if (element != 0)
			{
				return element;
			}
		}
		//*/
	}
	
	// Continue with default MenuElement behavior
	//return MenuElement::getSelectedElement();
	return 0;
}


template<typename ChildType>
const MenuElementInterface* ContainerBase<ChildType>::getSelectedElement() const
{
	if (isStylusOnly())
	{
		return 0;
	}
	
	const MenuElementInterface* element = 0;
	
	// Begin with focus child
	if (m_focusChild != 0)
	{
		// Focus child should always have selected flag set
		if (m_focusChild->isSelected() == false)
		{
			//TT_Printf("ContainerBase::getSelectedElement: [%s] Focus child (%d, '%s') does not have selection flag set!\n",
			//          getName().c_str(), m_focusChildIndex, m_focusChild->getName().c_str());
		}
		
		element = m_focusChild->getSelectedElement();
		if (element != 0)
		{
			return element;
		}
		
		return m_focusChild;
	}
	
	// Check all children for the deepest selected element
	int index = 0;
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		element = (*it)->getSelectedElement();
		if (element != 0 && element->isStylusOnly() == false)
		{
			if (m_focusChild != element || m_focusChildIndex != index)
			{
				//TT_Printf("ContainerBase::getSelectedElement: [%s] Selection is in incorrect state! "
				//          "Focus child is '%s' (%d), selected is '%s' (%d).\n",
				//          getName().c_str(), m_focusChild != 0 ? m_focusChild->getName().c_str() : "{none}",
				//          m_focusChildIndex, element->getName().c_str(), index);
			}
			
			//TT_Printf("ContainerBase::getSelectedElement: [%s] Selected element is %d ('%s').\n",
			//          getName().c_str(), index, element->getName().c_str());
			return element;
		}
	}
	
	// Continue with default MenuElement behavior
	//return MenuElement::getSelectedElement();
	return 0;
}


template<typename ChildType>
bool ContainerBase<ChildType>::getSelectedElementRect(math::PointRect& p_rect) const
{
	if (isStylusOnly())
	{
		return false;
	}
	
	
	// Build a scratch rectangle offset with this container's position
	math::PointRect rect(p_rect);
	rect.translate(getRectangle().getPosition());
	rect.setWidth(getRectangle().getWidth());
	rect.setHeight(getRectangle().getHeight());
	
	if (m_focusChild != 0)
	{
		math::PointRect scratch(rect);
		if (m_focusChild->getSelectedElementRect(scratch))
		{
			p_rect = scratch;
			return true;
		}
		
		return false;
	}
	
	
	// Check all children for the deepest selected element
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		math::PointRect scratch(rect);
		if ((*it)->getSelectedElementRect(scratch))
		{
			p_rect = scratch;
			return true;
		}
	}
	
	// Return our own rectangle if this container is selected
	/*
	// Don't do this! This will cause the selection cursor
	// to point to a container, which is never a good thing.
	if (isSelected())
	{
		p_rect = rect;
		return true;
	}
	//*/
	
	return false;
}


template<typename ChildType>
void ContainerBase<ChildType>::setInitialChildByName(const std::string& p_name)
{
	m_initialChildName = p_name;
}


template<typename ChildType>
void ContainerBase<ChildType>::selectChildByName(const std::string& p_name)
{
	// Select the child with the specified name
	int index = 0;
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		if ((*it)->getName() == p_name)
		{
			if (isSelectableElement(*it))
			{
				selectChildByIndex(index);
				
				MENU_Printf("ContainerBase::selectChildByName: Element '%s': "
				            "Selected child '%s' (index %d).\n",
				            getName().c_str(), p_name.c_str(), index);
				MENU_Printf("ContainerBase::selectChildByName: Element '%s': "
				            "New focus child: %p (name '%s'), index %d.\n",
				            getName().c_str(), m_focusChild,
				            m_focusChild == 0 ? "{none}" : m_focusChild->getName().c_str(),
				            m_focusChildIndex);
			}
			/*
			else
			{
				// Cannot select child; select the first selectable one instead
				int first_child = getFirstSelectableChildIndex();
				if (first_child != -1)
				{
					selectChildByIndex(first_child);
				}
			}
			//*/
			return;
		}
	}
	
	// Child not found
	TT_PANIC("Container '%s' does not have a child with name '%s'.",
	         getName().c_str(), p_name.c_str());
}


template<typename ChildType>
bool ContainerBase<ChildType>::selectPreviousChild()
{
	// Select previous child
	MENU_Printf("ContainerBase::selectPreviousChild: Element '%s': "
	            "Current selection: %s\n", getName().c_str(),
	            m_focusChild == 0 ? "{none}" : m_focusChild->getName().c_str());
	
	if (isStylusOnly())
	{
		return false;
	}
	
	// Determine looping behavior
	// - Get the number of selectable children
	int selectableChildren = getSelectableChildrenCount();
	MENU_Printf("ContainerBase::selectPreviousChild: Element '%s': "
	            "Number of selectable children: %d\n",
	            getName().c_str(), selectableChildren);
	
	// If there are 2 or more selectable children,
	// the direct children with the same ordering should not loop
	if (selectableChildren >= 2)
	{
		MENU_Printf("ContainerBase::selectPreviousChild: Element '%s': "
		            "More than two selectable children; "
		            "disabling looping for all children.\n",
		            getName().c_str());
		
		MenuLayout::OrderType ourOrder = getLayout().getOrder();
		for (typename ChildVector::iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			MENU_Printf("ContainerBase::selectPreviousChild: Element '%s': "
			            "Disabling looping for child '%s'.\n",
			            getName().c_str(), (*it)->getName().c_str());
			//if ((*it)->getLayout().getOrder() == our_order)
			{
				(*it)->setContainerLoopEnable(false, ourOrder);
			}
		}
	}
	
	// Check whether this container should loop
	bool loop = isContainerLoopEnabled();
	if (isUserLoopEnabled() == false)
	{
		MENU_Printf("ContainerBase::selectPreviousChild: Element '%s': "
		            "User setting disabled container looping.\n",
		            getName().c_str());
		loop = false;
	}
	
	MENU_Printf("ContainerBase::selectPreviousChild: Element '%s': "
	            "Looping selection: %s\n",
	            getName().c_str(), loop ? "yes" : "no");
	
	int newIndex = -1;
	
	if (m_focusChild != 0)
	{
		// There is already a selected child; get the previous one
		newIndex = getPreviousSelectableChildIndex(m_focusChildIndex, loop);
	}
	else
	{
		// No child currently selected:
		// Set the last child that can receive focus to get focus
		newIndex = getLastSelectableChildIndex();
	}
	
	// Return false if the selection didn't change
	if (newIndex == -1)
	{
		return false;
	}
	
	// New selectable child found; deselect old child, select new child
	selectChildByIndex(newIndex);
	
	MENU_Printf("ContainerBase::selectPreviousChild: "
	            "Element '%s': New selection: %s\n",
	            getName().c_str(), m_focusChild == 0 ? "{none}" :
	            m_focusChild->getName().c_str());
	
	// The selection changed
	return true;
}


template<typename ChildType>
bool ContainerBase<ChildType>::selectNextChild()
{
	// Select next child
	MENU_Printf("ContainerBase::selectNextChild: Element '%s': "
	            "Moving to the next menu item. Current selection: %s\n",
	            getName().c_str(), m_focusChild == 0 ? "{none}" :
	            m_focusChild->getName().c_str());
	
	if (isStylusOnly())
	{
		return false;
	}
	
	// Determine looping behavior
	int selectableChildren = getSelectableChildrenCount();
	MENU_Printf("ContainerBase::selectNextChild: Element '%s': "
	            "Number of selectable children: %d\n",
	            getName().c_str(), selectableChildren);
	
	// If there are 2 or more selectable children,
	// the direct children with the same ordering should not loop
	if (selectableChildren >= 2)
	{
		MENU_Printf("ContainerBase::selectNextChild: Element '%s': "
		            "More than two selectable children; "
		            "disabling looping for all children.\n",
		            getName().c_str());
		
		MenuLayout::OrderType ourOrder = getLayout().getOrder();
		for (typename ChildVector::iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			MENU_Printf("ContainerBase::selectNextChild: Element '%s': "
			            "Disabling looping for child '%s'.\n",
			            getName().c_str(), (*it)->getName().c_str());
			//if ((*it)->getLayout().getOrder() == our_order)
			{
				(*it)->setContainerLoopEnable(false, ourOrder);
			}
		}
	}
	
	// Check whether this container should loop
	bool loop = isContainerLoopEnabled();
	if (isUserLoopEnabled() == false)
	{
		MENU_Printf("ContainerBase::selectNextChild: Element '%s': "
		            "User setting disabled container looping.\n",
		            getName().c_str());
		loop = false;
	}
	
	MENU_Printf("ContainerBase::selectNextChild: Element '%s': "
	            "Looping selection: %s\n",
	            getName().c_str(), loop ? "yes" : "no");
	
	int newIndex = -1;
	
	if (m_focusChild != 0)
	{
		// There is already a selected child; get the next one
		newIndex = getNextSelectableChildIndex(m_focusChildIndex, loop);
	}
	else
	{
		// No child currently selected:
		// Set the first child that can receive focus to get focus
		newIndex = getFirstSelectableChildIndex();
	}
	
	// Return false if the selection didn't change
	if (newIndex == -1)
	{
		return false;
	}
	
	
	// New selectable child found; deselect old child, select new child
	selectChildByIndex(newIndex);
	
	MENU_Printf("ContainerBase::selectNextChild: "
	            "Element '%s': New selection: %s\n",
	            getName().c_str(), m_focusChild == 0 ? "{none}" :
	            m_focusChild->getName().c_str());
	
	// The selection changed
	return true;
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getDepth() const
{
	// Get the maximum depth of all the child elements
	s32 maxDepth = MenuElement::getDepth();
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		s32 childDepth = (*it)->getDepth();
		if (childDepth > maxDepth)
		{
			maxDepth = childDepth;
		}
	}
	
	return maxDepth;
}


template<typename ChildType>
void ContainerBase<ChildType>::onLayoutDone()
{
	// Send event to all children
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->onLayoutDone();
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::onMenuActivated()
{
	// Send event to all children
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->onMenuActivated();
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::onMenuDeactivated()
{
	// Send event to all children
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->onMenuDeactivated();
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::setSelectionPath(SelectionPath& p_path)
{
	// Only restore selection if possible
	if (p_path.front() < getChildCount())
	{
		setSelected(true);
		
		// Restore selection for this node
		selectChildByIndex(p_path.front());
		
		// Remove selection index for this node from the path
		p_path.erase(p_path.begin());
		
		// Continue down selection path if a child was selected
		if (p_path.empty() == false && m_focusChild != 0)
		{
			m_focusChild->setSelectionPath(p_path);
		}
	}
	else
	{
		TT_WARN("!! Unable to restore menu selections, "
		        "because selection path specifies an "
		        "item that does not exist.");
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::getSelectionPath(SelectionPath& p_path) const
{
	// Only save selection if there is one
	s32 selChild = static_cast<s32>(getSelectedChildIndex());
	if (selChild != -1)
	{
		// Add this node's selection to the path
		p_path.push_back(selChild);
		
		// Traverse further down the path
		if (m_focusChild != 0)
		{
			m_focusChild->getSelectionPath(p_path);
		}
	}
}


template<typename ChildType>
bool ContainerBase<ChildType>::getSelectionPathForElement(
		SelectionPath&     p_path,
		const std::string& p_name) const
{
	if (getName() == p_name)
	{
		// Cannot set focus to this element from here,
		// should be done one level up (parent container)
		return false;
	}
	
	// Check all children of this container
	int index = 0;
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		// Check if this child can set the focus
		if ((*it)->getSelectionPathForElement(p_path, p_name))
		{
			// This child set the focus internally;
			// select this child
			if (p_path.empty() == false)
			{
				p_path.insert(p_path.begin(), index);
			}
			else
			{
				p_path.push_back(index);
			}
			return true;
		}
		
		// Check if this child is the one focus should be set to
		if ((*it)->getName() == p_name)
		{
			// This child is the one we're looking for;
			// select this child
			if (p_path.empty() == false)
			{
				p_path.insert(p_path.begin(), index);
			}
			else
			{
				p_path.push_back(index);
			}
			return true;
		}
	}
	
	// No children found with specified name
	return false;
}


template<typename ChildType>
void ContainerBase<ChildType>::dumpSelectionTree(int p_treeLevel) const
{
	{
		std::string indent(static_cast<std::string::size_type>(p_treeLevel), '-');
		TT_Printf("%s ContainerBase '%s'. Selected: %s. Child count: %d. "
		          "Selected child index: %d (name '%s')\n",
		          indent.c_str(), getName().c_str(),
		          isSelected() ? "yes" : "no",
		          m_children.size(), getSelectedChildIndex(),
		          m_focusChild != 0 ? m_focusChild->getName().c_str() : "{none}");
	}
	
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->dumpSelectionTree(p_treeLevel + 1);
	}
}


template<typename ChildType>
void ContainerBase<ChildType>::recalculateChildSelection()
{
	/*
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->recalculateChildSelection();
	}
	//*/
	
	bool updateParent = false;
	
	// Check if current selection still valid
	if (m_focusChild != 0)
	{
		if (isSelectableElement(m_focusChild) == false ||
		    m_focusChild->isStylusOnly())
		{
			// Currently selected element cannot be selected anymore;
			// reset selection
			m_focusChild->setSelected(false);
			m_focusChild       = 0;
			//m_focusChildIndex = -1;
			
			// This container is candidate for updating its parent
			updateParent = true;
		}
	}
	
	// Check if a selection can be made, if no selection set
	if (m_focusChild == 0)
	{
		if (m_focusChildIndex != -1)
		{
			// Existing selection was invalidated;
			// check for next or previous selectable element
			int newIndex = getNextSelectableChildIndex(m_focusChildIndex, false);
			if (newIndex != -1)
			{
				selectChildByIndex(newIndex);
				
				// Only update the parent if a selection was newly made,
				// not if an existing selection was removed and a new one made
				if (updateParent == false)
				{
					// Selection was newly made; update parent
					updateParent = true;
				}
				else if (getSelectedChildIndex() != -1)
				{
					// Selection was simply removed and re-created;
					// no need to update parent
					updateParent = false;
				}
			}
			else
			{
				newIndex = getPreviousSelectableChildIndex(m_focusChildIndex, false);
				if (newIndex != -1)
				{
					selectChildByIndex(newIndex);
					
					// Only update the parent if a selection was newly made,
					// not if an existing selection was removed and a new one made
					if (updateParent == false)
					{
						// Selection was newly made; update parent
						updateParent = true;
					}
					else if (getSelectedChildIndex() != -1)
					{
						// Selection was simply removed and re-created;
						// no need to update parent
						updateParent = false;
					}
				}
			}
		}
		
		if (m_focusChild == 0 && setInitialSelection())
		{
			// Selection was made
			
			// Only update the parent if a selection was newly made,
			// not if an existing selection was removed and a new one made
			if (updateParent == false)
			{
				// Selection was newly made; update parent
				updateParent = true;
			}
			else if (getSelectedChildIndex() != -1)
			{
				// Selection was simply removed and re-created;
				// no need to update parent
				updateParent = false;
			}
		}
	}
	
	if (m_focusChild == 0)
	{
		m_focusChildIndex = -1;
	}
	
	if (updateParent && getParent() != 0)
	{
		getParent()->recalculateChildSelection();
	}
}


template<typename ChildType>
ContainerBase<ChildType>* ContainerBase<ChildType>::clone() const
{
	return new ContainerBase<ChildType>(*this);
}


template<typename ChildType>
void ContainerBase<ChildType>::selectChildByIndex(int  p_index,
                                                  bool p_forceSelected)
{
	// Check for a selection of "nothing"
	if (p_index == -1)
	{
		if (m_focusChild != 0)
		{
			m_focusChild->setSelected(false);
		}
		m_focusChild       = 0;
		m_focusChildIndex = -1;
		return;
	}
	
	TT_ASSERTMSG(p_index >= 0 && p_index < getChildCount(),
	             "Element '%s': Child index %d out of range [0 - %d).",
	             getName().c_str(), p_index, getChildCount());
	
	MENU_Printf("ContainerBase::selectChildByIndex: Element '%s': "
	            "Current selection: %p (name '%s'), index %d.\n",
	            getName().c_str(),
	            m_focusChild,
	            m_focusChild == 0 ? "{none}" : m_focusChild->getName().c_str(),
	            m_focusChildIndex);
	
	// Only change the selection if the selection actually was changed
	if (p_index != m_focusChildIndex)
	{
		if (m_focusChild != 0)
		{
			m_focusChild->setSelected(false);
		}
		m_focusChildIndex = p_index;
		m_focusChild      = m_children.at(
			static_cast<typename ChildVector::size_type>(p_index));
		
		//if (isSelectableElement(m_focusChild))
		{
			if (p_forceSelected == false)
			{
				// Update the new child's selected status to our selection status
				m_focusChild->setSelected(isSelected());
			}
			else
			{
				// Force the child to be selected
				m_focusChild->setSelected(true);
			}
		}
		/*
		else
		{
			// If new child not selectable; reset our selection
			setSelected(false);
		}
		//*/
	}
	
	MENU_Printf("ContainerBase::selectChildByIndex: Element '%s': "
	            "New selection: %p (name '%s'), index %d.\n",
	            getName().c_str(),
	            m_focusChild,
	            m_focusChild == 0 ? "{none}" : m_focusChild->getName().c_str(),
	            m_focusChildIndex);
}


template<typename ChildType>
int ContainerBase<ChildType>::getSelectedChildIndex() const
{
	if (m_focusChild == 0)
	{
		return -1;
	}
	return m_focusChildIndex;
}


template<typename ChildType>
int ContainerBase<ChildType>::getChildCount() const
{
	return static_cast<int>(m_children.size());
}


//------------------------------------------------------------------------------
// Protected member functions

template<typename ChildType>
bool ContainerBase<ChildType>::isSelectableElement(
		const value_type* p_element) const
{
	return (p_element->canHaveFocus() &&
	        p_element->isVisible()/*    &&
	        p_element->isStylusOnly() == false*/);
}


template<typename ChildType>
int ContainerBase<ChildType>::getSelectableChildrenCount() const
{
	int selCount = 0;
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		if (isSelectableElement(*it) &&
		    (*it)->isStylusOnly() == false)
		{
			++selCount;
		}
	}
	
	return selCount;
}


template<typename ChildType>
int ContainerBase<ChildType>::getLastSelectableChildIndex() const
{
	int index = static_cast<int>(m_children.size() - 1);
	for (typename ChildVector::const_reverse_iterator it = m_children.rbegin();
	     it != m_children.rend(); ++it, --index)
	{
		if (isSelectableElement(*it) &&
		    (*it)->isStylusOnly() == false)
		{
			return index;
		}
	}
	
	// No selectable children
	return -1;
}


template<typename ChildType>
int ContainerBase<ChildType>::getFirstSelectableChildIndex() const
{
	int index = 0;
	for (typename ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		if (isSelectableElement(*it) &&
		    (*it)->isStylusOnly() == false)
		{
			return index;
		}
	}
	
	// No selectable children
	return -1;
}


template<typename ChildType>
int ContainerBase<ChildType>::getNextSelectableChildIndex(int  p_index,
                                                          bool p_wrap) const
{
	// Check all children after this index to see if it is selectable
	for (int i = p_index + 1; i < static_cast<int>(m_children.size()); ++i)
	{
		value_type* child = m_children.at(
			static_cast<typename ChildVector::size_type>(i));
		if (isSelectableElement(child) && child->isStylusOnly() == false)
		{
			return i;
		}
	}
	
	if (p_wrap)
	{
		// Check all children up to this index to see if it is selectable
		for (int i = 0; i < p_index; ++i)
		{
			value_type* child = m_children.at(
				static_cast<typename ChildVector::size_type>(i));
			if (isSelectableElement(child) && child->isStylusOnly() == false)
			{
				return i;
			}
		}
	}
	
	// No next selectable child
	return -1;
}


template<typename ChildType>
int ContainerBase<ChildType>::getPreviousSelectableChildIndex(int  p_index,
                                                              bool p_wrap) const
{
	// Check all children before this index to see if it is selectable
	for (int i = p_index - 1; i >= 0; --i)
	{
		value_type* child = m_children.at(
			static_cast<typename ChildVector::size_type>(i));
		if (isSelectableElement(child) && child->isStylusOnly() == false)
		{
			return i;
		}
	}
	
	if (p_wrap)
	{
		// Check all children up to this index to see if it is selectable
		for (int i = static_cast<int>(m_children.size()) - 1; i > p_index; --i)
		{
			value_type* child = m_children.at(
				static_cast<typename ChildVector::size_type>(i));
			if (isSelectableElement(child) && child->isStylusOnly() == false)
			{
				return i;
			}
		}
	}
	
	// No previous selectable child
	return -1;
}


template<typename ChildType>
bool ContainerBase<ChildType>::setInitialSelection()
{
	// Set this container to selected
	if (isStylusOnly() == false)
	{
		MenuElement::setSelected(true);
	}
	
	// Select child by name, if name was specified
	if (m_initialChildName.empty() == false)
	{
		// Select the child by name
		selectChildByName(m_initialChildName);
		if (getSelectedChildIndex() != -1)
		{
			// Selection succeeded
			return true;
		}
	}
	
	// Set the focus to the first focusable child if no focus was set yet
	int index = 0;
	for (typename ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		(*it)->setSelected(false);
		
		if (isSelectableElement(*it) && (*it)->isStylusOnly() == false)
		{
			//m_focusChild       = *it;
			//m_focusChildIndex = index;
			
			//m_focusChild->setSelected(true);
			selectChildByIndex(index, true);
			return true;
		}
	}
	
	// No selection was set
	return false;
}


//------------------------------------------------------------------------------
// Layout code

template<typename ChildType>
s32 ContainerBase<ChildType>::getMinimumChildrenWidth(
		const ChildVector& p_children) const
{
	return MenuLayoutManager::getElementWidth(getLayout(),
		MenuLayoutManager::getElements(p_children), true);
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getMinimumChildrenHeight(
		const ChildVector& p_children) const
{
	return MenuLayoutManager::getElementHeight(getLayout(),
		MenuLayoutManager::getElements(p_children), true);
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getRequestedChildrenWidth(
		const ChildVector& p_children) const
{
	return MenuLayoutManager::getElementWidth(getLayout(),
		MenuLayoutManager::getElements(p_children), false);
}


template<typename ChildType>
s32 ContainerBase<ChildType>::getRequestedChildrenHeight(
		const ChildVector& p_children) const
{
	return MenuLayoutManager::getElementHeight(getLayout(),
		MenuLayoutManager::getElements(p_children), false);
}


template<typename ChildType>
void ContainerBase<ChildType>::renderChildren(
		ChildVector&           p_children,
		const math::PointRect& p_rect,
		s32                    p_z,
		s32                    p_start,
		s32                    p_childCount)
{
	s32 count = 0;
	for (typename ChildVector::iterator it = p_children.begin();
	     it != p_children.end(); ++it, ++count)
	{
		if (count >= p_start)
		{
			if (p_childCount == -1 || (count - p_start) < p_childCount)
			{
				const math::PointRect& rect((*it)->getRectangle());
				math::PointRect renderRect(p_rect.getPosition() + rect.getPosition(),
				                           rect.getWidth(),
				                           rect.getHeight());
				(*it)->render(renderRect, p_z - 1);
			}
		}
	}
}


template<typename ChildType>
ContainerBase<ChildType>::ContainerBase(const ContainerBase& p_rhs)
:
MenuElement(p_rhs),
m_focusChild(0),
m_stylusFocusChild(0),
m_focusChildIndex(p_rhs.m_focusChildIndex),
m_initialChildName(p_rhs.m_initialChildName),
m_userLoopEnabled(p_rhs.m_userLoopEnabled),
m_containerLoopEnabled(p_rhs.m_containerLoopEnabled)
{
	// Clone all children
	for (typename ChildVector::const_iterator it = p_rhs.m_children.begin();
	     it != p_rhs.m_children.end(); ++it)
	{
		m_children.push_back((*it)->clone());
	}
	
	// Retrieve focus child pointer based on index
	if (p_rhs.m_focusChild != 0 && m_focusChildIndex >= 0)
	{
		m_focusChild = m_children.at(
			static_cast<typename ChildVector::size_type>(m_focusChildIndex));
	}
}

// Namespace end
}
}
}
