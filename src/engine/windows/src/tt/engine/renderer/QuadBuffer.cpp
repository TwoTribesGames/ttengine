#include <tt/engine/renderer/QuadBuffer.h>

#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {


IDirect3DIndexBuffer9* QuadBuffer::ms_sharedIndexBuffer = 0;

enum
{
	VerticesPerQuad = 4,
	IndicesPerQuad  = 6
};


QuadBuffer::QuadBuffer(s32 p_maxQuadCount, const TexturePtr& p_texture, BatchFlagQuad p_flag)
:
m_useVtxColor((p_flag & BatchFlagQuad_UseVertexColor) == BatchFlagQuad_UseVertexColor),
m_capacity(p_maxQuadCount),
m_vertexType  (VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0),
m_texture     (p_texture),
m_vertexBuffer(0)
{
	m_quads.reserve(m_capacity);
	
	if(m_capacity > maxBatchSize)
	{
		TT_PANIC("Requested size (%d) exceeds maximum capacity (%d)!", m_capacity, maxBatchSize);
		m_capacity = maxBatchSize;
	}
	
	D3DResourceRegistry::registerResource(this);
}


QuadBuffer::~QuadBuffer()
{
	D3DResourceRegistry::unregisterResource(this);
	Renderer::addToDeathRow(m_vertexBuffer);
}


void QuadBuffer::applyChanges()
{
	if(m_quads.empty()) return;
	
	Renderer::getInstance()->checkFromRenderThread();
	
	if(m_vertexBuffer == 0)
	{
		deviceCreated();
		deviceReset();
		return;
	}
	
	// Lock vertex buffer -> discard all data in it
	DWORD lockFlags = 0;
	lockFlags |= D3DLOCK_DISCARD;
	void* vertices = 0;
	const s32 sizeOfData = static_cast<s32>(m_quads.size() * sizeof(BatchQuad));
	
	if (checkD3DSucceeded(m_vertexBuffer->Lock(0, sizeOfData, &vertices, lockFlags)))
	{
		// NOTE: Swapping colors here is a performance bottleneck, therefore the client must provide
		//       the color channels already swapped.
		{
			// Copy all to vertex buffer
			// NOTE: We assume color channels are the same, making it not important to swap
			mem::copy8(vertices, &m_quads[0], sizeOfData);
		}
		checkD3DSucceeded(m_vertexBuffer->Unlock());
	}
}


void QuadBuffer::render() const
{
	// Check if there is anything to do
	if(m_quads.empty()) return;
	
	// Error checking
	if(m_vertexBuffer == 0)
	{
		TT_PANIC("Vertex Buffer does not exist, call applyChanges() first.");
		return;
	}
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	// Apply current transform
	MatrixStack::getInstance()->updateWorldMatrix();
	
	// Activate the batch texture
	Renderer::getInstance()->setTexture(m_texture);
	Renderer::getInstance()->setVertexType(m_vertexType);
	
	checkD3DSucceeded( device->SetIndices(ms_sharedIndexBuffer) );
	
	checkD3DSucceeded( device->SetStreamSource(0,                    // Stream ID
	                                           m_vertexBuffer,       // Vertex Buffer
	                                           0,                    // Offset
	                                           sizeof(BufferVtx)) ); // Stride
	
	
	checkD3DSucceeded( device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,               // Draw triangles
	                                                0,                                // Start at vertex 0
	                                                0,                                // Minimum index
	                                                static_cast<UINT>(m_quads.size() * VerticesPerQuad), // Nr of vertices
	                                                0,                                // Start at index 0
	                                                static_cast<UINT>(m_quads.size() * 2)) );            // Triangles to draw
	
	debug::DebugStats::addToQuadsRendered(static_cast<s32>(m_quads.size()));
}


