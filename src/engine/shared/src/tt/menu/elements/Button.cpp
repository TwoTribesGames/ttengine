#include <tt/platform/tt_error.h>

#include <tt/menu/elements/Button.h>
#include <tt/menu/elements/Label.h>
#include <tt/menu/elements/Image.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSystem.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

Button::Button(const std::string&    p_name,
               const MenuLayout&     p_layout,
               const std::string&    p_caption,
               const std::string&    p_image,
               const std::string&    p_pressed,
               const std::string&    p_selected,
               const std::string&    p_disabled,
               real                  p_u,
               real                  p_v,
               s32                   p_imageWidth,
               s32                   p_imageHeight,
               MenuKeyboard::MenuKey p_actionTriggerKey,
               bool                  p_handleRepeat)
:
MenuElement(p_name, p_layout),
m_label(0),
m_image(0),
m_imageName(p_image),
m_pressedName(p_pressed),
m_selectedName(p_selected),
m_disabledName(p_disabled),
m_buttonDepressed(false),
m_handleRepeat(p_handleRepeat),
m_innerRect(math::Point2(0, 0), 1, 1),
m_actionTriggerKey(p_actionTriggerKey),
m_soundEffect(MenuSound_ButtonClicked)
{
	MENU_CREATION_Printf("Button::Button: Element '%s'.\n",
	                     getName().c_str());
	
	TT_ASSERTMSG(p_caption.empty() == false || p_image.empty() == false,
	             "Button needs to have at least a caption or an image specified.");
	
	if (p_caption.empty() == false)
	{
		// Create a label for the button caption
		MenuLayout labelLayout(p_layout);
		labelLayout.setVerticalPositionType(MenuLayout::Position_Center);
		// HACK: [Daniel] Set bottomline offset so text of label aligns properly
		m_label = new Label("", labelLayout, p_caption, false, 2);
	}
	
	if (p_image.empty() == false)
	{
		// Create image
		m_image = new Image("", p_layout, p_image,
		                    p_u, p_v, p_imageWidth, p_imageHeight);
	}
	
	// Buttons can have focus
	setCanHaveFocus(true);
	
	// Create the quads for the border appearance
	createBorder();
	
	// Update the button state
	updateButtonState(m_buttonDepressed);
	
	// Set the minimum and requested dimensions
	s32 minWidth  = 0;
	s32 minHeight = 0;
	s32 reqWidth  = 0;
	s32 reqHeight = 0;
	
	if (m_image == 0 && m_label != 0)
	{
		minWidth  = BORDER_WIDTH * 2;
		minHeight = BORDER_MARGIN_HEIGHT +
		            static_cast<s32>(m_blockSize.y) +
		            BORDER_MARGIN_HEIGHT;
		reqWidth  = BORDER_WIDTH * 2;
		reqHeight = BORDER_MARGIN_HEIGHT +
		            static_cast<s32>(m_blockSize.y) +
		            BORDER_MARGIN_HEIGHT;
	}
	
	if (m_label != 0)
	{
		minWidth  = std::max(minWidth, BORDER_WIDTH +
		                               m_label->getMinimumWidth() +
		                               BORDER_WIDTH);
		minHeight = std::max(minHeight, m_label->getMinimumHeight());
		reqWidth  = std::max(reqWidth, BORDER_WIDTH +
		                               m_label->getRequestedWidth() +
		                               BORDER_WIDTH);
		reqHeight = std::max(reqHeight, m_label->getRequestedHeight());
	}
	
	if (m_image != 0)
	{
		minWidth  = std::max(minWidth,  m_image->getMinimumWidth());
		minHeight = std::max(minHeight, m_image->getMinimumHeight());
		reqWidth  = std::max(reqWidth,  m_image->getRequestedWidth());
		reqHeight = std::max(reqHeight, m_image->getRequestedHeight());
	}
	
	setMinimumWidth(minWidth);
	setMinimumHeight(minHeight);
	setRequestedWidth(reqWidth);
	setRequestedHeight(reqHeight);
	
	s32 baseDepth = 0;
	if (m_image == 0 && m_label != 0)
	{
		// Add depth for border
		++baseDepth;
	}
	
	s32 extraDepth = 0;
	if (m_image != 0)
	{
		extraDepth = std::max(extraDepth, m_image->getDepth());
	}
	if (m_label != 0)
	{
		extraDepth = std::max(extraDepth, m_label->getDepth());
	}
	
	setDepth(baseDepth + extraDepth);
}


