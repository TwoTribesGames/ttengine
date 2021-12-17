#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
//#include <tt/system/Pad.h>

#include <tt/menu/elements/ScrollableList.h>
#include <tt/menu/elements/Scrollbar.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSystem.h>


//#define USE_SCROLLABLELIST_DEBUG_OUTPUT

#if defined(USE_SCROLLABLELIST_DEBUG_OUTPUT)
	#define SL_Printf(...) TT_Printf(__VA_ARGS__)
#else
	#define SL_Printf(...)
#endif


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

ScrollableList::ScrollableList(const std::string& p_name,
                               const MenuLayout&  p_layout)
:
ContainerBase<>(p_name, p_layout),
m_useScrollbar(false),
m_scrollX(0),
m_topChild(0),
m_showableChildren(0)
{
	TT_ASSERT(p_layout.getOrder() == MenuLayout::Order_Vertical);
	MENU_CREATION_Printf("ScrollableList::ScrollableList: Element '%s': "
	                     "New ScrollableList.\n", getName().c_str());
	m_scrollbar = new Scrollbar("", p_layout);
	TT_ASSERTMSG(m_scrollbar != 0,
	                "Creating scrollbar failed (out of memory?).");
}


ScrollableList::~ScrollableList()
{
	MENU_CREATION_Printf("ScrollableList::~ScrollableList: Element '%s': "
	                     "Freeing memory for all children.\n",
	                     getName().c_str());
	
	delete m_scrollbar;
}


void ScrollableList::loadResources()
{
	ContainerBase<>::loadResources();
	updateChildrenResources();
}


void ScrollableList::render(const PointRect& p_rect, s32 p_z)
{
	if (m_children.empty() == false && isVisible())
	{
		// Render the scrollbar, if the scrollbar is present
		PointRect childRect(p_rect);
		if (m_useScrollbar)
		{
			childRect.setWidth(p_rect.getWidth() - m_scrollbar->getMinimumWidth());
			
			PointRect scrollRect(p_rect);
			scrollRect.setWidth(m_scrollbar->getMinimumWidth());
			scrollRect.setPosition(math::Point2(
				scrollRect.getPosition().x + p_rect.getWidth() - scrollRect.getWidth(),
				scrollRect.getPosition().y));
			m_scrollbar->render(scrollRect, p_z);
		}
		
		// Offset the render rectangle
		s32 count = 0;
		for (ChildVector::iterator it = m_children.begin();
		     it != m_children.end() && count < m_topChild; ++it, ++count)
		{
			childRect.setPosition(math::Point2(childRect.getPosition().x,
				childRect.getPosition().y - (*it)->getRequestedHeight()));
		}
		
		// Render the children that are visible
		renderChildren(m_children, childRect, p_z, m_topChild, m_showableChildren);
	}
}


void ScrollableList::doLayout(const PointRect& p_rect)
{
	PointRect rect(p_rect);
	
	// Modify the layout rectangle to accomodate the scrollbar
	if (p_rect.getHeight() < getRequestedHeight())
	{
		rect.setHeight(getRequestedHeight());
		rect.setWidth(p_rect.getWidth() - m_scrollbar->getMinimumWidth());
		m_useScrollbar = true;
		m_scrollX      = p_rect.getWidth() - m_scrollbar->getMinimumWidth();
		
		// Run through all children and stop using hands
		for (ChildVector::iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			switch ((*it)->wantCursor())
			{
			case SelectionCursorType_Both:  (*it)->setWantCursor(SelectionCursorType_Left); break;
			case SelectionCursorType_Right: (*it)->setWantCursor(SelectionCursorType_None); break;
			default: break;
			}
		}
	}
	else
	{
		m_useScrollbar = false;
	}
	
	// Allow the container to lay out the children
	ContainerBase<>::doLayout(rect);
	
	
	// Determine how many children fit in the available space
	s32 height = 0;
	m_showableChildren = 0;
	
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		s32 child_height = (*it)->getRectangle().getHeight();
		//s32 child_height = (*it)->getRequestedHeight();
		if ((height + child_height) <= p_rect.getHeight())
		{
			++m_showableChildren;
			height += child_height;
		}
	}
	
	if (m_useScrollbar)
	{
		m_scrollbar->setStepSize(1.0f);
		s32 invisibleChildren =
			static_cast<s32>(m_children.size()) - m_showableChildren;
		m_scrollbar->setRange(0.0f, static_cast<real>(invisibleChildren));
	}
	
	// Determine the initial selection
	ContainerBase<>::setInitialSelection();
}


