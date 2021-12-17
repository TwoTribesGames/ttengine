#include <tt/code/helpers.h>
#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {

enum
{
	VerticesPerQuad  = 4
};

#ifdef __GNUC__
const s32 QuadBuffer::maxBatchSize;
#endif
u32  QuadBuffer::ms_indexBuffer[QuadBuffer::maxBatchSize * IndicesPerQuad] = { 0 };
bool QuadBuffer::ms_indexBufferInitialized = false;
	
// Possible Optimizations:
// 1) Use VBOs
// 2) Use GL_QUADS primitives (less indices needed)


//--------------------------------------------------------------------------------------------------
// Public member functions
QuadBuffer::QuadBuffer(s32 p_size, const TexturePtr& p_texture /* = TexturePtr() */, 
					   BatchFlagQuad p_flag /* = BatchFlagQuad_None */)
:
m_useVtxColor((p_flag & BatchFlagQuad_UseVertexColor) == BatchFlagQuad_UseVertexColor),
m_capacity(std::min(p_size, maxBatchSize)),
m_quadCount(0),
m_texture(p_texture),
m_vertexCoordinates(0),
m_textureCoordinates(0),
m_colors(0)
{
	// Initialize the index buffer, if this was not done yet
	if (ms_indexBufferInitialized == false)
	{
		u32* indexBuffer     = ms_indexBuffer;
		u32  quadVertexIndex = 0;
		for (s32 i = 0; i < maxBatchSize; ++i)
		{
			// Quads are made out of 2 triangles like this:
			//   0 -- 1
			//   |  / |
			//   | /  |
			//   2 -- 3
			
			// FIXME: The triangles are created in opposite order. (One is CW and one is CCW.)
			// First triangle (0 - 1 - 2)
			indexBuffer[0] = static_cast<u32>(quadVertexIndex + 0);
			indexBuffer[1] = static_cast<u32>(quadVertexIndex + 1);
			indexBuffer[2] = static_cast<u32>(quadVertexIndex + 2);
			
			// Second triangle (1 - 2 - 3)
			indexBuffer[3] = static_cast<u32>(quadVertexIndex + 1);
			indexBuffer[4] = static_cast<u32>(quadVertexIndex + 2);
			indexBuffer[5] = static_cast<u32>(quadVertexIndex + 3);
			
			indexBuffer     += IndicesPerQuad;
			quadVertexIndex += VerticesPerQuad;
		}
		
		ms_indexBufferInitialized = true;
	}
	
	// Sanity check the requested size
	if (m_capacity <= 0)
	{
		TT_PANIC("QuadBuffer cannot have zero or negative size (requested size: %d).", p_size);
		m_capacity = 1;
	}
	TT_ASSERTMSG(m_capacity <= maxBatchSize,
	             "Requested size (%d) exceeds maximum capacity (%d)!", p_size, maxBatchSize);
	
	// Allocate memory for the quad buffer
	m_vertexCoordinates  = new math::Vector3[m_capacity * VerticesPerQuad];
	m_textureCoordinates = new math::Vector2[m_capacity * VerticesPerQuad];
	
	mem::zero8(m_vertexCoordinates,  static_cast<mem::size_type>(m_capacity * VerticesPerQuad * sizeof(math::Vector3)));
	mem::zero8(m_textureCoordinates, static_cast<mem::size_type>(m_capacity * VerticesPerQuad * sizeof(math::Vector2)));
	
	if (m_useVtxColor)
	{
		m_colors = new ColorRGBA[m_capacity * VerticesPerQuad];
		mem::zero8(m_colors, static_cast<mem::size_type>(m_capacity * VerticesPerQuad * sizeof(ColorRGBA)));
	}
}


QuadBuffer::~QuadBuffer()
{
	code::helpers::safeDeleteArray(m_vertexCoordinates);
	code::helpers::safeDeleteArray(m_textureCoordinates);
	code::helpers::safeDeleteArray(m_colors);
}


void QuadBuffer::applyChanges()
{
	// No separate update needed to apply changes for OpenGL (already done in setCollection)
}


