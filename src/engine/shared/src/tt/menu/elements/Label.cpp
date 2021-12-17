#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/elements/Label.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/common.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;
using engine::glyph::GlyphSet;


//------------------------------------------------------------------------------
// Public member functions

Label::Label(const std::string& p_name,
             const MenuLayout&  p_layout,
             const std::string& p_caption,
             bool               p_multiLine,
             s32                p_bottomLineOffset)
:
MenuElement(p_name, p_layout),
m_text(MenuSystem::getInstance()->translateString(p_caption))
{
	MENU_CREATION_Printf("Label::Label: Element '%s': "
	                     "New Label with caption '%s'.\n",
	                     getName().c_str(), p_caption.c_str());
	
	construct(p_multiLine, p_bottomLineOffset);
}


Label::Label(const std::string&  p_name,
             const MenuLayout&   p_layout,
             const std::wstring& p_caption,
             bool                p_multiLine,
             s32                 p_bottomLineOffset)
:
MenuElement(p_name, p_layout),
m_text(p_caption)
{
	MENU_CREATION_Printf("Label::Label: Element '%s': "
	                     "New Label with literal caption.\n", getName().c_str());
	
	construct(p_multiLine, p_bottomLineOffset);
}


Label::~Label()
{
	MENU_CREATION_Printf("Label::~Label: Element '%s': Freeing resources.\n",
	                     getName().c_str());
	
	unloadResources();
	delete m_scrollbar;
}


void Label::loadResources()
{
	// Don't initialize if the texture dimensions are 0
	if (m_textureWidth == 0 || m_textureHeight == 0)
	{
		MENU_Printf("Label::loadResources: Texture size is 0; not creating "
		            "texture or quads.\n");
		m_texture.reset();
		m_body.reset();
		m_shade.reset();
		return;
	}
	
	if (m_texture != 0)
	{
		// Resources already loaded
		return;
	}
	
	// Create quads for the text and the shadow
	//m_shouldRender = true;
	
	// Create the texture and quads
	m_texture = engine::renderer::Texture::createForText(
		static_cast<s16>(m_textureWidth),
		static_cast<s16>(m_textureHeight));
	
	/*
	bool valid = m_texture->Initialise(static_cast<s16>(m_textureWidth),
	                                   static_cast<s16>(m_textureHeight),
	                                   CO3DTexture::eFORMAT_8Alpha32Col);
	
	TT_ASSERTMSG(valid, "Initializing texture failed.");
	
	m_texture->SetFlag(CO3DTexture::eFLAG_RemoveFromMemory);
	
	// Make palette
	GXRgb* pal = m_texture->LockPalette();
	
	for (int c = 0; c < 32; ++c)
	{
		pal[c] = GX_RGB(31 - c, 31 - c, 31 - c);
	}
	
	// Unlock pal, upload
	m_texture->UnlockPalette();
	*/
	
	// Create the two quads
	m_body = engine::renderer::QuadSprite::createQuad(m_texture,
	                                                  getBodyQuadColor());
	m_shade = engine::renderer::QuadSprite::createQuad(m_texture,
	                                                   getShadowQuadColor());
	
	/*
	// Set alpha so the blending between text and shade will go right
	m_body->SetAlpha(31);
	m_shade->SetAlpha(31);
	*/
	
	
	// Render the text onto the texture
	renderTexture();
	m_shouldRender = false;
	
	/*
	{
		std::string captionNarrow(str::narrow(m_text));
		if (captionNarrow.length() > 30)
		{
			captionNarrow.erase(30);
			captionNarrow += " ...";
		}
		
		TT_Printf("Label::loadResources: [%s] Caption: '%s'. Rect: (%d, %d, %d, %d).\n",
		          getName().c_str(), captionNarrow.c_str(),
		          getRectangle().getX(),     getRectangle().getY(),
		          getRectangle().getWidth(), getRectangle().getHeight());
	}
	//*/
}


void Label::unloadResources()
{
	m_body.reset();
	m_shade.reset();
	m_texture.reset();
}


void Label::update()
{
	if (m_shouldRender && renderTexture())
	{
		m_shouldRender = false;
	}
}