void ScrollableList::addChild(value_type* p_child)
{
	if (m_useScrollbar)
	{
		switch (p_child->wantCursor())
		{
		case SelectionCursorType_Both:  p_child->setWantCursor(SelectionCursorType_Left); break;
		case SelectionCursorType_Right: p_child->setWantCursor(SelectionCursorType_None); break;
		default: break;
		}
	}
	
	ContainerBase<>::addChild(p_child);
}


void ScrollableList::removeChildren()
{
	ContainerBase<>::removeChildren();
	m_useScrollbar = false;
	m_topChild     = 0;
}


s32 ScrollableList::getMinimumWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		return getMinimumChildrenWidth(m_children) + m_scrollbar->getMinimumWidth();
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("Element '%s' has undefined width type!",
		            getName().c_str());
		return 0;
	}
}


s32 ScrollableList::getMinimumHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		s32 minheight = 0;
		for (ChildVector::const_iterator it = m_children.begin();
		     it != m_children.end(); ++it)
		{
			minheight = std::max(minheight, (*it)->getMinimumHeight());
		}
		return minheight;
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("Element '%s' has undefined height type!",
		            getName().c_str());
		return 0;
	}
}


s32 ScrollableList::getRequestedWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		return getRequestedChildrenWidth(m_children) +
		       m_scrollbar->getMinimumWidth();
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


void ScrollableList::setSelected(bool p_selected)
{
	bool newSelection = (isSelected() == false && p_selected);
	
	ContainerBase<>::setSelected(p_selected);
	
	if (newSelection)
	{
		SL_Printf("ScrollableList::setSelected: [%s] Newly selected.\n",
		          getName().c_str());
		
		// Scrollable list becomes selected and was not yet selected
		/* FIXME: Direct key input hacks must be adapted to new input code!
		if (system::Pad::keyUp())
		{
			// Little sneaky hack here, see if up button is pressed
			if (m_useScrollbar)
			{
				SL_Printf("ScrollableList::setSelected: [%s] Up key pressed, "
				          "and using scrollbar; selecting last visible child (%d).\n",
				          getName().c_str(), m_topChild + m_showableChildren - 1);
				
				// Select bottom-most visible child
				selectChildByIndex(m_topChild + m_showableChildren - 1);
			}
		}
		else if (system::Pad::keyDown())
		{
			// Little sneaky hack here, see if down button is pressed
			if (m_useScrollbar)
			{
				SL_Printf("ScrollableList::setSelected: [%s] Down key pressed, "
				          "and using scrollbar; selecting first visible child (%d).\n",
				          getName().c_str(), m_topChild);
				
				// Select top most visible child
				selectChildByIndex(m_topChild);
			}
		}
		else if (system::Pad::keyL() || system::Pad::keyR())
		{
			if (m_useScrollbar)
			{
				if (getSelectedChildIndex() != -1)
				{
					SL_Printf("ScrollableList::setSelected: [%s] L or R button pressed, "
					          "and using scrollbar; updating top-most visible child to %d.\n",
					          getName().c_str(), getSelectedChildIndex());
					m_topChild = getSelectedChildIndex();
					m_scrollbar->setValue(m_topChild);
				}
			}
		}
		*/
	}
}


