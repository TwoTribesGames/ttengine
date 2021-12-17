#if !defined(INC_TT_PLATFORM_TT_ERROR_SDL2_H)
#define INC_TT_PLATFORM_TT_ERROR_SDL2_H

struct SDLWindow;

namespace tt {
namespace platform {
namespace error {

/*! \brief Sets the window handle for the assert dialog to use. */
void setAppWindowHandle(SDL_Window* p_window);

// Namespace end
}
}
}


#endif  // !defined(INC_TT_PLATFORM_TT_ERROR_SDL2_H)
