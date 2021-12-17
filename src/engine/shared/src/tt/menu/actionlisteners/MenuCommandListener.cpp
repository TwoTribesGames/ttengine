#include <tt/code/ErrorStatus.h>
//#include <tt/memory/MemoryProfiler.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSoundPlayer.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuUtils.h>
#include <tt/menu/actionlisteners/MenuCommandListener.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
//#include <tt/save/SaveFile.h>
//#include <tt/save/SaveFS.h>
#include <tt/str/str.h>


namespace tt {
namespace menu {
namespace actionlisteners {

//------------------------------------------------------------------------------
// Public member functions

MenuCommandListener::~MenuCommandListener()
{
}


void MenuCommandListener::createListener()
{
	// Create a listener and register it with the menu system
	MenuSystem* sys = MenuSystem::getInstance();
	sys->registerDefaultActionListener(new MenuCommandListener);
}


bool MenuCommandListener::doAction(const MenuAction& p_action)
{
	std::string command(p_action.getCommand());
	
	MENU_Printf("MenuCommandListener::doAction: Received command '%s'.\n",
	            command.c_str());
	
	if (command == "goto_menu")
	{
		// Go to a new menu in the menu tree
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' requires one parameter "
		             "(the name of the tree-menu to switch to).",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Moving to menu '%s'.\n",
		            p_action.getParameter(0).c_str());
		MenuSystem::getInstance()->gotoMenu(p_action.getParameter(0));
		
		return true;
	}
	else if (command == "jump_to_menu")
	{
		// Go to a new menu in the menu tree
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'jump_to_menu' requires one parameter "
		             "(the name of the tree-menu to jump to).",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Jumping to menu '%s'.\n",
		            p_action.getParameter(0).c_str());
		MenuSystem::getInstance()->jumpToMenu(p_action.getParameter(0));
		
		return true;
	}
	else if (command == "open_menu")
	{
		// Go to a new menu in the menu tree
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command 'open_menu' requires one parameter "
		             "(the name of the pop-up menu to open).",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Opening pop-up menu '%s'.\n",
		            p_action.getParameter(0).c_str());
		MenuSystem::getInstance()->openPopupMenu(p_action.getParameter(0));
		
		return true;
	}
	else if (command == "close_menu")
	{
		// Close a pop-up menu
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command 'close_menu' does not take any parameters.",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Closing top-most pop-up menu.\n");
		MenuSystem::getInstance()->closePopupMenu();
		
