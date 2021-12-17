#include <SDL2/SDL_video.h>
#include <tt/math/Point2.h>


// Forward declaration
struct SDL_Window;

namespace tt {
namespace engine {
namespace renderer {


class OpenGLContextWrapper
{
public:
	OpenGLContextWrapper(SDL_Window* p_window, SDL_GLContext p_context);
	~OpenGLContextWrapper();
	
	bool init();
	
	void setActive();
	void swapBuffers();
	
	math::Point2 getBackBufferSize() const;
	
private:
	void mapExtensions();

	SDL_Window*   m_window;
	SDL_GLContext m_context;
};


// Namespace end
}
}
}

