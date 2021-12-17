#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/menu/elements/DynamicLabel.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/elements/Marker.h>
#include <tt/menu/elements/SoftwareKeyboard.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuUtils.h>
#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

SoftwareKeyboard::SoftwareKeyboard(const std::string&  p_name,
                                   const MenuLayout&   p_layout,
                                   const std::string&  p_varName,
                                   const std::string&  p_label,
                                   const std::string&  p_okay,
                                   const std::wstring& p_defaultValue,
                                   s32                 p_length,
                                   s32                 p_pixelLength)
:
MenuElement(p_name, p_layout),
m_onKeyPressed(false),
m_keyInput(0),
m_keyValue(0),
m_shiftRect(0),
m_capsRect(0),
m_specialRect(0),
m_marker(0),
m_onStylusPressed(false),
m_timeStylusPressed(0),
m_timeStylusActionHandled(0),
m_stylusInput(0),
m_stylusValue(0),
m_mode(MODE_LOWER_CASE),
m_charset(CHARSET_NORMAL),
m_length(p_length),
m_pixelLength(p_pixelLength),
m_varName(p_varName),
m_label(p_label),
m_okay(p_okay),
m_text(p_defaultValue),
m_cursorBlink(false),
m_cursorTime(0)
{
	MENU_CREATION_Printf("SoftwareKeyboard::SoftwareKeyboard: Element '%s'.\n",
	                     getName().c_str());
	TT_ASSERTMSG(p_varName.empty() == false,
	             "SoftwareKeyboard '%s' has no variable name set!",
	             getName().c_str());
	
	// Buttons can have focus
	setCanHaveFocus(true);
	
	// Size of keyboard texture
	//setMinimumWidth(KeyboardImageWidth);
	//setMinimumHeight(KeyboardImageHeight);
	setRequestedWidth(KeyboardImageWidth);
	setRequestedHeight(KeyboardImageHeight);
	
	
	using tt::engine::renderer::Texture;
	using tt::engine::renderer::QuadSprite;
	using tt::engine::renderer::ColorRGBA;
	
	// FIXME: Remove hard-coded texture dimensions.
	m_keyTexture = Texture::createForText(256, 128);
	/*
	m_keyTexture->initialise(256, 128, CO3DTexture::eFORMAT_8Alpha32Col);
	m_keyTexture->setFlag(CO3DTexture::eFLAG_TextureColor0Trans);
	m_keyTexture->resetFlag(CO3DTexture::eFLAG_RemoveFromMemory);
	m_keyTexture->uploadAndWait();
	*/
	
	m_keyboardQuad = QuadSprite::createQuad(m_keyTexture);
	m_keyboardQuad->setWidth(KeyboardImageWidth);
	m_keyboardQuad->setHeight(KeyboardImageHeight);
	/*
	m_keyboardQuad->setTexcoordLeft(0.0f);
	m_keyboardQuad->setTexcoordTop(0.0f);
	m_keyboardQuad->setTexcoordRight(KeyboardImageWidth /
		static_cast<real>(m_keyTexture->getWidth()));
	m_keyboardQuad->setTexcoordBottom(KeyboardImageHeight /
		static_cast<real>(m_keyTexture->getHeight()));
	*/
	
	createLowerCase();
	
	m_marker = new Marker("", p_layout);
	
	m_stylusQuad = QuadSprite::createQuad(64.0f, 64.0f,
		ColorRGBA(64, 64, 64, 127));
	
	m_keyQuad = QuadSprite::createQuad(64.0f, 64.0f,
		ColorRGBA(0, 0, 0, 127));
	
	m_highlight = QuadSprite::createQuad(64.0f, 64.0f,
		ColorRGBA(0, 0, 0, 127));
}


SoftwareKeyboard::~SoftwareKeyboard()
{
	MENU_CREATION_Printf("SoftwareKeyboard::~SoftwareKeyboard: Element '%s': "
	                     "Freeing memory.\n",
	                     getName().c_str());
	
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		delete (*it).second;
	}
	
	delete m_marker;
}


bool SoftwareKeyboard::doAction(const MenuElementAction& p_action)
{
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	
	if (p_action.getCommand() == "dismiss")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Action 'dismiss' takes no parameters.");
		dismissKeyboard();
		return true;
	}
	else if (p_action.getCommand() == "set_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Action 'set_value' takes no parameters.");
		
		setCursor(false);
		m_text = MenuUtils::hexToWideString(p_action.getParameter(0));
		TT_ASSERTMSG(static_cast<s32>(m_text.length()) <= m_length,
		             "Value to set is too long.");
		
		// Update the keyboard label, if one is specified
		if (m_label.empty() == false)
		{
			MenuElementAction mea(m_label, "set_caption_hex");
			mea.addParameter(MenuUtils::wideStringToHex(m_text));
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		
		if (m_okay.empty() == false)
		{
			removeCursor();
			MenuElementAction mea(m_okay, "set_enabled");
			mea.addParameter(m_text.empty() ? "false" : "true");
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
			addCursor();
		}
		
		return true;
	}
	
	return false;
}


