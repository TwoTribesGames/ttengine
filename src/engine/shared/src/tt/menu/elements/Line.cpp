#include <tt/menu/elements/Line.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

Line::Line(const std::string& p_name,
           const MenuLayout&  p_layout)
:
ContainerBase<>(p_name, p_layout)
{
	MENU_CREATION_Printf("Line::Line: Element '%s': New Line.\n",
	                     getName().c_str());
	
	// Default ordering is horizontal
	MenuLayout& layout(getLayout());
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Horizontal);
	}
}


Line::~Line()
{
	MENU_CREATION_Printf("Line::~Line: Element '%s': "
	                     "Destructing.\n", getName().c_str());
}


void Line::doLayout(const math::PointRect& p_rect)
{
	// Allow the container to lay out
	ContainerBase<>::doLayout(p_rect);
	
	// Reset the selection
	setSelected(isSelected());
}


void Line::setSelected(bool p_selected)
{
	// Allow the base to handle this first
	MenuElement::setSelected(p_selected);
	
	// Set all children to the selection that was passed
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->setSelected(p_selected);
	}
}


bool Line::onKeyPressed(const MenuKeyboard& p_keys)
{
	// Let d-pad input go to base, depending on ordering
	if (getLayout().getOrder() == MenuLayout::Order_Horizontal)
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_UP) ||
		    p_keys.isKeySet(MenuKeyboard::MENU_DOWN))
		{
			return ContainerBase<>::onKeyPressed(p_keys);
		}
	}
	else
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_LEFT) ||
		    p_keys.isKeySet(MenuKeyboard::MENU_RIGHT))
		{
			return ContainerBase<>::onKeyPressed(p_keys);
		}
	}
	
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// All other input goes to all children
	bool inputHandled = false;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		if ((*it)->onKeyPressed(p_keys))
		{
			inputHandled = true;
		}
	}
	
	return inputHandled;
}


bool Line::onKeyHold(const MenuKeyboard& p_keys)
{
	// Let d-pad input go to base, depending on ordering
	if (getLayout().getOrder() == MenuLayout::Order_Horizontal)
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_UP) ||
		    p_keys.isKeySet(MenuKeyboard::MENU_DOWN))
		{
			return ContainerBase<>::onKeyHold(p_keys);
		}
	}
	else
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_LEFT) ||
		    p_keys.isKeySet(MenuKeyboard::MENU_RIGHT))
		{
			return ContainerBase<>::onKeyHold(p_keys);
		}
	}
	
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// All other input goes to all children
	bool inputHandled = false;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		if ((*it)->onKeyHold(p_keys))
		{
			inputHandled = true;
		}
	}
	
	return inputHandled;
}


bool Line::onKeyReleased(const MenuKeyboard& p_keys)
{
	// Let d-pad input go to base, depending on ordering
	if (getLayout().getOrder() == MenuLayout::Order_Horizontal)
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_UP) ||
		    p_keys.isKeySet(MenuKeyboard::MENU_DOWN))
		{
			return ContainerBase<>::onKeyReleased(p_keys);
		}
	}
	else
	{
		if (p_keys.isKeySet(MenuKeyboard::MENU_LEFT) ||
		    p_keys.isKeySet(MenuKeyboard::MENU_RIGHT))
		{
			return ContainerBase<>::onKeyReleased(p_keys);
		}
	}
	
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	// All other input goes to all children
	bool inputHandled = false;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		if ((*it)->onKeyReleased(p_keys))
		{
			inputHandled = true;
		}
	}
	
	return inputHandled;
}


MenuElementInterface* Line::getSelectedElement()
{
	// Line should behave like a MenuElement, not like a container
	return MenuElement::getSelectedElement();
}


const MenuElementInterface* Line::getSelectedElement() const
{
	// Line should behave like a MenuElement, not like a container
	return MenuElement::getSelectedElement();
}


bool Line::getSelectedElementRect(math::PointRect& p_rect) const
{
	// Line should behave like a MenuElement, not like a container
	return MenuElement::getSelectedElementRect(p_rect);
}


Line* Line::clone() const
{
	return new Line(*this);
}


void Line::makeSelectable(bool p_selectable)
{
	setCanHaveFocus(p_selectable);
}


//------------------------------------------------------------------------------
// Protected member functions

Line::Line(const Line& p_rhs)
:
ContainerBase<>(p_rhs)
{
}

// Namespace end
}
}
}