bool ScrollableList::onStylusPressed(s32 p_x, s32 p_y)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	m_scrollPick = false;
	if (m_useScrollbar && p_x >= m_scrollX)
	{
		m_scrollPick = true;
		bool ret = m_scrollbar->onStylusPressed(p_x - m_scrollX, p_y);
		
		s32 old_top_child = m_topChild;
		
		// Get the index of the top-most visible child
		m_topChild = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topChild != old_top_child)
		{
			updateChildrenResources();
		}
		return ret;
	}
	
	s32 count = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end() && count < m_topChild; ++it, ++count)
	{
		//p_y += (*it)->getRequestedHeight();
		p_y += (*it)->getRectangle().getHeight();
	}
	
	
	// Check if any of the children handled the input
	int child_index = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++child_index)
	{
		// Check if the input is inside the rectangle for this child
		const PointRect& rect((*it)->getRectangle());
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			// Check if this child can receive focus
			//if ((*it)->canHaveFocus())
			if (isSelectableElement(*it))
			{
				// Deselect the previous focus child
				if (m_focusChild != 0)
				{
					m_focusChild->setSelected(false);
				}
				
				// Set the focus to this child
				m_focusChild       = *it;
				m_focusChildIndex = child_index;
				m_focusChild->setSelected(true);
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


bool ScrollableList::onStylusDragging(s32 p_x, s32 p_y, bool /* p_isInside */)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_useScrollbar && m_scrollPick)
	{
		bool ret = m_scrollbar->onStylusDragging(p_x - m_scrollX, p_y, (p_x >= m_scrollX));
		
		s32 old_top_child = m_topChild;
		
		// Get the index of the top-most visible child
		m_topChild = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topChild != old_top_child)
		{
			updateChildrenResources();
		}
		return ret;
	}
	
	s32 count = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end() && count < m_topChild; ++it, ++count)
	{
		//p_y += (*it)->getRequestedHeight();
		p_y += (*it)->getRectangle().getHeight();
	}
	
	// Send drag input directly to the child that has focus
	if (m_focusChild != 0)
	{
		// Check whether the coordinates are inside the child
		value_type*      child = m_focusChild;
		const PointRect& rect(m_focusChild->getRectangle());
		bool             isInside = false;
		
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			isInside = true;
		}
		
		// Send input to the child
		//if (child->isEnabled())
		{
			return child->onStylusDragging(p_x - rect.getPosition().x,
			                               p_y - rect.getPosition().y,
			                               isInside);
		}
	}
	
	// Otherwise ignore the input
	return false;
}


bool ScrollableList::onStylusReleased(s32 p_x, s32 p_y)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}

	if (m_useScrollbar && m_scrollPick)
	{
		bool ret = m_scrollbar->onStylusReleased(p_x - m_scrollX, p_y);
		
		s32 old_top_child = m_topChild;
		
		// Get the index of the top-most visible child
		m_topChild = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topChild != old_top_child)
		{
			updateChildrenResources();
		}
		return ret;
	}
	
	s32 count = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end() && count < m_topChild; ++it, ++count)
	{
		//p_y += (*it)->getRequestedHeight();
		p_y += (*it)->getRectangle().getHeight();
	}
	
	// Send stylus released input directly to the child that has focus
	if (m_focusChild != 0)
	{
		// Check whether the coordinates are inside the child
		value_type*      child = m_focusChild;
		const PointRect& rect(m_focusChild->getRectangle());
		
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			//if (child->isEnabled())
			{
				// Send input to the child
				return child->onStylusReleased(p_x - rect.getPosition().x,
				                               p_y - rect.getPosition().y);
			}
		}
	}
	
	// Otherwise ignore the input
	return false;
}



bool ScrollableList::onStylusRepeat(s32 p_x, s32 p_y)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_useScrollbar && m_scrollPick)
	{
		bool ret = m_scrollbar->onStylusRepeat(p_x - m_scrollX, p_y);
		
		s32 old_top_child = m_topChild;
		
		// Get the index of the top-most visible child
		m_topChild = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topChild != old_top_child)
		{
			updateChildrenResources();
		}
		return ret;
	}

	s32 count = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end() && count < m_topChild; ++it, ++count)
	{
		//p_y += (*it)->getRequestedHeight();
		p_y += (*it)->getRectangle().getHeight();
	}
	
	// Send drag input directly to the child that has focus
	if (m_focusChild != 0)
	{
		// Check whether the coordinates are inside the child
		value_type*      child = m_focusChild;
		const PointRect& rect(m_focusChild->getRectangle());
		
		if (rect.contains(math::Point2(p_x, p_y)))
		{
			// Send input to the child
			//if (child->isEnabled())
			{
				return child->onStylusRepeat(p_x - rect.getPosition().x,
				                             p_y - rect.getPosition().y);
			}
		}
	}
	
	// Otherwise ignore the input
	return false;
}



