#if !defined(INC_TT_MENU_ELEMENTS_SOFTWAREKEYBOARD_H)
#define INC_TT_MENU_ELEMENTS_SOFTWAREKEYBOARD_H


#include <map>

#include <tt/engine/renderer/fwd.h>
#include <tt/math/Rect.h>
#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/MenuKeyboard.h>


namespace tt {
namespace menu {
namespace elements {

class Marker;

/*! \brief A virtual on-screen keyboard. */
class SoftwareKeyboard : public MenuElement
{
public:
	SoftwareKeyboard(const std::string&  p_name,
	                 const MenuLayout&   p_layout,
	                 const std::string&  p_varName,
	                 const std::string&  p_label,
	                 const std::string&  p_okay,
	                 const std::wstring& p_defaultValue = L"",
	                 const s32           p_length        = -1,
	                 const s32           p_pixelLength   = -1);
	virtual ~SoftwareKeyboard();
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual bool onStylusPressed(s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyHold(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	
	virtual SoftwareKeyboard* clone() const;
	
protected:
	SoftwareKeyboard(const SoftwareKeyboard& p_rhs);
	
private:
	enum
	{
		BUTTON_BORDER = 1,
		BUTTON_DELAY  = 500,
		BUTTON_REPEAT = 100
	};
	
	enum Mode
	{
		MODE_LOWER_CASE,
		MODE_CAPS,
		MODE_SHIFT
	};
	
	enum Charset
	{
		CHARSET_NORMAL,
		CHARSET_SPECIAL
	};
	
	enum SpecialKey
	{
		KEY_SPECIAL   = 0x0006,
		KEY_CAPS      = 0x0007,
		KEY_BACKSPACE = 0x0008,
		KEY_SHIFT     = 0x0009,
		
		KEY_ENTER     = 0x000D
	};
	
	enum
	{
		CursorBlinkSpeed    = 10,
		KeyboardImageWidth  = 211,
		KeyboardImageHeight = 105
	};
	
	typedef std::map<wchar_t, math::PointRect*>   KeyboardSegments;
	typedef KeyboardSegments::value_type KeyboardEntry;
	typedef KeyboardSegments::iterator   KeyboardIter;
	
	
	void updateCursor();
	void setCursor(bool p_onOff);
	void addCursor();
	void removeCursor();
	
	void createLowerCase();
	void createCaps();
	void createShift();
	void createSpecialChars();
	void createSpecialShift();
	void createSpecialCaps();
	
	void setMode(Mode p_mode);
	void setCharset(Charset p_charset);
	void cleanCurrentMode();
	
	void addRectangle(wchar_t p_value,
	                  s32     p_x      = -1,
	                  s32     p_y      = -1,
	                  s32     p_width  = -1,
	                  s32     p_height = -1);
	
	void handleKeyPress(wchar_t p_value);
	bool selectLeft();
	bool selectRight();
	bool selectUp();
	bool selectDown();
	void dismissKeyboard();
	
	math::PointRect* getEquivalentPointer(math::PointRect*        p_orgPtr,
	                                      const KeyboardSegments& p_rhs) const;
	
	// No assignment
	const SoftwareKeyboard& operator=(const SoftwareKeyboard&);
	
	
	engine::renderer::TexturePtr m_keyTexture;
	KeyboardSegments             m_keyMap;
	
	bool                            m_onKeyPressed;
	engine::renderer::QuadSpritePtr m_keyQuad;
	math::PointRect*                m_keyInput;
	wchar_t                         m_keyValue;
	
	math::PointRect*                m_shiftRect;
	math::PointRect*                m_capsRect;
	math::PointRect*                m_specialRect;
	engine::renderer::QuadSpritePtr m_highlight;
	
	Marker* m_marker;
	
	bool                            m_onStylusPressed;
	u64                             m_timeStylusPressed;
	u64                             m_timeStylusActionHandled;
	engine::renderer::QuadSpritePtr m_stylusQuad;
	math::PointRect*                m_stylusInput;
	wchar_t                         m_stylusValue;
	
	engine::renderer::QuadSpritePtr m_keyboardQuad;
	
	Mode    m_mode;
	Charset m_charset;
	
	s32 m_length;
	s32 m_pixelLength;
	s32 m_rectX;
	s32 m_rectY;
	s32 m_rectWidth;
	s32 m_rectHeight;
	
	const std::string m_varName;
	const std::string m_label;
	const std::string m_okay;
	
	std::wstring m_text;
	
	bool m_cursorBlink;
	s32  m_cursorTime;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SOFTWAREKEYBOARD_H)