Button::Button(const std::string&    p_name,
               const MenuLayout&     p_layout,
               const std::wstring&   p_caption,
               const std::string&    p_image,
               const std::string&    p_pressed,
               const std::string&    p_selected,
               const std::string&    p_disabled,
               real                  p_u,
               real                  p_v,
               s32                   p_imageWidth,
               s32                   p_imageHeight,
               MenuKeyboard::MenuKey p_actionTriggerKey,
               bool                  p_handleRepeat)
:
MenuElement(p_name, p_layout),
m_label(0),
m_image(0),
m_imageName(p_image),
m_pressedName(p_pressed),
m_selectedName(p_selected),
m_disabledName(p_disabled),
m_buttonDepressed(false),
m_handleRepeat(p_handleRepeat),
m_innerRect(math::Point2(0, 0), 1, 1),
m_actionTriggerKey(p_actionTriggerKey),
m_soundEffect(MenuSound_ButtonClicked)
{
	MENU_CREATION_Printf("Button::Button: Element '%s'.\n",
	                     getName().c_str());
	
	TT_ASSERTMSG(p_caption.empty() == false || p_image.empty() == false,
	             "Button needs to have at least a caption or an image specified.");
	
	if (p_caption.empty() == false)
	{
		// Create a label for the button caption
		MenuLayout labelLayout(p_layout);
		labelLayout.setVerticalPositionType(MenuLayout::Position_Center);
		// HACK: [Daniel] Set bottomline offset so text of label aligns properly
		m_label = new Label("", labelLayout, p_caption, false, 2);
		m_label->setSelectedColor(m_label->getTextColor());
	}
	
	if (p_image.empty() == false)
	{
		// Create image
		m_image = new Image("", p_layout, p_image,
		                    p_u, p_v, p_imageWidth, p_imageHeight);
	}
	
	// Buttons can have focus
	setCanHaveFocus(true);
	
	// Create the quads for the border appearance
	createBorder();
	
	// Update the button state
	updateButtonState(m_buttonDepressed);
	
	// Set the minimum and requested dimensions
	s32 minWidth  = 0;
	s32 minHeight = 0;
	s32 reqWidth  = 0;
	s32 reqHeight = 0;
	
	if (m_image == 0 && m_label != 0)
	{
		minWidth  = BORDER_WIDTH * 2;
		minHeight = BORDER_MARGIN_HEIGHT +
		            static_cast<s32>(m_blockSize.y) +
		            BORDER_MARGIN_HEIGHT;
		reqWidth  = BORDER_WIDTH * 2;
		reqHeight = BORDER_MARGIN_HEIGHT +
		            static_cast<s32>(m_blockSize.y) +
		            BORDER_MARGIN_HEIGHT;
	}
	
	if (m_label != 0)
	{
		minWidth  = std::max(minWidth, BORDER_WIDTH +
		                               m_label->getMinimumWidth() +
		                               BORDER_WIDTH);
		minHeight = std::max(minHeight, m_label->getMinimumHeight());
		reqWidth  = std::max(reqWidth, BORDER_WIDTH +
		                               m_label->getRequestedWidth() +
		                               BORDER_WIDTH);
		reqHeight = std::max(reqHeight, m_label->getRequestedHeight());
	}
	
	if (m_image != 0)
	{
		minWidth  = std::max(minWidth,  m_image->getMinimumWidth());
		minHeight = std::max(minHeight, m_image->getMinimumHeight());
		reqWidth  = std::max(reqWidth,  m_image->getRequestedWidth());
		reqHeight = std::max(reqHeight, m_image->getRequestedHeight());
	}
	
	setMinimumWidth(minWidth);
	setMinimumHeight(minHeight);
	setRequestedWidth(reqWidth);
	setRequestedHeight(reqHeight);
	
	s32 baseDepth = 0;
	if (m_image == 0 && m_label != 0)
	{
		// Add depth for border
		++baseDepth;
	}
	
	s32 extraDepth = 0;
	if (m_image != 0)
	{
		extraDepth = std::max(extraDepth, m_image->getDepth());
	}
	if (m_label != 0)
	{
		extraDepth = std::max(extraDepth, m_label->getDepth());
	}
	
	setDepth(baseDepth + extraDepth);
}



