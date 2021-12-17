#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShellAPI.h>
#include <ShlObj.h>

#include <tt/engine/renderer/DXUT/DXUT.h>
#include <tt/mem/util.h>
#include <tt/str/common.h>
#include <tt/str/format.h>
#include <tt/str/manip.h>
#include <tt/system/utils.h>
#include <tt/platform/tt_error.h>

#if defined(_WIN64)
#pragma warning( disable : 4090 )
#endif

namespace tt {
namespace system {

static inline std::string getErrorMessage(DWORD p_errorCode)
{
	char msg[512] = { 0 };
	::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, p_errorCode, 0, msg, 512, 0);
	return msg;
}


static bool shellExecute(const std::wstring& p_item,
	                     const std::wstring& p_action,
	                     const std::wstring& p_parameters,
	                     const std::wstring& p_workingDir,
	                     INT p_showCommand)
{
	HINSTANCE resultValue = ShellExecute(NULL,
		p_action.c_str(), p_item.c_str(), p_parameters.c_str(), p_workingDir.c_str(), p_showCommand);

	// Windows is retarded
	int result = *reinterpret_cast<int*>(&resultValue);

	if(result > 32)
	{
		return true;
	}

#ifndef TT_BUILD_FINAL
	// Error occured
	std::string errorMsg;

	switch(result)
	{
	case ERROR_FILE_NOT_FOUND:   errorMsg = "The specified file was not found."; break;
	case ERROR_PATH_NOT_FOUND:   errorMsg = "The specified path was not found."; break;
	case ERROR_BAD_FORMAT:       errorMsg = "The .exe file is invalid (non-Win32 .exe or error in .exe image)."; break;
	case SE_ERR_ACCESSDENIED:    errorMsg = "The operating system denied access to the specified file."; break;
	case SE_ERR_ASSOCINCOMPLETE: errorMsg = "The file name association is incomplete or invalid."; break;
	case SE_ERR_DDEBUSY:         errorMsg = "The DDE transaction could not be completed because other DDE transactions were being processed."; break;
	case SE_ERR_DDEFAIL:         errorMsg = "The DDE transaction failed."; break;
	case SE_ERR_DDETIMEOUT:      errorMsg = "The DDE transaction could not be completed because the request timed out."; break;
	case SE_ERR_DLLNOTFOUND:     errorMsg = "The specified DLL was not found."; break;
	case SE_ERR_NOASSOC:         errorMsg = "There is no application associated with the given file name extension. This error will also be returned if you attempt to print a file that is not printable."; break;
	case SE_ERR_OOM:             errorMsg = "There was not enough memory to complete the operation."; break;
	case SE_ERR_SHARE:           errorMsg = "A sharing violation occurred."; break;
	default: errorMsg = "Unknown error occured.";
	}

	TT_PANIC("Failed to execute ShellExecute: '%s'", errorMsg.c_str());
#endif
	return false;
}


bool openWithDefaultApplication(const std::string& p_item)
{
	return shellExecute(tt::str::widen(p_item), L"open", L"", L"", SW_SHOWDEFAULT);
}


bool editWithDefaultApplication(const std::string& p_item)
{
	return shellExecute(tt::str::widen(p_item), L"edit", L"", L"", SW_SHOWDEFAULT);
}


bool showFileInFileNavigator(const std::string& p_item)
{
	ITEMIDLIST* itemIdList = ILCreateFromPathA(p_item.c_str());
	if (itemIdList == 0)
	{
		TT_PANIC("Could not create shell item ID list from path '%s'.", p_item.c_str());
		return false;
	}
	
	HRESULT resultValue = SHOpenFolderAndSelectItems(itemIdList, 0, 0, 0);
	
	TT_ASSERTMSG(resultValue == S_OK,
	             "Showing item '%s' in Explorer failed with error code %d.",
	             p_item.c_str(), resultValue);
	
	ILFree(itemIdList);
	
	return resultValue == S_OK;
}


bool setSystemClipboardText(const str::Strings& p_lines)
{
	if (::OpenClipboard(DXUTGetHWND()) == FALSE)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not open the system clipboard.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		return false;
	}
	
	if (::EmptyClipboard() == FALSE)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not empty the system clipboard.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		::CloseClipboard();
		return false;
	}
	
	const std::string clipboardText(str::implode(p_lines, "\r\n"));
	
	HGLOBAL globalData = ::GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(clipboardText.length() + 1));
	if (globalData == 0)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not allocate global memory for the new system clipboard text.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		::CloseClipboard();
		return false;
	}
	
	void* data = ::GlobalLock(globalData);
	mem::copy8(data, clipboardText.c_str(), static_cast<mem::size_type>(clipboardText.length() + 1));
	::GlobalUnlock(globalData);
	
	// Place the data on the clipboard
	if (::SetClipboardData(CF_TEXT, globalData) == 0)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not set the new system clipboard text.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		::GlobalFree(globalData);
	}
	
	if (::CloseClipboard() == FALSE)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not close the system clipboard.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		return false;
	}
	
	return true;
}


bool getSystemClipboardText(str::Strings* p_lines_OUT)
{
	TT_NULL_ASSERT(p_lines_OUT);
	if (p_lines_OUT == 0)
	{
		return false;
	}
	
	if (::IsClipboardFormatAvailable(CF_TEXT) == FALSE)
	{
		// Clipboard does not contain text: return 0 lines of text
		p_lines_OUT->clear();
		return true;
	}
	
	if (::OpenClipboard(DXUTGetHWND()) == FALSE)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not open the system clipboard.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		return false;
	}
	
	HGLOBAL globalData = ::GetClipboardData(CF_TEXT);
	if (globalData == 0)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not get the system clipboard data.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		::CloseClipboard();
		return false;
	}
	
	const char* rawText = reinterpret_cast<const char*>(::GlobalLock(globalData));
	if (rawText == 0)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not lock the system clipboard data.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		::CloseClipboard();
		return false;
	}
	
	std::string text(rawText);
	str::replace(text, "\r\n", "\n");
	*p_lines_OUT = str::explode(text, "\n", true);
	
	if (::GlobalUnlock(globalData) == FALSE)
	{
		const DWORD errCode = ::GetLastError();
		if (errCode != NO_ERROR)
		{
			TT_PANIC("Could not unlock the system clipboard data.\nError 0x%08X: %s",
			         errCode, getErrorMessage(errCode).c_str());
		}
	}
	
	if (::CloseClipboard() == FALSE)
	{
		const DWORD errCode = ::GetLastError();
		TT_PANIC("Could not close the system clipboard.\nError 0x%08X: %s",
		         errCode, getErrorMessage(errCode).c_str());
		return false;
	}
	
	return true;
}


std::string getDesktopPath()
{
	char path[MAX_PATH] = { 0 };
	BOOL pathSuccess = SHGetSpecialFolderPathA(0, path, CSIDL_DESKTOPDIRECTORY, FALSE);
	
	TT_ASSERTMSG(pathSuccess, "Could not retrieve desktop directory path.");
	
	std::string desktopPath = path;
	if (desktopPath.empty() == false && *desktopPath.rbegin() != '\\')
	{
		desktopPath += "\\";
	}
	return desktopPath;
}


// Namespace end
}
}
