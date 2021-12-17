#include <tt/menu/elements/Scrollbar.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/elements/SkinElementIDs.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

Scrollbar::Scrollbar(const std::string& p_name,
                     const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout),
m_value(0.0f),
m_minValue(0.0f),
m_maxValue(1.0f),
m_picked(false),
m_pickUp(false),
m_pickDown(false),
m_pressUp(false),
m_pressDown(false),
m_stepSize(0.1f)
{
	MENU_CREATION_Printf("Scrollbar::Scrollbar: Element '%s': New Scrollbar.\n",
	                     getName().c_str());
	
	
	// Get skinning information for Scrollbar
	MenuSystem*     sys  = MenuSystem::getInstance();
	const MenuSkin* skin = sys->getSkin();
	
	TT_ASSERTMSG(skin != 0, "Cannot create menu elements without a menu skin.");
	TT_ASSERTMSG(skin->hasElementSkin(SkinElement_Scrollbar),
	             "Skin does not provide skinning information for Scrollbar.");
	
	const MenuSkin::ElementSkin& element(
		skin->getElementSkin(SkinElement_Scrollbar));
	
	const MenuSkin::SkinTexture& barSkinTexture(
		element.getTexture(ScrollbarSkin_BgTexture));
	const MenuSkin::SkinTexture& blockSkinTexture(
		element.getTexture(ScrollbarSkin_PickerTexture));
	const MenuSkin::SkinTexture& arrowSkinTexture(
		element.getTexture(ScrollbarSkin_ArrowTexture));
	const MenuSkin::SkinTexture& arrowPressedSkinTexture(
		element.getTexture(ScrollbarSkin_ArrowPressedTexture));
	
	// Load the scrollbar textures
	// FIXME: MUST SUPPORT NAMESPACES
	m_barTexture = engine::renderer::TextureCache::get(barSkinTexture.getFilename(), "");
	if(m_barTexture == 0)
	{
		TT_PANIC("Loading Scrollbar bar texture '%s' failed.",
		         barSkinTexture.getFilename().c_str());
	}
	
	m_blockTexture = engine::renderer::TextureCache::get(blockSkinTexture.getFilename(), "");
	if(m_blockTexture == 0)
	{
		TT_PANIC("Loading Scrollbar block texture '%s' failed.",
		         blockSkinTexture.getFilename().c_str());
	}
	
	m_arrowTexture = engine::renderer::TextureCache::get(arrowSkinTexture.getFilename(), "");
	if(m_arrowTexture == 0)
	{
		TT_PANIC("Loading Scrollbar arrow texture '%s' failed.",
		         arrowSkinTexture.getFilename().c_str());
	}
	
	m_arrowPressedTexture = engine::renderer::TextureCache::get(
	    arrowPressedSkinTexture.getFilename(), "");
	if(m_arrowPressedTexture == 0)
	{
		TT_PANIC("Loading Scrollbar arrow pressed texture '%s' failed.",
		         arrowPressedSkinTexture.getFilename().c_str());
	}
	
	
	// Create the quads for the scrollbar
	setBarTexture(barSkinTexture.getU(),     barSkinTexture.getV(),
	              barSkinTexture.getWidth(), barSkinTexture.getHeight(),
	              element.getVertexColor(ScrollbarSkin_BgColor));
	
	setPickerTexture(blockSkinTexture.getU(),     blockSkinTexture.getV(),
	                 blockSkinTexture.getWidth(), blockSkinTexture.getHeight(),
	                 element.getVertexColor(ScrollbarSkin_PickerColor));
	
	setArrowTexture(arrowSkinTexture.getU(),     arrowSkinTexture.getV(),
	                arrowSkinTexture.getWidth(), arrowSkinTexture.getHeight(),
	                element.getVertexColor(ScrollbarSkin_ArrowColor));
	
	setCanHaveFocus(true);
}


Scrollbar::~Scrollbar()
{
	MENU_CREATION_Printf("Scrollbar::~Scrollbar: Element '%s': "
	                     "Freeing resources.\n", getName().c_str());
}


