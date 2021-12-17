#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/PrimitiveCollectionBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TriangleBuffer.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {


TriangleBuffer::TriangleBuffer(s32               p_maxTriangleCount,
                               s32               p_uvSize,
                               const TexturePtr& p_texture,
                               BatchFlagTriangle p_flag)
:
m_capacity(p_maxTriangleCount),
m_triangles(3, m_capacity, p_uvSize),
m_useVtxColor(p_flag == BatchFlagTriangle_UseVertexColor),
m_vertexType(VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0),
m_texture(p_texture),
m_vertexBuffer(0)
{
	if (p_uvSize > 1) m_vertexType |= VertexBuffer::Property_Texture1;
	if (p_uvSize > 2) m_vertexType |= VertexBuffer::Property_Texture2;
	if (p_uvSize > 3) m_vertexType |= VertexBuffer::Property_Texture3;
	if (p_uvSize > 4)
	{
		TT_PANIC("Too many UV sets (%d), max 4 supported.", p_uvSize);
	}
	
	if(m_capacity > maxBatchSize)
	{
		TT_PANIC("Requested size (%d) exceeds maximum capacity (%d)!", m_capacity, maxBatchSize);
		m_capacity = maxBatchSize;
	}
	
	D3DResourceRegistry::registerResource(this);
}


TriangleBuffer::~TriangleBuffer()
{
	D3DResourceRegistry::unregisterResource(this);
	Renderer::addToDeathRow(m_vertexBuffer);
}


void TriangleBuffer::applyChanges()
{
	if(m_triangles.isEmpty()) return;
	
	if(m_vertexBuffer == 0)
	{
		// Must return after reset, because deviceReset() will call update() if successful
		deviceReset();
		return;
	}
	
	// Lock vertex buffer -> discard all data in it
	DWORD lockFlags = 0;
	lockFlags |= D3DLOCK_DISCARD;
	void* vertices = 0;
	if (checkD3DSucceeded(m_vertexBuffer->Lock(0, m_triangles.getRawVertexDataSize(), &vertices, lockFlags)))
	{
		// NOTE: Swapping colors here is a performance bottleneck, therefore the client must provide
		//       the color channels already swapped.
		{
			// Copy all to vertex buffer
			// NOTE: We assume color channels are the same, making it not important to swap
			mem::copy8(vertices, m_triangles.getRawVertexData(), m_triangles.getRawVertexDataSize());
		}
		
		checkD3DSucceeded(m_vertexBuffer->Unlock());
	}
}


void TriangleBuffer::render() const
{
	if(m_triangles.isEmpty()) return;
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	if(m_vertexBuffer == 0)
	{
		TT_PANIC("Vertex Buffer does not exist, call applyChanges() first.");
		return;
	}
	
	Renderer* renderer = Renderer::getInstance();
	
	// Apply current transform
	MatrixStack::getInstance()->updateWorldMatrix();
	
	// Activate the batch texture
	renderer->setTexture(m_texture);
	renderer->setVertexType(m_vertexType);
	
	checkD3DSucceeded( device->SetStreamSource(0,                                // Stream ID
	                                           m_vertexBuffer,                   // Vertex Buffer
	                                           0,                                // Offset
	                                           m_triangles.getVertexSize()) );   // Stride
	
	checkD3DSucceeded( device->DrawPrimitive(D3DPT_TRIANGLELIST,                 // Draw triangles
	                                         0,                                  // Start at vertex 0
	                                         m_triangles.getPrimitiveCount()) ); // Triangles to draw
}


void TriangleBuffer::setTexture(const TexturePtr& p_texture)
{
	m_texture = p_texture;
}


void TriangleBuffer::deviceLost()
{
	safeRelease(m_vertexBuffer);
}


void TriangleBuffer::deviceReset()
{
	if (m_vertexBuffer != 0)
	{
		TT_PANIC("Vertex Buffer already created!");
		safeRelease(m_vertexBuffer);
	}
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	DWORD usageFlags = 0;
	usageFlags |= D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	
	if (checkD3DSucceeded(device->CreateVertexBuffer(m_triangles.getBufferSize(),
	                                                 usageFlags,
	                                                 Renderer::getInstance()->getFVFFromVertexType(m_vertexType),
	                                                 D3DPOOL_DEFAULT,
	                                                 &m_vertexBuffer,
	                                                 0)))
	{
		TT_NULL_ASSERT(m_vertexBuffer);
		applyChanges();
	}
}


//--------------------------------------------------------------------------------------------------
// Private Functions



// Namespace end
}
}
}