void Label::render(const PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Don't render if no quads are available
	if (m_body == 0 || m_shade == 0 || m_texture == 0)
	{
		return;
	}
	
	
	if (m_multiLine == false && m_captionWidth > p_rect.getWidth())
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
	
	/*
	s32 width  = p_rect.getWidth();
	s32 height = p_rect.getHeight();
	if (height > m_textureHeight) height = m_textureHeight;
	
	if (m_multiLine || m_halign == GlyphSet::ALIGN_CENTER)
	{
		if (width > m_textureWidth) width = m_textureWidth;
	}
	else
	{
		s32 maxWidth = std::min(m_textureWidth, m_captionWidth);
		if (width > maxWidth) width = maxWidth;
	}
	//*/
	
	// Render label shadow
	m_shade->setPosition(pos + m_shadowOff);
	/*
	m_shade->setWidth (width);
	m_shade->setHeight(height);
	m_shade->setTexcoordLeft(static_cast<u16>(m_scrollOffset));
	m_shade->setTexcoordRight(static_cast<u16>(width + m_scrollOffset));
	m_shade->setTexcoordBottom(static_cast<u16>(height));
	*/
	
	m_shade->update();
	m_shade->render();
	
	// Render label text
	m_body->setPosition(pos);
	/*
	m_body->setWidth (width);
	m_body->setHeight(height);
	m_body->setTexcoordLeft(static_cast<u16>(m_scrollOffset));
	m_body->setTexcoordRight(static_cast<u16>(width + m_scrollOffset));
	m_body->setTexcoordBottom(static_cast<u16>(height));
	*/
	
	m_body->update();
	m_body->render();
	
	if (m_scrollbar != 0)
	{
		PointRect scrollRect(p_rect);
		scrollRect.setWidth(m_scrollbar->getMinimumWidth());
		scrollRect.setPosition(math::Point2(
			scrollRect.getPosition().x + p_rect.getWidth() - scrollRect.getWidth(),
			scrollRect.getPosition().y));
		m_scrollbar->render(scrollRect, p_z);
	}
}


void Label::doLayout(const PointRect& p_rect)
{
	if (m_multiLine)
	{
		// Get valid texture dimensions based on the element dimensions
		math::Point2 texSize(engine::renderer::Texture::getMinimalDimensions(
			p_rect.getWidth(), p_rect.getHeight()));
		m_textureWidth  = texSize.x;
		m_textureHeight = texSize.y;
		
		//*
		MENU_Printf("Label::doLayout: Element '%s': Rect dimensions: (%d, %d).\n",
		            getName().c_str(), p_rect.getWidth(), p_rect.getHeight());
		MENU_Printf("Label::doLayout: Element '%s': Creating new texture of size (%d, %d).\n",
		            getName().c_str(), m_textureWidth, m_textureHeight);
		//*/
	}
}


bool Label::doAction(const MenuElementAction& p_action)
{
	// Allow base to handle it first
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	
	std::string command(p_action.getCommand());
	if (command == "set_horizontal_text_align")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "the horizontal text alignment to set.",
		             command.c_str());
		
		setHorizontalAlign(getAlignmentFromString(p_action.getParameter(0)));
		
		return true;
	}
	else if (command == "set_vertical_text_align")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "the vertical text alignment to set.",
		             command.c_str());
		
		setVerticalAlign(getAlignmentFromString(p_action.getParameter(0)));
		
		return true;
	}
	
	// Did not handle action
	return false;
}


bool Label::onStylusPressed(s32 p_x, s32 p_y)
{
	m_scrollPick = false;
	if (m_scrollbar != 0 && p_x >= m_scrollX)
	{
		m_scrollPick = true;
		bool ret = m_scrollbar->onStylusPressed(p_x - m_scrollX, p_y);
		
		s32 oldTopLine = m_topLine;
		
		// Get the index of the top-most visible line
		m_topLine = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topLine != oldTopLine)
		{
			// Trigger redraw
			m_shouldRender = true;
		}
		return ret;
	}
	
	return false;
}


bool Label::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	if (m_scrollbar != 0 && m_scrollPick)
	{
		bool ret = m_scrollbar->onStylusDragging(p_x - m_scrollX, p_y, p_isInside);
		
		s32 oldTopLine = m_topLine;
		
		// Get the index of the top-most visible line
		m_topLine = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topLine != oldTopLine)
		{
			// Trigger redraw
			m_shouldRender = true;
		}
		return ret;
	}
	
	return false;
}


