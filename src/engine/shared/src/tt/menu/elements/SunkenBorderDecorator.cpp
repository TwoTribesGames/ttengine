#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <tt/menu/elements/SunkenBorderDecorator.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/SkinElementIDs.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

SunkenBorderDecorator::SunkenBorderDecorator(
		MenuElementInterface* p_targetElement)
:
Decorator(p_targetElement),
m_borderRect(math::Point2(0, 0), 2 * BORDER_SIZE, 2 * BORDER_SIZE)
{
	MENU_CREATION_Printf("SunkenBorderDecorator::SunkenBorderDecorator: "
	                     "Decorating element '%s'.\n",
	                     getName().c_str());
	
	// Get skinning information for SunkenBorderDecorator
	MenuSystem*     sys  = MenuSystem::getInstance();
	const MenuSkin* skin = sys->getSkin();
	
	TT_ASSERTMSG(skin != 0, "Cannot create menu elements without a menu skin.");
	TT_ASSERTMSG(skin->hasElementSkin(SkinElement_SunkenBorderDecorator),
	             "Skin does not provide skinning information for "
	             "SunkenBorderDecorator.");
	
	const MenuSkin::ElementSkin& element(
		skin->getElementSkin(SkinElement_SunkenBorderDecorator));
	const MenuSkin::SkinTexture& skinTexture(
		element.getTexture(SunkenBorderSkin_BgTexture));

	// FIXME: MUST SUPPORT NAMESPACES
	m_texture = engine::renderer::TextureCache::get(skinTexture.getFilename(), "");
	if(m_texture == 0)
	{
		TT_PANIC("Loading sunken border decorator background texture "
		         "'%s' failed.", skinTexture.getFilename().c_str());
		return;
	}
	
	setBorderTexture(skinTexture.getU(),     skinTexture.getV(),
	                 skinTexture.getWidth(), skinTexture.getHeight(),
	                 element.getVertexColor(SunkenBorderSkin_BgColor));
}


SunkenBorderDecorator::~SunkenBorderDecorator()
{
	MENU_CREATION_Printf("SunkenBorderDecorator::~SunkenBorderDecorator: "
	                     "Destructing decorator for element '%s'.\n",
	                     getName().c_str());
}


void SunkenBorderDecorator::doLayout(const math::PointRect& p_rect)
{
	math::PointRect childRect(p_rect);
	childRect.setWidth(childRect.getWidth() - (2 * BORDER_SIZE));
	childRect.setHeight(childRect.getHeight() - (2 * BORDER_SIZE));
	Decorator::doLayout(childRect);
}


void SunkenBorderDecorator::render(const math::PointRect& p_rect, s32 p_z)
{
	// Only render border if element is visible
	if (isVisible())
	{
		math::Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
		                  static_cast<real>(p_rect.getCenterPosition().y),
		                  static_cast<real>(p_z));
		
		m_size.x = static_cast<real>(p_rect.getWidth());
		m_size.y = static_cast<real>(p_rect.getHeight());
		
		// Readability shorts
		real xp = (m_size.x - m_blockSize.x) * 0.5f;
		real yp = (m_size.y - m_blockSize.y) * 0.5f;
		
		engine::renderer::QuadSpritePtr sprite;
		
		// Center block
		sprite = m_borderSegs[BorderSection_Center];
		sprite->setPosition(pos);
		sprite->setWidth (m_size.x - (m_blockSize.x * 2.0f));
		sprite->setHeight(m_size.y - (m_blockSize.y * 2.0f));
		
		// Top edge
		sprite = m_borderSegs[BorderSection_EdgeTop];
		sprite->setPosition(pos + math::Vector3(0.0f, -yp , 0.0f));
		sprite->setWidth (m_size.x - (m_blockSize.x * 2.0f));
		sprite->setHeight(m_blockSize.y);
		
		// Bot edge
		sprite = m_borderSegs[BorderSection_EdgeBottom];
		sprite->setPosition(pos + math::Vector3(0.0f, yp, 0.0f));
		sprite->setWidth(m_size.x - (m_blockSize.x * 2.0f));
		sprite->setHeight(m_blockSize.y);
		
		// Left edge
		sprite = m_borderSegs[BorderSection_EdgeLeft];
		sprite->setPosition(pos + math::Vector3(-xp, 0.0f, 0.0f));
		sprite->setWidth(m_blockSize.x);
		sprite->setHeight(m_size.y - (m_blockSize.y * 2.0f));
		
		// Right edge
		sprite = m_borderSegs[BorderSection_EdgeRight];
		sprite->setPosition(pos + math::Vector3(xp, 0, 0));
		sprite->setWidth(m_blockSize.x);
		sprite->setHeight(m_size.y - (m_blockSize.y * 2.0f));
		
		
		// Corner top left
		sprite = m_borderSegs[BorderSection_CornerTopLeft];
		sprite->setPosition(pos + math::Vector3(-xp, -yp, 0.0f));
		sprite->setWidth(m_blockSize.x);
		sprite->setHeight(m_blockSize.y);
		
		
		// Corner top right
		sprite = m_borderSegs[BorderSection_CornerTopRight];
		sprite->setPosition(pos + math::Vector3(xp, -yp, 0.0f));
		sprite->setWidth(m_blockSize.x);
		sprite->setHeight(m_blockSize.y);
		
		
		// Corner bot left
		sprite = m_borderSegs[BorderSection_CornerBottomLeft];
		sprite->setPosition(pos + math::Vector3(-xp, yp, 0.0f));
		sprite->setWidth(m_blockSize.x);
		sprite->setHeight(m_blockSize.y);
		
		// Corner bot right
		sprite = m_borderSegs[BorderSection_CornerBottomRight];
		sprite->setPosition(pos + math::Vector3(xp, yp, 0.0f));
		sprite->setWidth(m_blockSize.x);
		sprite->setHeight(m_blockSize.y);
		
		// Update all
		for (int i = 0; i < BorderSection_Count; ++i)
		{
			m_borderSegs[i]->update();
			m_borderSegs[i]->render();
		}
	}
	
	math::PointRect childRect(p_rect);
	childRect.translate(math::Point2(BORDER_SIZE, BORDER_SIZE));
	childRect.setWidth(childRect.getWidth() - (2 * BORDER_SIZE));
	childRect.setHeight(childRect.getHeight() - (2 * BORDER_SIZE));
	Decorator::render(childRect, p_z - 1);
}


