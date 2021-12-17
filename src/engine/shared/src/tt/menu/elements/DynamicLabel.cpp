#include <tt/engine/renderer/TexturePainter.h>
#include <tt/platform/tt_error.h>
#include <tt/menu/elements/DynamicLabel.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuUtils.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;
using engine::glyph::GlyphSet;


//------------------------------------------------------------------------------
// Public member functions

DynamicLabel::DynamicLabel(const std::string& p_name,
                           const MenuLayout&  p_layout,
                           const std::string& p_caption)
:
MenuElement(p_name, p_layout),
m_text(MenuSystem::getInstance()->translateString(p_caption))
{
	MENU_CREATION_Printf("DynamicLabel::DynamicLabel: Element '%s': "
	                     "New DynamicLabel with caption '%s'.\n",
	                     getName().c_str(), p_caption.c_str());
	
	construct();
}


DynamicLabel::DynamicLabel(const std::string&  p_name,
                           const MenuLayout&   p_layout,
                           const std::wstring& p_caption)
:
MenuElement(p_name, p_layout),
m_text(p_caption)
{
	MENU_CREATION_Printf("DynamicLabel::DynamicLabel: Element '%s': "
	                     "New DynamicLabel with literal caption.\n",
	                     getName().c_str());
	
	construct();
}


DynamicLabel::~DynamicLabel()
{
	MENU_CREATION_Printf("DynamicLabel::~DynamicLabel: Element '%s': "
	                     "Freeing resources.\n", getName().c_str());
	
	unloadResources();
}


void DynamicLabel::loadResources()
{
	// Bail if resources already loaded
	if (m_texture != 0)
	{
		return;
	}
	
	createTexture();
	
	// Don't create quads if texture size is 0
	if (m_textureWidth == 0 || m_textureHeight == 0)
	{
		m_body.reset();
		m_shade.reset();
		return;
	}
	
	// Create the two quads
	using engine::renderer::QuadSprite;
	m_body  = QuadSprite::createQuad(m_texture, getBodyQuadColor());
	m_shade = QuadSprite::createQuad(m_texture, getShadowQuadColor());
	
	// Set alpha so the blending between text and shade will go right
	//m_body->setAlpha(31);
	//m_shade->setAlpha(31);
	
	m_scrollOffset = 0;
	m_scrollRight  = true;
	m_scrollDelay  = 0;
}


void DynamicLabel::unloadResources()
{
	m_body.reset();
	m_shade.reset();
	m_texture.reset();
}


void DynamicLabel::update()
{
	if (m_shouldRender && renderTexture())
	{
		m_shouldRender = false;
	}
}


void DynamicLabel::render(const PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Don't render if no texture or quads are available
	if (m_texture == 0 || m_body == 0 || m_shade == 0)
	{
		return;
	}
	
	
	if (m_captionWidth > p_rect.getWidth())
	{
		if (m_scrollDelay > 0)
		{
			++m_scrollDelay;
			if (m_scrollDelay >= LABEL_SCROLL_DELAY)
			{
				m_scrollDelay = 0;
			}
		}
		else
		{
			if (m_scrollRight)
			{
				++m_scrollOffset;
				if (m_scrollOffset >= m_captionWidth - p_rect.getWidth())
				{
					m_scrollOffset = m_captionWidth - p_rect.getWidth();
					m_scrollRight  = false;
					m_scrollDelay  = 1;
				}
			}
			else
			{
				--m_scrollOffset;
				if (m_scrollOffset <= 0)
				{
					m_scrollOffset = 0;
					m_scrollRight  = true;
					m_scrollDelay  = 1;
				}
			}
		}
	}
	else
	{
		m_scrollOffset = 0;
		m_scrollRight  = true;
		m_scrollDelay  = 0;
	}
	
	
	math::Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
	                  static_cast<real>(p_rect.getCenterPosition().y),
	                  static_cast<real>(p_z - 1));
	
	// Render label shadow
	/*
	m_shade->setWidth (p_rect.getWidth());
	m_shade->setHeight(p_rect.getHeight());
	m_shade->setTexcoordLeft(static_cast<u16>(m_scrollOffset));
	m_shade->setTexcoordRight(static_cast<u16>(p_rect.getWidth() + m_scrollOffset));
	m_shade->setTexcoordBottom(static_cast<u16>(p_rect.getHeight()));
	*/
	
	m_shade->setPosition(pos + m_shadowOffset);
	m_shade->update();
	m_shade->render();
	
	
	// Render label text
	/*
	m_body->setWidth (p_rect.getWidth());
	m_body->setHeight(p_rect.getHeight());
	m_body->setTexcoordLeft(static_cast<u16>(m_scrollOffset));
	m_body->setTexcoordRight(static_cast<u16>(p_rect.getWidth() + m_scrollOffset));
	m_body->setTexcoordBottom(static_cast<u16>(p_rect.getHeight()));
	*/
	
	m_body->setPosition(pos);
	
	m_body->update();
	m_body->render();
}