void QuadBuffer::setCollection(const BatchQuadCollection& p_collection, s32 p_startIndex /* = 0 */)
{
	BatchQuadCollection::size_type neededSpace = p_collection.size() + p_startIndex;
	
	// Check available size
	if(m_quads.size() < neededSpace)
	{
		if(neededSpace > static_cast<BatchQuadCollection::size_type>(m_capacity))
		{
			TT_PANIC("Too many quads (%d), can only store %d, increase capacity.", neededSpace, m_capacity);
			return;
		}
		
		m_quads.resize(neededSpace);
	}
	
	// Copy quads
	if(p_collection.empty() == false)
	{
		std::copy(p_collection.begin(), p_collection.end(), m_quads.begin() + p_startIndex);
	}
}


void QuadBuffer::fillBuffer(const BatchQuadCollection::iterator& p_begin, const BatchQuadCollection::iterator& p_end)
{
	// FIXME:
	// Optimize by bypassing the internal quad collection

	m_quads.assign(p_begin, p_end);
	applyChanges();
}


void QuadBuffer::deviceCreated()
{
	// Only the first batch to be called will create this
	if(ms_sharedIndexBuffer == 0)
	{
		Renderer::getInstance()->checkFromRenderThread();
		generateSharedIndexBuffer();
	}
}


void QuadBuffer::deviceLost()
{
	Renderer::getInstance()->checkFromRenderThread();
	safeRelease(m_vertexBuffer);
}


void QuadBuffer::deviceReset()
{
	if(m_capacity == 0) return;
	Renderer::getInstance()->checkFromRenderThread();
	
	TT_ASSERTMSG(m_vertexBuffer == 0, "Vertex Buffer already created!");
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	DWORD usageFlags = 0;
	usageFlags |= D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	
	if (checkD3DSucceeded(device->CreateVertexBuffer(m_capacity * sizeof(BatchQuad),
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


void QuadBuffer::deviceDestroyed()
{
	Renderer::getInstance()->checkFromRenderThread();
	safeRelease(ms_sharedIndexBuffer);
}


//////////////////////////
// Privates

bool QuadBuffer::generateSharedIndexBuffer()
{
	// Make sure this is only called when not already created
	TT_ASSERT(ms_sharedIndexBuffer == 0);
	
	// Create Index Buffer for maximum batch size
	u32 maxIndices = maxBatchSize * IndicesPerQuad; // 6 indices per quad (= 2 tris)
	
	TT_ASSERTMSG(maxIndices < std::numeric_limits<u32>::max(), "Too many indices for u32");
	
	// Indices are created in the managed pool -> no need to recreate on device loss
	if (checkD3DSucceeded(getRenderDevice(true)->CreateIndexBuffer(maxIndices * sizeof(u32),
	                                                               D3DUSAGE_WRITEONLY,
	                                                               D3DFMT_INDEX32,
	                                                               D3DPOOL_MANAGED,
	                                                               &ms_sharedIndexBuffer,
	                                                               0)))
	{
		u32* indices = 0;
		if (checkD3DSucceeded(ms_sharedIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&indices), 0)))
		{
			// Generate index buffer (we can keep this one, because it never changes)
			for (s32 i = 0; i < maxBatchSize; ++i)
			{
				s32 offset = i * IndicesPerQuad;
				
				// Quads are made out of 2 triangles like this:
				//   1 - - 3
				//   | \   |
				//   |   \ | 
				//   0 - - 2
				
				// First triangle (0 - 1 - 2)
				indices[0 + offset] = static_cast<u32>(0 + i * VerticesPerQuad);
				indices[1 + offset] = static_cast<u32>(1 + i * VerticesPerQuad);
				indices[2 + offset] = static_cast<u32>(2 + i * VerticesPerQuad);
				
				// Second triangle (1 - 3 - 2)
				indices[3 + offset] = static_cast<u32>(1 + i * VerticesPerQuad);
				indices[4 + offset] = static_cast<u32>(3 + i * VerticesPerQuad);
				indices[5 + offset] = static_cast<u32>(2 + i * VerticesPerQuad);
			}
			
			return checkD3DSucceeded(ms_sharedIndexBuffer->Unlock());
		}
	}
	
	return false;
}


// Namespace end
}
}
}