void SoftwareKeyboard::render(const PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	updateCursor();
	
	PointRect keyboardRect(p_rect);
	
	math::Vector3 pos(static_cast<real>(keyboardRect.getPosition().x),
	                  static_cast<real>(keyboardRect.getPosition().y),
	                  static_cast<real>(p_z));
	
	m_keyboardQuad->setPosition(static_cast<real>(keyboardRect.getCenterPosition().x),
	                            static_cast<real>(keyboardRect.getCenterPosition().y),
	                            static_cast<real>(p_z));
	m_keyboardQuad->update();
	m_keyboardQuad->render();
	
	if (m_stylusInput != 0 && m_onStylusPressed)
	{
		m_stylusQuad->setWidth(static_cast<real>(m_stylusInput->getWidth()));
		m_stylusQuad->setHeight(static_cast<real>(m_stylusInput->getHeight()));
		m_stylusQuad->setPosition(pos + math::Vector3(
			static_cast<real>(m_stylusInput->getCenterPosition().x),
			static_cast<real>(m_stylusInput->getCenterPosition().y),
			-1.0f));
		m_stylusQuad->update();
		m_stylusQuad->render();
	}
	
	if (m_keyInput != 0)
	{
		if (isSelected())
		{
			math::PointRect r(*m_keyInput);
			r.translate(math::Point2(static_cast<s32>(pos.x),
			                         static_cast<s32>(pos.y)));
			m_marker->render(r, static_cast<s32>(pos.z - 2));
		}
		
		if (m_onKeyPressed)
		{
			m_keyQuad->setWidth(static_cast<real>(m_keyInput->getWidth()));
			m_keyQuad->setHeight(static_cast<real>(m_keyInput->getHeight()));
			m_keyQuad->setPosition(pos + math::Vector3(
				static_cast<real>(m_keyInput->getCenterPosition().x),
				static_cast<real>(m_keyInput->getCenterPosition().y),
				-2.0f));
			m_keyQuad->update();
			m_keyQuad->render();
		}
	}
	
	if (m_mode == MODE_SHIFT)
	{
		if (m_shiftRect != 0)
		{
			m_highlight->setWidth(static_cast<real>(m_shiftRect->getWidth()));
			m_highlight->setHeight(static_cast<real>(m_shiftRect->getHeight()));
			m_highlight->setPosition(pos + math::Vector3(
				static_cast<real>(m_shiftRect->getCenterPosition().x),
				static_cast<real>(m_shiftRect->getCenterPosition().y),
				-1.0f));
			m_highlight->update();
			m_highlight->render();
		}
	}
	else if (m_mode == MODE_CAPS)
	{
		if (m_capsRect != 0)
		{
			m_highlight->setWidth(static_cast<real>(m_capsRect->getWidth()));
			m_highlight->setHeight(static_cast<real>(m_capsRect->getHeight()));
			m_highlight->setPosition(pos + math::Vector3(
				static_cast<real>(m_capsRect->getCenterPosition().x),
				static_cast<real>(m_capsRect->getCenterPosition().y),
				-1.0f));
			m_highlight->update();
			m_highlight->render();
		}
	}
	
	if (m_charset == CHARSET_SPECIAL)
	{
		if (m_specialRect != 0)
		{
			m_highlight->setWidth(static_cast<real>(m_specialRect->getWidth()));
			m_highlight->setHeight(static_cast<real>(m_specialRect->getHeight()));
			m_highlight->setPosition(pos + math::Vector3(
				static_cast<real>(m_specialRect->getCenterPosition().x),
				static_cast<real>(m_specialRect->getCenterPosition().y),
				-1.0f));
			m_highlight->update();
			m_highlight->render();
		}
	}
}


bool SoftwareKeyboard::onStylusPressed(s32 p_x, s32 p_y)
{
	MENU_STYLUS_Printf("SoftwareKeyboard::onStylusPressed: Element '%s': "
	                   "SoftwareKeyboard pressed at (%d, %d).\n",
	                   getName().c_str(), p_x, p_y);
	
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		if ((*it).second->contains(math::Point2(p_x, p_y)))
		{
			m_stylusInput       = (*it).second;
			m_keyInput          = m_stylusInput;
			m_stylusValue       = (*it).first;
			m_keyValue          = m_stylusValue;
			m_onStylusPressed   = true;
			m_timeStylusPressed = system::Time::getInstance()->getMilliSeconds();
			MENU_Printf("SoftwareKeyboard::onStylusPressed: Pressed key %04X\n",
			            (*it).first);
			break;
		}
	}
	
	return true;
}


bool SoftwareKeyboard::onStylusDragging(s32 p_x, s32 p_y, bool /* p_isInside */)
{
	if (m_stylusInput != 0)
	{
		if (m_stylusInput->contains(math::Point2(p_x, p_y)))
		{
			if (m_onStylusPressed == false)
			{
				m_timeStylusPressed = system::Time::getInstance()->getMilliSeconds();
				m_onStylusPressed   = true;
			}
			else if (m_timeStylusActionHandled == 0)
			{
				if (system::Time::getInstance()->getMilliSeconds() - m_timeStylusPressed >= BUTTON_DELAY)
				{
					m_timeStylusActionHandled = system::Time::getInstance()->getMilliSeconds();
					handleKeyPress(m_stylusValue);
				}
			}
			else if (system::Time::getInstance()->getMilliSeconds() - m_timeStylusActionHandled >= BUTTON_REPEAT)
			{
				m_timeStylusActionHandled = system::Time::getInstance()->getMilliSeconds();
				handleKeyPress(m_stylusValue);
			}
		}
		else
		{
			m_onStylusPressed         = false;
			m_timeStylusActionHandled = 0;
		}
	}
	
	return true;
}


bool SoftwareKeyboard::onStylusReleased(s32 p_x, s32 p_y)
{
	MENU_STYLUS_Printf("SoftwareKeyboard::onStylusReleased: Element '%s': "
	                   "Stylus released at (%d, %d) == SoftwareKeyboard pressed. "
	                   "Perform SoftwareKeyboard action.\n",
	                   getName().c_str(), p_x, p_y);
	if (m_stylusInput != 0)
	{
		if (m_stylusInput->contains(math::Point2(p_x, p_y)))
		{
			if (m_timeStylusActionHandled == 0)
			{
				handleKeyPress(m_stylusValue);
			}
		}
	}
	
	m_onStylusPressed         = false;
	m_timeStylusActionHandled = 0;
	m_stylusInput             = 0;
	m_stylusValue             = 0;
	
	return true;
}


bool SoftwareKeyboard::onStylusRepeat(s32 /* p_x */, s32 /* p_y */)
{
	// Do absolutely nothing
	return true;
}


bool SoftwareKeyboard::onKeyPressed(const MenuKeyboard& p_keys)
{
	bool handled = false;
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_LEFT) && selectLeft())
	{
		handled = true;
	}
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_RIGHT) && selectRight())
	{
		handled = true;
	}
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_DOWN) && selectDown())
	{
		handled = true;
	}
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_UP) && selectUp())
	{
		handled = true;
	}
	
	if (p_keys.isKeySet(MenuKeyboard::MENU_ACCEPT))
	{
		m_onKeyPressed = true;
		
		handleKeyPress(m_keyValue);
		handled = true;
	}
	
	return handled;
}