bool Label::onStylusReleased(s32 p_x, s32 p_y)
{
	if (m_scrollbar != 0 && m_scrollPick)
	{
		bool ret = m_scrollbar->onStylusReleased(p_x - m_scrollX, p_y);
		
		s32 oldTopLine = m_topLine;
		
		// Get the index of the top-most visible line
		m_topLine = static_cast<s32>(m_scrollbar->getValue());
		
		if (m_topLine != oldTopLine)
		{
			// Trigger redraw
			m_shouldRender = true;
		}
		return ret;
	}
	
	return false;
}


bool Label::onKeyPressed(const MenuKeyboard& p_keys)
{
	// Ignore input when not enabled or not visible
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_scrollbar == 0)
	{
		return false;
	}
	
	s32  oldTopLine = m_topLine;
	bool handled    = false;
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_DOWN))
	{
		// Next line selected
		++m_topLine;
		if (m_topLine > m_lines - m_linesPerPage)
		{
			m_topLine = m_lines - m_linesPerPage;
			return false;
		}
		m_scrollbar->setValue(static_cast<real>(m_topLine));
		handled = true;
	}
	else if (p_keys.isKeySet(MenuKeyboard::MENU_UP))
	{
		// Previous line selected
		--m_topLine;
		if (m_topLine < 0)
		{
			m_topLine = 0;
			return false;
		}
		
		m_scrollbar->setValue(static_cast<real>(m_topLine));
		handled = true;
	}
	
	if (m_topLine != oldTopLine)
	{
		m_shouldRender = true;
	}
	
	return handled;
}


bool Label::canHaveFocus() const
{
	// Element can have focus if it has a scroll bar
	return m_scrollbar != 0;
}


void Label::setEnabled(bool p_enabled)
{
	MenuElement::setEnabled(p_enabled);
	updateQuadColor();
}


void Label::setSelected(bool p_selected)
{
	MenuElement::setSelected(p_selected);
	updateQuadColor();
}


Label* Label::clone() const
{
	return new Label(*this);
}


GlyphSet::Alignment Label::getHorizontalAlign() const
{
	return m_halign;
}


GlyphSet::Alignment Label::getVerticalAlign() const
{
	return m_valign;
}


engine::renderer::ColorRGBA Label::getTextColor() const
{
	return m_textColor;
}


engine::renderer::ColorRGBA Label::getShadowColor() const
{
	return m_shadowColor;
}


engine::renderer::ColorRGBA Label::getDisabledColor() const
{
	return m_disabledColor;
}


engine::renderer::ColorRGBA Label::getSelectedColor() const
{
	return m_selectedColor;
}


void Label::setHorizontalAlign(GlyphSet::Alignment p_align)
{
	if (p_align != GlyphSet::ALIGN_LEFT   &&
	    p_align != GlyphSet::ALIGN_CENTER &&
	    p_align != GlyphSet::ALIGN_RIGHT)
	{
		TT_PANIC("Invalid horizontal text alignment: %d", p_align);
		return;
	}
	
	m_halign = p_align;
	//m_shouldRender = true;
}


void Label::setVerticalAlign(GlyphSet::Alignment p_align)
{
	if (p_align != GlyphSet::ALIGN_TOP    &&
	    p_align != GlyphSet::ALIGN_CENTER &&
	    p_align != GlyphSet::ALIGN_BOTTOM)
	{
		TT_PANIC("Invalid vertical text alignment: %d", p_align);
		return;
	}
	
	m_valign = p_align;
	//m_shouldRender = true;
}


void Label::setTextColor(const engine::renderer::ColorRGBA& p_color)
{
	m_textColor = p_color;
	updateQuadColor();
}


void Label::setShadowColor(const engine::renderer::ColorRGBA& p_color)
{
	m_shadowColor = p_color;
	updateQuadColor();
}


void Label::setDisabledColor(const engine::renderer::ColorRGBA& p_color)
{
	m_disabledColor = p_color;
	updateQuadColor();
}


void Label::setSelectedColor(const engine::renderer::ColorRGBA& p_color)
{
	m_selectedColor = p_color;
	updateQuadColor();
}


GlyphSet::Alignment Label::getAlignmentFromString(const std::string& p_align)
{
	if (p_align == "left")
	{
		return GlyphSet::ALIGN_LEFT;
	}
	else if (p_align == "center")
	{
		return GlyphSet::ALIGN_CENTER;
	}
	else if (p_align == "right")
	{
		return GlyphSet::ALIGN_RIGHT;
	}
	else if (p_align == "top")
	{
		return GlyphSet::ALIGN_TOP;
	}
	else if (p_align == "bottom")
	{
		return GlyphSet::ALIGN_BOTTOM;
	}
	
	TT_PANIC("Invalid alignment specified: '%s'", p_align.c_str());
	return GlyphSet::ALIGN_CENTER;
}