bool SunkenBorderDecorator::onStylusPressed(s32 p_x, s32 p_y)
{
	return Decorator::onStylusPressed(p_x - BORDER_SIZE, p_y - BORDER_SIZE);
}


bool SunkenBorderDecorator::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	return Decorator::onStylusDragging(p_x - BORDER_SIZE, p_y - BORDER_SIZE,
	                                   p_isInside);
}


bool SunkenBorderDecorator::onStylusReleased(s32 p_x, s32 p_y)
{
	return Decorator::onStylusReleased(p_x - BORDER_SIZE, p_y - BORDER_SIZE);
}


bool SunkenBorderDecorator::onStylusRepeat(s32 p_x, s32 p_y)
{
	return Decorator::onStylusRepeat(p_x - BORDER_SIZE, p_y - BORDER_SIZE);
}


s32 SunkenBorderDecorator::getMinimumWidth() const
{
	return Decorator::getMinimumWidth() + (2 * BORDER_SIZE);
}


s32 SunkenBorderDecorator::getMinimumHeight() const
{
	return Decorator::getMinimumHeight() + (2 * BORDER_SIZE);
}


s32 SunkenBorderDecorator::getRequestedWidth() const
{
	return Decorator::getRequestedWidth() + (2 * BORDER_SIZE);
}


s32 SunkenBorderDecorator::getRequestedHeight() const
{
	return Decorator::getRequestedHeight() + (2 * BORDER_SIZE);
}


const math::PointRect& SunkenBorderDecorator::getRectangle() const
{
	/*
	const math::PointRect& us(m_borderRect);
	const math::PointRect& der(Decorator::getRectangle());
	TT_Printf("SunkenBorderDecorator::getRectangle: "
	          "Our rect: (%d, %d, %d, %d). "
	          "Decorated element rect: (%d, %d, %d, %d).\n",
	          us.getX(), us.getY(), us.getWidth(), us.getHeight(),
	          der.getX(), der.getY(), der.getWidth(), der.getHeight());
	//*/
	
	return m_borderRect;
}


void SunkenBorderDecorator::setRectangle(const math::PointRect& p_rect)
{
	// Save the original rectangle
	m_borderRect = p_rect;
	
	// Offset and resize the rectangle for the border
	math::PointRect elementRect(p_rect);
	elementRect.translate(math::Point2(BORDER_SIZE, BORDER_SIZE));
	
	s32 newWidth  = p_rect.getWidth()  - (2 * BORDER_SIZE);
	s32 newHeight = p_rect.getHeight() - (2 * BORDER_SIZE);
	if (newWidth  < 0) newWidth  = 0;
	if (newHeight < 0) newHeight = 0;
	elementRect.setWidth(newWidth);
	elementRect.setHeight(newHeight);
	
	/*
	TT_Printf("SunkenBorderDecorator::setRectangle: "
	          "Setting our rect to (%d, %d, %d, %d). "
	          "Decorated element rect: (%d, %d, %d, %d).\n",
	          p_rect.getX(), p_rect.getY(), p_rect.getWidth(), p_rect.getHeight(),
	          elementRect.getX(), elementRect.getY(), elementRect.getWidth(), elementRect.getHeight());
	//*/
	
	Decorator::setRectangle(elementRect);
}


