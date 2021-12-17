#if !defined(INC_TT_ENGINE_RENDERER_TRIANGLESTRIPBUFFER_H)
#define INC_TT_ENGINE_RENDERER_TRIANGLESTRIPBUFFER_H

#include <tt/engine/renderer/BufferVtx.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/PrimitiveCollectionBuffer.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {


// BatchTrianglestrip vertex collection types for multiple texture coordinate sets (add more if needed)
typedef std::vector<BufferVtxUV<1> > TrianglestripVertices; // backwards compatibility
typedef std::vector<BufferVtxUV<2> > TrianglestripVertices2UVSets;
typedef std::vector<BufferVtxUV<3> > TrianglestripVertices3UVSets;
typedef std::vector<BufferVtxUV<4> > TrianglestripVertices4UVSets;

typedef std::vector<u16>       TrianglestripVertexIndices;

enum BatchFlagTrianglestrip
{
	BatchFlagTrianglestrip_None,
	BatchFlagTrianglestrip_UseVertexColor
};


/*! \brief The TrianglestripBuffer creates a VertexBuffer once, and an IndexBuffer when the client requires it
           steps: instantiate class -> set VertexBuffer and IndexBuffer -> applyChanges -> render
           note: the use of indices is mandatory */
class TrianglestripBuffer
{
public:
	enum PrimitiveType
	{
		PrimitiveType_TriangleStrip,
		PrimitiveType_Triangles,
		PrimitiveType_TriangleFan,
		PrimitiveType_LineStrip,
		PrimitiveType_Lines
	};
	
	// At most 65000 vertices/indices per batch (near u16 index buffer limit)
	static const s32 maxVtxBufSize = 65000;
	static const TrianglestripVertexIndices::size_type maxIndicesBufSize = 65000;
	
	/*! \brief Constructor for creating a trianglestripbuffer
	    \param p_maxVertexCount Maximum nr of vertices to be stored
	    \param p_UVSetCount     Number of UV sets
	    \param p_texture        Texture for this batchcollection
	    \param p_flag           Flags to control the batchcollection */
	TrianglestripBuffer(s32 p_maxVertexCount, s32 p_UVSetCount = 1,
	                    const TexturePtr& p_texture = TexturePtr(),
	                    BatchFlagTrianglestrip p_flag = BatchFlagTrianglestrip_None,
	                    PrimitiveType p_primitiveType = PrimitiveType_TriangleStrip);
	
	/*! \brief Destructor that releases vertex data */
	~TrianglestripBuffer();
	
	void resizeBuffers(s32 p_vertexCount);
	
	/*! \brief Update the trianglestrip (must be called to make changes to properties take effect) */
	void applyChanges();
	
	/*! \brief Render this trianglestrip on the screen */
	void render() const;
	
	/*! \brief Sets the batchcollection with a templated UVSetCount
	    \param p_trianglestripVertices The templatized collection to copy from
	    \param p_startIndex            The start index for the internal buffer */
	template <s32 UVSetCount>
	void setCollection(const std::vector<BufferVtxUV<UVSetCount> >& p_tristripVertices, s32 p_startIndex = 0)
	{ m_vertices.setCollection(p_tristripVertices, p_startIndex); }
	
	template <s32 UVSetCount>
	inline       BufferVtxUV<UVSetCount>& modifyVtx(s32 p_index)
	{ TT_MINMAX_ASSERT(p_index, 0, m_capacity - 1); return m_vertices.modifyVtx<UVSetCount>(p_index); }
	
	template <s32 UVSetCount>
	inline const BufferVtxUV<UVSetCount>& getVtx(s32 p_index) const
	{ TT_MINMAX_ASSERT(p_index, 0, m_capacity - 1); return m_vertices.getVtx<UVSetCount>(p_index); }
	
	template <s32 UVSetCount>
	inline void resize(s32 p_size, BufferVtxUV<UVSetCount> p_defaultValue = BufferVtxUV<UVSetCount>())
	{ m_vertices.resize(p_size, p_defaultValue); }
	
	inline s32 getTotalVerticesCount() const { return m_vertices.getTotalVerticesCount(); }
	
	/*! \brief Sets the indices */
	void setIndices(const TrianglestripVertexIndices& p_indices);
	
	/*! \brief Set/Get the optional texture to use for this batchcollection 
	    \note  An existing multitexture will be reset */
	void setTexture(const TexturePtr& p_texture);
	inline const TexturePtr& getTexture() const { return m_texture; }
	
	/*! \brief clears the quad buffer and depending counters */
	void clear();
	
	/*! \brief Set/Get the use of indices for rendering, 
	           this is always true with the current draw method.
	           present for compatibility */
	void setIndexBufferEnabled(bool p_useIndexBuffer);
	inline bool isIndexBufferEnabled() const { return m_useCustomIndices; }
	
	inline void setPrimitiveType(PrimitiveType p_primitiveType) { m_primitiveType = p_primitiveType; }
    inline PrimitiveType getPrimitiveType() const { return m_primitiveType; }
	
private:
	// No copying
	TrianglestripBuffer(const TrianglestripBuffer&);            // Disable copy.      Not implemented.
	TrianglestripBuffer& operator=(const TrianglestripBuffer&); // Disable assigment. Not implemented.
	
	PrimitiveType   m_primitiveType;
	
	// Vertex Data
	bool m_useVtxColor;
	bool m_useCustomIndices;
	// FIXME: PrimitiveType and these 2 bools can be packed into a single byte.
	
	s32 m_capacity;
	
	PrimitiveCollectionBuffer m_vertices;
	
	TexturePtr      m_texture;
	
	/*! \brief Index buffer for trianglestrip. */
	TrianglestripVertexIndices m_indexBuffer;
};


// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_RENDERER_TRIANGLESTRIPBUFFER_H)
