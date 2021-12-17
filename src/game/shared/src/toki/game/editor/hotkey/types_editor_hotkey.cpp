#include <cctype>

#include <tt/platform/tt_error.h>

#include <toki/game/editor/hotkey/types.h>


namespace toki {
namespace game {
namespace editor {
namespace hotkey {

inline const char* const getKeyDisplayName(tt::input::Key p_key)
{
	using namespace tt::input;
	switch (p_key)
	{
	case Key_Backspace:          return "Backspace";
	case Key_Tab:                return "Tab";
	case Key_Clear:              return "Clear";
	case Key_Enter:              return "Enter";
	case Key_Shift:              return "Shift";
	case Key_Control:            return "Ctrl";
	case Key_Alt:                return "Alt";
	case Key_Break:              return "Break";
	case Key_CapsLock:           return "Caps Lock";
	case Key_Escape:             return "Esc";
	case Key_Space:              return "Space";
	case Key_PageUp:             return "Page Up";
	case Key_PageDown:           return "Page Down";
	case Key_End:                return "End";
	case Key_Home:               return "Home";
	case Key_Left:               return "Left";
	case Key_Up:                 return "Up";
	case Key_Right:              return "Right";
	case Key_Down:               return "Down";
	case Key_Insert:             return "Insert";
	case Key_Delete:             return "Del";
	case Key_0:                  return "0";
	case Key_1:                  return "1";
	case Key_2:                  return "2";
	case Key_3:                  return "3";
	case Key_4:                  return "4";
	case Key_5:                  return "5";
	case Key_6:                  return "6";
	case Key_7:                  return "7";
	case Key_8:                  return "8";
	case Key_9:                  return "9";
	case Key_A:                  return "A";
	case Key_B:                  return "B";
	case Key_C:                  return "C";
	case Key_D:                  return "D";
	case Key_E:                  return "E";
	case Key_F:                  return "F";
	case Key_G:                  return "G";
	case Key_H:                  return "H";
	case Key_I:                  return "I";
	case Key_J:                  return "J";
	case Key_K:                  return "K";
	case Key_L:                  return "L";
	case Key_M:                  return "M";
	case Key_N:                  return "N";
	case Key_O:                  return "O";
	case Key_P:                  return "P";
	case Key_Q:                  return "Q";
	case Key_R:                  return "R";
	case Key_S:                  return "S";
	case Key_T:                  return "T";
	case Key_U:                  return "U";
	case Key_V:                  return "V";
	case Key_W:                  return "W";
	case Key_X:                  return "X";
	case Key_Y:                  return "Y";
	case Key_Z:                  return "Z";
	case Key_Numpad0:            return "0";
	case Key_Numpad1:            return "1";
	case Key_Numpad2:            return "2";
	case Key_Numpad3:            return "3";
	case Key_Numpad4:            return "4";
	case Key_Numpad5:            return "5";
	case Key_Numpad6:            return "6";
	case Key_Numpad7:            return "7";
	case Key_Numpad8:            return "8";
	case Key_Numpad9:            return "9";
	case Key_Multiply:           return "*";
	case Key_Slash:              return "/";
	case Key_F1:                 return "F1";
	case Key_F2:                 return "F2";
	case Key_F3:                 return "F3";
	case Key_F4:                 return "F4";
	case Key_F5:                 return "F5";
	case Key_F6:                 return "F6";
	case Key_F7:                 return "F7";
	case Key_F8:                 return "F8";
	case Key_F9:                 return "F9";
	case Key_F10:                return "F10";
	case Key_F11:                return "F11";
	case Key_F12:                return "F12";
	case Key_NumLock:            return "Num Lock";
	case Key_ScrollLock:         return "Scroll Lock";
	case Key_Semicolon:          return ";";
	case Key_Plus:               return "+";
	case Key_Comma:              return ",";
	case Key_Minus:              return "-";
	case Key_Period:             return ".";
	case Key_Grave:              return "`";
	case Key_LeftSquareBracket:  return "[";
	case Key_Backslash:          return "\\";
	case Key_RightSquareBracket: return "]";
	case Key_Apostrophe:         return "'";
	default:                     return "UNKNOWN";
	}
}


std::string getDisplayName(tt::input::Key p_actionKey, const Modifiers& p_modifiers)
{
	std::string displayName;
	if (p_modifiers.checkFlag(hotkey::Modifier_Ctrl))  displayName += "Ctrl+";
	if (p_modifiers.checkFlag(hotkey::Modifier_Shift)) displayName += "Shift+";
	if (p_modifiers.checkFlag(hotkey::Modifier_Alt))   displayName += "Alt+";
	displayName += getKeyDisplayName(p_actionKey);
	
	return displayName;
}

// Namespace end
}
}
}
}
