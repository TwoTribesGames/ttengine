
#include <SDL2/SDL.h>

#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/OpenGLContextWrapper.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace renderer {


OpenGLContextWrapper::OpenGLContextWrapper(SDL_Window* p_window, SDL_GLContext p_context)
:
m_window(p_window),
m_context(p_context)
{
}


OpenGLContextWrapper::~OpenGLContextWrapper()
{
	SDL_GL_DeleteContext(m_context);
}


bool OpenGLContextWrapper::init()
{
	if (m_context == 0) return false;

		// Outpt Renderer Information
	TT_Printf("OpenGL Information:\n\tVendor: %s\n\tRenderer: %s\n\tVersion: %s\n",
			glGetString(GL_VENDOR),
			glGetString(GL_RENDERER),
			glGetString(GL_VERSION));

	return true;
}


void OpenGLContextWrapper::setActive()
{
	int result = SDL_GL_MakeCurrent(m_window, m_context);

	if (result < 0)
	{
		TT_PANIC("Making OpenGL Context current failed: %s", SDL_GetError());
	}
}


void OpenGLContextWrapper::swapBuffers()
{
	SDL_GL_SwapWindow(m_window);
}


math::Point2 OpenGLContextWrapper::getBackBufferSize() const
{
	int x, y;
	SDL_GL_GetDrawableSize(m_window, &x, &y);
	
	return 	math::Point2(x, y);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private


// Namespace end
}
}
}

