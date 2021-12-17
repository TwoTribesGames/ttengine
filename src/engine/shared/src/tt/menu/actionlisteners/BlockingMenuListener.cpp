#include <wchar.h>
#include <tt/platform/tt_error.h>

#include <tt/menu/actionlisteners/BlockingMenuListener.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuUtils.h>


namespace tt {
namespace menu {
namespace actionlisteners {

//------------------------------------------------------------------------------
// Public member functions

BlockingMenuListener::~BlockingMenuListener()
{
}


void BlockingMenuListener::createListener()
{
	// Create a listener and register it with the menu system
	MenuSystem* sys = MenuSystem::getInstance();
	sys->registerDefaultBlockingActionListener(new BlockingMenuListener);
}


bool BlockingMenuListener::doAction(const MenuAction& p_action)
{
	std::string command(p_action.getCommand());
	
	if (command == "open_softwarekeyboard")
	{
		// Open the software keyboard
		TT_ASSERTMSG(p_action.getParameterCount() >= 3,
		             "Command '%s' requires at least three parameters: "
		             "the title and the variable of the keyboard and whether "
		             "to allow using the back button to get out of the "
		             "software keyboard.",
		             command.c_str());
		
		// Prepare system variables for the software keyboard
		MenuSystem* sys = MenuSystem::getInstance();
		MENU_Printf("BlockingMenuListener::doAction: Opening software "
		            "keyboard with title '%s'.\n",
		            p_action.getParameter(0).c_str());
		sys->setSystemVar("softwarekeyboard_title",
		                  p_action.getParameter(0));
		sys->setSystemVar("softwarekeyboard_variable",
		                  p_action.getParameter(1));
		sys->setSystemVar("softwarekeyboard_cangoback",
		                  p_action.getParameter(2));
		
		if (p_action.getParameterCount() > 3)
		{
			sys->setSystemVar("softwarekeyboard_length",
			                  p_action.getParameter(3));
		}
		else
		{
			sys->setSystemVar("softwarekeyboard_length", "");
		}
		
		if (p_action.getParameterCount() > 4)
		{
			sys->setSystemVar("softwarekeyboard_pixellength",
			                  p_action.getParameter(4));
		}
		else
		{
			sys->setSystemVar("softwarekeyboard_pixellength", "");
		}
		
		if (p_action.getParameterCount() > 5)
		{
			sys->setSystemVar("softwarekeyboard_default_value",
			                  p_action.getParameter(5));
		}
		else
		{
			sys->setSystemVar("softwarekeyboard_default_value", "");
		}
		
		// Open the software keyboard
		sys->openPopupMenu("softwarekeyboard");
		
		return true;
	}
	else if (command == "open_random_name")
	{
		// Open the software keyboard
		TT_ASSERTMSG(p_action.getParameterCount() >= 2,
		             "Command '%s' requires at least two parameters: "
		             "the title and the variable of the keyboard.",
		             command.c_str());
		
		// Prepare system variables for the software keyboard
		MenuSystem* sys = MenuSystem::getInstance();
		MENU_Printf("BlockingMenuListener::doAction: Opening software keyboard "
		            "with title '%s'.\n", p_action.getParameter(0).c_str());
		sys->setSystemVar("softwarekeyboard_title",
		                  p_action.getParameter(0));
		sys->setSystemVar("softwarekeyboard_variable",
		                  p_action.getParameter(1));
		
		if (p_action.getParameterCount() > 2)
		{
			sys->setSystemVar("softwarekeyboard_length",
			                  p_action.getParameter(2));
		}
		else
		{
			sys->setSystemVar("softwarekeyboard_length", "");
		}

		if (p_action.getParameterCount() > 3)
		{
			sys->setSystemVar("softwarekeyboard_default_value",
			                  p_action.getParameter(3));
		}
		else
		{
			sys->setSystemVar("softwarekeyboard_default_value", "");
		}
		
		// Open the software keyboard
		sys->openPopupMenu("randomname");
		
		return true;
	}
	else if (command == "open_menu_and_wait")
	{
		// Open a pop-up menu
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' requires one parameter: "
		             "the name of the menu to open.",
		             command.c_str());
		
		MenuSystem::getInstance()->openPopupMenu(p_action.getParameter(0));
		
		return true;
	}
	else if (command == "confirm")
	{
		// Required:
		// 0. display text (will be localized)
		// 1. variable to replace in text
		// 2. name of the action set to execute when the 'yes' button is pressed
		// 3. name of the action set to execute when the 'no' button is pressed
		
		// Optional:
		// 4. default button ('yes' or 'no')
		// 5. window title (will be localized)
		// 6. caption for 'yes' button (will be localized)
		// 7. caption for 'no' button (will be localized)
		// 8. whether to render the underlying menus
		
		enum
		{
			Param_DisplayText   = 0,
			Param_TextVariable  = 1,
			Param_YesActionSet  = 2,
			Param_NoActionSet   = 3,
			
			Param_MaxRequired,
			
			Param_DefaultButton = Param_MaxRequired,
			Param_WindowTitle   = 5,
			Param_YesCaption    = 6,
			Param_NoCaption     = 7,
			Param_RenderBackground = 8,
			
			Param_MaxTotal
		};
		
		// Verify parameter count
		TT_ASSERTMSG(p_action.getParameterCount() >= Param_MaxRequired &&
		             p_action.getParameterCount() <= Param_MaxTotal,
		             "Command '%s' requires at least %d parameters (%d maximum): "
		             "the text to display, variable value to replace in the text,"
		             "action set for no button, action set for yes button.",
		             command.c_str(), Param_MaxRequired, Param_MaxTotal);
		
		int         paramCount = p_action.getParameterCount();
		MenuSystem* sys        = MenuSystem::getInstance();
		
		// Parse the display text to incorporate the variable
		std::wstring displayTextFormat(sys->translateString(
			p_action.getParameter(Param_DisplayText)));
		
		const std::wstring token(L"$VAR$");
		std::wstring::size_type idx = displayTextFormat.find(token);
		
		std::wstring displayText;
		if (idx != std::wstring::npos)
		{
			// A variable is expected in the format string; parse string
			displayText = displayTextFormat.substr(0, idx);
			displayText += MenuUtils::hexToWideString(
				p_action.getParameter(Param_TextVariable));
			displayText += displayTextFormat.substr(idx + token.length());
		}
		else
		{
			// No variable expected
			displayText = displayTextFormat;
		}
		
		// Set required system variables
		sys->setSystemVar("g_confirm_display_text",
		                  MenuUtils::wideStringToHex(displayText));
		sys->setSystemVar("g_confirm_yes_actions",
		                  p_action.getParameter(Param_YesActionSet));
		sys->setSystemVar("g_confirm_no_actions",
		                  p_action.getParameter(Param_NoActionSet));
		
		// Set optional system variables
		// - Default button
		if (paramCount > Param_DefaultButton)
		{
			sys->setSystemVar("g_confirm_default_button",
			                  p_action.getParameter(Param_DefaultButton));
		}
		else
		{
			sys->setSystemVar("g_confirm_default_button", "no");
		}
		
		// - Window title
		if (paramCount > Param_WindowTitle)
		{
			sys->setSystemVar("g_confirm_window_title",
			                  p_action.getParameter(Param_WindowTitle));
		}
		else
		{
			sys->setSystemVar("g_confirm_window_title",
			                  "CONFIRM_DEFAULT_WINDOW_TITLE");
		}
		
		// - 'Yes' button caption
		if (paramCount > Param_YesCaption)
		{
			sys->setSystemVar("g_confirm_yes_caption",
			                  p_action.getParameter(Param_YesCaption));
		}
		else
		{
			sys->setSystemVar("g_confirm_yes_caption",
			                  "CONFIRM_DEFAULT_BTN_YES");
		}
		
		// - 'No' button caption
		if (paramCount > Param_NoCaption)
		{
			sys->setSystemVar("g_confirm_no_caption",
			                  p_action.getParameter(Param_NoCaption));
		}
		else
		{
			sys->setSystemVar("g_confirm_no_caption",
			                  "CONFIRM_DEFAULT_BTN_NO");
		}
		
		// - Whether to render underlying menus
		if (paramCount > Param_RenderBackground)
		{
			sys->setSystemVar("g_confirm_render_background",
			                  p_action.getParameter(Param_RenderBackground));
		}
		else
		{
			sys->setSystemVar("g_confirm_render_background", "true");
		}
		
		
		// Open the confirmation menu
		sys->openPopupMenu("generic_confirm");
		
		return true;
	}
	
	// Listener cannot handle this command
	return false;
}


std::string BlockingMenuListener::getName() const
{
	return "blocking_menu_actions";
}


BlockingMenuListener::BlockingMenuListener()
{
}

// Namespace end
}
}
}
