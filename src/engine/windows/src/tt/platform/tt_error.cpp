#if !defined(TT_BUILD_FINAL)

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#if defined(_MSC_VER)
	#include <intrin.h>
	#include <crtdbg.h>
#endif
#include <tt/platform/tt_error_win.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/version/Version.h>

#include <tt/system/Time.h>

#include <tt/thread/CriticalSection.h>
#include <tt/thread/Mutex.h>

#if defined(_MSC_VER)
	#include <tt/assert/Assert.h>
	#include <tt/assert/dll.h>
#endif


namespace tt {
namespace platform {
namespace error {

static bool          g_supressAssertsAndWarnings = false;
static bool          g_headLessModeOn            = false;
static PanicCallback g_panicCallback             = 0;
static HWND          g_appWindowHandle           = 0;


// Mute mode
static bool          g_muteAsserts = false;
static u64           g_muteAssertsTimeStamp = 0;
const  u64           g_muteAssertsTimeout   = 2000;	// in milliseconds


void registerPanicCallback(PanicCallback p_fun)
{
	g_panicCallback = p_fun;
}


// Implementation of function declared in tt_error_win.h:
void setAppWindowHandle(HWND p_window)
{
	g_appWindowHandle = p_window;
}


enum TTConstants
{
	TTConstants_CommonBufferSize = 8192
};


void turnHeadlessModeOn()
{
	g_headLessModeOn = true;
}


void supressAssertsAndWarnings()
{
	g_supressAssertsAndWarnings = true;
}


void resetAssertMuteMode()
{
	g_muteAssertsTimeStamp = 0;
	g_muteAsserts = false;
}


#if defined(_MSC_VER)
// helper class to automatically link & unlink the functions we need from tt_assert.dll
// upon program initialization and exit;
struct AssertAutoHelper
{
	typedef int  (WINAPI *dllAssert) (const char*, int, const char*, const char*);
	typedef int  (WINAPI *dllIsAssertInIgnoreList) (const char*, int);
	typedef void (WINAPI *dllSetBuildInfo) (int, int);
	typedef void (WINAPI *dllSetParentWindow) (HWND);
	
	dllAssert          procAssert;           // pointer to the assert function in the dll
	dllIsAssertInIgnoreList procIsAssertInIgnoreList; // points to IsAssertInIgnoreList
	dllSetBuildInfo    procSetBuildInfo;     // pointer to setbuildinfo function in dll
	dllSetParentWindow procSetParentWindow;  // pointer to setParentInfo function in dll
	HMODULE            hTTAssert;            // handle to the dll module
	
	// init
	AssertAutoHelper()
	:
	procAssert(0),
	procIsAssertInIgnoreList(0),
	procSetBuildInfo(0),
	procSetParentWindow(0),
	hTTAssert(0)
	{
		// Attempt to load the dll
#if defined(_WIN64)
		hTTAssert = LoadLibraryA("tt_assert64.dll");
#else
		hTTAssert = LoadLibraryA("tt_assert.dll");
#endif
		
		// If successful
		if (hTTAssert != 0)
		{
			// Get the assert function
			procAssert = (dllAssert)GetProcAddress(hTTAssert, "HandleAssert");
			if (procAssert == 0)
			{
				char buf[256] = { 0 };
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 256, 0);
				MessageBoxA(0, buf, "Error cannot load HandleAssert proc", MB_ICONWARNING | MB_OK);
			}
			
			procSetBuildInfo = (dllSetBuildInfo)GetProcAddress(hTTAssert, "SetBuildInfo");
			if (procSetBuildInfo == 0)
			{
				char buf[256] = { 0 };
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 256, 0);
				MessageBoxA(0, buf, "Error cannot load SetBuildInfo proc", MB_ICONWARNING | MB_OK);
			}
			
			// NOTE: These functions are not required to exist
			procSetParentWindow = (dllSetParentWindow)GetProcAddress(hTTAssert, "SetParentWindow");
			procIsAssertInIgnoreList = (dllIsAssertInIgnoreList)GetProcAddress(hTTAssert, "IsAssertInIgnoreList");
		}
	}
	