bool SoftwareKeyboard::onKeyHold(const MenuKeyboard& /* p_keys */)
{
	return false;
}


bool SoftwareKeyboard::onKeyReleased(const MenuKeyboard& p_keys)
{
	bool handled = false;
	if (p_keys.isKeySet(MenuKeyboard::MENU_ACCEPT))
	{
		m_onKeyPressed = false;
	}
	
	return handled;
}


SoftwareKeyboard* SoftwareKeyboard::clone() const
{
	return new SoftwareKeyboard(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

SoftwareKeyboard::SoftwareKeyboard(const SoftwareKeyboard& p_rhs)
:
MenuElement(p_rhs),
m_onKeyPressed(p_rhs.m_onKeyPressed),
m_keyValue(p_rhs.m_keyValue),
m_onStylusPressed(p_rhs.m_onStylusPressed),
m_timeStylusPressed(p_rhs.m_timeStylusPressed),
m_timeStylusActionHandled(p_rhs.m_timeStylusActionHandled),
m_stylusValue(p_rhs.m_stylusValue),
m_mode(p_rhs.m_mode),
m_charset(p_rhs.m_charset),
m_length(p_rhs.m_length),
m_pixelLength(p_rhs.m_pixelLength),
m_rectX(p_rhs.m_rectX),
m_rectY(p_rhs.m_rectY),
m_rectWidth(p_rhs.m_rectWidth),
m_rectHeight(p_rhs.m_rectHeight),
m_varName(p_rhs.m_varName),
m_label(p_rhs.m_label),
m_okay(p_rhs.m_okay),
m_text(p_rhs.m_text),
m_cursorBlink(p_rhs.m_cursorBlink),
m_cursorTime(p_rhs.m_cursorTime)
{
	using engine::renderer::QuadSprite;
	using engine::renderer::Texture;
	
	// Copy the keyboard texture
	m_keyTexture = Texture::createForText(256, 128);
	//m_keyTexture.reset(new Texture(*(p_rhs.m_keyTexture)));
	
	// Copy the quad sprites
	m_keyQuad.reset(new QuadSprite(*(p_rhs.m_keyQuad)));
	m_highlight.reset(new QuadSprite(*(p_rhs.m_highlight)));
	m_stylusQuad.reset(new QuadSprite(*(p_rhs.m_stylusQuad)));
	m_keyboardQuad.reset(new QuadSprite(*(p_rhs.m_keyboardQuad)));
	
	m_keyboardQuad->setTexture(m_keyTexture);
	
	// Copy the key rectangles
	for (KeyboardSegments::const_iterator it = p_rhs.m_keyMap.begin();
	     it != p_rhs.m_keyMap.end(); ++it)
	{
		m_keyMap.insert(KeyboardEntry((*it).first,
		                               new PointRect(*(*it).second)));
	}
	
	// Restore the rectangle pointers
	m_keyInput    = getEquivalentPointer(p_rhs.m_keyInput,    p_rhs.m_keyMap);
	m_shiftRect   = getEquivalentPointer(p_rhs.m_shiftRect,   p_rhs.m_keyMap);
	m_capsRect    = getEquivalentPointer(p_rhs.m_capsRect,    p_rhs.m_keyMap);
	m_specialRect = getEquivalentPointer(p_rhs.m_specialRect, p_rhs.m_keyMap);
	m_stylusInput = getEquivalentPointer(p_rhs.m_stylusInput, p_rhs.m_keyMap);
}


//------------------------------------------------------------------------------
// Private member functions

void SoftwareKeyboard::updateCursor()
{
	++m_cursorTime;
	
	if (m_cursorTime == CursorBlinkSpeed)
	{
		m_cursorTime = 0;
		
		if (m_cursorBlink)
		{
			removeCursor();
			m_cursorBlink = false;
		}
		else
		{
			m_cursorBlink = true;
			addCursor();
		}
		
		// Update the keyboard label, if one is specified
		if (m_label.empty() == false)
		{
			MenuElementAction mea(m_label, "set_caption_hex");
			mea.addParameter(MenuUtils::wideStringToHex(m_text));
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
	}
}


void SoftwareKeyboard::setCursor(bool p_onOff)
{
	m_cursorTime = 0;
	
	if (p_onOff == m_cursorBlink)
	{
		return;
	}
	
	if (m_cursorBlink)
	{
		removeCursor();
		m_cursorBlink = false;
	}
	else
	{
		m_cursorBlink = true;
		addCursor();
	}
}


void SoftwareKeyboard::addCursor()
{
	if (m_cursorBlink)
	{
		m_text += L"_";
	}
}


void SoftwareKeyboard::removeCursor()
{
	if (m_cursorBlink)
	{
		m_text.erase(m_text.length() - 1);
	}
}


void SoftwareKeyboard::createLowerCase()
{
	// Replace the key texture with the lower-case texture
	{
		const std::string filename("MenuElements\\lowercase");

		// FIXME: MUST SUPPORT NAMESPACES
		engine::renderer::TexturePtr texture = engine::renderer::TextureCache::get(filename, "");
		if(texture == 0)
		{
			TT_PANIC("Loading software keyboard lowercase texture '%s' failed.",
			         filename.c_str());
			return;
		}
		
		/* FIXME: Need image data copying solution.
		u8* destdata = m_keyTexture->LockImage();
		u8* srcdata  = texture->LockImage();
		MI_CpuCopyFast(srcdata, destdata,
			static_cast<u32>(m_keyTexture->GetWidth() *
			                 m_keyTexture->GetHeight()));
		texture->UnlockImage(false);
		m_keyTexture->UnlockImage();
		
		u16* srcpal  = texture->LockPalette();
		u16* destpal = m_keyTexture->LockPalette();
		MI_CpuCopyFast(srcpal, destpal, 32UL * 2UL);
		m_keyTexture->UnlockPalette();
		texture->UnlockPalette(false);
		
		m_keyTexture->UploadAndWait();
		*/
	}
	
	// Only set X, Y, width and height first time
	addRectangle(L'1', 1, 23, 15, 15);
	addRectangle(L'2');
	addRectangle(L'3');
	addRectangle(L'4');
	addRectangle(L'5');
	addRectangle(L'6');
	addRectangle(L'7');
	addRectangle(L'8');
	addRectangle(L'9');
	addRectangle(L'0');
	addRectangle(L'-');
	addRectangle(L'=');
	addRectangle(KEY_BACKSPACE, -1, -1, 17, -1);
	
	// New line, set new X and Y
	addRectangle(L'q', 10, 39, 15, 15);
	addRectangle(L'w');
	addRectangle(L'e');
	addRectangle(L'r');
	addRectangle(L't');
	addRectangle(L'y');
	addRectangle(L'u');
	addRectangle(L'i');
	addRectangle(L'o');
	addRectangle(L'p');
	addRectangle(L'[');
	addRectangle(L']');
	
	// New line, set new X and Y
	addRectangle(L'a', 17, 55, 15, 15);
	addRectangle(L's');
	addRectangle(L'd');
	addRectangle(L'f');
	addRectangle(L'g');
	addRectangle(L'h');
	addRectangle(L'j');
	addRectangle(L'k');
	addRectangle(L'l');
	addRectangle(L';');
	addRectangle(L'\'');
	
	// New line, set new X and Y
	addRectangle(L'z', 25, 71, 15, 15);
	addRectangle(L'x');
	addRectangle(L'c');
	addRectangle(L'v');
	addRectangle(L'b');
	addRectangle(L'n');
	addRectangle(L'm');
	addRectangle(L',');
	addRectangle(L'.');
	addRectangle(L'/');
	
	// New line, set new X and Y
	addRectangle(L' ', 60, 87, 89, -1); // space
	
	// Caps, shift, special
	addRectangle(KEY_CAPS, 90, 1, 29, 15);
	addRectangle(KEY_SHIFT, 59, 1, 29, 15);
	addRectangle(KEY_SPECIAL, 121, 1, 29, 15);
}


void SoftwareKeyboard::createShift()
{
	// Replace the key texture with the shift texture
	{
		const std::string filename("MenuElements\\shift");
		engine::renderer::TexturePtr texture = engine::renderer::TextureCache::get(filename, "");
		if(texture == 0)
		{
			TT_PANIC("Loading software keyboard shift texture '%s' failed.",
			         filename.c_str());
			return;
		}
		
		/* FIXME: Need image data copying solution.
		u8* destdata = m_keyTexture->LockImage();
		u8* srcdata  = texture->LockImage();
		MI_CpuCopyFast(srcdata, destdata,
		               static_cast<u32>(m_keyTexture->GetWidth() *
		                                m_keyTexture->GetHeight()));
		texture->UnlockImage(false);
		m_keyTexture->UnlockImage();
		
		u16* srcpal  = texture->LockPalette();
		u16* destpal = m_keyTexture->LockPalette();
		MI_CpuCopyFast(srcpal, destpal, 32UL * 2UL);
		m_keyTexture->UnlockPalette();
		texture->UnlockPalette(false);
		
		m_keyTexture->UploadAndWait();
		*/
	}
	
	// Only set X, Y, width and height first time
	addRectangle(L'!', 1, 23, 15, 15);
	addRectangle(L'@');
	addRectangle(L'#');
	addRectangle(L'$');
	addRectangle(L'%');
	addRectangle(L'^');
	addRectangle(L'&');
	addRectangle(L'*');
	addRectangle(L'(');
	addRectangle(L')');
	addRectangle(L'_');
	addRectangle(L'+');
	addRectangle(KEY_BACKSPACE, -1, -1, 17, -1);
	
	// New line, set new X and Y
	addRectangle(L'Q', 10, 39, 15, 15);
	addRectangle(L'W');
	addRectangle(L'E');
	addRectangle(L'R');
	addRectangle(L'T');
	addRectangle(L'Y');
	addRectangle(L'U');
	addRectangle(L'I');
	addRectangle(L'O');
	addRectangle(L'P');
	addRectangle(L'{');
	addRectangle(L'}');
	
	// New line, set new X and Y
	addRectangle(L'A', 17, 55, 15, 15);
	addRectangle(L'S');
	addRectangle(L'D');
	addRectangle(L'F');
	addRectangle(L'G');
	addRectangle(L'H');
	addRectangle(L'J');
	addRectangle(L'K');
	addRectangle(L'L');
	addRectangle(L':');
	addRectangle(L'"');
	
	// New line, set new X and Y
	addRectangle(L'Z', 25, 71, 15, 15);
	addRectangle(L'X');
	addRectangle(L'C');
	addRectangle(L'V');
	addRectangle(L'B');
	addRectangle(L'N');
	addRectangle(L'M');
	addRectangle(L'<');
	addRectangle(L'>');
	addRectangle(L'?');
	
	// New line, set new X and Y
	addRectangle(L' ', 60, 87, 89, -1);
	
	// Caps, shift, special
	addRectangle(KEY_CAPS, 90, 1, 29, -1);
	addRectangle(KEY_SHIFT, 59, 1, 29, 15);
	addRectangle(KEY_SPECIAL, 121, 1, 29, 15);
}


void SoftwareKeyboard::createCaps()
{
	// Replace the key texture with the upper-case texture
	{
		const std::string filename("MenuElements\\uppercase");

		// FIXME: MUST SUPPORT NAMESPACES
		engine::renderer::TexturePtr texture = engine::renderer::TextureCache::get(filename, "");
		if(texture == 0)
		{
			TT_PANIC("Loading software keyboard upper-case texture '%s' failed.",
			         filename.c_str());
			return;
		}
		
		/* FIXME: Need image data copying solution.
		u8* destdata = m_keyTexture->LockImage();
		u8* srcdata  = texture->LockImage();
		MI_CpuCopyFast(srcdata, destdata,
		               static_cast<u32>(m_keyTexture->GetWidth() *
		                                m_keyTexture->GetHeight()));
		texture->UnlockImage(false);
		m_keyTexture->UnlockImage();
		
		u16* srcpal  = texture->LockPalette();
		u16* destpal = m_keyTexture->LockPalette();
		MI_CpuCopyFast(srcpal, destpal, 32UL * 2UL);
		m_keyTexture->UnlockPalette();
		texture->UnlockPalette(false);
		
		m_keyTexture->UploadAndWait();
		*/
	}
	
	// Only set X, Y, width and height first time
	addRectangle(L'1', 1, 23, 15, 15);
	addRectangle(L'2');
	addRectangle(L'3');
	addRectangle(L'4');
	addRectangle(L'5');
	addRectangle(L'6');
	addRectangle(L'7');
	addRectangle(L'8');
	addRectangle(L'9');
	addRectangle(L'0');
	addRectangle(L'-');
	addRectangle(L'=');
	addRectangle(KEY_BACKSPACE, -1, -1, 17, -1);
	
	// New line, set new X and Y
	addRectangle(L'Q', 10, 39, 15, 15);
	addRectangle(L'W');
	addRectangle(L'E');
	addRectangle(L'R');
	addRectangle(L'T');
	addRectangle(L'Y');
	addRectangle(L'U');
	addRectangle(L'I');
	addRectangle(L'O');
	addRectangle(L'P');
	addRectangle(L'[');
	addRectangle(L']');
	
	// New line, set new X and Y
	addRectangle(L'A', 17, 55, 15, 15);
	addRectangle(L'S');
	addRectangle(L'D');
	addRectangle(L'F');
	addRectangle(L'G');
	addRectangle(L'H');
	addRectangle(L'J');
	addRectangle(L'K');
	addRectangle(L'L');
	addRectangle(L';');
	addRectangle(L'\'');
	
	// New line, set new X and Y
	addRectangle(L'Z', 25, 71, 15, 15);
	addRectangle(L'X');
	addRectangle(L'C');
	addRectangle(L'V');
	addRectangle(L'B');
	addRectangle(L'N');
	addRectangle(L'M');
	addRectangle(L',');
	addRectangle(L'.');
	addRectangle(L'/');
	
	// New line, set new X and Y
	addRectangle(L' ', 60, 87, 89, -1);
	
	// Caps, shift, special
	addRectangle(KEY_CAPS, 90, 1, 29, -1);
	addRectangle(KEY_SHIFT, 59, 1, 29, 15);
	addRectangle(KEY_SPECIAL, 121, 1, 29, 15);
}


void SoftwareKeyboard::createSpecialChars()
{
	// Replace the key texture with the special lower-case texture
	{
		const std::string filename("MenuElements\\special_Lowercase");

		// FIXME: MUST SUPPORT NAMESPACES
		engine::renderer::TexturePtr texture = engine::renderer::TextureCache::get(filename, "");
		if(texture == 0)
		{
			TT_PANIC("Loading software keyboard special lower-case "
			         "texture '%s' failed.", filename.c_str());
			return;
		}
		
		/* FIXME: Need image data copying solution.
		u8* destdata = m_keyTexture->LockImage();
		u8* srcdata  = texture->LockImage();
		MI_CpuCopyFast(srcdata, destdata,
		               static_cast<u32>(m_keyTexture->GetWidth() *
		                                m_keyTexture->GetHeight()));
		texture->UnlockImage(false);
		m_keyTexture->UnlockImage();
		
		u16* srcpal  = texture->LockPalette();
		u16* destpal = m_keyTexture->LockPalette();
		MI_CpuCopyFast(srcpal, destpal, 32UL * 2UL);
		m_keyTexture->UnlockPalette();
		texture->UnlockPalette(false);
		
		m_keyTexture->UploadAndWait();
		*/
	}
	
	
	// Only set X, Y, width and height first time
	addRectangle(KEY_BACKSPACE, 193, 23, 17, 15);
	
	// New line, set new X and Y
	addRectangle(0x00E0, 10, 39, 15, 15); // ‡
	addRectangle(0x00E1); // ·
	addRectangle(0x00E2); // ‚
	addRectangle(0x00E3); // „
	addRectangle(0x00E4); // ‰
	addRectangle(0x00E5); // Â
	addRectangle(0x00E6); // Ê
	addRectangle(0x00E7); // Á
	addRectangle(0x00F1); // Ò
	
	// New line, set new X and Y
	addRectangle(0x00E8, 17, 55, 15, 15); // Ë
	addRectangle(0x00E9); // È
	addRectangle(0x00EA); // Í
	addRectangle(0x00EB); // Î
	addRectangle(0x00F4); // Ù
	addRectangle(0x00F5); // ı
	addRectangle(0x00F6); // ˆ
	addRectangle(0x00F2); // Ú
	addRectangle(0x00F3); // Û
	addRectangle(0x00F8); // ¯
	
	// New line, set new X and Y
	addRectangle(0x00EC, 25, 71, 15, 15); // Ï
	addRectangle(0x00ED); // Ì
	addRectangle(0x00EE); // Ó
	addRectangle(0x00EF); // Ô
	addRectangle(0x00F9); // ˘
	addRectangle(0x00FA); // ˙
	addRectangle(0x00FB); // ˚
	addRectangle(0x00FC); // ¸
	addRectangle(0x00FF); // ˇ
	addRectangle(0x00FD); // ˝
	
	// New line, set new X and Y
	addRectangle(L' ', 60, 87, 89, -1);	// space
	
	// Caps, shift, special
	addRectangle(KEY_CAPS, 90, 1, 29, 15);
	addRectangle(KEY_SHIFT, 59, 1, 29, 15);
	addRectangle(KEY_SPECIAL, 121, 1, 29, 15);
}


void SoftwareKeyboard::createSpecialShift()
{
	// Replace the key texture with the special upper-case texture
	{
		const std::string filename("MenuElements\\special_Uppercase");

		// FIXME: MUST SUPPORT NAMESPACES
		engine::renderer::TexturePtr texture = engine::renderer::TextureCache::get(filename, "");
		if(texture == 0)
		{
			TT_PANIC("Loading software keyboard special upper-case "
			         "texture '%s' failed.", filename.c_str());
			return;
		}
		
		/* FIXME: Need image data copying solution.
		u8* destdata = m_keyTexture->LockImage();
		u8* srcdata  = texture->LockImage();
		MI_CpuCopyFast(srcdata, destdata,
		               static_cast<u32>(m_keyTexture->GetWidth() *
		                                m_keyTexture->GetHeight()));
		texture->UnlockImage(false);
		m_keyTexture->UnlockImage();
		
		u16* srcpal  = texture->LockPalette();
		u16* destpal = m_keyTexture->LockPalette();
		MI_CpuCopyFast(srcpal, destpal, 32UL * 2UL);
		m_keyTexture->UnlockPalette();
		texture->UnlockPalette(false);
		
		m_keyTexture->UploadAndWait();
		*/
	}
	
	
	// Only set X, Y, width and height first time
	addRectangle(KEY_BACKSPACE, 193, 23, 17, 15);
	
	// New line, set new X and Y
	addRectangle(0x00C0, 10, 39, 15, 15); // ¿
	addRectangle(0x00C1); // ¡
	addRectangle(0x00C2); // ¬
	addRectangle(0x00C3); // √
	addRectangle(0x00C4); // ƒ
	addRectangle(0x00C5); // ≈
	addRectangle(0x00C6); // ∆
	addRectangle(0x00C7); // «
	addRectangle(0x00D1); // —
	
	// New line, set new X and Y
	addRectangle(0x00C8, 17, 55, 15, 15); // »
	addRectangle(0x00C9); // …
	addRectangle(0x00CA); //  
	addRectangle(0x00CB); // À
	addRectangle(0x00D4); // ‘
	addRectangle(0x00D5); // ’
	addRectangle(0x00D6); // ÷
	addRectangle(0x00D2); // “
	addRectangle(0x00D3); // ”
	addRectangle(0x00D8); // ÿ
	
	// New line, set new X and Y
	addRectangle(0x00CC, 25, 71, 15, 15); // Ã
	addRectangle(0x00CD); // Õ
	addRectangle(0x00CE); // Œ
	addRectangle(0x00CF); // œ
	addRectangle(0x00D9); // Ÿ
	addRectangle(0x00DA); // ⁄
	addRectangle(0x00DB); // €
	addRectangle(0x00DC); // ‹
	addRectangle(0x00DF); // ﬂ
	addRectangle(0x00DD); // ›
	
	
	// New line, set new X and Y
	addRectangle(L' ', 60, 87, 89, -1); // space
	
	// caps, shift, special
	addRectangle(KEY_CAPS, 90, 1, 29, 15);
	addRectangle(KEY_SHIFT, 59, 1, 29, 15);
	addRectangle(KEY_SPECIAL, 121, 1, 29, 15);
}


void SoftwareKeyboard::createSpecialCaps()
{
	createSpecialShift();
}


void SoftwareKeyboard::addRectangle(wchar_t p_value,
                                    s32     p_x,
                                    s32     p_y,
                                    s32     p_width,
                                    s32     p_height)
{
	if (p_x == -1)
	{
		p_x = m_rectX;
	}
	else
	{
		m_rectX = p_x;
	}
	
	if (p_y == -1)
	{
		p_y = m_rectY;
	}
	else
	{
		m_rectY = p_y;
	}
	
	if (p_width == -1)
	{
		p_width = m_rectWidth;
	}
	else
	{
		m_rectWidth = p_width;
	}
	
	if (p_height == -1)
	{
		p_height = m_rectHeight;
	}
	else
	{
		m_rectHeight = p_height;
	}
	
	PointRect* rect = new PointRect(math::Point2(p_x, p_y), p_width, p_height);
	m_keyMap.insert(KeyboardEntry(p_value, rect));
	
	m_rectX += m_rectWidth + BUTTON_BORDER;
	
	if (m_keyInput == 0)
	{
		m_keyInput = rect;
		m_keyValue = p_value;
	}
	
	if (p_value == KEY_SHIFT)
	{
		m_shiftRect = rect;
	}
	else if (p_value == KEY_CAPS)
	{
		m_capsRect = rect;
	}
	else if (p_value == KEY_SPECIAL)
	{
		m_specialRect = rect;
	}
}


void SoftwareKeyboard::handleKeyPress(wchar_t p_value)
{
	bool modified = false;
	if (p_value == KEY_BACKSPACE)
	{
		removeCursor();
		if (m_text.length() > 0)
		{
			// Remove one character from the back of the string
			m_text.erase(m_text.length() - 1);
			
			modified = true;
		}
		addCursor();
	}
	else if (p_value == KEY_ENTER)
	{
		dismissKeyboard();
	}
	else if (p_value == KEY_CAPS)
	{
		if (m_mode != MODE_CAPS)
		{
			setMode(MODE_CAPS);
		}
		else
		{
			setMode(MODE_LOWER_CASE);
		}
	}
	else if (p_value == KEY_SHIFT)
	{
		if (m_mode != MODE_SHIFT)
		{
			setMode(MODE_SHIFT);
		}
		else
		{
			setMode(MODE_LOWER_CASE);
		}
	}
	else if (p_value == KEY_SPECIAL)
	{
		if (m_charset != CHARSET_SPECIAL)
		{
			setCharset(CHARSET_SPECIAL);
		}
		else
		{
			setCharset(CHARSET_NORMAL);
		}
	}
	else
	{
		removeCursor();
		
		// Add the character that was pressed to the string
		if (m_length == -1 || static_cast<s32>(m_text.length()) < m_length)
		{
			if (m_text.empty() == false || p_value != L' ')
			{
				m_text += p_value;
				modified = true;
			}
			
			if (m_pixelLength != -1)
			{
				engine::glyph::GlyphSet* glyphset =
					MenuSystem::getInstance()->getGlyphSet();
				if (glyphset->getStringPixelWidth(m_text) > m_pixelLength)
				{
					m_text.erase(m_text.length() - 1);
					modified = false;
				}
			}
			
			if (modified && m_mode == MODE_SHIFT)
			{
				setMode(MODE_LOWER_CASE);
			}
		}
		addCursor();
	}
	
	if (modified)
	{
		// Update the keyboard label, if one is specified
		if (m_label.empty() == false)
		{
			MenuElementAction mea(m_label, "set_caption_hex");
			mea.addParameter(MenuUtils::wideStringToHex(m_text));
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
		}
		
		if (m_okay.empty() == false)
		{
			removeCursor();
			MenuElementAction mea(m_okay, "set_enabled");
			mea.addParameter(m_text.empty() ? "false" : "true");
			
			MenuSystem::getInstance()->doMenuElementAction(mea);
			addCursor();
		}
	}
}


bool SoftwareKeyboard::selectLeft()
{
	PointRect* candidate      = 0;
	wchar_t    candidateValue = 0;
	PointRect* farRight       = 0;
	wchar_t    farRightValue  = 0;
	
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		if ((*it).second != m_keyInput)
		{
			if ((*it).second->getCenterPosition().y == m_keyInput->getCenterPosition().y) // on same line
			{
				if ((*it).second->getCenterPosition().x < m_keyInput->getCenterPosition().x) // left of selected
				{
					if (candidate != 0)
					{
						if ((*it).second->getCenterPosition().x > candidate->getCenterPosition().x) // right of candidate
						{
							candidate      = (*it).second;
							candidateValue = (*it).first;
						}
					}
					else
					{
						candidate      = (*it).second;
						candidateValue = (*it).first;
					}
				}
				else
				{
					if (farRight != 0)
					{
						if ((*it).second->getCenterPosition().x > farRight->getCenterPosition().x) // right of farRight
						{
							farRight      = (*it).second;
							farRightValue = (*it).first;
						}
					}
					else
					{
						farRight      = (*it).second;
						farRightValue = (*it).first;
					}
				}
			}
		}
	}
	
	if (candidate != 0)
	{
		m_keyInput = candidate;
		m_keyValue = candidateValue;
		return true;
	}
	else if (farRight != 0)
	{
		m_keyInput = farRight;
		m_keyValue = farRightValue;
		return true;
	}
	
	return false;
}


bool SoftwareKeyboard::selectRight()
{
	PointRect* candidate      = 0;
	wchar_t    candidateValue = 0;
	PointRect* farLeft        = 0;
	wchar_t    farLeftValue   = 0;
	
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		if ((*it).second != m_keyInput)
		{
			if ((*it).second->getCenterPosition().y == m_keyInput->getCenterPosition().y) // on same line
			{
				if ((*it).second->getCenterPosition().x > m_keyInput->getCenterPosition().x) // right of selected
				{
					if (candidate != 0)
					{
						if ((*it).second->getCenterPosition().x < candidate->getCenterPosition().x) // left of candidate
						{
							candidate      = (*it).second;
							candidateValue = (*it).first;
						}
					}
					else
					{
						candidate      = (*it).second;
						candidateValue = (*it).first;
					}
				}
				else
				{
					if (farLeft != 0)
					{
						if ((*it).second->getCenterPosition().x < farLeft->getCenterPosition().x) // left of farLeft
						{
							farLeft      = (*it).second;
							farLeftValue = (*it).first;
						}
					}
					else
					{
						farLeft      = (*it).second;
						farLeftValue = (*it).first;
					}
				}
			}
		}
	}
	
	if (candidate != 0)
	{
		m_keyInput = candidate;
		m_keyValue = candidateValue;
		return true;
	}
	else if (farLeft != 0)
	{
		m_keyInput = farLeft;
		m_keyValue = farLeftValue;
		return true;
	}
	
	return false;
}


bool SoftwareKeyboard::selectDown()
{
	PointRect* candidate      = 0;
	wchar_t    candidateValue = 0;
	PointRect* farTop         = 0;
	wchar_t    farTopValue    = 0;
	
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		if ((*it).second != m_keyInput)
		{
			if ((*it).second->getCenterPosition().y > m_keyInput->getCenterPosition().y) // below selected
			{
				if (candidate != 0)
				{
					if (candidate->getCenterPosition().y > (*it).second->getCenterPosition().y) // above candidate
					{
						candidate      = (*it).second;
						candidateValue = (*it).first;
					}
					else if (candidate->getCenterPosition().y == (*it).second->getCenterPosition().y) // on same level as candidate
					{
						if (std::abs(candidate->getCenterPosition().x - m_keyInput->getCenterPosition().x) >
						    std::abs((*it).second->getCenterPosition().x - m_keyInput->getCenterPosition().x)) // closer to selected horizontally than candidate
						{
							candidate      = (*it).second;
							candidateValue = (*it).first;
						}
					}
				}
				else // no candidate yet
				{
					candidate      = (*it).second;
					candidateValue = (*it).first;
				}
			}
			else if ((*it).second->getCenterPosition().y < m_keyInput->getCenterPosition().y) // above selected
			{
				if (farTop != 0)
				{
					if (farTop->getCenterPosition().y > (*it).second->getCenterPosition().y) // above farTop
					{
						farTop      = (*it).second;
						farTopValue = (*it).first;
					}
					else if (farTop->getCenterPosition().y == (*it).second->getCenterPosition().y) // on same level as farTop
					{
						if (std::abs(farTop->getCenterPosition().x - m_keyInput->getCenterPosition().x) >
						    std::abs((*it).second->getCenterPosition().x - m_keyInput->getCenterPosition().x)) // closer to selected horizontally than farTop
						{
							farTop      = (*it).second;
							farTopValue = (*it).first;
						}
					}
				}
				else // no farTop yet
				{
					farTop      = (*it).second;
					farTopValue = (*it).first;
				}
			}
		}
	}
	
	if (candidate != 0)
	{
		m_keyInput = candidate;
		m_keyValue = candidateValue;
		return true;
	}
	else if (farTop != 0)
	{
		return false;
		//m_keyInput = farTop;
	}
	
	return false;
}


