#include <tt/platform/tt_error.h>
#include <tt/menu/elements/ColourItem.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/elements/DefaultColours.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

ColorItem::ColorItem(const std::string& p_name,
                     const MenuLayout&  p_layout,
                     u32                p_color)
:
MenuElement(p_name, p_layout),
m_color(p_color)
{
	MENU_CREATION_Printf("ColorItem::ColorItem: Element '%s':\n",
	                     getName().c_str());
	
	using engine::renderer::QuadSprite;
	m_quad = QuadSprite::createQuad(10.0f, 10.0f, g_defaultColors[p_color]);
	
	setCanHaveFocus(true);
}


ColorItem::~ColorItem()
{
	MENU_CREATION_Printf("ColorItem::~ColorItem: Element '%s'.\n ",
	                     getName().c_str());
}


void ColorItem::doLayout(const PointRect& p_rect)
{
	m_quad->setWidth(static_cast<real>(p_rect.getWidth()));
	m_quad->setHeight(static_cast<real>(p_rect.getHeight()));
	
	MenuElement::doLayout(p_rect);
}


void ColorItem::render(const PointRect& p_rect, s32 p_z)
{
	//MENU_Printf("ColorItem::render: Render '%s' at (%d, %d).\n",
	//            getName().c_str(), p_rect.getX(), p_rect.getY());
	
	m_quad->setPosition(math::Vector3(
		static_cast<real>(p_rect.getCenterPosition().x),
		static_cast<real>(p_rect.getCenterPosition().y),
		static_cast<real>(p_z)));
	m_quad->update();
	m_quad->render();
}


void ColorItem::addChild(MenuElement* /* p_child */)
{
	TT_PANIC("ColorItem cannot have any children.");
}


s32 ColorItem::getMinimumWidth() const
{
	return getLayout().getWidth();
}


s32 ColorItem::getMinimumHeight() const
{
	return getLayout().getHeight();
}


s32 ColorItem::getRequestedWidth() const
{
	return getLayout().getWidth();
}


s32 ColorItem::getRequestedHeight() const
{
	return getLayout().getHeight();
}


s32 ColorItem::getRequestedHorizontalPosition() const
{
	return 0;
}


s32 ColorItem::getRequestedVerticalPosition() const
{
	return 0;
}


bool ColorItem::onStylusPressed(s32 p_x, s32 p_y)
{
	MENU_STYLUS_Printf("ColorItem::onStylusPressed: Element '%s': "
	                   "ColorItem pressed at (%d, %d).\n",
	                   getName().c_str(), p_x, p_y);
	(void)p_x;
	(void)p_y;
	performActions();
	
	return true;
}


bool ColorItem::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	if (p_isInside)
	{
		MENU_STYLUS_Printf("ColorItem::onStylusDragging: Element '%s': "
		                   "Dragging on ColorItem at (%d, %d).\n",
		                   getName().c_str(), p_x, p_y);
	}
	else
	{
		MENU_STYLUS_Printf("ColorItem::onStylusDragging: Element '%s': "
		                   "Dragging outside ColorItem at (%d, %d).\n",
		                   getName().c_str(), p_x, p_y);
	}
	(void)p_x;
	(void)p_y;
	
	return true;
}


bool ColorItem::onStylusReleased(s32 p_x, s32 p_y)
{
	MENU_STYLUS_Printf("ColorItem::onStylusReleased: Element '%s': "
	                   "Stylus released at (%d, %d) == button pressed. "
	                   "Perform ColorItem action.\n",
	                   getName().c_str(), p_x, p_y);
	
	(void)p_x;
	(void)p_y;
	
	return true;
}


bool ColorItem::onKeyPressed(const MenuKeyboard& p_keys)
{
	if (p_keys.isKeySet(MenuKeyboard::MENU_ACCEPT))
	{
		MENU_KEY_Printf("ColorItem::onKeyPressed: Element '%s': "
		                "Perform ColorItem action.\n",
		                getName().c_str());
		
		performActions();
		
		return true;
	}
	
	return false;
}


ColorItem* ColorItem::clone() const
{
	return new ColorItem(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

ColorItem::ColorItem(const ColorItem& p_rhs)
:
MenuElement(p_rhs),
m_color(p_rhs.m_color),
m_quad(new engine::renderer::QuadSprite(*(p_rhs.m_quad)))
{
}

// Namespace end
}
}
}
