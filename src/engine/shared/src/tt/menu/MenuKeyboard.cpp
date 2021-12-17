#include <tt/platform/tt_error.h>
#include <tt/xml/XmlFileReader.h>
#include <tt/menu/MenuKeyboard.h>


namespace tt {
namespace menu {

// FIXME: Dummy definitions for the PAD_* identifiers,
//        so that current code compiles with minimal modifications.

#ifdef PAD_KEY_UP
	#undef PAD_KEY_UP
#endif

#ifdef PAD_BUTTON_A
	#undef PAD_BUTTON_A
#endif

#ifdef PAD_BUTTON_B
	#undef PAD_BUTTON_B
#endif

#ifdef PAD_BUTTON_B
	#undef PAD_BUTTON_B
#endif

#ifdef PAD_BUTTON_X
	#undef PAD_BUTTON_X
#endif

#ifdef PAD_BUTTON_Y
	#undef PAD_BUTTON_Y
#endif

#ifdef PAD_BUTTON_R
	#undef PAD_BUTTON_R
#endif

#ifdef PAD_BUTTON_SELECT
	#undef PAD_BUTTON_SELECT
#endif

#ifdef PAD_BUTTON_START
	#undef PAD_BUTTON_START
#endif


enum PadDummy
{
	PAD_KEY_UP        = 0x0001,
	PAD_KEY_DOWN      = 0x0002,
	PAD_KEY_LEFT      = 0x0004,
	PAD_KEY_RIGHT     = 0x0008,
	PAD_BUTTON_A      = 0x0010,
	PAD_BUTTON_B      = 0x0020,
	PAD_BUTTON_X      = 0x0040,
	PAD_BUTTON_Y      = 0x0080,
	PAD_BUTTON_L      = 0x0100,
	PAD_BUTTON_R      = 0x0200,
	PAD_BUTTON_START  = 0x0400,
	PAD_BUTTON_SELECT = 0x0800
};


// Default virtual key mapping; can be changed by loadVirtualKeyMapping
MenuKeyboard::MenuKey MenuKeyboard::MENU_UP        = PAD_KEY_UP;
MenuKeyboard::MenuKey MenuKeyboard::MENU_DOWN      = PAD_KEY_DOWN;
MenuKeyboard::MenuKey MenuKeyboard::MENU_LEFT      = PAD_KEY_LEFT;
MenuKeyboard::MenuKey MenuKeyboard::MENU_RIGHT     = PAD_KEY_RIGHT;
MenuKeyboard::MenuKey MenuKeyboard::MENU_ACCEPT    = PAD_BUTTON_A;
MenuKeyboard::MenuKey MenuKeyboard::MENU_CANCEL    = PAD_BUTTON_B;
MenuKeyboard::MenuKey MenuKeyboard::MENU_BACK      = PAD_BUTTON_B;
MenuKeyboard::MenuKey MenuKeyboard::MENU_L_TRIGGER = PAD_BUTTON_L;
MenuKeyboard::MenuKey MenuKeyboard::MENU_R_TRIGGER = PAD_BUTTON_R;
MenuKeyboard::MenuKey MenuKeyboard::MENU_EDIT      = PAD_BUTTON_X;
MenuKeyboard::MenuKey MenuKeyboard::MENU_START     = PAD_BUTTON_START;

MenuKeyboard::MenuKey MenuKeyboard::MENU_NOT_A_VALID_VIRTUAL_KEY = 0;


//------------------------------------------------------------------------------
// Public member functions

void MenuKeyboard::loadVirtualKeyMapping(const std::string& p_filename)
{
	using namespace xml;
	
	// Load the virtual key mapping XML file
	XmlFileReader xml;
	if (xml.loadFile(p_filename) == false)
	{
		TT_PANIC("Failed to load XML file '%s'.", p_filename.c_str());
	}
	
	// Parse the XML file
	while (xml.read())
	{
		// Look for key elements
		if (xml.getNodeType() == EnumNode_Element &&
		    xml.getNodeName() == "key")
		{
			const std::string virt = xml.getAttributeValue("virtual");
			const std::string real = xml.getAttributeValue("real");
			
			TT_ASSERTMSG(virt.empty() == false,
			             "Missing required key attribute 'virtual'.");
			TT_ASSERTMSG(real.empty() == false,
			             "Missing required key attribute 'real'.");
			
			// Check which virtual key this element is setting
			getKeyMappingFromString(virt) = getKeyCode(real);
		}
	}
}


MenuKeyboard::MenuKey& MenuKeyboard::getKeyMappingFromString(
		const std::string& p_key)
{
	// Return the value mapped to the specified key name.
	if (p_key == "up")
	{
		return MENU_UP;
	}
	else if (p_key == "down")
	{
		return MENU_DOWN;
	}
	else if (p_key == "left")
	{
		return MENU_LEFT;
	}
	else if (p_key == "right")
	{
		return MENU_RIGHT;
	}
	else if (p_key == "accept")
	{
		return MENU_ACCEPT;
	}
	else if (p_key == "cancel")
	{
		return MENU_CANCEL;
	}
	else if (p_key == "back")
	{
		return MENU_BACK;
	}
	else if (p_key == "L")
	{
		return MENU_L_TRIGGER;
	}
	else if (p_key == "R")
	{
		return MENU_R_TRIGGER;
	}
	else if (p_key == "edit")
	{
		return MENU_EDIT;
	}
	else if (p_key == "start")
	{
		return MENU_START;
	}
	else
	{
		TT_PANIC("Invalid virtual key name '%s' specified.", p_key.c_str());
		return MENU_NOT_A_VALID_VIRTUAL_KEY;
	}
}


MenuKeyboard::MenuKeyboard(u32 p_keys)
:
m_keys(p_keys)
{
}


//------------------------------------------------------------------------------
// Private member functions

MenuKeyboard::MenuKey MenuKeyboard::getKeyCode(const std::string& p_realKeyName)
{
	if (p_realKeyName == "dpad_up")
	{
		return PAD_KEY_UP;
	}
	else if (p_realKeyName == "dpad_down")
	{
		return PAD_KEY_DOWN;
	}
	else if (p_realKeyName == "dpad_left") 
	{
		return PAD_KEY_LEFT;
	}
	else if (p_realKeyName == "dpad_right") 
	{
		return PAD_KEY_RIGHT;
	}
	else if (p_realKeyName == "a") 
	{
		return PAD_BUTTON_A;
	}
	else if (p_realKeyName == "b") 
	{
		return PAD_BUTTON_B;
	}
	else if (p_realKeyName == "x") 
	{
		return PAD_BUTTON_X;
	}
	else if (p_realKeyName == "y") 
	{
		return PAD_BUTTON_Y;
	}
	else if (p_realKeyName == "l") 
	{
		return PAD_BUTTON_L;
	}
	else if (p_realKeyName == "r") 
	{
		return PAD_BUTTON_R;
	}
	else if (p_realKeyName == "start") 
	{
		return PAD_BUTTON_START;
	}
	else if (p_realKeyName == "select") 
	{
		return PAD_BUTTON_SELECT;
	}
	else
	{
		TT_PANIC("Invalid real key name specified: '%s'",
		         p_realKeyName.c_str());
	}
	
	return 0;
}

// Namespace end
}
}
