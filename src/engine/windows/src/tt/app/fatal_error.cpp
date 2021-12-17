#define NOMINMAX
#include <windows.h>
#include <tt/engine/renderer/DXUT/DXUT.h>

#include <tt/app/fatal_error.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace app {

void reportFatalError(const std::string& p_message)
{
	// Show fatal error
#if defined(TT_BUILD_FINAL)
	if (DXUTIsWindowed() == false)
	{
		// Switch out of fullscreen to properly display dialog box
		DXUTToggleFullScreen();
	}
	MessageBoxA(DXUTGetHWND(), p_message.c_str(), "Fatal Error", MB_OK|MB_ICONERROR);
#else
	TT_PANIC("%s", p_message.c_str());
#endif
	
	exit(1);
}

// Namespace end
}
}