//------------------------------------------------------------------------------
// Protected member functions

Label::Label(const Label& p_rhs)
:
MenuElement(p_rhs),
m_text(p_rhs.m_text),
m_captionWidth(p_rhs.m_captionWidth),
m_captionHeight(p_rhs.m_captionHeight),
m_textureWidth(p_rhs.m_textureWidth),
m_textureHeight(p_rhs.m_textureHeight),
m_shadowOff(p_rhs.m_shadowOff),
m_halign(p_rhs.m_halign),
m_valign(p_rhs.m_valign),
m_shouldRender(p_rhs.m_shouldRender),
m_textColor(p_rhs.m_textColor),
m_shadowColor(p_rhs.m_shadowColor),
m_disabledColor(p_rhs.m_disabledColor),
m_selectedColor(p_rhs.m_selectedColor),
m_multiLine(p_rhs.m_multiLine),
m_bottomLineOffset(p_rhs.m_bottomLineOffset),
m_linesPerPage(p_rhs.m_linesPerPage),
m_lines(p_rhs.m_lines),
m_topLine(p_rhs.m_topLine),
m_scrollOffset(p_rhs.m_scrollOffset),
m_scrollRight(p_rhs.m_scrollRight),
m_scrollDelay(p_rhs.m_scrollDelay),
m_scrollbar(0),
m_scrollPick(p_rhs.m_scrollPick),
m_scrollX(p_rhs.m_scrollX)
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
	
	// Clone the scrollbar, if any
	if (p_rhs.m_scrollbar != 0)
	{
		m_scrollbar = p_rhs.m_scrollbar->clone();
	}
}


//------------------------------------------------------------------------------
// Private member functions

void Label::construct(bool p_multiLine, s32 p_bottomLineOffset)
{
	m_halign = GlyphSet::ALIGN_LEFT;
	m_valign = GlyphSet::ALIGN_TOP; // Default to top vertical alignment
	m_shadowOff.setValues(static_cast<real>(LABEL_SHADOW_OFFSET_X),
	                      static_cast<real>(LABEL_SHADOW_OFFSET_Y),
	                      1.0f);
	m_shouldRender     = false;
	m_textColor        = LABEL_DEFAULT_TEXT_COLOR;
	m_shadowColor      = LABEL_DEFAULT_SHADOW_COLOR;
	m_disabledColor    = LABEL_DEFAULT_DISABLED_COLOR;
	m_selectedColor    = LABEL_DEFAULT_SELECTED_COLOR;
	m_multiLine        = p_multiLine;
	m_bottomLineOffset = p_bottomLineOffset;
	
	m_lines            = 0;
	m_linesPerPage     = 0;
	m_scrollbar        = 0;
	m_scrollX          = 0;
	m_scrollPick       = false;
	
	m_scrollOffset = 0;
	m_scrollRight  = true;
	m_scrollDelay  = 0;
	
	// Get the dimensions of the caption string, in pixels
	engine::glyph::GlyphSet* glyphset = MenuSystem::getInstance()->getGlyphSet();
	m_captionWidth  = glyphset->getStringPixelWidth(m_text);
	m_captionHeight = glyphset->getHeight();
	
	// Get valid texture dimensions based on the caption dimensions
	math::Point2 texSize(engine::renderer::Texture::getMinimalDimensions(
		m_captionWidth, m_captionHeight));
	m_textureWidth  = texSize.x;
	m_textureHeight = texSize.y;
	
	MENU_CREATION_Printf("Label::construct: Element '%s': "
	                     "Caption dimensions: (%d, %d)\n",
	                     getName().c_str(), m_captionWidth, m_captionHeight);
	MENU_CREATION_Printf("Label::construct: Element '%s': "
	                     "Texture dimensions: (%d, %d)\n",
	                     getName().c_str(), m_textureWidth, m_textureHeight);
	
	// Set the minimum and requested dimensions
	if (m_multiLine == false)
	{
		//setMinimumWidth (m_captionWidth  + std::abs(LABEL_SHADOW_OFFSET_X));
		setMinimumWidth(0);
		setMinimumHeight(m_captionHeight + std::abs(LABEL_SHADOW_OFFSET_Y));
		setRequestedWidth (m_captionWidth  + std::abs(LABEL_SHADOW_OFFSET_X));
		setRequestedHeight(m_captionHeight + std::abs(LABEL_SHADOW_OFFSET_Y));
	}
	else
	{
		TT_ASSERTMSG(getLayout().getWidthType() == MenuLayout::Size_Absolute,
		             "Label '%s' is multiline but does not have absolute width.",
		             getName().c_str());
		setMinimumWidth (getLayout().getWidth());
		setMinimumHeight(m_captionHeight + std::abs(LABEL_SHADOW_OFFSET_Y));
		s32 lineCount = glyphset->getLineCount(m_text,
			getLayout().getWidth() - std::abs(LABEL_SHADOW_OFFSET_X),
			0, 0);
		setRequestedWidth (m_captionWidth  + std::abs(LABEL_SHADOW_OFFSET_X));
		setRequestedHeight(lineCount * glyphset->getHeight());
	}
	
	
	// Label takes two Z levels (text and shadow)
	setDepth(2);
}