bool SoftwareKeyboard::selectUp()
{
	PointRect* candidate      = 0;
	wchar_t    candidateValue = 0;
	PointRect* farDown        = 0;
	wchar_t    farDownValue   = 0;
	
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		if ((*it).second != m_keyInput)
		{
			if ((*it).second->getCenterPosition().y < m_keyInput->getCenterPosition().y) // above selected
			{
				if (candidate != 0)
				{
					if (candidate->getCenterPosition().y < (*it).second->getCenterPosition().y) // below candidate
					{
						candidate      = (*it).second;
						candidateValue = (*it).first;
					}
					else if (candidate->getCenterPosition().y == (*it).second->getCenterPosition().y) // on same level as candidate
					{
						if (std::abs(candidate->getCenterPosition().x - m_keyInput->getCenterPosition().x) >
						    std::abs((*it).second->getCenterPosition().x - m_keyInput->getCenterPosition().x)) // closer to selected horizontally than candidate
						{
							candidate      = (*it).second;
							candidateValue = (*it).first;
						}
					}
				}
				else // no candidate yet
				{
					candidate      = (*it).second;
					candidateValue = (*it).first;
				}
			}
			else if ((*it).second->getCenterPosition().y > m_keyInput->getCenterPosition().y) // below selected
			{
				if (farDown != 0)
				{
					if (farDown->getCenterPosition().y < (*it).second->getCenterPosition().y) // below farDown
					{
						farDown      = (*it).second;
						farDownValue = (*it).first;
					}
					else if (farDown->getCenterPosition().y == (*it).second->getCenterPosition().y) // on same level as farDown
					{
						if (std::abs(farDown->getCenterPosition().x - m_keyInput->getCenterPosition().x) >
						    std::abs((*it).second->getCenterPosition().x - m_keyInput->getCenterPosition().x)) // closer to selected horizontally than farDown
						{
							farDown      = (*it).second;
							farDownValue = (*it).first;
						}
					}
				}
				else // no farDown yet
				{
					farDown      = (*it).second;
					farDownValue = (*it).first;
				}
			}
		}
	}
	
	if (candidate != 0)
	{
		m_keyInput = candidate;
		m_keyValue = candidateValue;
		return true;
	}
	else if (farDown != 0)
	{
		return false;
		//m_keyInput = farDown;
	}
	
	return false;
}