bool ScrollableList::onKeyPressed(const MenuKeyboard& p_keys)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	s32 old_top_child = m_topChild;
	
	
	// Allow the base class to handle the input
	bool handled = ContainerBase<>::onKeyPressed(p_keys);
	
	
	MenuLayout& l(getLayout());
	if ((l.getOrder() == MenuLayout::Order_Vertical   && p_keys.isKeySet(MenuKeyboard::MENU_DOWN)) ||
		(l.getOrder() == MenuLayout::Order_Horizontal && p_keys.isKeySet(MenuKeyboard::MENU_RIGHT)))
	{
		// Next child selected
		if (m_focusChild != 0)
		{
			MenuSystem::getInstance()->playSound(MenuSound_FE_HighlightShift);
			
			// Update the scrollbar
			if (m_useScrollbar)
			{
				if (m_focusChildIndex >= (m_showableChildren + m_topChild))
				{
					m_topChild = (m_focusChildIndex - m_showableChildren) + 1;
					m_scrollbar->setValue(static_cast<real>(m_topChild));
				}
				else if (m_focusChildIndex == 0)
				{
					m_topChild = 0;
					m_scrollbar->setValue(static_cast<real>(m_topChild));
				}
			}
		}
	}
	else if ((l.getOrder() == MenuLayout::Order_Vertical   && p_keys.isKeySet(MenuKeyboard::MENU_UP)) ||
	         (l.getOrder() == MenuLayout::Order_Horizontal && p_keys.isKeySet(MenuKeyboard::MENU_LEFT)))
	{
		// Previous child selected
		if (m_focusChild != 0)
		{
			MenuSystem::getInstance()->playSound(MenuSound_FE_HighlightShift);
			if (m_useScrollbar)
			{
				if (m_focusChildIndex < m_topChild)
				{
					m_topChild = m_focusChildIndex;
					m_scrollbar->setValue(static_cast<real>(m_topChild));
				}
				else if (m_focusChildIndex >= (m_showableChildren + m_topChild))
				{
					m_topChild = (m_focusChildIndex - m_showableChildren) + 1;
					m_scrollbar->setValue(static_cast<real>(m_topChild));
				}
			}
		}
		else
		{
			if (m_useScrollbar)
			{
				m_topChild = m_focusChildIndex - (m_showableChildren - 1);
				m_scrollbar->setValue(static_cast<real>(m_topChild));
			}
		}
	}
	
	if (m_topChild != old_top_child)
	{
		updateChildrenResources();
	}
	
	return handled;
}


bool ScrollableList::getSelectedElementRect(PointRect& p_rect) const
{
    /*
	if (isSelected() == false)
	{
		return false;
	}
	//*/
	
	// Offset the rectangle to start at the first showing child
	s32 topChild = static_cast<s32>(m_scrollbar->getValue());
	s32 count    = 0;
	for (ChildVector::const_iterator it = m_children.begin();
	     it != m_children.end() && count < topChild; ++it, ++count)
	{
		//p_rect.translate(math::Point2(0, -(*it)->getRequestedHeight()));
		p_rect.translate(math::Point2(0, -(*it)->getRectangle().getHeight()));
	}
	
	
	s32 width = getRectangle().getWidth();
	if (m_useScrollbar)
	{
		width -= m_scrollbar->getRectangle().getWidth();
	}
	
	// UGLY HACK BELOW (Daniel)
	//width -= 14;  // 2 * width of sunken border decorator border
	              // (assuming a ScrollableList always has a sunken border)
	
	s32 x = p_rect.getPosition().x;
	
	// Continue with default ContainerBase behavior
	bool ret = ContainerBase<>::getSelectedElementRect(p_rect);
	if (ret)
	{
		p_rect.setPosition(math::Point2(x, p_rect.getPosition().y));
		p_rect.setWidth(width);
	}
	
	return ret;
}