void DynamicLabel::doLayout(const PointRect& p_rect)
{
	math::Point2 texSize(engine::renderer::Texture::getMinimalDimensions(
		p_rect.getWidth(), p_rect.getHeight()));
	
	m_textureWidth    = texSize.x;
	m_textureHeight   = texSize.y;
	m_minTextureWidth = m_textureWidth;
}


void DynamicLabel::setEnabled(bool p_enabled)
{
	MenuElement::setEnabled(p_enabled);
	updateQuadColor();
}


void DynamicLabel::setSelected(bool p_selected)
{
	MenuElement::setSelected(p_selected);
	updateQuadColor();
}


GlyphSet::Alignment DynamicLabel::getHorizontalAlign() const
{
	return m_halign;
}


GlyphSet::Alignment DynamicLabel::getVerticalAlign() const
{
	return m_valign;
}


engine::renderer::ColorRGBA DynamicLabel::getTextColor() const
{
	return m_textColor;
}


engine::renderer::ColorRGBA DynamicLabel::getShadowColor() const
{
	return m_shadowColor;
}


engine::renderer::ColorRGBA DynamicLabel::getDisabledColor() const
{
	return m_disabledColor;
}


engine::renderer::ColorRGBA DynamicLabel::getSelectedColor() const
{
	return m_selectedColor;
}


void DynamicLabel::setHorizontalAlign(GlyphSet::Alignment p_align)
{
	m_halign       = p_align;
	m_shouldRender = true;
}


void DynamicLabel::setVerticalAlign(GlyphSet::Alignment p_align)
{
	m_valign       = p_align;
	m_shouldRender = true;
}


void DynamicLabel::setTextColor(const engine::renderer::ColorRGBA& p_color)
{
	m_textColor = p_color;
	updateQuadColor();
}


void DynamicLabel::setShadowColor(const engine::renderer::ColorRGBA& p_color)
{
	m_shadowColor = p_color;
	updateQuadColor();
}


void DynamicLabel::setDisabledColor(const engine::renderer::ColorRGBA& p_color)
{
	m_disabledColor = p_color;
	updateQuadColor();
}


void DynamicLabel::setSelectedColor(const engine::renderer::ColorRGBA& p_color)
{
	m_selectedColor = p_color;
	updateQuadColor();
}


void DynamicLabel::setCaption(const std::wstring& p_caption)
{
	m_text = p_caption;
	
	// Get the dimensions of the caption string, in pixels
	GlyphSet* glyphset = MenuSystem::getInstance()->getGlyphSet();
	m_captionWidth  = glyphset->getStringPixelWidth(m_text);
	m_captionHeight = glyphset->getHeight();
	math::Point2 newTexSize(engine::renderer::Texture::getMinimalDimensions(
		m_captionWidth, m_captionHeight));
	if (newTexSize.x < m_minTextureWidth) newTexSize.x = m_minTextureWidth;
	
	if (m_textureWidth != newTexSize.x)
	{
		m_textureWidth = newTexSize.x;
		m_texture = engine::renderer::Texture::createForText(
			static_cast<s16>(m_textureWidth), static_cast<s16>(m_textureHeight));
		
		/*
		if (m_texture != 0)
		{
			m_texture->Initialise(static_cast<s16>(m_textureWidth),
			                      static_cast<s16>(m_textureHeight),
			                      CO3DImage::eFORMAT_8Alpha32Col);
			m_texture->ResetFlag(CO3DTexture::eFLAG_RemoveFromMemory);
			
			// Make palette
			GXRgb* pal = m_texture->LockPalette();
			
			for (int c = 0; c < 32; ++c)
			{
				pal[c] = GX_RGB(31 - c, 31 - c, 31 - c);
			}
			
			// Unlock palette
			m_texture->UnlockPalette();
		}
		*/
	}
	
	m_shouldRender = true;
}