void Scrollbar::render(const math::PointRect& p_rect, s32 p_z)
{
	math::clamp(m_value, m_minValue, m_maxValue);
	
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	using math::Vector3;
	const Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
	                  static_cast<real>(p_rect.getCenterPosition().y),
	                  static_cast<real>(p_z));
	
	m_size.x = static_cast<real>(p_rect.getWidth());
	m_size.y = static_cast<real>(p_rect.getHeight());
	math::Vector2 size(m_size);
	size.y -= (m_barBlockSize.y * 2.0f) - (m_arrowBlockSize.y * 2.0f);
	
	// Readability shorts
	real yp = math::getHalf(size.y + m_barBlockSize.y);
	
	// Position the scrollbar background
	// - Bar top
	m_barSegs[BarSection_Top]->setPosition(pos + Vector3(0.0f, -yp, 0.0f));
	m_barSegs[BarSection_Top]->setWidth(size.x);
	m_barSegs[BarSection_Top]->setHeight(m_barBlockSize.y);
	
	// - Bar center
	m_barSegs[BarSection_Middle]->setPosition(pos);
	m_barSegs[BarSection_Middle]->setWidth(size.x);
	m_barSegs[BarSection_Middle]->setHeight(size.y);
	
	// - Bar bottom
	m_barSegs[BarSection_Bottom]->setPosition(pos + Vector3(0.0f, yp, 0.0f));
	m_barSegs[BarSection_Bottom]->setWidth(size.x);
	m_barSegs[BarSection_Bottom]->setHeight(m_barBlockSize.y);
	
	
	// Calculate the range over which the picker may move
	// Can also be seen as the number of free pixels in the scrollbar
	// (not occupied by the picker)
	real range = p_rect.getHeight() - m_pickerBlockSize.y -
	             (BORDER_SIZE * 2.0f) - (m_arrowBlockSize.y * 2.0f);
	
	if (m_picked == false)
	{
		// Picker is "snapped" to position
		
		// Calculate the amount of pixels per scrollposition
		real perseg = range / (m_maxValue - m_minValue);
		// The top and bottom scrollposition have half the range
		//real botoff = math::getHalf(perseg);
		
		// Calculate the position off the picker
		yp = perseg * (m_value - m_minValue);
		
		yp -= math::getHalf(range);
	}
	else
	{
		yp = static_cast<real>(m_pickerPos);
		yp -= math::getHalf(range);
	}
	
	
	// Position the picker
	m_pickerQuad->setPosition(pos + Vector3(0.0f, yp, -1.0f));
	m_pickerQuad->setWidth(m_pickerBlockSize.x);
	m_pickerQuad->setHeight(m_pickerBlockSize.y);
	
	yp = math::getHalf(p_rect.getHeight() - m_arrowBlockSize.y);
	
	// Position the scrollbar arrows
	// - Up
	m_arrowSegs[ArrowSection_Up]->setPosition(pos + Vector3(0.0f, -yp, 0.0f));
	m_arrowSegs[ArrowSection_Up]->setWidth(16.0f); // Not sure what was done here, but now it's hardcoded :)
	m_arrowSegs[ArrowSection_Up]->setHeight(13.0f); // Not sure what was done here, but now it's hardcoded :)
	
	// - Down
	m_arrowSegs[ArrowSection_Down]->setPosition(pos + Vector3(0.0f, yp, 0.0f));
	m_arrowSegs[ArrowSection_Down]->setWidth(16.0f); // Not sure what was done here, but now it's hardcoded :)
	m_arrowSegs[ArrowSection_Down]->setHeight(13.0f); // Not sure what was done here, but now it's hardcoded :)
	
	
	// Update and render all quads
	for (int i = 0; i < BarSection_Count; ++i)
	{
		m_barSegs[i]->update();
		m_barSegs[i]->render();
	}
	
	m_pickerQuad->update();
	m_pickerQuad->render();
	
	for (int i = 0; i < ArrowSection_Count; ++i)
	{
		m_arrowSegs[i]->update();
		m_arrowSegs[i]->render();
	}
}