Button::~Button()
{
	MENU_CREATION_Printf("Button::~Button: Element '%s': "
	                     "Freeing memory for button label and image.\n",
	                     getName().c_str());
	delete m_label;
	delete m_image;
}


void Button::loadResources()
{
	if (m_label != 0)
	{
		m_label->loadResources();
	}
	
	if (m_image != 0)
	{
		m_image->loadResources();
	}
}


void Button::unloadResources()
{
	if (m_label != 0)
	{
		m_label->unloadResources();
	}
	
	if (m_image != 0)
	{
		m_image->unloadResources();
	}
}


void Button::doLayout(const PointRect& p_rect)
{
	if (m_label != 0)
	{
		// Subtract the border from the rectangle
		const PointRect& myRect(getRectangle());
		PointRect        labelRect(m_label->getRectangle());
		
		labelRect.setPosition(math::Point2(BORDER_WIDTH, 0));
		labelRect.setWidth (myRect.getWidth() - (2 * BORDER_WIDTH));
		labelRect.setHeight(myRect.getHeight());
		
		m_label->setRectangle(labelRect);
		
		// Allow the label to perform its layout
		m_label->doLayout(labelRect);
	}
	
	if (m_image != 0)
	{
		// Subtract the border from the rectangle
		PointRect imageRect(m_image->getRectangle());
		imageRect.setPosition(math::Point2(0, 0));
		imageRect.setWidth (getRectangle().getWidth());
		imageRect.setHeight(getRectangle().getHeight());
		
		m_image->setRectangle(imageRect);
		
		// Allow the image to perform its layout
		m_image->doLayout(imageRect);
	}
	
	MenuElement::doLayout(p_rect);
}


void Button::render(const PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Only draw border/background when not displaying image
	if (m_image == 0 && m_label != 0)
	{
		m_innerRect.setWidth (p_rect.getWidth() -
		                      static_cast<s32>(m_blockSize.x * 2));
		m_innerRect.setHeight(p_rect.getHeight());
		
		m_innerRect.setPosition(
			math::Point2(p_rect.getPosition().x + static_cast<s32>(m_blockSize.x),
			             p_rect.getPosition().y));
		
		using math::Vector3;
		Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
		            static_cast<real>(p_rect.getCenterPosition().y),
		            static_cast<real>(p_z));
		
		m_size.x = static_cast<real>(m_innerRect.getWidth());
		m_size.y = static_cast<real>(m_innerRect.getHeight());
		
		// Readability shorts
		real xp = (m_size.x + m_blockSize.x) * 0.5f;
		real bgOffsetX = -1.0f;
		real bgOffsetY = -1.0f;
		
		// Left edge
		m_borderSegs[BorderSection_LeftEdge]->setPosition(pos +
			Vector3(-xp + bgOffsetX, bgOffsetY, 0.0f));
		m_borderSegs[BorderSection_LeftEdge]->setWidth (m_blockSize.x);
		m_borderSegs[BorderSection_LeftEdge]->setHeight(m_blockSize.y);
		
		// Center block
		m_borderSegs[BorderSection_Center]->setPosition(pos +
			Vector3(bgOffsetX, bgOffsetY, 0.0f));
		m_borderSegs[BorderSection_Center]->setWidth (m_size.x);
		m_borderSegs[BorderSection_Center]->setHeight(m_blockSize.y);
		
		// Right edge
		m_borderSegs[BorderSection_RightEdge]->setPosition(pos +
			Vector3(xp + bgOffsetX, bgOffsetY, 0.0f));
		m_borderSegs[BorderSection_RightEdge]->setWidth (m_blockSize.x);
		m_borderSegs[BorderSection_RightEdge]->setHeight(m_blockSize.y);
		
		
		// Update and render all sections
		for (int i = 0; i < BorderSection_Count; ++i)
		{
			m_borderSegs[i]->update();
			m_borderSegs[i]->render();
		}
	}
	
	if (m_label != 0)
	{
		// Translate the label rect to the render position
		PointRect labelRect(m_label->getRectangle());
		labelRect.translate(p_rect.getPosition());
		
		// Render the label
		m_label->render(labelRect, p_z - 1);
	}
	
	if (m_image != 0)
	{
		// Translate the image rect to the render position
		PointRect imageRect(m_image->getRectangle());
		imageRect.translate(p_rect.getPosition());
		
		// Render the image
		m_image->render(imageRect, p_z - 1);
	}
}


