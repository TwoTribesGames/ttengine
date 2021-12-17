#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <tt/app/StartupStateWin.h>
#include <tt/fs/WindowsFileSystem.h>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

StartupStateWin::StartupStateWin(s32 p_clientRevision, s32 p_libRevision,
                                 const std::string& p_appName)
:
StartupState(p_clientRevision, p_libRevision),
m_stateRegKeyPath("Software\\Two Tribes\\Games\\" + fs::WindowsFileSystem::sanitizeFilename(p_appName))
{
	// Trigger base initialization
	init();
}


StartupStateWin::~StartupStateWin()
{
}


bool StartupStateWin::writeStateFileWithData(const u8* p_fileData, s32 p_dataLen)
{
	HKEY key = 0;
	if (RegCreateKeyExA(HKEY_CURRENT_USER, m_stateRegKeyPath.c_str(), 0, 0, REG_OPTION_NON_VOLATILE,
	                    KEY_WRITE, 0, &key, 0) != ERROR_SUCCESS)
	{
		char msg[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, msg, 256, 0);
		TT_PANIC("Creating or opening registry key 'HKEY_CURRENT_USER\\%s' failed: '%s'",
		         m_stateRegKeyPath.c_str(), msg);
		return false;
	}
	
	const bool saveOk = RegSetValueExA(key, "startupstate", 0, REG_BINARY,
	                                   reinterpret_cast<const BYTE*>(p_fileData),
	                                   static_cast<DWORD>(p_dataLen)) == ERROR_SUCCESS;
	
	RegCloseKey(key);
	
	return saveOk;
}


bool StartupStateWin::readStateFile(u8* p_fileData, s32 p_expectedLen)
{
	HKEY key = 0;
	LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, m_stateRegKeyPath.c_str(),
	                            REG_OPTION_NON_VOLATILE, KEY_READ, &key);
	if (result != ERROR_SUCCESS)
	{
		/*
		char msg[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, msg, 256, 0);
		TT_PANIC("Opening registry key 'HKEY_CURRENT_USER\\%s' failed: '%s'",
		         m_stateRegKeyPath.c_str(), msg);
		*/
		return false;
	}
	
	DWORD valueType = 0;
	DWORD dataSize  = p_expectedLen;
	
	const bool readOk =
		RegQueryValueExA(key, "startupstate", 0, &valueType, reinterpret_cast<LPBYTE>(p_fileData),
		                 &dataSize) == ERROR_SUCCESS &&
		valueType == REG_BINARY &&
		static_cast<s32>(dataSize) == p_expectedLen;
	
	RegCloseKey(key);
	return readOk;
}

// Namespace end
}
}