s32 Scrollbar::getMinimumWidth() const
{
	if (getLayout().getWidthType() != MenuLayout::Size_Absolute)
	{
		return static_cast<s32>(std::max(m_barBlockSize.x, m_arrowBlockSize.x));
	}
	else
	{
		return getLayout().getWidth();
	}
}


s32 Scrollbar::getMinimumHeight() const
{
	if (getLayout().getHeightType() != MenuLayout::Size_Absolute)
	{
		return static_cast<s32>((2.0f * m_barBlockSize.y) +
		                        m_pickerBlockSize.y +
		                        (2.0f * m_arrowBlockSize.y));
	}
	else
	{
		return getLayout().getHeight();
	}
}


s32 Scrollbar::getRequestedWidth() const
{
	if (getLayout().getWidthType() != MenuLayout::Size_Absolute)
	{
		return static_cast<s32>(std::max(m_barBlockSize.x, m_arrowBlockSize.x));
	}
	else
	{
		return getLayout().getWidth();
	}
}


s32 Scrollbar::getRequestedHeight() const
{
	if (getLayout().getHeightType() != MenuLayout::Size_Absolute)
	{
		return static_cast<s32>(m_barBlockSize.y * 10.0f);
	}
	else
	{
		return getLayout().getHeight();
	}
}


bool Scrollbar::onStylusPressed(s32 /* p_x */, s32 p_y)
{
	real y = static_cast<real>(p_y);
	
	if (y < m_arrowBlockSize.y)
	{
		// Up button pressed
		m_picked  = false;
		m_value  -= m_stepSize;
		math::clamp(m_value, m_minValue, m_maxValue);
		m_pickUp  = true;
		setArrowPressed(ArrowSection_Up, true);
		return true;
	}
	
	if (y > m_size.y - m_arrowBlockSize.y)
	{
		// Down button pressed
		m_picked    = false;
		m_value    += m_stepSize;
		math::clamp(m_value, m_minValue, m_maxValue);
		m_pickDown  = true;
		setArrowPressed(ArrowSection_Down, true);
		return true;
	}
	
	// Calculate point where picker is positioned
	real range = m_size.y - m_pickerBlockSize.y - (BORDER_SIZE * 2) -
	            (m_arrowBlockSize.y * 2.0f);
	real pos = (((m_value - m_minValue) * range) / (m_maxValue - m_minValue));
	// Subtract half height of position
	pos -= math::getHalf(m_pickerBlockSize.y);
	
	y -= m_arrowBlockSize.y;
	y -= BORDER_SIZE;
	y -= math::getHalf(m_pickerBlockSize.y);
	
	if (y >= pos && y < pos + m_pickerBlockSize.y)
	{
		// Picker picked
		m_picked     = true;
		m_pickOffset = (y - pos) - math::getHalf(m_pickerBlockSize.y);
		return true;
	}
	
	m_picked = false;
	// Could do some quick moving here
	return true;
}


bool Scrollbar::onStylusDragging(s32 /* p_x */, s32 p_y, bool p_isInside)
{
	real y = static_cast<real>(p_y);
	if (m_pickUp)
	{
		if (p_isInside && y < m_arrowBlockSize.y)
		{
			// Up button pressed
			setArrowPressed(ArrowSection_Up, true);
			return true;
		}
		else
		{
			setArrowPressed(ArrowSection_Up, false);
			return false;
		}
	}
	
	if (m_pickDown)
	{
		if (p_isInside && y > m_size.y - m_arrowBlockSize.y)
		{
			// Down button pressed
			setArrowPressed(ArrowSection_Down, true);
			return true;
		}
		else
		{
			setArrowPressed(ArrowSection_Down, false);
			return false;
		}
	}
	
	if (m_picked)
	{
		y -= m_pickOffset;
		m_pickerPos  = y;
		m_pickerPos -= BORDER_SIZE;
		m_pickerPos -= m_arrowBlockSize.y;
		m_pickerPos -= math::getHalf(m_pickerBlockSize.y);
		
		// Do moving
		real range = m_size.y - m_pickerBlockSize.y -
		             (BORDER_SIZE * 2) - math::getTimesTwo(m_arrowBlockSize.y);
		math::clamp(m_pickerPos, real(0.0f), range);
		
		y = m_pickerPos;
		real perseg = range / (m_maxValue - m_minValue);
		real botoff = math::getHalf(perseg);
		y += botoff;
		y /= perseg;
		y += m_minValue;
		
		m_value = y;
		math::clamp(m_value, m_minValue, m_maxValue);
		
		return true;
	}
	return false;
}