void QuadBuffer::render() const
{
	// Check if there is anything to do
	if (m_quadCount <= 0) return;
	
	// Apply current transform
	MatrixStack::getInstance()->updateWorldMatrix();
	
	Renderer* renderer = Renderer::getInstance();
	
	// Activate the batch texture
	renderer->setTexture(m_texture);
	
	u32 vertexType = 0;
	if (m_useVtxColor)  vertexType |= VertexBuffer::Property_Diffuse;
	if (m_texture != 0) vertexType |= VertexBuffer::Property_Texture0;
	renderer->setVertexType(vertexType);
	
	// Set up the correct format
	if (m_useVtxColor)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_colors);
	}
	
	if (m_texture != 0)
	{
		glTexCoordPointer(2, GL_FLOAT, 0, m_textureCoordinates);
	}
	glVertexPointer(3, GL_FLOAT, 0, m_vertexCoordinates);

	renderer->stateCache()->apply();

	glDrawElements(GL_TRIANGLES, m_quadCount * IndicesPerQuad, GL_UNSIGNED_INT, ms_indexBuffer);
	
	TT_CHECK_OPENGL_ERROR();
	
	debug::DebugStats::addToQuadsRendered(m_quadCount);
}


void QuadBuffer::setCollection(const BatchQuadCollection& p_collection, s32 p_startIndex)
{
	copyToInternalBuffers(p_collection.begin(), p_collection.end(), p_startIndex);
}
	

void QuadBuffer::fillBuffer(const BatchQuadCollection::iterator& p_begin, const BatchQuadCollection::iterator& p_end)
{
	copyToInternalBuffers(p_begin, p_end, 0);
}


void QuadBuffer::clear()
{ 
	m_quadCount = 0;

	mem::zero8(m_vertexCoordinates, static_cast<mem::size_type>(m_capacity * sizeof(math::Vector3)));
	mem::zero8(m_textureCoordinates, static_cast<mem::size_type>(m_capacity * sizeof(math::Vector2)));
	if (m_useVtxColor)
	{
		mem::zero8(m_colors, static_cast<mem::size_type>(m_capacity * sizeof(ColorRGBA)));
	}
}

	
//------------------------------------
// Private
	
void QuadBuffer::copyToInternalBuffers(const BatchQuadCollection::const_iterator& p_begin, const BatchQuadCollection::const_iterator& p_end, s32 p_startIndex)
{
	s32 neededSpace = static_cast<s32>(std::distance(p_begin, p_end)) + p_startIndex;
	
	// Check available size
	if (neededSpace > m_capacity)
	{
		TT_PANIC("Too many quads (%d), can only store %d, increase capacity.", neededSpace, m_capacity);
		return;
	}
	
	m_quadCount = neededSpace;
	
	// Iterate the quads from the collection and add the required information to the arrays
	s32 vertexIndex = p_startIndex * VerticesPerQuad;
	
	for (BatchQuadCollection::const_iterator it = p_begin; it != p_end; ++it)
	{
		// Top-left
		m_vertexCoordinates [vertexIndex] = (*it).topLeft.getPosition();
		m_textureCoordinates[vertexIndex] = (*it).topLeft.getTexCoord();
		if (m_useVtxColor)
		{
			m_colors[vertexIndex] = (*it).topLeft.getColor();
		}
		++vertexIndex;
		
		// Top-right
		m_vertexCoordinates [vertexIndex] = (*it).topRight.getPosition();
		m_textureCoordinates[vertexIndex] = (*it).topRight.getTexCoord();
		if (m_useVtxColor)
		{
			m_colors[vertexIndex] = (*it).topRight.getColor();
		}
		++vertexIndex;
		
		// Bottom-left
		m_vertexCoordinates [vertexIndex] = (*it).bottomLeft.getPosition();
		m_textureCoordinates[vertexIndex] = (*it).bottomLeft.getTexCoord();
		if (m_useVtxColor)
		{
			m_colors[vertexIndex] = (*it).bottomLeft.getColor();
		}
		++vertexIndex;
		
		// Bottom-right
		m_vertexCoordinates [vertexIndex] = (*it).bottomRight.getPosition();
		m_textureCoordinates[vertexIndex] = (*it).bottomRight.getTexCoord();
		if (m_useVtxColor)
		{
			m_colors[vertexIndex] = (*it).bottomRight.getColor();
		}
		++vertexIndex;
	}
}
	
	
// Namespace end
}
}
}
