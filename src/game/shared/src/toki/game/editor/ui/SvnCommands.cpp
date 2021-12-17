#if defined(TT_PLATFORM_WIN)
#include <Windows.h>
#endif

#include <tt/platform/tt_error.h>

#include <toki/game/editor/ui/SvnCommands.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

SvnCommandsPtr SvnCommands::create(Editor* p_editor, Gwen::Controls::MenuItem* p_targetMenu)
{
	TT_NULL_ASSERT(p_editor);
	TT_NULL_ASSERT(p_targetMenu);
	if (p_editor == 0 || p_targetMenu == 0)
	{
		return SvnCommandsPtr();
	}
	
	return SvnCommandsPtr(new SvnCommands(p_editor, p_targetMenu));
}


//--------------------------------------------------------------------------------------------------
// Private member functions

SvnCommands::SvnCommands(Editor* p_editor, Gwen::Controls::MenuItem* p_targetMenu)
{
#if EDITOR_SUPPORTS_SUBVERSION
	
	m_editor = p_editor;
	
	if (AppGlobal::isInDeveloperMode() == false)
	{
		// Subversion support is only available in developer mode
		return;
	}
	
	// Create a sub-menu for the Subversion commands
	p_targetMenu->GetMenu()->AddDivider();
	Gwen::Controls::MenuItem* subMenuItem = p_targetMenu->GetMenu()->AddItem(L"Subversion");
	Gwen::Controls::Menu*     subMenu     = subMenuItem->GetMenu();
	
	// Add a menu item for each command
	using namespace hotkey;
	const Modifiers ctrlShift   (M(Modifier_Ctrl, Modifier_Shift));
	const Modifiers ctrlShiftAlt(M(Modifier_Ctrl, Modifier_Shift, Modifier_Alt));
	
	addMenuItem(subMenu, "Lock...",     &SvnCommands::onCommandLock,    p_editor, tt::input::Key_L, ctrlShift);
	addMenuItem(subMenu, "Unlock...",   &SvnCommands::onCommandUnlock,  p_editor, tt::input::Key_L, ctrlShiftAlt);
	addMenuItem(subMenu, "Commit...",   &SvnCommands::onCommandCommit,  p_editor, tt::input::Key_C, ctrlShift);
	addMenuItem(subMenu, "Update...",   &SvnCommands::onCommandUpdate,  p_editor, tt::input::Key_U, ctrlShift);
	addMenuItem(subMenu, "Show Log...", &SvnCommands::onCommandShowLog, p_editor, tt::input::Key_G, ctrlShift);
	
#else
	(void)p_editor;
	(void)p_targetMenu;
#endif
}


#if EDITOR_SUPPORTS_SUBVERSION

void SvnCommands::addMenuItem(Gwen::Controls::Menu* p_targetMenu,
                              const std::string&    p_displayName,
                              Action                p_actionFunc)
{
	Gwen::Controls::MenuItem* item = p_targetMenu->AddItem(p_displayName, "", "");
	item->onMenuItemSelected.Add(this, p_actionFunc);
}


void SvnCommands::addMenuItem(Gwen::Controls::Menu*    p_targetMenu,
                              const std::string&       p_displayName,
                              Action                   p_actionFunc,
                              Editor*                  p_editor,
                              tt::input::Key           p_actionKey,
                              const hotkey::Modifiers& p_modifiers)
{
	const std::string hotkeyDisplayName(hotkey::getDisplayName(p_actionKey, p_modifiers));
	Gwen::Controls::MenuItem* item = p_targetMenu->AddItem(p_displayName, "", hotkeyDisplayName);
	item->onMenuItemSelected.Add(this, p_actionFunc);
	p_editor->getHotKeyMgr().addHotKey(p_actionKey, p_modifiers,
			hotkey::Handler<SvnCommands>::create(this, p_actionFunc));
}


void SvnCommands::runTortoiseProc(const std::string& p_command, const std::string& p_extraArguments)
{
	const game::StartInfo& startInfo(m_editor->getCurrentLevelInfo());
	if (startInfo.isUserLevel())
	{
		// Cannot perform Subversion operations on user levels (assumed not to be versioned)
		m_editor->showGenericDialog(L"Subversion Command Unavailable",
		                            L"Cannot perform Subversion commands on user levels.",
		                            DialogButtons_OK, true);
		return;
	}
	
	// FIXME: Also add a check to see if the file is versioned at all? How to do this?
	
	// Find the path to TortoiseProc.exe
	std::string tortoiseProcPath;
	{
		HKEY tortoiseKey;
		LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\TortoiseSVN",
		                            REG_OPTION_NON_VOLATILE, KEY_READ, &tortoiseKey);
		if (result != ERROR_SUCCESS)
		{
			TT_PANIC("Could not open the registry key for TortoiseSVN.\n"
			         "Subversion operations only work if TortoiseSVN is installed.");
			return;
		}
		
		char  path[512] = { 0 };
		DWORD size      = 512;
		DWORD type;
		
		if (RegQueryValueExA(tortoiseKey, "ProcPath", 0, &type,
				reinterpret_cast<LPBYTE>(path), &size) != ERROR_SUCCESS)
		{
			TT_PANIC("Could not find the TortoiseSVN install path in the registry.\n"
			         "Subversion operations only work if TortoiseSVN is installed.");
			RegCloseKey(tortoiseKey);
			return;
		}
		
		tortoiseProcPath = path;
		
		RegCloseKey(tortoiseKey);
	}
	
	// Start TortoiseProc with a request to lock the level file (it will show a GUI as necessary)
	const std::string levelsSourceDir(getLevelsSourceDir());
	const std::string levelFilename(levelsSourceDir + startInfo.getLevelName() + ".ttlvl");
	std::string cmdLine(tortoiseProcPath);
	cmdLine += " /command:" + p_command + " /path:\"" + levelFilename + "\"";
	if (p_extraArguments.empty() == false) cmdLine += " " + p_extraArguments;
	TT_Printf("Running:\n%s\n\n", cmdLine.c_str());
	
	STARTUPINFOA        startupInfo = { 0 };
	PROCESS_INFORMATION processInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	
	std::size_t bufSize    = cmdLine.length() + 1;
	char*       cmdLineStr = new char[bufSize];
	strcpy_s(cmdLineStr, bufSize, cmdLine.c_str());
	BOOL result = CreateProcessA(0, cmdLineStr, 0, 0, FALSE, 0, 0,
	                             levelsSourceDir.c_str(), &startupInfo, &processInfo);
	delete[] cmdLineStr;
	
	if (result == FALSE)
	{
		const DWORD errCode     = GetLastError();
		char        reason[512] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, errCode, 0, reason, 512, 0);
		
		TT_PANIC("Could not start TortoiseSVN to perform Subversion command '%s'.\n"
		         "Error (code 0x%08X): %s\nLevel file: %s",
		         p_command.c_str(), errCode, reason, levelFilename.c_str());
	}
}


// UI handlers

void SvnCommands::onCommandLock()
{
	runTortoiseProc("lock", "/closeonend:1");
}


void SvnCommands::onCommandUnlock()
{
	runTortoiseProc("unlock", "/closeonend:1");
}


void SvnCommands::onCommandCommit()
{
	runTortoiseProc("commit");
}


void SvnCommands::onCommandShowLog()
{
	runTortoiseProc("log");
}


void SvnCommands::onCommandUpdate()
{
	runTortoiseProc("update");
}

#endif  // EDITOR_SUPPORTS_SUBVERSION

// Namespace end
}
}
}
}