		return true;
	}
	else if (command == "go_back_menu")
	{
		// Go a menu back. (only in tree menu)
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command 'go_back_menu' does not take any parameters.",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Going back a menu.\n");
		MenuSystem::getInstance()->goBackMenu();
		
		return true;
	}
	else if (command == "close_all_popups")
	{
		// Close a pop-up menu
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' does not take any parameters.",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Closing all open pop-up menus.\n");
		MenuSystem* sys = MenuSystem::getInstance();
		while (sys->isPopupMenuOpen())
		{
			sys->closePopupMenu();
		}
		
		return true;
	}
	else if (command == "set_system_var")
	{
		// set a system variable
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "Command '%s' requires 2 parameters "
		             "(variable name and value).",
		             command.c_str());
		
		MENU_Printf("MenuCommandListener::doAction: Creating system var '%s' "
		            "with value '%s'\n",
		            p_action.getParameter(0).c_str(),
		            p_action.getParameter(1).c_str());
		MenuSystem::getInstance()->setSystemVar(p_action.getParameter(0),
		                                        p_action.getParameter(1));
		return true;
	}
	else if (command == "remove_system_var")
	{
		// Remove a system variable
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "the name of the system variable to remove.",
		             command.c_str());
		
		MenuSystem::getInstance()->removeSystemVar(p_action.getParameter(0));
		return true;
	}
	else if (command == "open_treemenu")
	{
		// Open the tree-menu
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters.",
		             command.c_str());
		MenuSystem::getInstance()->openTreeMenu();
		return true;
	}
	else if (command == "close_treemenu")
	{
		// Close the tree-menu (delayed; destroyed next frame)
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters.",
		             command.c_str());
		MenuSystem::getInstance()->closeTreeMenuDelayed();
		return true;
	}
	else if (command == "set_startup_menu")
	{
		// Set the start-up menu
		TT_ASSERTMSG(p_action.getParameterCount() == 1 ||
		             p_action.getParameterCount() == 2,
		             "Command '%s' takes one required parameter and one "
		             "optional parameter: "
		             "the name of the menu to set as start-up menu, "
		             "and optionally the name of its parent menu.",
		             command.c_str());
		
		std::string parent;
		if (p_action.getParameterCount() == 2)
		{
			parent = p_action.getParameter(1);
		}
		MenuSystem::getInstance()->setStartupTreeMenu(p_action.getParameter(0), parent);
		return true;
	}
	else if (command == "execute_action_set")
	{
		// Close the tree-menu (delayed; destroyed next frame)
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "the name of the action set to execute.",
		             command.c_str());
		
		MenuSystem::getInstance()->executeActionSet(p_action.getParameter(0));
		return true;
	}
	else if (command == "string_to_hex")
	{
		// Convert a string to hex
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "Command '%s' requires 2 parameters: "
		             "string to convert and menu variable to store the result in.",
		             command.c_str());
		
		std::string hexStr(MenuUtils::wideStringToHex(str::widen(p_action.getParameter(0))));
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(1), hexStr);
		
		return true;
	}
	else if (command == "delete_file_from_variable")
	{
		// Delete a file
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "Command '%s' requires 2 parameters: "
		             "the variables holding the filename and type in hex.",
		             command.c_str());
		
		/*
		using save::SaveFS;
		using save::SaveFile;
		std::wstring filename = MenuUtils::hexToWideString(MenuSystem::getInstance()->getSystemVar(p_action.getParameter(0)));
		std::wstring filetype = MenuUtils::hexToWideString(MenuSystem::getInstance()->getSystemVar(p_action.getParameter(1)));
		SaveFile* file = SaveFS::openFile(filename, filetype.at(0), false);
		
		TT_ASSERTMSG(file != 0, "Command '%s': file does not exist.",
		             command.c_str());
		
		SaveFS::deleteFile(file);
		*/
		
		return true;
	}
	else if (command == "play_menu_sound")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1 ||
		             p_action.getParameterCount() == 2,
		             "Command '%s' requires 1 parameter, with one optional parameter: "
		             "the name of the sound effect to play and whether the sound should loop.",
		             command.c_str());
		
		MenuSound sound = MenuSoundPlayer::getSoundEnum(p_action.getParameter(0));
		
		bool loop = false;
		if (p_action.getParameterCount() == 2)
		{
			TT_ERR_CREATE(command);
			loop = str::parseBool(p_action.getParameter(1), &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Invalid looping parameter specified: '%s'",
				         p_action.getParameter(1).c_str());
			}
		}
		
		MenuSystem::getInstance()->playSound(sound, loop);
		
		return true;
	}
	else if (command == "stop_menu_looping_sounds")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters.",
		             command.c_str());
		
		MenuSystem::getInstance()->stopLoopingSounds();
		
		return true;
	}
	else if (command == "add_integer")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 3,
		             "Command '%s' takes three parameters: "
		             "two integers to add and the name of the "
		             "system variable to store the result in.",
		             command.c_str());
		
		TT_ERR_CREATE(command);
		
		s32 a = str::parseS32(p_action.getParameter(0), &errStatus);
		s32 b = str::parseS32(p_action.getParameter(1), &errStatus);
		
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid first or second integer specified: '%s' or '%s'",
			         p_action.getParameter(0).c_str(), p_action.getParameter(1).c_str());
		}
		
		std::string added(str::toStr(a + b));
		MenuSystem::getInstance()->setSystemVar(p_action.getParameter(2), added);
		
		return true;
	}
	else if (command == "translate_string")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "Command '%s' takes two parameters: "
		             "the string identifier to translate and "
		             "the name of a system variable to store the result in.",
		             command.c_str());
		
		MenuSystem*  sys = MenuSystem::getInstance();
		std::wstring trans(sys->translateString(p_action.getParameter(0)));
		sys->setSystemVar(p_action.getParameter(1),
		                  MenuUtils::wideStringToHex(trans));
		
		return true;
	}
	else if (command == "append_string")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 3,
		             "Command '%s' takes three parameters: "
		             "the first string, the string to append and "
		             "the name of a system variable to store the result in.",
		             command.c_str());
		
		MenuSystem::getInstance()->setSystemVar(p_action.getParameter(2),
		                                        p_action.getParameter(0) +
		                                        p_action.getParameter(1));
		
		return true;
	}
	else if (command == "clear_treemenu_selection_path")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters.",
		             command.c_str());
		
		MenuSystem::getInstance()->clearSelectionPath();
		
		return true;
	}
	else if (command == "clear_treemenu_selection_path_after_destroy")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters.",
		             command.c_str());
		
		MenuSystem::getInstance()->clearSelectionPathAfterDestroy();
		
		return true;
	}
	else if (command == "set_focus")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "the name of the menu element to set focus to.",
		             command.c_str());
		
		MenuSystem::getInstance()->setFocus(p_action.getParameter(0));
		
		return true;
	}
	else if (command == "memprofile_set_reference_point")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters.",
		             p_action.getCommand().c_str());
		
