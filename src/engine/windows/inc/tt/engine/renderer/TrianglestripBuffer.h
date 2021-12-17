#if !defined(INC_TT_ENGINE_RENDERER_TRIANGLESTRIPBUFFER_H)
#define INC_TT_ENGINE_RENDERER_TRIANGLESTRIPBUFFER_H

#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/BufferVtx.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/PrimitiveCollectionBuffer.h>


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


/*! \brief The TrianglestripBuffer creates a VertexBuffer once, and an IndexBuffer when the client requires it.
           steps: instantiate class -> set VertexBuffer and optional IndexBuffer -> applyChanges -> render
           note: the default use of indices is false, call setIndexBufferEnabled(true) for use */
class TrianglestripBuffer : public D3DResource
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
	
	/*! \brief At most 650.000 vertices/indices per batch (u16 size for now) */
	static const s32 maxVtxBufSize = 650000;
	static const TrianglestripVertexIndices::size_type maxIndicesBufSize = 650000;
	
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
	
	/*! \brief Update the trianglebuffer (must be called to make changes to properties take effect) */
	void applyChanges();
	
	/*! \brief Render this trianglebuffer on the screen */
	void render() const;
	
	/*! \brief Set/Get the optional texture to use for this batchcollection 
	    \note  An existing multitexture will be reset */
	void setTexture(const TexturePtr& p_texture);
	inline const TexturePtr& getTexture() { return m_texture; }
	
	/*! \brief Sets the batchcollection with a templated UVSetCount
	    \param p_trianglestripVertices The templatized collection to copy from
	    \param p_startIndex            The start index for the internal buffer */
	template <s32 UVSetCount>
	void setCollection(const std::vector<BufferVtxUV<UVSetCount> >& p_tristripVertices, s32 p_startIndex = 0)
		{ m_vertices.setCollection(p_tristripVertices, p_startIndex); m_verticesDirty = true; }
	
	template <s32 UVSetCount>
	inline       BufferVtxUV<UVSetCount>& modifyVtx(s32 p_index)
	{ TT_MINMAX_ASSERT(p_index, 0, m_capacity - 1); m_verticesDirty = true; return m_vertices.modifyVtx<UVSetCount>(p_index); }
	
	template <s32 UVSetCount>
	inline const BufferVtxUV<UVSetCount>& getVtx(s32 p_index) const
	{ TT_MINMAX_ASSERT(p_index, 0, m_capacity - 1); return m_vertices.getVtx<UVSetCount>(p_index); }
	
	template <s32 UVSetCount>
	inline void resize(s32 p_size, BufferVtxUV<UVSetCount> p_defaultValue = BufferVtxUV<UVSetCount>())
	{ m_vertices.resize(p_size, p_defaultValue); m_verticesDirty = true; }
	
	inline s32 getTotalVerticesCount() const { return m_vertices.getTotalVerticesCount(); }
	
	/*! \brief Sets the indices */
	void setIndices(const TrianglestripVertexIndices& p_indices);
	
	/*! \brief Clears the vertices and indices */
	void clear();
	
	/*! \brief Releases device resources (WINDOWS ONLY) */
	virtual void deviceLost();
	
	/*! \brief (Re-)creates device resources (WINDOWS ONLY) */
	virtual void deviceReset();
	
	
	/*! \brief Set/Get the use of indices for rendering */
	inline void setIndexBufferEnabled(bool p_useIndexBuffer) { m_useIndexBuffer = p_useIndexBuffer; }
	inline bool isIndexBufferEnabled() const { return m_useIndexBuffer; }
	
	void setPrimitiveType(PrimitiveType p_primitiveType);
	inline PrimitiveType  getPrimitiveType() const { return m_primitiveType; }
	
private:
	TrianglestripBuffer(const TrianglestripBuffer& p_rhs);            // Disable copy.     Not implemented.
	TrianglestripBuffer& operator=(const TrianglestripBuffer& p_rhs); // Disable assigment Not implemented.
	
	/*! \brief Create vertex/index buffers */
	void createVertexBuffer();
	bool createIndexBuffer();
	
	/*! \brief Update vertex/index buffers */
	void updateVertices();
	void updateIndices();
	
	void updatePrimitiveInfo();
	
	// Trianglestrip data
	s32                        m_capacity;
	PrimitiveCollectionBuffer  m_vertices;
	TrianglestripVertexIndices m_indices;
	bool                       m_useVtxColor;
	u32                        m_vertexType;
	TexturePtr                 m_texture;
	
	PrimitiveType              m_primitiveType;
	bool                       m_useIndexBuffer;
	bool                       m_verticesDirty;
	bool                       m_indicesDirty;
	
	// Win specific
	IDirect3DVertexBuffer9*    m_vertexBuffer;
	IDirect3DIndexBuffer9*     m_indexBuffer;
	D3DPRIMITIVETYPE           m_d3dPrimitiveType;
	UINT                       m_primitiveCount;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_RENDERER_TRIANGLESTRIPBUFFER_H)