void DynamicLabel::setCaption(const std::string& p_caption)
{
	setCaption(MenuSystem::getInstance()->translateString(p_caption));
}


bool DynamicLabel::doAction(const MenuElementAction& p_action)
{
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	if (p_action.getCommand() == "set_caption_hex")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "DynamicLabel: Command 'set_caption_hex' should have "
		             "1 parameter.");
		setCaption(MenuUtils::hexToWideString(p_action.getParameter(0)));
		return true;
	}
	else if (p_action.getCommand() == "set_caption")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "DynamicLabel: Command 'set_caption' should have "
		             "1 parameter.");
		setCaption(p_action.getParameter(0));
		return true;
	}
	else if (p_action.getCommand() == "get_caption")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Element command '%s' takes one parameter: "
		             "the name of the menu variable to store the "
		             "hex-encoded caption in.");
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(0),
		                                      MenuUtils::wideStringToHex(m_text));
		return true;
	}
	
	return false;
}


DynamicLabel* DynamicLabel::clone() const
{
	return new DynamicLabel(*this);
}


//------------------------------------------------------------------------------
// Protected functions

DynamicLabel::DynamicLabel(const DynamicLabel& p_rhs)
:
MenuElement(p_rhs),
m_text(p_rhs.m_text),
m_captionWidth(p_rhs.m_captionWidth),
m_captionHeight(p_rhs.m_captionHeight),
m_textureWidth(p_rhs.m_textureWidth),
m_textureHeight(p_rhs.m_textureHeight),
m_shadowOffset(p_rhs.m_shadowOffset),
m_halign(p_rhs.m_halign),
m_valign(p_rhs.m_valign),
m_shouldRender(p_rhs.m_shouldRender),
m_textColor(p_rhs.m_textColor),
m_shadowColor(p_rhs.m_shadowColor),
m_disabledColor(p_rhs.m_disabledColor),
m_selectedColor(p_rhs.m_selectedColor),
m_scrollOffset(p_rhs.m_scrollOffset),
m_scrollRight(p_rhs.m_scrollRight),
m_scrollDelay(p_rhs.m_scrollDelay)
{
	// Copy the texture
	if (p_rhs.m_texture != 0)
	{
		m_texture = engine::renderer::Texture::createForText(
			static_cast<s16>(m_textureWidth),
			static_cast<s16>(m_textureHeight));
		bool prevRenderFlag = m_shouldRender;
		renderTexture();
		m_shouldRender = prevRenderFlag;
	}
	
	// Copy the quads and update their texture pointers
	if (p_rhs.m_body != 0)
	{
		m_body.reset(new engine::renderer::QuadSprite(*(p_rhs.m_body)));
		m_body->setTexture(m_texture);
	}
	
	if (p_rhs.m_shade != 0)
	{
		m_shade.reset(new engine::renderer::QuadSprite(*(p_rhs.m_shade)));
		m_shade->setTexture(m_texture);
	}
}


//------------------------------------------------------------------------------
// Private functions

void DynamicLabel::construct()
{
	// Initialize member variables
	m_halign        = GlyphSet::ALIGN_LEFT;
	m_valign        = GlyphSet::ALIGN_CENTER;
	m_shadowOffset.setValues(static_cast<real>(LABEL_SHADOW_OFFSET_X),
	                         static_cast<real>(LABEL_SHADOW_OFFSET_Y),
	                         static_cast<real>(1));
	m_textColor     = LABEL_DEFAULT_TEXT_COLOR;
	m_shadowColor   = LABEL_DEFAULT_SHADOW_COLOR;
	m_disabledColor = LABEL_DEFAULT_DISABLED_COLOR;
	m_selectedColor = LABEL_DEFAULT_SELECTED_COLOR;
	m_shouldRender  = false;
	m_scrollOffset  = 0;
	m_scrollRight   = true;
	m_scrollDelay   = 0;
	
	
	// Get the dimensions of the caption string, in pixels
	GlyphSet* glyphset = MenuSystem::getInstance()->getGlyphSet();
	
	m_captionWidth  = glyphset->getStringPixelWidth(m_text);
	m_captionHeight = glyphset->getHeight();
	
	// Calculate the smallest power of 2 for the texture,
	// based on the caption dimensions
	MENU_CREATION_Printf("DynamicLabel::construct: Caption dimensions: (%d, %d)\n",
	                     m_captionWidth, m_captionHeight);
	
	// Set the minimum and requested dimensions
	setMinimumWidth   (getLayout().getWidth()  + std::abs(LABEL_SHADOW_OFFSET_X));
	setMinimumHeight  (getLayout().getHeight() + std::abs(LABEL_SHADOW_OFFSET_Y));
	setRequestedWidth (std::max(m_captionWidth,  getLayout().getWidth())  +
	                   std::abs(LABEL_SHADOW_OFFSET_X));
	setRequestedHeight(std::max(m_captionHeight, getLayout().getHeight()) +
	                   std::abs(LABEL_SHADOW_OFFSET_Y));
	
	setDepth(2);
}