bool Button::doAction(const MenuElementAction& p_action)
{
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	if (m_label != 0 && m_label->doAction(p_action))
	{
		return true;
	}
	
	if (m_image != 0 && m_image->doAction(p_action))
	{
		return true;
	}
	
	return false;
}


void Button::setSelected(bool p_selected)
{
	// Set this element and the button label
	MenuElement::setSelected(p_selected);
	if (m_label != 0)
	{
		m_label->setSelected(p_selected);
	}
	
	if (m_image != 0)
	{
		m_image->setSelected(p_selected);
	}
	
	if (p_selected == false)
	{
		m_buttonDepressed = false;
	}
	
	// Update the button state
	updateButtonState(m_buttonDepressed);
}


void Button::setEnabled(bool p_enabled)
{
	//bool change = (isEnabled() == p_enabled);
	
	MenuElement::setEnabled(p_enabled);
	if (m_label != 0)
	{
		m_label->setEnabled(p_enabled);
	}
	
	if (m_image != 0)
	{
		m_image->setEnabled(p_enabled);
	}
	
	// Update the button state
	updateButtonState(m_buttonDepressed);
}


bool Button::onStylusPressed(s32 /* p_x */, s32 /* p_y */)
{
	// Don't handle input if we shouldn't
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	MENU_STYLUS_Printf("Button::onStylusPressed: Element '%s': "
	                   "Button pressed at (%d, %d). Show pressed image.\n",
	                   getName().c_str(), p_x, p_y);
	updateButtonState(true);
	return true;
}


bool Button::onStylusDragging(s32 /* p_x */, s32 /* p_y */, bool p_isInside)
{
	// Don't handle input if we shouldn't
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	if (m_buttonDepressed != p_isInside)
	{
		updateButtonState(p_isInside);
	}
	
	return true;
}


bool Button::onStylusReleased(s32 /* p_x */, s32 /* p_y */)
{
	// Don't handle input if we shouldn't
	if (shouldHandleInput() == false)
	{
		return false;
	}
	
	MENU_STYLUS_Printf("Button::onStylusReleased: Element '%s': "
	                   "Stylus released at (%d, %d) == button pressed. "
	                   "Perform button action.\n",
	                   getName().c_str(), p_x, p_y);
	buttonClicked();
	return true;
}


bool Button::onStylusRepeat(s32 p_x, s32 p_y)
{
	if (m_handleRepeat)
	{
		return MenuElement::onStylusRepeat(p_x, p_y);
	}
	return false;
}


bool Button::onKeyPressed(const MenuKeyboard& p_keys)
{
	// Don't handle input if we shouldn't
	//if (shouldHandleInput() == false)
	// NOTE: Need this here, because hidden buttons are used to
	//       provide global key behaviour for specific buttons.
	if (isEnabled() == false)
	{
		return false;
	}
	
	if (p_keys.isKeySet(m_actionTriggerKey))
	{
		MENU_KEY_Printf("Button::onKeyPressed: Element '%s': "
		                "Perform button action.\n",
		                getName().c_str());
		updateButtonState(true);
		buttonClicked();
		return true;
	}
	
	return false;
}