void SoftwareKeyboard::setMode(Mode p_mode)
{
	if (p_mode == m_mode)
	{
		return;
	}
	
	cleanCurrentMode();
	m_mode = p_mode;
	
	switch (m_mode)
	{
	case MODE_LOWER_CASE:
		if (m_charset == CHARSET_NORMAL)
		{
			createLowerCase();
		}
		else
		{
			createSpecialChars();
		}
		break;
		
	case MODE_SHIFT:
		if (m_charset == CHARSET_NORMAL)
		{
			createShift();
		}
		else
		{
			createSpecialShift();
		}
		break;
		
	case MODE_CAPS:
		if (m_charset == CHARSET_NORMAL)
		{
			createCaps();
		}
		else
		{
			createSpecialCaps();
		}
		break;
	}
}


void SoftwareKeyboard::setCharset(Charset p_charset)
{
	if (p_charset != m_charset)
	{
		cleanCurrentMode();
		m_charset = p_charset;
		
		switch (m_mode)
		{
		case MODE_LOWER_CASE:
			if (m_charset == CHARSET_NORMAL)
			{
				createLowerCase();
			}
			else
			{
				createSpecialChars();
			}
			break;
			
		case MODE_SHIFT:
			if (m_charset == CHARSET_NORMAL)
			{
				createShift();
			}
			else
			{
				createSpecialShift();
			}
			break;
			
		case MODE_CAPS:
			if (m_charset == CHARSET_NORMAL)
			{
				createCaps();
			}
			else
			{
				createSpecialCaps();
			}
			break;
		}
	}
}


