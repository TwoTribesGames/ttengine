#include <tt/engine/glyph/Glyph.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/menu/elements/MultiPageLabel.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuDebug.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;
using engine::glyph::GlyphSet;
using engine::glyph::Glyph;


//------------------------------------------------------------------------------
// Public member functions

MultiPageLabel::MultiPageLabel(const std::string& p_name,
                               const MenuLayout&  p_layout,
                               const std::string& p_caption,
                               const std::string& p_prev,
                               const std::string& p_next,
                               bool               p_prevHasOther,
                               bool               p_nextHasOther,
                               const std::string& p_buttonContainer)
:
MenuElement(p_name, p_layout),
m_text(MenuSystem::getInstance()->translateString(p_caption)),
m_prev(p_prev),
m_next(p_next),
m_prevHasOther(p_prevHasOther),
m_nextHasOther(p_nextHasOther),
m_buttonContainer(p_buttonContainer)
{
	MENU_CREATION_Printf("MultiPageLabel::MultiPageLabel: Element '%s': "
	                     "New MultiPageLabel with caption '%s'.\n",
	                     getName().c_str(), p_caption.c_str());
	
	construct();
}


MultiPageLabel::MultiPageLabel(const std::string&  p_name,
                               const MenuLayout&   p_layout,
                               const std::wstring& p_caption,
                               const std::string&  p_prev,
                               const std::string&  p_next,
                               bool                p_prevHasOther,
                               bool                p_nextHasOther,
                               const std::string&  p_buttonContainer)
:
MenuElement(p_name, p_layout),
m_text(p_caption),
m_prev(p_prev),
m_next(p_next),
m_prevHasOther(p_prevHasOther),
m_nextHasOther(p_nextHasOther),
m_buttonContainer(p_buttonContainer)
{
	MENU_CREATION_Printf("MultiPageLabel::MultiPageLabel: Element '%s': "
	                     "New MultiPageLabel with literal caption.\n",
	                     getName().c_str());
	
	construct();
}


MultiPageLabel::~MultiPageLabel()
{
	MENU_CREATION_Printf("MultiPageLabel::~MultiPageLabel: Element '%s': "
	                     "Freeing resources.\n", getName().c_str());
	
	unloadResources();
}


