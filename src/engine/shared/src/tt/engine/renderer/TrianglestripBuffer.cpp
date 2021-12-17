#include <tt/code/helpers.h>
#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {

const s32 TrianglestripBuffer::maxVtxBufSize;


//--------------------------------------------------------------------------------------------------
// Public member functions

TrianglestripBuffer::TrianglestripBuffer(s32 p_maxVertexCount, s32 p_UVSetCount, const TexturePtr& p_texture,
                                         BatchFlagTrianglestrip p_flag, PrimitiveType p_primitiveType)
:
m_primitiveType(p_primitiveType),
m_useVtxColor((p_flag & BatchFlagTrianglestrip_UseVertexColor) == BatchFlagTrianglestrip_UseVertexColor),
m_useCustomIndices(false),
m_capacity(0),
m_vertices(1, 0, p_UVSetCount),
m_texture(p_texture),
m_indexBuffer()
{
	resizeBuffers(p_maxVertexCount);
}


TrianglestripBuffer::~TrianglestripBuffer()
{
}


void TrianglestripBuffer::resizeBuffers(s32 p_vertexCount)
{
	if (m_capacity != p_vertexCount)
	{
		TT_ASSERTMSG(p_vertexCount <= maxVtxBufSize,
					 "Requested size (%d) exceeds maximum capacity (%d)!", p_vertexCount, maxVtxBufSize);
		m_capacity = std::min(p_vertexCount, maxVtxBufSize);
		m_vertices.reservePrimitiveCount(m_capacity, false);
		m_indexBuffer.reserve(m_capacity);
	}
}


void TrianglestripBuffer::applyChanges()
{
	TT_ASSERTMSG(m_vertices.getPrimitiveCount() > 0, "cannot apply an empty vertex buffer");
	
	if (m_useCustomIndices == false)
	{
		const u32 indexCountNeeded = m_vertices.getTotalVerticesCount();
		
		// Did the primitiveCount change? (Then indexBuffer also needs updating.)
		if (m_indexBuffer.size() < indexCountNeeded)
		{
			// indexBuffer needs to grow.
			
			// Create indices in the order of the vertex buffer
			m_indexBuffer.reserve(static_cast<std::vector<u16>::size_type>(indexCountNeeded));
			for (u16 i = m_indexBuffer.size(); i < indexCountNeeded; ++i)
			{
				m_indexBuffer.push_back(i);
			}
		}
		else if (m_indexBuffer.size() > indexCountNeeded)
		{
			// indexBuffer needs to shrink.
			
			m_indexBuffer.resize(indexCountNeeded);
		}
	}
	else
	{
		TT_ASSERTMSG(m_indexBuffer.size() > 0, "cannot apply an empty index buffer");
	}
}


void TrianglestripBuffer::setIndexBufferEnabled(bool p_useIndexBuffer)
{ 
	if (p_useIndexBuffer != m_useCustomIndices && m_indexBuffer.empty() == false)
	{
		// Change in index buffer use, clear old data
		m_indexBuffer.clear();
	}
	m_useCustomIndices = p_useIndexBuffer;
}


