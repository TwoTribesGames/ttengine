#include <tt/platform/tt_error.h>

#include <tt/menu/elements/BusyBar.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/SkinElementIDs.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

BusyBar::BusyBar(const std::string& p_name,
                 const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout),
m_counter(0.0f)
{
	createBar();
	
	// Set minimum and requested dimensions
	if (m_texture != 0)
	{
		const s32 width  = m_texture->getWidth();
		const s32 height = m_texture->getHeight();
		
		setMinimumWidth(width * 5);
		setMinimumHeight(height);
		setRequestedWidth(width * 5);
		setRequestedHeight(height);
	}
}


BusyBar::~BusyBar()
{
}


void BusyBar::update()
{
	if (MenuElement::isEnabled() == false)
	{
		return;
	}
	
	m_counter += 0.03f;
	if (m_counter >= 1.0f)
	{
		m_counter = 0.0f;
	}
}


void BusyBar::render(const math::PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Render 3 "dots"
	renderDot(p_rect, -m_texture->getWidth(), p_z);
	renderDot(p_rect, 0,                      p_z);
	renderDot(p_rect, m_texture->getWidth(),  p_z);
}


BusyBar* BusyBar::clone() const
{
	return new BusyBar(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

BusyBar::BusyBar(const BusyBar& p_rhs)
:
MenuElement(p_rhs),
m_texture(p_rhs.m_texture),
m_quad(new engine::renderer::QuadSprite(*(p_rhs.m_quad))),
m_counter(p_rhs.m_counter)
{
}


//------------------------------------------------------------------------------
// Private member functions

void BusyBar::createBar()
{
	// Get skinning information for BusyBar
	MenuSystem*     sys  = MenuSystem::getInstance();
	const MenuSkin* skin = sys->getSkin();
	
	TT_ASSERTMSG(skin != 0, "Cannot create menu elements without a menu skin.");
	TT_ASSERTMSG(skin->hasElementSkin(SkinElement_BusyBar),
	             "Skin does not provide skinning information for BusyBar.");
	
	const MenuSkin::ElementSkin& element(
		skin->getElementSkin(SkinElement_BusyBar));
	
	
	// Load the overlay texture
	const MenuSkin::SkinTexture& overlayTexture(
		element.getTexture(BusyBarSkin_OverlayTexture));

	// FIXME: SUPPORT NAMESPACES
	m_texture = engine::renderer::TextureCache::get(overlayTexture.getFilename(), "");
	if(m_texture == 0)
	{
		TT_PANIC("Loading busy bar overlay texture '%s' failed.",
		         overlayTexture.getFilename().c_str());
		return;
	}
	
	// Create a quad for the dots
	m_quad = engine::renderer::QuadSprite::createQuad(m_texture,
		element.getVertexColor(BusyBarSkin_OverlayColor));
}


void BusyBar::renderDot(const math::PointRect& p_rect, s32 p_x, s32 p_z)
{
	s32  width      = p_rect.getWidth();
	s32  range      = width + (3 * m_texture->getWidth());
	s32  border     = m_texture->getWidth() / 2;
	s32  outOfRange = (range - width) / 2;
	real pos        = m_counter * range; // Position in pixels within available space
	pos += p_x; // Offset
	
	s32 right = static_cast<s32>(pos) + border;
	s32 left  = static_cast<s32>(pos) - border;
	
	if (right < outOfRange)
	{
		return;
	}
	
	if (left > width + outOfRange)
	{
		return;
	}
	
	s32 leftBorder  = std::max(left,  outOfRange);
	s32 rightBorder = std::min(right, width + outOfRange);
	
	if (leftBorder == rightBorder)
	{
		return;
	}
	
	
	s32 u0;
	s32 u1;
	
	if (left < leftBorder)
	{
		u0 = leftBorder - left;
	}
	else
	{
		u0 = 0;
	}
	
	if (right > rightBorder)
	{
		u1 = m_texture->getWidth() - (right - rightBorder);
	}
	else
	{
		u1 = m_texture->getWidth();
	}
	
	/*
	m_quad->setTexcoordLeft(static_cast<real>(u0));
	m_quad->setTexcoordRight(static_cast<real>(u1));
	*/ (void)u0; (void)u1;
	
	m_quad->setWidth(static_cast<real>(rightBorder - leftBorder));
	
	s32 center = (rightBorder + leftBorder) / 2;
	center -= range / 2;
	center += p_rect.getCenterPosition().x;
	
	m_quad->setPosition(static_cast<real>(center),
	                    static_cast<real>(p_rect.getCenterPosition().y),
	                    static_cast<real>(p_z));
	m_quad->update();
	m_quad->render();
}

// Namespace end
}
}
}