void Label::updateQuadColor()
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


engine::renderer::ColorRGBA Label::getBodyQuadColor() const
{
	if (isEnabled() == false)
	{
		return m_disabledColor;
	}
	
	if (isSelected() == false || m_scrollbar != 0)
	{
		return m_textColor;
	}
	
	return m_selectedColor;
}


engine::renderer::ColorRGBA Label::getShadowQuadColor() const
{
	return m_shadowColor;
}


bool Label::renderTexture()
{
	if (m_texture == 0)
	{
		return false;
	}
	
	
	engine::renderer::TexturePainter painter(m_texture->lock());
	painter.clear();
	
	// Render the string onto the texture
	GlyphSet* glyphset = MenuSystem::getInstance()->getGlyphSet();
	
	s32 rightMargin  = m_textureWidth  - getRectangle().getWidth();
	s32 bottomMargin = m_textureHeight - getRectangle().getHeight();
	if (rightMargin  < 0) rightMargin  = 0;
	if (bottomMargin < 0) bottomMargin = 0;
	
	if (m_multiLine)
	{
		if (m_lines == 0) // uninitialized
		{
			m_lines = glyphset->getLineCount(m_text, m_textureWidth,
			                                 0, rightMargin);
			
			s32 height = getRectangle().getHeight();
			if (height > m_textureHeight) height = m_textureHeight;
			m_linesPerPage = height / glyphset->getHeight();
			
			m_topLine = 0;
			
			// Check if a scroll bar is needed
			if (m_lines > m_linesPerPage)
			{
				MenuLayout m;
				m_scrollbar = new Scrollbar("", m);
				m_scrollX   = getRectangle().getWidth() -
				              m_scrollbar->getMinimumWidth();
				
				m_lines = glyphset->getLineCount(
					m_text, m_textureWidth, 0,
					rightMargin + m_scrollbar->getMinimumWidth());
				m_linesPerPage = height / glyphset->getHeight();
				
				m_scrollbar->setRange(0, static_cast<real>(m_lines - m_linesPerPage));
				m_scrollbar->setValue(0);
				m_scrollbar->setStepSize(1);
				//m_texture->ResetFlag(CO3DTexture::eFLAG_RemoveFromMemory);
			}
		}
		
		if (m_scrollbar == 0)
		{
			glyphset->drawMultiLineString(
				m_text,
				painter,
				engine::renderer::ColorRGB::white,
				m_halign,
				m_valign,
				m_bottomLineOffset,
				// margin left, top, right, bottom
				0,
				-(m_topLine * glyphset->getHeight()),
				rightMargin,
				bottomMargin);
		}
		else
		{
			glyphset->drawMultiLineString(
				m_text,
				painter,
				engine::renderer::ColorRGB::white,
				m_halign,
				m_valign,
				m_bottomLineOffset,
				// margin left, top, right, bottom
				0,
				-(m_topLine * glyphset->getHeight()),
				rightMargin + m_scrollbar->getMinimumWidth(),
				bottomMargin);
		}
	}
	else
	{
		if (m_captionWidth > getRectangle().getWidth())
		{
			rightMargin = m_textureWidth - m_captionWidth;
		}
		
		glyphset->drawTruncatedString(
			m_text,
			painter,
			engine::renderer::ColorRGB::white,
			m_halign,
			m_valign,
			m_bottomLineOffset,
			// margin left, top, right, bottom
			0, 0,
			rightMargin,
			0);
	}
	
	m_shouldRender = false;
	
	return true;
}

// Namespace end
}
}
}