void ScrollableList::selectChildByIndex(int p_index, bool p_forceSelected)
{
	// Use default selection behavior
	ContainerBase<>::selectChildByIndex(p_index, p_forceSelected);
	
	// Update the scroll bar, if we have one
	if (m_useScrollbar)
	{
		if (p_index < m_topChild)
		{
			SL_Printf("ScrollableList::selectChildByIndex: [%s] Selected child (%d) above visible range [%d - %d).\n",
			          getName().c_str(), p_index, m_topChild, m_topChild + m_showableChildren);
			m_topChild = p_index;
			m_scrollbar->setValue(static_cast<real>(m_topChild));
			updateChildrenResources();
		}
		else if (p_index >= (m_topChild + m_showableChildren))
		{
			SL_Printf("ScrollableList::selectChildByIndex: [%s] Selected child (%d) below visible range [%d - %d).\n",
			          getName().c_str(), p_index, m_topChild, m_topChild + m_showableChildren);
			m_topChild = p_index - (m_showableChildren - 1);
			m_scrollbar->setValue(static_cast<real>(m_topChild));
			updateChildrenResources();
		}
	}
}


void ScrollableList::setContainerLoopEnable(bool p_enabled,
                                            MenuLayout::OrderType p_parentOrder)
{
	bool prev_enabled = isContainerLoopEnabled();
	ContainerBase<>::setContainerLoopEnable(p_enabled, p_parentOrder);
	m_containerLoopEnabled = prev_enabled;
}


ScrollableList* ScrollableList::clone() const
{
	return new ScrollableList(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

ScrollableList::ScrollableList(const ScrollableList& p_rhs)
:
ContainerBase<>(p_rhs),
m_scrollbar(0),
m_useScrollbar(p_rhs.m_useScrollbar),
m_scrollPick(p_rhs.m_scrollPick),
m_scrollX(p_rhs.m_scrollX),
m_topChild(p_rhs.m_topChild),
m_showableChildren(p_rhs.m_showableChildren)
{
	// Clone the scrollbar
	if (p_rhs.m_scrollbar != 0)
	{
		m_scrollbar = p_rhs.m_scrollbar->clone();
	}
}


bool ScrollableList::setInitialSelection()
{
	// Do not set selection here
	// (base implementation is used in doLayout)
	return true;
}


//------------------------------------------------------------------------------
// Private member functions

void ScrollableList::updateChildrenResources()
{
	// Update the resources of all children in the list
	s32 count = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++count)
	{
		if (count < m_topChild)
		{
			// Child above the visible range
			(*it)->unloadResources();
		}
		else if (count < m_topChild + m_showableChildren)
		{
			// Child visible
			(*it)->loadResources();
		}
		else
		{
			// Child below the visible range
			(*it)->unloadResources();
		}
	}
	
	s32 childindex = getSelectedChildIndex();
	if (childindex != -1)
	{
		SL_Printf("ScrollableList::updateChildrenResources: [%s] There is a selection (%d). Number of children: %d\n",
		          getName().c_str(), childindex, getChildCount());
		if (childindex < m_topChild)
		{
			SL_Printf("ScrollableList::updateChildrenResources: [%s] Selection (%d) is above visible range [%d - %d); selecting first visible element (%d).\n",
			          getName().c_str(), childindex, m_topChild, m_topChild + m_showableChildren, m_topChild);
			selectChildByIndex(m_topChild);
		}
		else if (childindex >= m_topChild + m_showableChildren)
		{
			SL_Printf("ScrollableList::updateChildrenResources: [%s] Selection (%d) below visible range [%d - %d); selecting last visible element (%d).\n",
			          getName().c_str(), childindex, m_topChild, m_topChild + m_showableChildren, m_topChild + m_showableChildren - 1);
			selectChildByIndex(m_topChild + m_showableChildren - 1);
		}
		else
		{
			SL_Printf("ScrollableList::updateChildrenResources: [%s] Selection (%d) in range; not touching it.\n",
			          getName().c_str(), childindex);
		}
	}
	
	MenuSystem::getInstance()->resetSelectedElement();
}

// Namespace end
}
}
}
