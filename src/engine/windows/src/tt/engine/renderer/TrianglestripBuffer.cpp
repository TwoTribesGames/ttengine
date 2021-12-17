#include <tt/engine/renderer/TrianglestripBuffer.h>

#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {


TrianglestripBuffer::TrianglestripBuffer(s32                    p_maxVertexCount,
                                         s32                    p_UVSetCount,
                                         const TexturePtr&      p_texture,
                                         BatchFlagTrianglestrip p_flag,
                                         PrimitiveType          p_primitiveType)
:
m_capacity(0),
m_vertices(1, 0, p_UVSetCount),
m_useVtxColor(p_flag == BatchFlagTrianglestrip_UseVertexColor),
m_vertexType(VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0),
m_texture(p_texture),
m_primitiveType(p_primitiveType),
m_useIndexBuffer(false),
m_verticesDirty(false),
m_indicesDirty(false),
m_vertexBuffer(0),
m_indexBuffer(0),
m_d3dPrimitiveType(D3DPT_TRIANGLELIST),
m_primitiveCount(0)
{
	if (p_UVSetCount > 1) m_vertexType |= VertexBuffer::Property_Texture1;
	if (p_UVSetCount > 2) m_vertexType |= VertexBuffer::Property_Texture2;
	if (p_UVSetCount > 3) m_vertexType |= VertexBuffer::Property_Texture3;
	if (p_UVSetCount > 4)
	{
		TT_PANIC("Too many UV sets (%d), max 4 supported.", p_UVSetCount);
	}
	
	resizeBuffers(p_maxVertexCount);
	
	D3DResourceRegistry::registerResource(this);
}


TrianglestripBuffer::~TrianglestripBuffer()
{
	D3DResourceRegistry::unregisterResource(this);
	Renderer::addToDeathRow(m_vertexBuffer);
	Renderer::addToDeathRow(m_indexBuffer);
}


void TrianglestripBuffer::resizeBuffers(s32 p_vertexCount)
{
	if (m_capacity != p_vertexCount)
	{
		if (m_capacity > 0)
		{
			// Release the old buffers.
			deviceLost();
		}
		
		m_capacity = p_vertexCount;
		
		// Check the size
		if(m_capacity > maxVtxBufSize)
		{
			TT_PANIC("Requested size (%d) exceeds maximum capacity (%d)!", m_capacity, maxVtxBufSize);
			m_capacity = maxVtxBufSize;
		}
		m_vertices.reservePrimitiveCount(m_capacity, false);
	}
}


void TrianglestripBuffer::applyChanges()
{
	if (m_vertices.getPrimitiveCount() > 0)
	{
		Renderer::getInstance()->checkFromRenderThread();
		updateVertices();
		updateIndices();
		updatePrimitiveInfo();
	}
}


void TrianglestripBuffer::render() const
{
	if(m_vertices.isEmpty()) return;
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	if (m_vertexBuffer == 0)
	{
		TT_PANIC("Vertex Buffer does not exist, call applyChanges() first.");
		return;
	}
	
	if (m_useIndexBuffer && m_indexBuffer == 0)
	{
		TT_PANIC("tried to render with indices without an IndexBuffer");
		return;
	}
	
	Renderer* renderer = Renderer::getInstance();
	
	// Apply current transform
	MatrixStack::getInstance()->updateWorldMatrix();
	
	// Activate the batch texture
	renderer->setTexture(m_texture);
	renderer->setVertexType(m_vertexType);
	
	checkD3DSucceeded( device->SetStreamSource(0,                               // Stream ID
	                                           m_vertexBuffer,                  // Vertex Buffer
	                                           0,                               // Offset
	                                           m_vertices.getVertexSize()) );   // Stride
	
	if (m_useIndexBuffer)
	{
		checkD3DSucceeded( device->SetIndices(m_indexBuffer) );
		
		checkD3DSucceeded(device->DrawIndexedPrimitive(m_d3dPrimitiveType,                 // Primitive Type
		                                               0,                                  // Start vertex
		                                               0,                                  // Minimum index
		                                               m_vertices.getTotalVerticesCount(), // Nr of vertices
		                                               0,                                  // Start index
		                                               m_primitiveCount) );                // Nr of primitives
	}
	else
	{
		checkD3DSucceeded(device->DrawPrimitive(m_d3dPrimitiveType,  // Primitive Type
		                                        0,                   // Start vertex
		                                        m_primitiveCount) ); // Nr of primitives
	}
}


void TrianglestripBuffer::setTexture(const TexturePtr& p_texture)
{
	m_texture = p_texture;
}


void TrianglestripBuffer::setIndices(const TrianglestripVertexIndices& p_indices)
{
	if (p_indices.empty())
	{
		TT_PANIC("Setting indices with an empty collection.");
		return;
	}
	
	TrianglestripVertexIndices::size_type neededSpace = p_indices.size();
	
	// Check available size
	if(neededSpace > maxIndicesBufSize)
	{
		TT_PANIC("Too many vertex indices (%d), can only store %d, increase capacity.", 
		         neededSpace, maxIndicesBufSize);
		neededSpace = m_capacity;
	}
	
	// the index buffer must not be larger (or smaller) than its data
	m_indices.resize(neededSpace);
	
	// Copy indices
	mem::copy8(&m_indices[0], &p_indices[0], static_cast<mem::size_type>(p_indices.size() * sizeof(u16)));
	
	m_indicesDirty = true;
}


