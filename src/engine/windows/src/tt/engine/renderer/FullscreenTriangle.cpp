
#include <tt/engine/renderer/FullscreenTriangle.h>

#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/TriangleBuffer.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {


IDirect3DVertexBuffer9* FullscreenTriangle::ms_vertexBuffer = 0;
static const u32 triangleFVF = D3DFVF_DIFFUSE|D3DFVF_XYZ|D3DFVF_TEX1;


void FullscreenTriangle::draw()
{
	TT_NULL_ASSERT(ms_vertexBuffer);

	Renderer::getInstance()->setVertexType(VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0);

	checkD3DSucceeded( getRenderDevice()->SetStreamSource(0, ms_vertexBuffer, 0, sizeof(BufferVtx)) );
	checkD3DSucceeded( getRenderDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1) );
}


void FullscreenTriangle::deviceCreated()
{
	if (ms_vertexBuffer != 0)
	{
		return;
	}
	
	Renderer::getInstance()->checkFromRenderThread();
	
	// Create geometry for fullscreen triangle
	// See http://www.altdevblogaday.com/2011/08/08/interesting-vertex-shader-trick/
	
	BatchTriangleUV<1> fullscreenTriangle;

	fullscreenTriangle.a.setPosition(-1, 1, 0);
	fullscreenTriangle.b.setPosition( 3, 1, 0);
	fullscreenTriangle.c.setPosition(-1,-3, 0);

	fullscreenTriangle.a.setTexCoord(0, 0);
	fullscreenTriangle.b.setTexCoord(2, 0);
	fullscreenTriangle.c.setTexCoord(0, 2);

	// Create a vertex buffer
	checkD3DSucceeded( getRenderDevice(true)->CreateVertexBuffer(sizeof(fullscreenTriangle),
		D3DUSAGE_WRITEONLY, triangleFVF, D3DPOOL_MANAGED, &ms_vertexBuffer, 0) );

	// Fill the vertex buffer
	{
		void* vertices(0);
		checkD3DSucceeded( ms_vertexBuffer->Lock(0, sizeof(fullscreenTriangle), &vertices, D3DLOCK_NOSYSLOCK) );

		memcpy(vertices, &fullscreenTriangle, sizeof(fullscreenTriangle));
		
		checkD3DSucceeded( ms_vertexBuffer->Unlock() );
	}
}


void FullscreenTriangle::deviceDestroyed()
{
	safeRelease(ms_vertexBuffer);
}


// Namespace end
}
}
}