void DynamicLabel::createTexture()
{
	TT_ASSERTMSG(m_texture == 0, "Texture was already created.");
	
	// Don't create a texture if its size is 0
	if (m_textureWidth == 0 || m_textureHeight == 0)
	{
		m_body.reset();
		m_shade.reset();
		m_texture.reset();
		return;
	}
	
	m_texture = engine::renderer::Texture::createForText(
		static_cast<s16>(m_textureWidth), static_cast<s16>(m_textureHeight));
	
	m_shouldRender = true;
}


void DynamicLabel::updateQuadColor()
{
	if (m_body != 0)
	{
		engine::renderer::ColorRGBA newColor = getBodyQuadColor();
		//if (m_body->getColor() != newColor)
		//{
			m_body->setColor(newColor);
		//}
	}
	
	if (m_shade != 0)
	{
		engine::renderer::ColorRGBA newColor = getShadowQuadColor();
		//if (m_shade->getColor() != newColor)
		//{
			m_shade->setColor(newColor);
		//}
	}
}


engine::renderer::ColorRGBA DynamicLabel::getBodyQuadColor() const
{
	if (isEnabled() == false)
	{
		return m_disabledColor;
	}
	
	if (isSelected() == false)
	{
		return m_textColor;
	}
	
	return m_selectedColor;
}


engine::renderer::ColorRGBA DynamicLabel::getShadowQuadColor() const
{
	return m_shadowColor;
}


bool DynamicLabel::renderTexture()
{
	if (m_texture == 0)
	{
		return false;
	}
	
	/*
	MENU_Printf("DynamicLabel: Caption dimensions: (%d, %d)\n",
	            m_captionWidth, m_captionHeight);
	MENU_Printf("DynamicLabel: Texture dimensions: (%d, %d)\n",
	            m_textureWidth, m_textureHeight);
	
	MENU_Printf("Render DynamicLabel '%s' of size: %d,%d @ %d,%d\n",
	            getName().c_str(), p_rect.getWidth(), p_rect.getHeight(),
	            p_rect.getX(), p_rect.getY());
	MENU_Printf("DynamicLabel::render: Element '%s': "
	            "Rendering text string onto texture.\n",
	            getName().c_str());
	//*/
	
	engine::renderer::TexturePainter painter(m_texture->lock());
	painter.clear();
	
	// Render the string onto the texture
	GlyphSet* glyphset = MenuSystem::getInstance()->getGlyphSet();
	
	s32 rightMargin  = m_textureWidth  - getRectangle().getWidth();
	s32 bottomMargin = m_textureHeight - getRectangle().getHeight();
	
	if (m_captionWidth > getRectangle().getWidth())
	{
		rightMargin = m_textureWidth - m_captionWidth;
	}
	else
	{
		rightMargin = m_textureWidth - getRectangle().getWidth();
	}
	
	if (rightMargin  < 0) rightMargin  = 0;
	if (bottomMargin < 0) bottomMargin = 0;
	
	
	glyphset->drawTruncatedString(
		m_text,
		painter,
		engine::renderer::ColorRGB::white,
		m_halign,
		m_valign,
		0,
		// margin left, top, right, bottom
		0, 0, rightMargin, bottomMargin);
	
	m_shouldRender = false;
	
	return true;
}

// Namespace end
}
}
}