#ifdef PROFILE_MEMORY
		memory::MemoryProfiler::startMeasure();
#endif
		
		return true;
	}
	else if (command == "memprofile_verify_memory")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "Command '%s' takes one parameter: "
		             "whether to break if errors are detected.",
		             command.c_str());
		
		TT_ERR_CREATE(command);
		bool shouldBreak = str::parseBool(p_action.getParameter(0), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid break Boolean specified: '%s'",
			         p_action.getParameter(0).c_str());
		}
		(void)shouldBreak;
		
#ifdef PROFILE_MEMORY
		memory::MemoryProfiler::stopMeasure(shouldBreak);
#endif
		
		return true;
	}
	else if (command == "debug_output")
	{
		for (int i = 0; i < p_action.getParameterCount(); ++i)
		{
			TT_Printf("MENU DEBUG OUTPUT [%d]: %s\n",
			          i, p_action.getParameter(i).c_str());
		}
		
		return true;
	}
	else if (command == "debug_hex_output")
	{
		for (int i = 0; i < p_action.getParameterCount(); ++i)
		{
			std::wstring wstr(MenuUtils::hexToWideString(p_action.getParameter(i)));
			TT_Printf("MENU DEBUG OUTPUT [%d]: %s\n",
			          i, str::narrow(wstr).c_str());
		}
		
		return true;
	}
	else if (command == "if")
	{
		// Required:
		// 0. variable
		// 1. value to check variable against
		// 2. name of the action set to execute when the value matches the variable's content
		// 3. name of the action set to execute when the value does not match the variable's content
		
		enum
		{
			Param_Variable       = 0,
			Param_Value          = 1,
			Param_TrueActionSet  = 2,
			Param_FalseActionSet = 3,
			
			Param_Required
		};
		
		// Verify parameter count
		TT_ASSERTMSG(p_action.getParameterCount() == Param_Required,
		             "Command '%s' requires %d parameters: "
		             "the variable, a value,"
		             "action set for true, action set for false.",
		             command.c_str(), Param_Required);
		
		bool outputLogicDebug = false;
		
		MenuSystem* sys = MenuSystem::getInstance();
		
		bool result = false;
		if (sys->hasSystemVar(p_action.getParameter(Param_Variable)))
		{
			if (outputLogicDebug)
			{
				TT_Printf("MenuCommandListener::doAction: [if] "
				          "System variable '%s' exists.\n",
				          p_action.getParameter(Param_Variable).c_str());
			}
			
			std::string value = sys->getSystemVar(p_action.getParameter(Param_Variable));
			if (value == p_action.getParameter(Param_Value))
			{
				if (outputLogicDebug)
				{
					TT_Printf("MenuCommandListener::doAction: [if] "
					          "System variable '%s' equals value '%s'.\n",
					          p_action.getParameter(Param_Variable).c_str(),
					          p_action.getParameter(Param_Value).c_str());
				}
				result = true;
			}
			else if (outputLogicDebug)
			{
				TT_Printf("MenuCommandListener::doAction: [if] System variable "
				          "'%s' does not equal value '%s'.\n",
				          p_action.getParameter(Param_Variable).c_str(),
				          p_action.getParameter(Param_Value).c_str());
			}
		}
		else if (outputLogicDebug)
		{
			TT_Printf("MenuCommandListener::doAction: [if] System variable "
			          "'%s' does not exist.\n",
			          p_action.getParameter(Param_Variable).c_str());
		}
		
		if (result)
		{
			if (outputLogicDebug)
			{
				TT_Printf("MenuCommandListener::doAction: [if] Executing "
				          "action set '%s'.\n",
				          p_action.getParameter(Param_TrueActionSet).c_str());
			}
			sys->executeActionSet(p_action.getParameter(Param_TrueActionSet));
		}
		else
		{
			if (outputLogicDebug)
			{
				TT_Printf("MenuCommandListener::doAction: [if] Executing "
				          "action set '%s'.\n",
				          p_action.getParameter(Param_FalseActionSet).c_str());
			}
			sys->executeActionSet(p_action.getParameter(Param_FalseActionSet));
		}
		
		return true;
	}
	else if (command == "if_in_range")
	{
		// Parameters:
		// 0. integer value to check
		// 1. lower bound of the range
		// 2. upper bound of the range
		// 3. name of the action set to execute when the integer is in range
		// 4. name of the action set to execute when the integer is not in range
		enum
		{
			Param_Value          = 0,
			Param_LowerBound     = 1,
			Param_UpperBound     = 2,
			Param_TrueActionSet  = 3,
			Param_FalseActionSet = 4,
			
			Param_Count
		};
		
		// Verify parameter count
		TT_ASSERTMSG(p_action.getParameterCount() == Param_Count,
		             "Command '%s' requires %d parameters: "
		             "integer value to check, lower bound of range, upper bound of range,"
		             "action set for in range, action set for not in range.",
		             command.c_str(), Param_Count);
		
		MenuSystem* sys = MenuSystem::getInstance();
		
		// Get the integers
		TT_ERR_CREATE(command);
		s32 integer = str::parseS32(p_action.getParameter(Param_Value), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid integer value specified: '%s'",
			         p_action.getParameter(Param_Value).c_str());
			errStatus.resetError();
		}
		
		s32 lower_bound = str::parseS32(p_action.getParameter(Param_LowerBound), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid range lower bound integer specified: '%s'",
			         p_action.getParameter(Param_LowerBound).c_str());
			errStatus.resetError();
		}
		
		s32 upper_bound = str::parseS32(p_action.getParameter(Param_UpperBound), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Invalid range upper bound integer specified: '%s'",
			         p_action.getParameter(Param_UpperBound).c_str());
			errStatus.resetError();
		}
		
		// Make sure the range makes sense
		if (lower_bound > upper_bound)
		{
			TT_WARN("Range lower bound (%d) is higher than the upper bound (%d)! Swapping bounds.",
			        lower_bound, upper_bound);
			std::swap(lower_bound, upper_bound);
		}
		
		// Check if the integer is in range
		if (integer >= lower_bound && integer <= upper_bound)
		{
			// Value is in range
			sys->executeActionSet(p_action.getParameter(Param_TrueActionSet));
		}
		else
		{
			// Value is not in range
			sys->executeActionSet(p_action.getParameter(Param_FalseActionSet));
		}
		
		return true;
	}
	
	// Listener cannot handle this command
	return false;
}


std::string MenuCommandListener::getName() const
{
	return "default_menu_commands";
}


//------------------------------------------------------------------------------
// Protected member functions

MenuCommandListener::MenuCommandListener()
{
}

// Namespace end
}
}
}