bool Button::onKeyReleased(const MenuKeyboard& p_keys)
{
	if (isEnabled() == false)
	{
		return false;
	}
	
	if (p_keys.isKeySet(m_actionTriggerKey))
	{
		// Set button state to not depressed
		updateButtonState(false);
	}
	
	return MenuElement::onKeyReleased(p_keys);
}


bool Button::onKeyRepeat(const MenuKeyboard& p_keys)
{
	if (m_handleRepeat)
	{
		return MenuElement::onKeyRepeat(p_keys);
	}
	return p_keys.isKeySet(m_actionTriggerKey);
}


Button* Button::clone() const
{
	return new Button(*this);
}


void Button::setSoundEffect(MenuSound p_effect)
{
	m_soundEffect = p_effect;
}


engine::renderer::ColorRGBA Button::getTextColor() const
{
	if (m_label != 0)
	{
		return m_label->getTextColor();
	}
	
	return engine::renderer::ColorRGB::black;
}


engine::renderer::ColorRGBA Button::getShadowColor() const
{
	if (m_label != 0)
	{
		return m_label->getShadowColor();
	}
	
	return engine::renderer::ColorRGB::black;
}


engine::renderer::ColorRGBA Button::getDisabledColor() const
{
	if (m_label != 0)
	{
		return m_label->getDisabledColor();
	}
	
	return engine::renderer::ColorRGB::black;
}


engine::renderer::ColorRGBA Button::getSelectedColor() const
{
	if (m_label != 0)
	{
		return m_label->getSelectedColor();
	}
	
	return engine::renderer::ColorRGB::black;
}


void Button::setTextColor(const engine::renderer::ColorRGBA& p_color)
{
	if (m_label != 0)
	{
		m_label->setTextColor(p_color);
	}
}


void Button::setShadowColor(const engine::renderer::ColorRGBA& p_color)
{
	if (m_label != 0)
	{
		m_label->setShadowColor(p_color);
	}
}


void Button::setDisabledColor(const engine::renderer::ColorRGBA& p_color)
{
	if (m_label != 0)
	{
		m_label->setSelectedColor(p_color);
	}
}


void Button::setSelectedColor(const engine::renderer::ColorRGBA& p_color)
{
	if (m_label != 0)
	{
		m_label->setSelectedColor(p_color);
	}
}


//------------------------------------------------------------------------------
// Protected member functions

Button::Button(const Button& p_rhs)
:
MenuElement(p_rhs),
m_label(0),
m_image(0),
m_imageName(p_rhs.m_imageName),
m_pressedName(p_rhs.m_pressedName),
m_selectedName(p_rhs.m_selectedName),
m_disabledName(p_rhs.m_disabledName),
m_buttonDepressed(p_rhs.m_buttonDepressed),
m_borderTexture(p_rhs.m_borderTexture),
m_handleRepeat(p_rhs.m_handleRepeat),
m_topLeft(p_rhs.m_topLeft),
m_botRight(p_rhs.m_botRight),
m_size(p_rhs.m_size),
m_blockSize(p_rhs.m_blockSize),
m_innerRect(p_rhs.m_innerRect),
m_actionTriggerKey(p_rhs.m_actionTriggerKey),
m_soundEffect(p_rhs.m_soundEffect)
{
	// Clone the label, if any
	if (p_rhs.m_label != 0)
	{
		m_label = p_rhs.m_label->clone();
	}
	
	// Clone the image, if any
	if (p_rhs.m_image != 0)
	{
		m_image = p_rhs.m_image->clone();
	}
	
	// Recreate the border segments
	using engine::renderer::QuadSprite;
	for (int i = 0; i < BorderSection_Count; ++i)
	{
		m_borderSegs[i].reset(new QuadSprite(*(p_rhs.m_borderSegs[i])));
	}
}


//------------------------------------------------------------------------------
// Private member functions