bool Scrollbar::onStylusReleased(s32 /* p_x */, s32 /* p_y */)
{
	if (m_pickUp)
	{
		setArrowPressed(ArrowSection_Up, false);
		m_pickUp = false;
		return true;
	}
	
	if (m_pickDown)
	{
		setArrowPressed(ArrowSection_Down, false);
		m_pickDown = false;
		return true;
	}
	
	m_picked = false;
	
	return false;
}

bool Scrollbar::onStylusRepeat(s32 p_x, s32 p_y)
{
	if (m_picked)
	{
		return false;
	}
	return MenuElement::onStylusRepeat(p_x, p_y);
}


Scrollbar* Scrollbar::clone() const
{
	return new Scrollbar(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

Scrollbar::Scrollbar(const Scrollbar& p_rhs)
:
MenuElement(p_rhs),
m_barTexture(p_rhs.m_barTexture),
m_blockTexture(p_rhs.m_blockTexture),
m_arrowTexture(p_rhs.m_arrowTexture),
m_value(p_rhs.m_value),
m_minValue(p_rhs.m_minValue),
m_maxValue(p_rhs.m_maxValue),
m_picked(p_rhs.m_picked),
m_pickOffset(p_rhs.m_pickOffset),
m_stepSize(p_rhs.m_stepSize),
m_size(p_rhs.m_size),
m_pickerBlockSize(p_rhs.m_pickerBlockSize),
m_barBlockSize(p_rhs.m_barBlockSize),
m_arrowBlockSize(p_rhs.m_arrowBlockSize)
{
	using engine::renderer::QuadSprite;
	
	// Clone the bar segments
	for (int i = 0; i < BarSection_Count; ++i)
	{
		m_barSegs[i].reset(new QuadSprite(*(p_rhs.m_barSegs[i])));
	}
	
	// Clone the picker quad
	m_pickerQuad.reset(new QuadSprite(*(p_rhs.m_pickerQuad)));
	
	// Clone the arrow segments
	for (int i = 0; i < ArrowSection_Count; ++i)
	{
		m_arrowSegs[i].reset(new QuadSprite(*(p_rhs.m_arrowSegs[i])));
	}
}


//------------------------------------------------------------------------------
// Private member functions

void Scrollbar::setArrowPressed(ArrowSection p_section, bool p_pressed)
{
	switch (p_section)
	{
	case ArrowSection_Up:
		if (m_pressUp == p_pressed)
		{
			return;
		}
		m_pressUp = p_pressed;
		break;
		
	case ArrowSection_Down:
		if (m_pressDown == p_pressed)
		{
			return;
		}
		m_pressDown = p_pressed;
		break;
		
	default: break;
	}
	
	const engine::renderer::QuadSpritePtr& arrow(m_arrowSegs[p_section]);
	
	real width  = arrow->getWidth();
	real height = arrow->getHeight();

	// FIXME: Use QuadSprite frames
	real u0     = 0;// Deprecated: arrow->getTexcoordLeft();
	real u1     = 0;// Deprecated: arrow->getTexcoordRight();
	real v0     = 0;// Deprecated: arrow->getTexcoordTop();
	real v1     = 0;// Deprecated: arrow->getTexcoordBottom();
	
	if (p_pressed)
	{
		arrow->setTexture(m_arrowPressedTexture);
	}
	else
	{
		arrow->setTexture(m_arrowTexture);
	}
	
	arrow->setWidth(width);
	arrow->setHeight(height);
	/*
	arrow->setTexcoordLeft(u0);
	arrow->setTexcoordRight(u1);
	arrow->setTexcoordTop(v0);
	arrow->setTexcoordBottom(v1);
	*/ (void)u0; (void)u1; (void)v0; (void)v1;
}


void Scrollbar::setBarTexture(real p_x,
                              real p_y,
                              real p_w,
                              real p_h,
                              const engine::renderer::ColorRGBA& p_color)
{
	m_barBlockSize.setValues(p_w, p_h / 3.0f);
	
	// Create the quads
	using engine::renderer::QuadSprite;
	
	// - Top
	m_barSegs[BarSection_Top] = QuadSprite::createQuad(m_barTexture, p_color);
	setUVs(m_barSegs[BarSection_Top], 0, 0, m_barBlockSize, p_x, p_y);
	
	// - Center
	m_barSegs[BarSection_Middle] = QuadSprite::createQuad(m_barTexture, p_color);
	setUVs(m_barSegs[BarSection_Middle], 0, 1, m_barBlockSize, p_x, p_y);
	
	// - Bottom
	m_barSegs[BarSection_Bottom] = QuadSprite::createQuad(m_barTexture, p_color);
	setUVs(m_barSegs[BarSection_Bottom], 0, 2, m_barBlockSize, p_x, p_y);
}


void Scrollbar::setPickerTexture(real p_x,
                                 real p_y,
                                 real p_w,
                                 real p_h,
                                 const engine::renderer::ColorRGBA& p_color)
{
	m_pickerBlockSize.setValues(p_w, p_h);
	
	// Create the picker quad
	using engine::renderer::QuadSprite;
	m_pickerQuad = QuadSprite::createQuad(m_blockTexture, p_color);
	setUVs(m_pickerQuad, 0, 0, m_pickerBlockSize, p_x, p_y);
}


void Scrollbar::setArrowTexture(real p_x,
                                real p_y,
                                real p_w,
                                real p_h,
                                const engine::renderer::ColorRGBA& p_color)
{
	m_arrowBlockSize.setValues(p_w, p_h * 0.5f);
	
	// Create the quads
	using engine::renderer::QuadSprite;
	
	// - Up arrow
	m_arrowSegs[ArrowSection_Up] = QuadSprite::createQuad(m_arrowTexture, p_color);
	setUVs(m_arrowSegs[ArrowSection_Up], 0, 0, m_arrowBlockSize, p_x, p_y);
	
	// - Down arrow
	m_arrowSegs[ArrowSection_Down] = QuadSprite::createQuad(m_arrowTexture, p_color);
	setUVs(m_arrowSegs[ArrowSection_Down], 0, 1, m_arrowBlockSize, p_x, p_y);
}


void Scrollbar::setUVs(const engine::renderer::QuadSpritePtr& p_quad,
                       s32                  p_blockX,
                       s32                  p_blockY,
                       const math::Vector2& p_blockSize,
                       real                 p_topLeftX,
                       real                 p_topLeftY)
{
	real blockW = p_blockSize.x;
	real blockH = p_blockSize.y;
	
	TT_ASSERTMSG(blockW > 0.0f && blockH > 0.0f,
	             "Attempt to construct Scrollbar with fishy UV bounds.");
	
	/*
	p_quad->setTexcoordLeft(p_topLeftX + (blockW * p_blockX));
	p_quad->setTexcoordTop(p_topLeftY + (blockH * p_blockY));
	
	p_quad->setTexcoordRight(p_topLeftX + (blockW * (p_blockX + 1)) - 1);
	p_quad->setTexcoordBottom(p_topLeftY + (blockH * (p_blockY + 1)) - 1);
	*/ (void)blockW; (void)blockH;
}

// Namespace end
}
}
}