void MultiPageLabel::loadResources()
{
	// Don't initialize if the texture dimensions are 0
	if (m_textureWidth == 0 || m_textureHeight == 0)
	{
		MENU_Printf("MultiPageLabel::doLayout: Texture size is 0; not creating "
		            "texture or quads.\n");
		m_texture.reset();
		m_body.reset();
		m_shade.reset();
		return;
	}
	
	if (m_texture != 0)
	{
		return;
	}
	
	// Create quads for the text and the shadow
	m_shouldRender = true;
	
	// Create the texture and quads
	m_texture = engine::renderer::Texture::createForText(
		static_cast<s16>(m_textureWidth), static_cast<s16>(m_textureHeight));
	
	/*
	bool valid = m_texture->Initialise(static_cast<s16>(m_textureWidth),
	                                   static_cast<s16>(m_textureHeight),
	                                   CO3DTexture::eFORMAT_8Alpha32Col);
	
	TT_ASSERTMSG(valid, "Initializing texture failed.");
	*/
	
	// Do not remove texture from memory
	//m_texture->ResetFlag(CO3DTexture::eFLAG_RemoveFromMemory);
	
	/*
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
	using engine::renderer::QuadSprite;
	m_body  = QuadSprite::createQuad(m_texture, getBodyQuadColor());
	m_shade = QuadSprite::createQuad(m_texture, getShadowQuadColor());
	
	// Set alpha so the blending between text and shade will go right
	//m_body->setAlpha(31);
	//m_shade->setAlpha(31);
}


void MultiPageLabel::unloadResources()
{
	m_body.reset();
	m_shade.reset();
	m_texture.reset();
}


void MultiPageLabel::update()
{
	if (m_shouldRender && renderTexture())
	{
		m_shouldRender = false;
	}
}


void MultiPageLabel::render(const PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Don't render if no quads are available
	if (m_body == 0 || m_shade == 0)
	{
		return;
	}
	
	
	math::Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
	                  static_cast<real>(p_rect.getCenterPosition().y),
	                  static_cast<real>(p_z - 1));
	
	// Render drop shadow
	m_shade->setPosition(pos + m_shadowOff);
	//m_shade->setWidth (std::min(p_rect.getWidth(), m_textureWidth));
	//m_shade->setHeight(std::min(p_rect.getHeight(), m_textureHeight));
	//m_shade->setTexcoordRight(static_cast<u16>(std::min(p_rect.getWidth(), m_textureWidth)));
	//m_shade->setTexcoordBottom(static_cast<u16>(std::min(p_rect.getHeight(), m_textureHeight)));
	
	m_shade->update();
	m_shade->render();
	
	// Render text
	m_body->setPosition(pos);
	//m_body->setWidth (std::min(p_rect.getWidth(),  m_textureWidth));
	//m_body->setHeight(std::min(p_rect.getHeight(), m_textureHeight));
	//m_body->setTexcoordRight(static_cast<u16>(std::min(p_rect.getWidth(),  m_textureWidth)));
	//m_body->setTexcoordBottom(static_cast<u16>(std::min(p_rect.getHeight(), m_textureHeight)));
	
	m_body->update();
	m_body->render();
}


void MultiPageLabel::doLayout(const PointRect& p_rect)
{
	TT_ASSERTMSG(m_pages.empty(), "Pages have already been created!");
	
	// Get valid texture dimensions based on the element dimensions
	math::Point2 texSize(engine::renderer::Texture::getMinimalDimensions(
		p_rect.getWidth(), p_rect.getHeight()));
	m_textureWidth  = texSize.x;
	m_textureHeight = texSize.y;
	
	// Create pages
	GlyphSet* gs = MenuSystem::getInstance()->getGlyphSet();
	gs->createPages(m_text, m_pages,
	                p_rect.getWidth()  - std::abs(LABEL_SHADOW_OFFSET_X),
	                p_rect.getHeight() - std::abs(LABEL_SHADOW_OFFSET_Y),
	                0, 0, 0, 0, true);
	m_text.clear();
}


void MultiPageLabel::onMenuActivated()
{
	if (m_prev.empty() == false)
	{
		MenuElementAction mea(m_prev, "show_value");
		mea.addParameter("other");
		
		MenuSystem::getInstance()->doMenuElementAction(mea);
		MenuSystem::getInstance()->resetSelectedElement();
	}
	
	if (m_pages.size() > 1)
	{
		// There is more than one page; "Next" should be available
		if (m_next.empty() == false)
		{
			MenuElementAction mea(m_next, "show_value");
			mea.addParameter("next");
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
			MenuSystem::getInstance()->resetSelectedElement();
		}
	}
	else
	{
		// There is only one page (or none at all); no "Next" available
		if (m_next.empty() == false)
		{
			MenuElementAction mea(m_next, "show_value");
			mea.addParameter("other");
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
			MenuSystem::getInstance()->resetSelectedElement();
		}
	}
}


bool MultiPageLabel::doAction(const MenuElementAction& p_action)
{
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	if (p_action.getCommand() == "prev")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "MultiPageLabel: Command 'prev' does not take any "
		             "parameters.");
		selectPrev();
		return true;
	}
	else if (p_action.getCommand() == "next")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "MultiPageLabel: Command 'next' does not take any "
		             "parameters.");
		selectNext();
		return true;
	}
	
	return false;
}


void MultiPageLabel::setEnabled(bool p_enabled)
{
	MenuElement::setEnabled(p_enabled);
	updateQuadColor();
}


void MultiPageLabel::setSelected(bool p_selected)
{
	MenuElement::setSelected(p_selected);
	updateQuadColor();
}


MultiPageLabel* MultiPageLabel::clone() const
{
	return new MultiPageLabel(*this);
}


GlyphSet::Alignment MultiPageLabel::getHorizontalAlign() const
{
	return m_halign;
}


GlyphSet::Alignment MultiPageLabel::getVerticalAlign() const
{
	return m_valign;
}


engine::renderer::ColorRGBA MultiPageLabel::getTextColor() const
{
	return m_textColor;
}


engine::renderer::ColorRGBA MultiPageLabel::getShadowColor() const
{
	return m_shadowColor;
}


engine::renderer::ColorRGBA MultiPageLabel::getDisabledColor() const
{
	return m_disabledColor;
}


engine::renderer::ColorRGBA MultiPageLabel::getSelectedColor() const
{
	return m_selectedColor;
}


void MultiPageLabel::setHorizontalAlign(GlyphSet::Alignment p_align)
{
	m_halign       = p_align;
	m_shouldRender = true;
}


void MultiPageLabel::setVerticalAlign(GlyphSet::Alignment p_align)
{
	m_valign       = p_align;
	m_shouldRender = true;
}


void MultiPageLabel::setTextColor(const engine::renderer::ColorRGBA& p_color)
{
	m_textColor = p_color;
	updateQuadColor();
}


void MultiPageLabel::setShadowColor(const engine::renderer::ColorRGBA& p_color)
{
	m_shadowColor = p_color;
	updateQuadColor();
}


void MultiPageLabel::setDisabledColor(const engine::renderer::ColorRGBA& p_color)
{
	m_disabledColor = p_color;
	updateQuadColor();
}


void MultiPageLabel::setSelectedColor(const engine::renderer::ColorRGBA& p_color)
{
	m_selectedColor = p_color;
	updateQuadColor();
}


//------------------------------------------------------------------------------
// Protected member functions

MultiPageLabel::MultiPageLabel(const MultiPageLabel& p_rhs)
:
MenuElement(p_rhs),
m_text(p_rhs.m_text),
m_prev(p_rhs.m_prev),
m_next(p_rhs.m_next),
m_prevHasOther(p_rhs.m_prevHasOther),
m_nextHasOther(p_rhs.m_nextHasOther),
m_buttonContainer(p_rhs.m_buttonContainer),
m_pages(p_rhs.m_pages),
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
m_page(p_rhs.m_page)
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


void MultiPageLabel::selectPrev()
{
	if (m_page > 0)
	{
		--m_page;
		if (m_texture != 0)
		{
			m_shouldRender = true;
		}
		
		if (m_page == 0)
		{
			if (m_prev.empty() == false)
			{
				MenuElementAction mea(m_prev, "show_value");
				mea.addParameter("other");
				MenuSystem::getInstance()->doMenuElementAction(mea);
				
				if (m_prevHasOther == false &&
				    m_buttonContainer.empty() == false &&
				    m_next.empty() == false)
				{
					MenuElementAction mea2(m_buttonContainer,
					                      "set_selected_child_name");
					mea2.addParameter(m_next);
					MenuSystem::getInstance()->doMenuElementAction(mea2);
				}
				
				MenuSystem::getInstance()->resetSelectedElement();
			}
		}
		
		if (m_page < static_cast<s32>(m_pages.size()) - 1)
		{
			if (m_next.empty() == false)
			{
				MenuElementAction mea(m_next, "show_value");
				mea.addParameter("next");
				MenuSystem::getInstance()->doMenuElementAction(mea);
				MenuSystem::getInstance()->resetSelectedElement();
			}
		}
	}
}


void MultiPageLabel::selectNext()
{
	if (m_page < static_cast<s32>(m_pages.size()) - 1)
	{
		++m_page;
		if (m_texture != 0)
		{
			m_shouldRender = true;
		}
		
		if (m_page == static_cast<s32>(m_pages.size()) - 1)
		{
			if (m_next.empty() == false)
			{
				MenuElementAction mea(m_next, "show_value");
				mea.addParameter("other");
				MenuSystem::getInstance()->doMenuElementAction(mea);
				
				if (m_nextHasOther == false &&
				    m_buttonContainer.empty() == false &&
				    m_prev.empty() == false)
				{
					MenuElementAction mea2(m_buttonContainer,
					                      "set_selected_child_name");
					mea2.addParameter(m_prev);
					MenuSystem::getInstance()->doMenuElementAction(mea2);
				}
				
				MenuSystem::getInstance()->resetSelectedElement();
			}
		}
		
		if (m_page > 0)
		{
			if (m_prev.empty() == false)
			{
				MenuElementAction mea(m_prev, "show_value");
				mea.addParameter("prev");
				
				MenuSystem::getInstance()->doMenuElementAction(mea);
				MenuSystem::getInstance()->resetSelectedElement();
			}
		}
	}
}


//------------------------------------------------------------------------------
// Private member functions

void MultiPageLabel::construct()
{
	m_halign = GlyphSet::ALIGN_LEFT;
	m_valign = GlyphSet::ALIGN_TOP; // Default to top vertical alignment
	m_shadowOff.setValues(static_cast<real>(LABEL_SHADOW_OFFSET_X),
	                      static_cast<real>(LABEL_SHADOW_OFFSET_Y),
	                      1.0f);
	//m_shouldRender       = true;
	m_textColor     = LABEL_DEFAULT_TEXT_COLOR;
	m_shadowColor   = LABEL_DEFAULT_SHADOW_COLOR;
	m_disabledColor = LABEL_DEFAULT_DISABLED_COLOR;
	m_selectedColor = LABEL_DEFAULT_SELECTED_COLOR;
	m_page          = 0;
	
	// This element takes two Z levels (text and shadow)
	setDepth(2);
}


void MultiPageLabel::updateQuadColor()
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


engine::renderer::ColorRGBA MultiPageLabel::getBodyQuadColor() const
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


engine::renderer::ColorRGBA MultiPageLabel::getShadowQuadColor() const
{
	return m_shadowColor;
}


bool MultiPageLabel::renderTexture()
{
	if (m_texture == 0)
	{
		return false;
	}
	
	
	// Render the string onto the texture
	engine::renderer::TexturePainter painter(m_texture->lock());
	painter.clear();
	
	engine::glyph::GlyphSet* set = MenuSystem::getInstance()->getGlyphSet();
	
	s32 rightMargin  = m_textureWidth  - getRectangle().getWidth();
	s32 bottomMargin = m_textureHeight - getRectangle().getHeight();
	if (rightMargin  < 0) rightMargin  = 0;
	if (bottomMargin < 0) bottomMargin = 0;
	
	set->drawMultiLineString(
		m_pages.at(static_cast<StringVector::size_type>(m_page)),
		painter,
		engine::renderer::ColorRGB::white,
		m_halign,
		m_valign,
		0,
		// margin left, top, right, bottom
		0,
		0,
		rightMargin,
		bottomMargin);
	
	
	/*
	u8* txtmem = m_texture->LockImage();
	TT_NULL_ASSERT(txtmem);
	
	// Clear the texture
	//MI_CpuFill32(txtmem, 0x80808080, static_cast<u32>(m_textureWidth * m_textureHeight));
	MI_CpuFill32(txtmem, 0, static_cast<u32>(m_textureWidth * m_textureHeight));
	
	// Render the string onto the texture
	localization::GlyphSet* glyphset = MenuSystem::getInstance()->getGlyphSet();
	
	
	glyphset->drawMultiLineString(
		m_pages.at(static_cast<StringVector::size_type>(m_page)),
		txtmem,
		m_textureWidth,
		m_textureHeight,
		224,        // palette offset (completely ignored)
		m_halign,
		m_valign,
		0,
		// margin left, top, right, bottom
		0,
		0,
		m_textureWidth  - std::min(getRectangle().getWidth(),  m_textureWidth),
		m_textureHeight - std::min(getRectangle().getHeight(),  m_textureHeight),
		Glyph::FBUF_I5A3);
	
	m_texture->UnlockImage();
	m_texture->UploadAndWait();
	*/
	
	
	m_shouldRender = false;
	
	return true;
}

// Namespace end
}
}
}