void TrianglestripBuffer::render() const
{
	// Check if there is anything to do
	if (m_vertices.isEmpty()) return;
	
	TT_ASSERTMSG(m_indexBuffer.empty() == false, "unable to render without an index buffer");
	
	// Apply current transform (not implemented for OSX)
	MatrixStack::getInstance()->updateWorldMatrix();
	
	Renderer* renderer = Renderer::getInstance();
	
	// TODO: Could all these basic geometry type settings for the renderer not be handled by this primitiveBuffer?
	//       This is a half solution at this moment, applying settings partially in the renderer and here
	//       (check for other uses of setVertexType and make a decision)
	
	u32 vertexType = 0;
	if (m_useVtxColor)  vertexType |= VertexBuffer::Property_Diffuse;
	if (m_texture != 0) 
	{
		vertexType |= VertexBuffer::Property_Texture0;
	}
	// Apply basic settings for diffuse, normals, textures
	renderer->setVertexType(vertexType);
	
	// Activate the batch texture
	renderer->setTexture(m_texture);
	
	
	//--- SET BUFFER POINTERS + STRIDE FOR POS, COL, TEX0..n ---
	const GLvoid* vtxPos = &reinterpret_cast<const BufferVtx*>(m_vertices.getRawVertexData())->pos;
	glVertexPointer(3, GL_FLOAT, m_vertices.getVertexSize(), vtxPos);
	
	if (m_useVtxColor)
	{
		const GLvoid* vtxCol = &reinterpret_cast<const BufferVtx*>(m_vertices.getRawVertexData())->col;
		glColorPointer(4, GL_UNSIGNED_BYTE, m_vertices.getVertexSize(), vtxCol);
	}
	
	if (m_texture != 0)
	{
		const GLvoid* vtxTex = 0;
		s32 texCoordSetIndex = 0;
		
		for (s32 i = 0; i < m_vertices.getUVSetCount(); ++i)
		{
			/*
			if (m_multitexture != 0)
			{
				texCoordSetIndex = m_multitexture->getStage(i).getTexCoordIndex();
				TT_ASSERTMSG(texCoordSetIndex < m_vertices.getUVSetCount(),
				             "Tried to access a non-existent texture coord set (%d), sets: %d",
				             texCoordSetIndex, m_vertices.getUVSetCount());
			}
			*/
			
			Texture::setActiveClientChannel(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
			vtxTex = &reinterpret_cast<const BufferVtx*>(m_vertices.getRawVertexData())->tex[texCoordSetIndex];
			glTexCoordPointer(2, GL_FLOAT, m_vertices.getVertexSize(), vtxTex);
		}
	}
	//--- SET BUFFER POINTERS + STRIDE FOR POS, COL, TEXn end ---

	renderer->stateCache()->apply();

	switch (m_primitiveType)
	{
	case PrimitiveType_TriangleStrip:
		glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_SHORT, &m_indexBuffer[0]);
		// On hardware rendering is slightly faster with index bufffer instead of with array.
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertices.getPrimitiveCount());
		break;
	case PrimitiveType_Triangles:
		glDrawElements(GL_TRIANGLES,      static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_SHORT, &m_indexBuffer[0]);
		break;
	case PrimitiveType_TriangleFan:
		//glDrawElements(GL_TRIANGLE_FAN,     m_indexBuffer.size(), GL_UNSIGNED_SHORT, &m_indexBuffer[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_vertices.getPrimitiveCount());
		break;
	case PrimitiveType_LineStrip:
		glDrawArrays(GL_LINE_STRIP, 0, m_vertices.getPrimitiveCount());
		break;
	case PrimitiveType_Lines:
		glDrawArrays(GL_LINES, 0, m_vertices.getPrimitiveCount());
		break;
	default:
		TT_PANIC("Unknown primitive type: %d", m_primitiveType);
		break;
	}	
	
	// Undo multiple OpenGL texture stage changes on client side (stage 0 is managed by renderer)
	for (s32 i = 1; i < m_vertices.getUVSetCount(); ++i)
	{
		Texture::setActiveClientChannel(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	Texture::setActiveClientChannel(0);
	
	renderer->resetVertexType();
	
	TT_CHECK_OPENGL_ERROR();
}


void TrianglestripBuffer::setIndices(const TrianglestripVertexIndices& p_indices)
{
	if (p_indices.empty())
	{
		return;
	}

	// Check available size
	if(p_indices.size() > maxIndicesBufSize)
	{
		TT_PANIC("Too many vertex indices (%d), can only store %d, increase capacity.", 
                 p_indices.size(), maxIndicesBufSize);
		return;
	}
	
	TT_ASSERTMSG(p_indices.size() >= static_cast<TrianglestripVertexIndices::size_type>(m_capacity), 
	             "index buffer should not be smaller than vertex buffer");
	
	if (p_indices.size() != m_indexBuffer.size())
	{
		m_indexBuffer.resize(p_indices.size());
	}

	// Copy indices
	mem::copy8(&m_indexBuffer[0], 
	           &p_indices[0], 
	           static_cast<mem::size_type>(p_indices.size() * sizeof(u16)));
}


void TrianglestripBuffer::setTexture(const TexturePtr& p_texture)
{
	m_texture = p_texture;
}


void TrianglestripBuffer::clear()
{ 
	m_vertices.clear();
	m_indexBuffer.clear();
}


// Namespace end
}
}
}
