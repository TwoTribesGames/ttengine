#include <tt/math/Point2.h>


namespace tt {
namespace engine {
namespace renderer {


class OpenGLContextWrapper
{
public:
	OpenGLContextWrapper(void* p_context);
	~OpenGLContextWrapper();
	
	bool init();
	
	void setActive();
	void swapBuffers();
	
	math::Point2 getBackBufferSize() const;
	
private:
	void* m_context;
};


// Namespace end
}
}
}

