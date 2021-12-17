#include <tt/engine/renderer/TriangleBuffer.h>
#include <tt/code/helpers.h>
#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/util.h>


namespace tt {
namespace engine {
namespace renderer {


#ifdef __GNUC__
const s32 TriangleBuffer::maxBatchSize;
#endif

//--------------------------------------------------------------------------------------------------
// Public member functions

TriangleBuffer::TriangleBuffer(s32 p_maxTriangleCount, s32 p_uvSize, const TexturePtr& p_texture,
                               BatchFlagTriangle p_flag)
:
m_capacity(p_maxTriangleCount),
m_triangles(3, m_capacity, p_uvSize),
m_useVtxColor((p_flag & BatchFlagTriangle_UseVertexColor) == BatchFlagTriangle_UseVertexColor),
m_texture(p_texture),
m_indexBuffer()
{
	// Sanity check the requested size
	if (m_capacity <= 0)
	{
		TT_PANIC("TriangleBuffer cannot have zero or negative size (requested size: %d).",
		         p_maxTriangleCount);
		m_capacity = 1;
	}
	TT_ASSERTMSG(m_capacity <= maxBatchSize,
	             "Requested size (%d) exceeds maximum capacity (%d)!", p_maxTriangleCount, maxBatchSize);
	
	m_indexBuffer.reserve(p_maxTriangleCount);
}


TriangleBuffer::~TriangleBuffer()
{
}


void TriangleBuffer::applyChanges()
{
	// Check if there is anything to apply
	if (m_triangles.getPrimitiveCount() <= 0) return;
	
	//reconstruct indices
	m_indexBuffer.clear();
	
	// create indices in the order of the vertex buffer
	const s32 indexCount = m_triangles.getPrimitiveCount() * m_triangles.getVerticesPerPrimitive();
	m_indexBuffer.reserve(static_cast<std::vector<u16>::size_type>(indexCount));
	for (s32 i = 0; i < indexCount; ++i)
	{
		m_indexBuffer.push_back(i);
	}
}


void TriangleBuffer::render() const
{
	// Check if there is anything to do
	if (m_triangles.getPrimitiveCount() <= 0) return;
	if (m_indexBuffer.empty()) return;
	
	// Apply current transform (not implemented for OSX)
	MatrixStack::getInstance()->updateWorldMatrix();
	
	Renderer* renderer = Renderer::getInstance();
	
	// TODO: could all these basic geometry type settings for the renderer not be handled by this primitiveBuffer?
	//       this is a half solution at this moment, applying settings partially in the renderer and here
	//       (check for other uses of setVertexType and make a decision)
	
	u32 vertexType = 0;
	if (m_useVtxColor)  vertexType |= VertexBuffer::Property_Diffuse;
	if (m_texture != 0)
	{
		vertexType |= VertexBuffer::Property_Texture0;
	}
	// Apply basic OpenGL state settings for diffuse, normals, textures
	renderer->setVertexType(vertexType);
	renderer->setTexture(m_texture);
	
	//--- SET BUFFER POINTERS + STRIDE FOR POS, COL, TEX0..n ---
	const GLvoid* vtxPos = &reinterpret_cast<const BufferVtx*>(m_triangles.getRawVertexData())->pos;
	glVertexPointer(3, GL_FLOAT, m_triangles.getVertexSize(), vtxPos);
	
	if (m_useVtxColor)
	{
		const GLvoid* vtxCol = &reinterpret_cast<const BufferVtx*>(m_triangles.getRawVertexData())->col;
		glColorPointer(4, GL_UNSIGNED_BYTE, m_triangles.getVertexSize(), vtxCol);
	}
	
	if (m_texture != 0)
	{
		const GLvoid* vtxTex = 0;
		s32 texCoordSetIndex = 0;
		
		for (s32 i = 0; i < m_triangles.getUVSetCount(); ++i)
		{
/*
			if (m_multitexture != 0)
			{
				texCoordSetIndex = m_multitexture->getStage(i).getTexCoordIndex();
				TT_ASSERTMSG(texCoordSetIndex < m_triangles.getUVSetCount(), 
				             "Tried to access a non-existent texture coord set (%d), number of sets: %d",
				             texCoordSetIndex, m_triangles.getUVSetCount());
			}
*/
			Texture::setActiveClientChannel(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			
			vtxTex = &reinterpret_cast<const BufferVtx*>(m_triangles.getRawVertexData())->tex[texCoordSetIndex];
			glTexCoordPointer(2, GL_FLOAT, m_triangles.getVertexSize(), vtxTex);
		}
	}
	//--- SET BUFFER POINTERS + STRIDE FOR POS, COL, TEXn end ---

	renderer->stateCache()->apply();

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_SHORT, &m_indexBuffer[0]);
	
	// Undo multiple OpenGL texture stage changes on client side (stage 0 is managed by renderer)
	for (s32 i = 0; i < m_triangles.getUVSetCount(); ++i)
	{
		Texture::setActiveClientChannel(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	Texture::setActiveClientChannel(0);

	renderer->resetVertexType();
	
	TT_CHECK_OPENGL_ERROR();
}


void TriangleBuffer::setTexture(const TexturePtr& p_texture)
{
	m_texture = p_texture;
}


void TriangleBuffer::clear()
{
	m_triangles.clear();
	m_indexBuffer.clear();
}

// Namespace end
}
}
}
