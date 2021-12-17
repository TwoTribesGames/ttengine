
#include <tt/engine/renderer/FullscreenTriangle.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/engine/renderer/VertexBuffer.h>

#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {
	
	
#define BUFFER_OFFSET(bytes) ((GLubyte*)0 + (bytes))
	
GLuint FullscreenTriangle::ms_vboID = 0;

static const u32 elementsPerVertex = 8;
static const GLfloat vertices[][elementsPerVertex] = {
	{0, 0,   1, 1, 1,  -1, -1, 0},
	{2, 0,   1, 1, 1,   3, -1, 0},
	{0, 2,   1, 1, 1,  -1,  3, 0}
};

	
void FullscreenTriangle::create()
{
	glGenBuffers(1, &ms_vboID);
	glBindBuffer(GL_ARRAY_BUFFER, ms_vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
	
	
void FullscreenTriangle::destroy()
{
	glDeleteBuffers(1, &ms_vboID);
}

	
void FullscreenTriangle::draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, ms_vboID);
	
	// Enable correct client states
	// Prevent Fixed Function shader from becoming active!
	Renderer* renderer = Renderer::getInstance();
	renderer->setVertexType(VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0, false);

	renderer->stateCache()->apply();

	glInterleavedArrays(GL_T2F_C3F_V3F, 0, BUFFER_OFFSET(0));
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	TT_CHECK_OPENGL_ERROR();
}


// Namespace end
}
}
}
