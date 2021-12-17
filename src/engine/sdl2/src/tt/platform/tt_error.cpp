#include <stdarg.h>
#include <stdio.h>
#include <sstream>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/version/Version.h>

#include <tt/system/Time.h>

#include <tt/thread/CriticalSection.h>
#include <tt/thread/Mutex.h>

#include <SDL2/SDL.h>

namespace tt {
namespace platform {
namespace error {

static SDL_Window*   g_appWindowHandle           = 0;

// Implementation of function declared in tt_error_sdl2.h:
void setAppWindowHandle(SDL_Window* p_window)
{
	g_appWindowHandle = p_window;
}

#if !defined(TT_BUILD_FINAL)

static bool          g_supressAssertsAndWarnings = false;
static bool          g_headLessModeOn            = false;
static PanicCallback g_panicCallback             = 0;

// Mute mode
static bool          g_muteAsserts = false;
static u64           g_muteAssertsTimeStamp = 0;
const  u64           g_muteAssertsTimeout   = 2000;	// in milliseconds


void registerPanicCallback(PanicCallback p_fun)
{
	g_panicCallback = p_fun;
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
		gs_isExiting = true;

		SDL_TriggerBreakpoint();

		exit(1);
		return;
	}

	if (g_panicCallback != 0)
	{
		g_panicCallback();
	}

	static char TTCommonBuffer[TTConstants_CommonBufferSize] = { 0 };

	//----------------------------------------------------------------------------------------------
	// Full-featured assert dialog implementation of panic handling

	va_start(vlist, p_fmt);
	(void)vsnprintf(TTCommonBuffer, TTConstants_CommonBufferSize, p_fmt, vlist);
	va_end(vlist);

	TTCommonBuffer[TTConstants_CommonBufferSize - 1] = 0;

	std::ostringstream informativeText;
	informativeText << TTCommonBuffer
	                << "\n\n\nFile: " << p_file
	                << "\nLine: " << p_line
	                << "\n\nFunction:\n" << p_function
	                << "\n\nRevision:\n"
	                << tt::version::getClientRevisionNumber() << "."
	                << tt::version::getLibRevisionNumber();
	std::string msg = informativeText.str();
	
	SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Ignore" },
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Exit" }
	};
	SDL_MessageBoxData msbBox = {
		SDL_MESSAGEBOX_ERROR,
		g_appWindowHandle,
		"TT Panic",
		msg.c_str(),
		2,
		(SDL_MessageBoxButtonData*)&buttons,
		NULL
	};
	int button = 0;
	SDL_ShowMessageBox(&msbBox, &button);
	if (button == 0) {
		gs_isExiting = true;
		exit(1);
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
	
	TT_ErrPrintf("WARNING file: %s:%d\n%s\n", p_file, p_line, p_function);
	// FIXME: Change to assert dialog box
	TT_ErrVPrintf(p_fmt, vlist);
	TT_ErrPrintf("\n");

	va_end(vlist);
}

#endif  // defined(TT_FINAL_BUILD)

// Namespace end
}
}

namespace app {

// Helper function to present the user with an error message and exit in final builds
// (simply triggers a panic when called in non-final builds)
void reportFatalError(const std::string& p_message)
{
#if defined(TT_BUILD_FINAL)
	SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Exit" }
	};
	SDL_MessageBoxData msg = {
		SDL_MESSAGEBOX_ERROR,
		tt::platform::error::g_appWindowHandle,
		"Fatal Error",
		p_message.c_str(),
		1,
		(SDL_MessageBoxButtonData*)&buttons,
		NULL
	};
	SDL_ShowMessageBox(&msg, 0);
	
	exit(1); // immediately exit
	
#else
	TT_PANIC("%s", p_message.c_str());
#endif
}

// Namespace end
}
}
