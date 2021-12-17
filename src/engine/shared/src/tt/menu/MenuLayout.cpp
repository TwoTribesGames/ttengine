#include <tt/menu/MenuLayout.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

// Provide sensible defaults in the default constructor
MenuLayout::MenuLayout()
:
m_order(Order_Vertical),
m_widthType(Size_Auto),
m_heightType(Size_Auto),
m_horizontalPositionType(Position_Left),
m_verticalPositionType(Position_Top),
m_width(0),
m_height(0),
m_left(0),
m_top(0)
{
}


MenuLayout::~MenuLayout()
{
}


bool MenuLayout::operator==(const MenuLayout& p_rhs) const
{
	return m_order                  == p_rhs.m_order &&
	       m_widthType              == p_rhs.m_widthType &&
	       m_heightType             == p_rhs.m_heightType &&
	       m_horizontalPositionType == p_rhs.m_horizontalPositionType &&
	       m_verticalPositionType   == p_rhs.m_verticalPositionType &&
	       m_width                  == p_rhs.m_width &&
	       m_height                 == p_rhs.m_height &&
	       m_left                   == p_rhs.m_left &&
	       m_top                    == p_rhs.m_top;
}


void MenuLayout::setUndefinedToDefault()
{
	if (getOrder() == MenuLayout::Order_Undefined)
	{
		setOrder(MenuLayout::Order_Vertical);
	}
	
	if (getWidthType() == MenuLayout::Size_Undefined)
	{
		setWidthType(MenuLayout::Size_Auto);
		setWidth(0);
	}
	
	if (getHeightType() == MenuLayout::Size_Undefined)
	{
		setHeightType(MenuLayout::Size_Auto);
		setHeight(0);
	}
	
	if (getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		setHorizontalPositionType(MenuLayout::Position_Center);
		setLeft(0);
	}
	
	if (getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		setVerticalPositionType(MenuLayout::Position_Center);
		setTop(0);
	}
}

// Namespace end
}
}