void Button::createBorder()
{
	m_topLeft.x = 0;
	m_topLeft.y = 0;
	
	m_botRight.x = BORDER_IMAGE_WIDTH;
	m_botRight.y = BORDER_IMAGE_HEIGHT;
	
	m_blockSize.x = (m_botRight.x - m_topLeft.x) / 3.0f;
	m_blockSize.y = static_cast<real>(m_botRight.y - m_topLeft.y);
	
	// Load the border texture
	std::string textureFilename("Shared\\ButtonBackground");

	// FIXME: SUPPORT NAMESPACES
	m_borderTexture = engine::renderer::TextureCache::get(textureFilename, "");
	if(m_borderTexture == 0)
	{
		TT_PANIC("Loading Button border texture '%s' failed.",
		         textureFilename.c_str());
		return;
	}
	
	// All zero? Use texture size
	if (m_topLeft == m_botRight)
	{
		m_botRight.setValues(m_borderTexture->getWidth(),
		                     m_borderTexture->getHeight());
	}
	
	using engine::renderer::QuadSprite;
	
	// Construct quad sprite for center
	m_borderSegs[BorderSection_Center] =
		QuadSprite::createQuad(m_borderTexture);
	setBorderUVs(m_borderSegs[BorderSection_Center], 1, 0);
	
	// Left edge
	m_borderSegs[BorderSection_LeftEdge] =
		QuadSprite::createQuad(m_borderTexture);
	setBorderUVs(m_borderSegs[BorderSection_LeftEdge], 0, 0);
	
	// Right edge
	m_borderSegs[BorderSection_RightEdge] =
		QuadSprite::createQuad(m_borderTexture);
	setBorderUVs(m_borderSegs[BorderSection_RightEdge], 2, 0);
}


void Button::setBorderUVs(const engine::renderer::QuadSpritePtr& p_quad,
                          s32 p_blockX,
                          s32 p_blockY)
{
	real blockW = m_blockSize.x;
	real blockH = m_blockSize.y;
	
	TT_ASSERTMSG(blockW > 0.0f && blockH > 0.0f,
	             "Attempt to construct button border with fishy UV bounds.");
	
	/*
	p_quad->setTexcoordLeft(m_topLeft.x + (blockW * p_blockX));
	p_quad->setTexcoordTop(m_topLeft.y + (blockH * p_blockY));
	
	p_quad->setTexcoordRight(m_topLeft.x + (blockW * (p_blockX + 1)) - 1);
	p_quad->setTexcoordBottom(m_topLeft.y + (blockH * (p_blockY + 1)) - 1);
	*/ (void)blockW; (void)blockH;
}


void Button::buttonClicked()
{
	// Button can only be clicked when it's enabled
	if (isEnabled())
	{
		//updateButtonState(false);
		
		MenuSystem::getInstance()->playSound(m_soundEffect);
		
		performActions();
	}
}


void Button::setBorderOpacity(u8 p_opacity)
{
	for (int i = 0; i < BorderSection_Count; ++i)
	{
		m_borderSegs[i]->setOpacity(p_opacity);
	}
}


void Button::updateButtonState(bool p_depressed)
{
	m_buttonDepressed = p_depressed;
	
	if (isEnabled() == false)
	{
		setBorderOpacity(BORDER_OPACITY_DISABLED);
		
		if (m_image != 0)
		{
			if (m_disabledName.empty() == false)
			{
				m_image->setImage(m_disabledName);
			}
			else
			{
				m_image->setImage(m_imageName);
			}
		}
	}
	else if (m_buttonDepressed)
	{
		setBorderOpacity(BORDER_OPACITY_DEPRESSED);
		
		if (m_image != 0)
		{
			if (m_pressedName.empty() == false)
			{
				m_image->setImage(m_pressedName);
			}
			else
			{
				m_image->setImage(m_imageName);
			}
		}
	}
	else
	{
		if (isSelected())
		{
			setBorderOpacity(BORDER_OPACITY_SELECTED);
			
			if (m_image != 0)
			{
				if (m_selectedName.empty() == false)
				{
					m_image->setImage(m_selectedName);
				}
				else
				{
					m_image->setImage(m_imageName);
				}
			}
		}
		else
		{
			setBorderOpacity(BORDER_OPACITY_NORMAL);
			
			if (m_image != 0)
			{
				if (m_imageName.empty() == false)
				{
					m_image->setImage(m_imageName);
				}
				else
				{
					m_image->setImage(m_imageName);
				}
			}
		}
	}
}

// Namespace end
}
}
}
