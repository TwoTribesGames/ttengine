#if !defined(INC_TT_PLATFORM_TT_ERROR_WIN_H)
#define INC_TT_PLATFORM_TT_ERROR_WIN_H

#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>


namespace tt {
namespace platform {
namespace error {

#if !defined(TT_BUILD_FINAL)

/*! \brief Sets the window handle for the assert dialog to use. */
void setAppWindowHandle(HWND p_window);

#else

// Stub implementation for final builds (tt_error facilities aren't available in final builds)
inline void setAppWindowHandle(HWND) { }

#endif

// Namespace end
}
}
}


#endif  // !defined(INC_TT_PLATFORM_TT_ERROR_WIN_H)
