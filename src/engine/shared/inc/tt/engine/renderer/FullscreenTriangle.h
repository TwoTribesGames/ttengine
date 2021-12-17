#if !defined(INC_TT_ENGINE_RENDERER_FULLSCREENTRIANGLE_H)
#define INC_TT_ENGINE_RENDERER_FULLSCREENTRIANGLE_H


#include <tt/engine/opengl_headers.h>


namespace tt {
namespace engine {
namespace renderer {


class FullscreenTriangle
{
public:
	static void create();
	static void destroy();
	
	/*! \brief Draws a fullscreen triangle to the screen */
	static void draw();
	
private:
	static GLuint ms_vboID;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_FULLSCREENTRIANGLE_H

