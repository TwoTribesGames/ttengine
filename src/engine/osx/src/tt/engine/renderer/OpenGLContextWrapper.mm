
#include <tt/app/TTOpenGLContext.h>

#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/OpenGLContextWrapper.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace renderer {


OpenGLContextWrapper::OpenGLContextWrapper(void* p_context)
:
m_context(p_context)
{
	TT_NULL_ASSERT(m_context);
}


OpenGLContextWrapper::~OpenGLContextWrapper()
{
}


bool OpenGLContextWrapper::init()
{
    // Outpt Renderer Information
    TT_Printf("OpenGL Information:\n\tVendor: %s\n\tRenderer: %s\n\tVersion: %s\n",
            glGetString(GL_VENDOR),
            glGetString(GL_RENDERER),
            glGetString(GL_VERSION));

	return true;
}


void OpenGLContextWrapper::setActive()
{
	[(TTOpenGLContext*)m_context makeCurrentContext];
}


void OpenGLContextWrapper::swapBuffers()
{
	[(TTOpenGLContext*)m_context flushBuffer];
}


math::Point2 OpenGLContextWrapper::getBackBufferSize() const
{
	return [(TTOpenGLContext*)m_context getScreenSize];
}


// Namespace end
}
}
}