	~AssertAutoHelper()
	{
		FreeLibrary(hTTAssert);
		hTTAssert = 0;
		
		procAssert               = 0;
		procIsAssertInIgnoreList = 0;
		procSetBuildInfo         = 0;
		procSetParentWindow      = 0;
	}
};

// single instance of the above 
static AssertAutoHelper g_assertWrapper;
#endif  // defined(_MSC_VER)

static bool gs_isExiting = false; // used to check if another thread already exited.

void TTPanic(const char* p_file, int p_line, const char* p_function, 
             const char* p_fmt, ...)
{
	if (g_supressAssertsAndWarnings || gs_isExiting)
	{
		return;
	}
	
	if (g_muteAsserts)
	{
		// check timestamp
		const u64 timestamp = system::Time::getInstance()->getMilliSeconds();
		if (timestamp - g_muteAssertsTimeStamp > g_muteAssertsTimeout)
		{
			resetAssertMuteMode();
		}
		else
		{
			return;
		}
	}
	
	va_list vlist;
	va_start(vlist, p_fmt);
	
	TT_ErrPrintf("\n------------ PANIC -----------\n\n");
	TT_ErrPrintf("FILE: %s\n",        p_file);
	TT_ErrPrintf("LINE: %d\n",        p_line);
	TT_ErrPrintf("FUNCTION: %s\n",    p_function);
	TT_ErrPrintf("REVISION: %d.%d\n", version::getClientRevisionNumber(), version::getLibRevisionNumber());
	
	TT_ErrPrintf("\n");
	
	TT_ErrPrintf("MESSAGE:\n");
	TT_ErrVPrintf(p_fmt, vlist);
	
	TT_ErrPrintf("\n");
	
	va_end(vlist);
	
	static tt::thread::Mutex mutex;
	tt::thread::CriticalSection section(&mutex);
	
	
	// Check if headless mode was set. (No assert popup in that case.).
	if (g_headLessModeOn)
	{
#if defined(TT_BUILD_DEV)
		_CrtSetDbgFlag(_crtDbgFlag & ~_CRTDBG_LEAK_CHECK_DF);
#endif
		gs_isExiting = true;
		
		if (IsDebuggerPresent())
		{
			// Break here when debugger was found.
			__debugbreak();
		}
		
		exit(1);
		return;
	}
	
	if (g_panicCallback != 0
	    &&
	    (g_assertWrapper.procIsAssertInIgnoreList == 0 ||
	     g_assertWrapper.procIsAssertInIgnoreList(p_file, p_line) == 0)
	    )
	{
		g_panicCallback();
	}
	
	static char __declspec(thread) TTCommonBuffer[TTConstants_CommonBufferSize] = { 0 };
	
	//----------------------------------------------------------------------------------------------
	// Full-featured Windows assert dialog implementation of panic handling
	
	va_start(vlist, p_fmt);
	(void)vsnprintf_s(TTCommonBuffer, TTConstants_CommonBufferSize,
	                  TTConstants_CommonBufferSize, p_fmt, vlist);
	va_end(vlist);
	
	TTCommonBuffer[TTConstants_CommonBufferSize - 1] = 0;
	
	int returnVal = 0;
	
	// do we have a pointer to the tt_assert dll call?
	if (g_assertWrapper.procAssert != 0)
	{
		// first set build info
		g_assertWrapper.procSetBuildInfo(version::getLibRevisionNumber(),
		                                 version::getClientRevisionNumber());
		
		if (g_assertWrapper.procSetParentWindow != 0)
		{
			g_assertWrapper.procSetParentWindow(g_appWindowHandle);
		}
		
		// handle assert
		returnVal = g_assertWrapper.procAssert(p_file, p_line, p_function, TTCommonBuffer);
	}
	else
	{
		// DLL assert failed, use the standard dialog which returns 1 for debug,
		// the crt library will automatically terminate the program if abort is selected so
		// the only other possible return is ignore
		
#if defined(_DEBUG)  // _CrtDbgReport is only available in the debug C runtime libraries
		if (_CrtDbgReport(_CRT_ASSERT, p_file, p_line, p_function, TTCommonBuffer) == 1)
		{
			//return ID_ASSERT_DEBUG;
		}
#endif
		
		returnVal = ID_ASSERT_IGNORE;
	}
	
	// was exit selected?
	switch (returnVal)
	{
	case ID_ASSERT_IGNORE_MUTE:
		g_muteAsserts = true;
		g_muteAssertsTimeStamp = system::Time::getInstance()->getMilliSeconds();
		break;
		
	case ID_ASSERT_DEBUG:
		__debugbreak();
		break;
		
	case ID_ASSERT_EXIT:
#if defined(TT_BUILD_DEV)
		// Turn off leak checks flag when exiting from an assert.
		// (Will always result in (false) leaks being detected.)
		_CrtSetDbgFlag(_crtDbgFlag & ~_CRTDBG_LEAK_CHECK_DF);
#endif
		
		// exit - could do something nicer here?
		gs_isExiting = true;
		exit(1);
		return;
	}
}


void TTWarning(const char* p_file, int p_line, const char* p_function, const char* p_fmt, ...)
{
	if (g_supressAssertsAndWarnings)
	{
		return;
	}
	
	va_list vlist;
	va_start(vlist, p_fmt);
	
#if defined(TT_PLATFORM_WIN)
	HANDLE promptHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(promptHandle, &info);
	SetConsoleTextAttribute(promptHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
	
	TT_ErrPrintf("WARNING file: %s:%d\n%s\n", p_file, p_line, p_function);
	// FIXME: Change to assert dialog box
	TT_ErrVPrintf(p_fmt, vlist);
	TT_ErrPrintf("\n");
	
#if defined(TT_PLATFORM_WIN)
	SetConsoleTextAttribute(promptHandle, info.wAttributes);
#endif
	
	va_end(vlist);
}

// Namespace end
}
}
}


#endif  // !defined(TT_BUILD_FINAL)