bool SunkenBorderDecorator::getSelectedElementRect(math::PointRect& p_rect) const
{
	//p_rect.translate(BORDER_SIZE, BORDER_SIZE);
	
	// FIXME: Why is the width and height not offset here?
	//p_rect.setHeight(p_rect.getHeight() - (2 * BORDER_SIZE));
	//p_rect.setWidth(p_rect.getWidth() - (2 * BORDER_SIZE));
	
	if (Decorator::getSelectedElementRect(p_rect))
	{
		p_rect.translate(math::Point2(BORDER_SIZE, 0));
		return true;
	}
	
	return false;
}


void SunkenBorderDecorator::dumpSelectionTree(int p_treeLevel) const
{
	{
		std::string indent(static_cast<std::string::size_type>(p_treeLevel), '-');
		TT_Printf("%s { SunkenBorderDecorator }\n",
		          indent.c_str());
	}
	
	Decorator::dumpSelectionTree(p_treeLevel + 1);
}


s32 SunkenBorderDecorator::getDepth() const
{
	// Add one Z level for the border
	return Decorator::getDepth() + 1;
}


SunkenBorderDecorator* SunkenBorderDecorator::clone() const
{
	return new SunkenBorderDecorator(*this);
}


//------------------------------------------------------------------------------
// Private member functions

SunkenBorderDecorator::SunkenBorderDecorator(const SunkenBorderDecorator& p_rhs)
:
Decorator(p_rhs),
m_texture(p_rhs.m_texture),
m_size(p_rhs.m_size),
m_blockSize(p_rhs.m_blockSize),
m_borderRect(p_rhs.m_borderRect)
{
	// Copy all segments
	for (int i = 0; i < BorderSection_Count; ++i)
	{
		using engine::renderer::QuadSprite;
		m_borderSegs[i].reset(new QuadSprite(*(p_rhs.m_borderSegs[i])));
	}
}


void SunkenBorderDecorator::setBorderTexture(
		real p_x, real p_y, real p_w, real p_h,
		const engine::renderer::ColorRGBA& p_color)
{
	m_blockSize.x = p_w / 3.0f;
	m_blockSize.y = p_h / 3.0f;
	
	// Build the quads
	using engine::renderer::QuadSprite;
	
	// Construct quad sprite for center
	m_borderSegs[BorderSection_Center] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_Center], 1, 1, m_blockSize, p_x, p_y);
	
	// Top edge
	m_borderSegs[BorderSection_EdgeTop] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_EdgeTop], 1, 0, m_blockSize, p_x, p_y);
	
	// Bot edge
	m_borderSegs[BorderSection_EdgeBottom] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_EdgeBottom], 1, 2, m_blockSize, p_x, p_y);
	
	// Left edge
	m_borderSegs[BorderSection_EdgeLeft] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_EdgeLeft], 0, 1, m_blockSize, p_x, p_y);
	
	// Right edge
	m_borderSegs[BorderSection_EdgeRight] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_EdgeRight], 2, 1, m_blockSize, p_x, p_y);
	
	// Topleft corner
	m_borderSegs[BorderSection_CornerTopLeft] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_CornerTopLeft], 0, 0, m_blockSize, p_x, p_y);
	
	// Topright corner
	m_borderSegs[BorderSection_CornerTopRight] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_CornerTopRight], 2, 0, m_blockSize, p_x, p_y);
	
	// Botleft corner
	m_borderSegs[BorderSection_CornerBottomLeft] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_CornerBottomLeft], 0, 2, m_blockSize, p_x, p_y);
	
	// Botright corner
	m_borderSegs[BorderSection_CornerBottomRight] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_borderSegs[BorderSection_CornerBottomRight], 2, 2, m_blockSize, p_x, p_y);
}


/**
 * Set the UVs of a quadsprite based on internal block grid
 * 
 * @param p_qspr   quad sprite
 * @param p_blockX x block 0-1
 * @param p_blockY y block 0-1
 */
void SunkenBorderDecorator::setUVs(
		const engine::renderer::QuadSpritePtr& p_quad,
		s32                                    p_blockX,
		s32                                    p_blockY,
		const math::Vector2&                   p_blockSize,
		real                                   p_topLeftX,
		real                                   p_topLeftY)
{
	real blockW = p_blockSize.x;
	real blockH = p_blockSize.y;
	
	TT_ASSERTMSG(blockW > 0.0f && blockH > 0.0f,
	             "Attempt to construct SunkenBorderDecorator with fishy UV bounds");
	
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
