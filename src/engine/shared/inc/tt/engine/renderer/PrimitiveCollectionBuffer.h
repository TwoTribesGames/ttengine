#if !defined(INC_TT_ENGINE_RENDERER_BATCHTRIANGLECOLLECTIONBUFFER_H)
#define INC_TT_ENGINE_RENDERER_BATCHTRIANGLECOLLECTIONBUFFER_H

#include <vector>

#include <tt/engine/renderer/BufferVtx.h>
#include <tt/engine/renderer/BatchTriangleUV.h>


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Manages a void pointer buffer for primitive buffers supporting multiple texture coordinate sets.
           Each primitive buffer uses an number of BufferVtx<UVSetCount> per primitive (e.g. quad has four) */
class PrimitiveCollectionBuffer
{
public:
	/*! \param p_initialReservedTriangleCount The maximum number of triangles (with the current UVSetCount)
	           which can be stored in the buffer
	    \param p_verticesPerPrimitive The number of vertices for one stride (a quad would have 4)
	    \param p_UVSetCount The number of UVSets in each triangle vertex
	    \note  The UVSetCount can be changed after initialization, this has influence on the number
	           of triangles that can be stored in the buffer */
	inline PrimitiveCollectionBuffer(s32 p_verticesPerPrimitive, s32 p_initialReservedPrimitiveCount,
	                                 s32 p_UVSetCount)
	:
	m_buffer(0),
	m_bufferSize(0),
	m_vertexCount(0),
	m_UVSetCount(p_UVSetCount),
	m_verticesPerPrimitive(p_verticesPerPrimitive)
	{
		TT_MIN_ASSERT(m_UVSetCount,           1);
		TT_MIN_ASSERT(m_verticesPerPrimitive, 1); // Need atleast 1 vertex per primitive.
		
		reservePrimitiveCount(p_initialReservedPrimitiveCount, false);
	}
	
	inline ~PrimitiveCollectionBuffer()
	{
		::operator delete(m_buffer);
	}
	
	inline bool isEmpty() const { return m_vertexCount <= 0; }
	
	//--- SETTING COLLECTION FOR SPECIFIC TYPES ---
	/* \brief If a collection is set with a different UVSetCount than previously set (by initialization or a 
	          prior call to this function) the maximum number of triangles possible will change as well.*/

	/* BatchTriangleUV */
	template <s32 UVSetCount>
	void setCollection(const std::vector<BatchTriangleUV<UVSetCount> >& p_source,
	                          s32 p_startIndexDestination = 0);
	
	/* BufferVtxUV */
	template <s32 UVSetCount>
	void setCollection(const std::vector<BufferVtxUV<UVSetCount> >& p_source, s32 p_startIndexDestination);
	
	template <s32 UVSetCount>
	BufferVtxUV<UVSetCount>& modifyVtx(s32 p_index);
	
	template <s32 UVSetCount>
	const BufferVtxUV<UVSetCount>& getVtx(s32 p_index) const;
	
	template <s32 UVSetCount>
	void resize(s32 p_size, BufferVtxUV<UVSetCount> p_defaultValue = BufferVtxUV<UVSetCount>());
	
	// Raw
	inline const void* getRawVertexData()     const { return m_buffer;         }
	inline void*       getRawVertexData()           { return m_buffer;         } //!< needed for debug rendering
	// FIXME: Rename - this returns the number of bytes USED in buffer, not buffer size.
	inline s32         getRawVertexDataSize() const { return m_vertexCount * getVertexSize(); }
	// FIXME: Rename - this is the reserved (total) buffer size (in bytes.)
	inline s32         getBufferSize()        const { return m_bufferSize;     }
	
	void        reservePrimitiveCount(s32 p_primitiveCount, bool p_copyOldData);
	
	inline void clear() { m_vertexCount = 0; }
	
	// status
	inline s32 getUVSetCount() const { return m_UVSetCount; }
	
	//inline s32 getReservedPrimitiveCountForCurrentUVSets() const
	//	{ return m_bufferSize / (getVertexSize() * getVerticesPerPrimitive()); }
	
	/* \brief Get the number of primitives currently in the buffer */
	inline s32 getPrimitiveCount() const { return m_vertexCount / getVerticesPerPrimitive(); }
	
	/* \brief Get the size of one vertex non-templated */
	inline s32 getVertexSize() const
		{ return sizeof(BufferVtxUV<1>) + sizeof(math::Vector2) * (m_UVSetCount - 1); }
	
	/* \brief Get the number of vertices per primitive */
	inline s32 getVerticesPerPrimitive() const { return m_verticesPerPrimitive; }
	
	/* \brief Get the number of vertices in total. */
	inline s32 getTotalVerticesCount() const { return m_vertexCount; }
	
private:
	//no copying
	PrimitiveCollectionBuffer(const PrimitiveCollectionBuffer&);
	const PrimitiveCollectionBuffer& operator=(const PrimitiveCollectionBuffer&);
	
	/*! \brief Copies the actual data to this PrimitiveCollectionBuffer
	    \param p_source                Buffer to copy from
	    \param p_sourceVertexCount     The number of vertices in the buffer
	    \param p_startIndexDestination Where to start copying this buffer */
	template <s32 UVSetCount>
	void setCollection(const void* p_source, s32 p_sourceVertexCount, s32 p_startIndexDestination = 0);
	
	void* m_buffer;   // The raw buffer.
	s32 m_bufferSize; // The size of the raw buffer. (In bytes.)
	
	s32 m_vertexCount; // Usage count of buffer. (In number of vertices.)
	
	s32 m_UVSetCount;                 // The number of UVsets per vertex.      (Used to get size of a vertex.)
	const s32 m_verticesPerPrimitive; // The number of vertices per primitive. (Used for indexing a vertex per primitive.)
	
};


// Namespace end
}
}
}

#include "PrimitiveCollectionBuffer.inl"

#endif // INC_TT_ENGINE_RENDERER_BATCHTRIANGLECOLLECTIONBUFFER_H