void TrianglestripBuffer::clear()
{ 
	m_vertices.clear();
	m_indices.clear();
	
	m_verticesDirty = false;
	m_indicesDirty = false;
}


void TrianglestripBuffer::deviceLost()
{
	safeRelease(m_vertexBuffer);
	safeRelease(m_indexBuffer);
}


void TrianglestripBuffer::deviceReset()
{
	// create vertex buffer
	createVertexBuffer();
	
	// (re)create index buffer if used
	createIndexBuffer();
	
	// Fill buffers
	applyChanges();
}


void TrianglestripBuffer::setPrimitiveType(PrimitiveType p_type)
{
	m_primitiveType = p_type;
	updatePrimitiveInfo();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TrianglestripBuffer::createVertexBuffer()
{
	if (m_vertices.getBufferSize() == 0)
	{
		// No need for a buffer.
		return;
	}
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	if (m_vertexBuffer != 0)
	{
		TT_PANIC("Vertex Buffer already created!");
		safeRelease(m_vertexBuffer);
	}
	
	DWORD usageFlags = 0;
	usageFlags |= D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	
	if (checkD3DSucceeded( device->CreateVertexBuffer(m_vertices.getBufferSize(),
	                                                  usageFlags,
	                                                  Renderer::getInstance()->getFVFFromVertexType(m_vertexType),
	                                                  D3DPOOL_DEFAULT,
	                                                  &m_vertexBuffer,
	                                                  0)))
	{
		m_verticesDirty = true;
	}
}


bool TrianglestripBuffer::createIndexBuffer()
{
	if (m_indices.empty())
	{
		return false;
	}
	
	// Check for size difference
	if (m_indexBuffer != 0)
	{
		D3DINDEXBUFFER_DESC desc;
		checkD3DSucceeded(m_indexBuffer->GetDesc(&desc));
		
		if (m_indices.size() == desc.Size)
		{
			// Same size, no need to recreate
			return true;
		}
		
		safeRelease(m_indexBuffer);
	}
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return false;
	}
	
	if (checkD3DSucceeded(device->CreateIndexBuffer(static_cast<UINT>(m_indices.size() * sizeof(u16)),
	                                                0,
	                                                D3DFMT_INDEX16,
	                                                D3DPOOL_DEFAULT,
	                                                &m_indexBuffer,
	                                                0)))
	{
		m_indicesDirty = true;
		return true;
	}
	
	return false;
}


void TrianglestripBuffer::updateVertices()
{
	if (m_verticesDirty == false || m_vertices.isEmpty())
	{
		return;
	}
	
	if (m_vertexBuffer == 0)
	{
		createVertexBuffer();
	}
	TT_NULL_ASSERT(m_vertexBuffer);
	
	DWORD lockFlags = 0;
	lockFlags |= D3DLOCK_DISCARD;
	
	void* vertices = 0;
	if (checkD3DSucceeded(m_vertexBuffer->Lock(0, m_vertices.getRawVertexDataSize(), &vertices, lockFlags)))
	{
		// NOTE: Swapping colors here is a performance bottleneck, therefore the client must provide
		//       the color channels already swapped.
		{
			// Copy all to vertex buffer
			// NOTE: We assume color channels are the same, making it not important to swap
			mem::copy8(vertices, m_vertices.getRawVertexData(), m_vertices.getRawVertexDataSize());
		}
		checkD3DSucceeded(m_vertexBuffer->Unlock());
		
		m_verticesDirty = false;
	}
}


void TrianglestripBuffer::updateIndices()
{
	if (m_indicesDirty == false || m_indices.empty())
	{
		return;
	}
	
	if (m_indexBuffer == 0)
	{
		if (createIndexBuffer() == false)
		{
			// Creation failed.
			return;
		}
	}
	
	DWORD lockFlags = 0;
	lockFlags |= D3DLOCK_DISCARD;
	
	void* indices = 0;
	if (checkD3DSucceeded(m_indexBuffer->Lock(0, 0, &indices, lockFlags)))
	{
		// copy indices to D3D index buffer
		mem::copy8(indices, &m_indices[0], static_cast<mem::size_type>(m_indices.size() * sizeof(u16)));
		
		checkD3DSucceeded(m_indexBuffer->Unlock());
		
		m_indicesDirty = false;
		
	}
}


void TrianglestripBuffer::updatePrimitiveInfo()
{
	switch (m_primitiveType)
	{
	case PrimitiveType_TriangleStrip:
		m_d3dPrimitiveType = D3DPT_TRIANGLESTRIP;
		m_primitiveCount   = m_vertices.getPrimitiveCount() - 2;
		break;
		
	case PrimitiveType_Triangles:
		m_d3dPrimitiveType = D3DPT_TRIANGLELIST;
		m_primitiveCount   = m_vertices.getPrimitiveCount() / 3;
		break;
		
	case PrimitiveType_TriangleFan:
		m_d3dPrimitiveType = D3DPT_TRIANGLEFAN;
		m_primitiveCount   = m_vertices.getPrimitiveCount() - 2;
		break;
		
	case PrimitiveType_LineStrip:
		m_d3dPrimitiveType = D3DPT_LINESTRIP;
		m_primitiveCount   = m_vertices.getPrimitiveCount() - 1;
		break;
		
	case PrimitiveType_Lines:
		m_d3dPrimitiveType = D3DPT_LINELIST;
		m_primitiveCount   = m_vertices.getPrimitiveCount() / 2;
		break;
		
	default:
		TT_PANIC("Unknown PrimitiveType: %d", m_primitiveType);
		break;
	}
}


// Namespace end
}
}
}