void SoftwareKeyboard::cleanCurrentMode()
{
	for (KeyboardIter it = m_keyMap.begin(); it != m_keyMap.end(); ++it)
	{
		delete (*it).second;
	}
	m_keyMap.clear();
	
	m_keyInput = 0;
	m_keyValue = 0;
	
	m_onStylusPressed         = false;
	m_timeStylusPressed       = 0;
	m_timeStylusActionHandled = 0;
	m_stylusInput             = 0;
	m_stylusValue             = 0;
	
	m_capsRect    = 0;
	m_shiftRect   = 0;
	m_specialRect = 0;
}


void SoftwareKeyboard::dismissKeyboard()
{
	// Cannot exit the keyboard if no string entered
	removeCursor();
	if (m_text.empty() == false)
	{
		// Set the specified system variable to the string that was entered
		std::string str = MenuUtils::wideStringToHex(m_text);
		if (m_varName.empty() == false)
		{
			MenuSystem::getInstance()->setSystemVar(m_varName, str);
		}
		else
		{
			MenuSystem::getInstance()->setSystemVar("return_value", str);
		}
		
		// Close the keyboard
		MenuActions actions;
		actions.push_back(MenuAction("close_menu"));
		MenuSystem::getInstance()->doActions(actions);
	}
	
	addCursor();
}


math::PointRect* SoftwareKeyboard::getEquivalentPointer(
		math::PointRect*                          p_orgPtr,
		const SoftwareKeyboard::KeyboardSegments& p_rhs) const
{
	if (p_orgPtr == 0)
	{
		return 0;
	}
	
	for (KeyboardSegments::const_iterator it = p_rhs.begin();
	     it != p_rhs.end(); ++it)
	{
		// Check if this item is the pointer we're looking for
		if ((*it).second == p_orgPtr)
		{
			// Find the equivalent item in our own map
			KeyboardSegments::const_iterator ptr = m_keyMap.find((*it).first);
			if (ptr == m_keyMap.end())
			{
				TT_PANIC("Equivalent pointer of %p not found in this map.",
				         p_orgPtr);
				return 0;
			}
			else
			{
				return (*ptr).second;
			}
		}
	}
	
	TT_PANIC("Pointer %p not found in foreign map.", p_orgPtr);
	return 0;
}

// Namespace end
}
}
}
