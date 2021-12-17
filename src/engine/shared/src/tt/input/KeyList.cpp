#include <tt/input/KeyList.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

const char* getKeyName(Key p_key)
{
	switch (p_key)
	{
	case Key_Backspace:          return "backspace";
	case Key_Tab:                return "tab";
	case Key_Clear:              return "clear";
	case Key_Enter:              return "enter";
	case Key_Shift:              return "shift";
	case Key_Control:            return "control";
	case Key_Alt:                return "alt";
	case Key_Break:              return "break";
	case Key_CapsLock:           return "capslock";
	case Key_Escape:             return "escape";
	case Key_Space:              return "space";
	case Key_PageUp:             return "pageup";
	case Key_PageDown:           return "pagedown";
	case Key_End:                return "end";
	case Key_Home:               return "home";
	case Key_Left:               return "left";
	case Key_Up:                 return "up";
	case Key_Right:              return "right";
	case Key_Down:               return "down";
	case Key_Insert:             return "insert";
	case Key_Delete:             return "delete";
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
	case Key_A:                  return "a";
	case Key_B:                  return "b";
	case Key_C:                  return "c";
	case Key_D:                  return "d";
	case Key_E:                  return "e";
	case Key_F:                  return "f";
	case Key_G:                  return "g";
	case Key_H:                  return "h";
	case Key_I:                  return "i";
	case Key_J:                  return "j";
	case Key_K:                  return "k";
	case Key_L:                  return "l";
	case Key_M:                  return "m";
	case Key_N:                  return "n";
	case Key_O:                  return "o";
	case Key_P:                  return "p";
	case Key_Q:                  return "q";
	case Key_R:                  return "r";
	case Key_S:                  return "s";
	case Key_T:                  return "t";
	case Key_U:                  return "u";
	case Key_V:                  return "v";
	case Key_W:                  return "w";
	case Key_X:                  return "x";
	case Key_Y:                  return "y";
	case Key_Z:                  return "z";
	case Key_Numpad0:            return "num0";
	case Key_Numpad1:            return "num1";
	case Key_Numpad2:            return "num2";
	case Key_Numpad3:            return "num3";
	case Key_Numpad4:            return "num4";
	case Key_Numpad5:            return "num5";
	case Key_Numpad6:            return "num6";
	case Key_Numpad7:            return "num7";
	case Key_Numpad8:            return "num8";
	case Key_Numpad9:            return "num9";
	case Key_Multiply:           return "*";
	case Key_Slash:              return "/";
	case Key_F1:                 return "f1";
	case Key_F2:                 return "f2";
	case Key_F3:                 return "f3";
	case Key_F4:                 return "f4";
	case Key_F5:                 return "f5";
	case Key_F6:                 return "f6";
	case Key_F7:                 return "f7";
	case Key_F8:                 return "f8";
	case Key_F9:                 return "f9";
	case Key_F10:                return "f10";
	case Key_F11:                return "f11";
	case Key_F12:                return "f12";
	case Key_NumLock:            return "numlock";
	case Key_ScrollLock:         return "scrolllock";
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
	default:                     return "unknown";
	}
}


Key getKeyFromName(const std::string& p_name)
{
	for (s32 i = 0; i < Key_Count; ++i)
	{
		Key key = static_cast<Key>(i);
		if (getKeyName(key) == p_name)
		{
			return key;
		}
	}
	
	return Key_Count;
}

// Namespace end
}
}
