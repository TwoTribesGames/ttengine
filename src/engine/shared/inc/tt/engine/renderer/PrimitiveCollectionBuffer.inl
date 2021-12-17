#include <algorithm>

#include <tt/math/math.h>
#include <tt/mem/util.h>

namespace tt {
namespace engine {
namespace renderer {


template <s32 UVSetCount>
inline void PrimitiveCollectionBuffer::setCollection(const std::vector<BatchTriangleUV<UVSetCount> >& p_source,
                                                     s32 p_startIndexDestination)
{
	TT_ASSERT(UVSetCount == m_UVSetCount);
	const s32 vertexCount = static_cast<s32>(getVerticesPerPrimitive() * p_source.size());
	TT_ASSERT(vertexCount > 0);
	setCollection<UVSetCount>(static_cast<const void*>(&p_source[0]), vertexCount, p_startIndexDestination);
}


template <s32 UVSetCount>
inline void PrimitiveCollectionBuffer::setCollection(const std::vector<BufferVtxUV<UVSetCount> >& p_source, 
                                                     s32 p_startIndexDestination)
{
	TT_ASSERT(UVSetCount == m_UVSetCount);
	const s32 vertexCount = static_cast<s32>(getVerticesPerPrimitive() * p_source.size());
	TT_ASSERT(vertexCount > 0);
	setCollection<UVSetCount>(static_cast<const void*>(&p_source[0]), vertexCount, p_startIndexDestination);
}


template <s32 UVSetCount>
inline BufferVtxUV<UVSetCount>& PrimitiveCollectionBuffer::modifyVtx(s32 p_index)
{
	TT_ASSERT(UVSetCount == m_UVSetCount);
	// Check if index is valid.
	TT_ASSERTMSG(p_index >= 0 && p_index < m_vertexCount,
	             "Invalid index: %d. Current vertex (usage) count: %d. (Use resize function to fill with empty vertices!).",
	             p_index, m_vertexCount);
	
	typedef BufferVtxUV<UVSetCount> BufferVtxType;
	BufferVtxType* ptr = reinterpret_cast<BufferVtxType*>(m_buffer);
	
	TT_ASSERTMSG(reinterpret_cast<void*>(ptr) == m_buffer,
	             "Alligment issue with buffer!");
	
	return ptr[p_index];
}


template <s32 UVSetCount>
inline const BufferVtxUV<UVSetCount>& PrimitiveCollectionBuffer::getVtx(s32 p_index) const
{
	TT_ASSERT(UVSetCount == m_UVSetCount);
	// Check if index is valid.
	TT_ASSERTMSG(p_index >= 0 && p_index < m_vertexCount,
	             "Invalid index: %d. Current vertex (usage) count: %d.",
	             p_index, m_vertexCount);
	
	typedef BufferVtxUV<UVSetCount> BufferVtxType;
	const BufferVtxType* ptr = reinterpret_cast<const BufferVtxType*>(m_buffer);
	
	TT_ASSERTMSG(reinterpret_cast<const void*>(ptr) == m_buffer,
	             "Alligment issue with buffer!");
	
	return ptr[p_index];
}


template <s32 UVSetCount>
inline void PrimitiveCollectionBuffer::resize(s32 p_size, BufferVtxUV<UVSetCount> p_defaultValue)
{
	TT_ASSERT(UVSetCount == m_UVSetCount);
	TT_MIN_ASSERT(p_size, 0);
	
	if (p_size == m_vertexCount)
	{
		// Same size, done.
		return;
	}
	
	typedef BufferVtxUV<UVSetCount>    BufferVtxType;
	TT_STATIC_ASSERT(sizeof(p_defaultValue) == sizeof(BufferVtxType));
	
	const s32 neededSpace = p_size * sizeof(BufferVtxType);
	
	// Make sure it fits
	TT_MAX_ASSERT(neededSpace, m_bufferSize);
	
	if (p_size < m_vertexCount)
	{
		// Smaller, just lower these and keep the memory as it was.
		m_vertexCount    = p_size;
		return;
	}
	
	TT_ASSERT(p_size > m_vertexCount);
	// Needs to be made larger.
	
	BufferVtxType* vtxPtr = reinterpret_cast<BufferVtxType*>(m_buffer);
	
	// Fill with default values.
	for (s32 i = m_vertexCount; i < p_size; ++i)
	{
		vtxPtr[i] = p_defaultValue;
	}
	
	m_vertexCount    = p_size;
}


inline void PrimitiveCollectionBuffer::reservePrimitiveCount(s32 p_primitiveCount, bool p_copyOldData)
{
	TT_MIN_ASSERT(p_primitiveCount, 0);
	
	// Check if old buffer needs te be deleted
	if (p_primitiveCount <= 0 || p_copyOldData == false)
	{
		TT_ASSERTMSG(p_primitiveCount >= 0,
		             "p_primitiveCount (%d) can't be negative", p_primitiveCount);
		TT_ASSERTMSG(p_copyOldData == false || m_vertexCount == 0, 
		             "Resizing buffer to 0 with copyOldData = true but vertexCount is: %d."
		             "Vertex data will be lost!\n", m_vertexCount);
		
		::operator delete(m_buffer);
		
		m_bufferSize     = 0;
		m_buffer         = 0;
		m_vertexCount    = 0;
		
		if (p_primitiveCount <= 0)
		{
			// No new data to copy.
			return;
		}
	}
	
	void* oldBuffer = m_buffer;
	
	// Calculate new buffer size.
	const s32 vertexCount = p_primitiveCount * getVerticesPerPrimitive();
	m_bufferSize    = vertexCount * getVertexSize();
	
	// Allocate buffer with new buffer size (in bytes)
	m_buffer = ::operator new(m_bufferSize);
	
	// Do I have something to copy.
	if (m_vertexCount > 0) // NOTE: Check existing count, not new count
	{
		const s32 oldBufferSize = getRawVertexDataSize();
		s32 newSizeUsed = oldBufferSize;
		const bool wasClamped = math::clamp(newSizeUsed, s32(1), m_bufferSize);
		TT_ASSERTMSG(wasClamped == false,
		             "While resizing PrimitiveCollectionBuffer the new buffer size used needed to be clamped. "
		             "%d wasn't in the range 1 - bufferSize(%d).",
		             oldBufferSize, m_bufferSize);
		
		mem::copy8(m_buffer, oldBuffer, newSizeUsed);
		
		// Set the usage size
		m_vertexCount = std::min(vertexCount, m_vertexCount);
	}
	
	// Remove old buffer.
	::operator delete(oldBuffer);
}


template <s32 UVSetCount>
inline void PrimitiveCollectionBuffer::setCollection(const void* p_source, s32 p_sourceVertexCount, 
                                                     s32 p_startIndexDestination)
{
	TT_ASSERTMSG(p_source != 0 && p_sourceVertexCount > 0 && p_startIndexDestination >= 0,
	             "sanity check: one or more parameters had invalid states");
	
	if (m_UVSetCount != UVSetCount)
	{
		// different UV set count
		if (p_startIndexDestination == 0)
		{
			// update dependent members
			m_UVSetCount = UVSetCount;
		}
		else
		{
			TT_PANIC("partial buffer updates need to have the same number of UV coordinate sets"
			         "the buffer already contains (%d), trying to add with UVSetCoord (%d)",
			         m_UVSetCount, UVSetCount);
			return;
		}
	}
	
	m_vertexCount = p_sourceVertexCount + (getVerticesPerPrimitive() * p_startIndexDestination);
	
	typedef BufferVtxUV<UVSetCount>    BufferVtxType;
	typedef std::vector<BufferVtxType> Container;
	s32 sizeofBufferVtxType = sizeof(BufferVtxType);
	
	typename Container::size_type verticesNeeded = m_vertexCount;
	typename Container::size_type neededSpace    = verticesNeeded * sizeofBufferVtxType;
	
	// Check available size
	if(static_cast<typename Container::size_type>(m_bufferSize) < neededSpace)
	{
		TT_PANIC("Not enough space in buffer. Need %u bytes, have: %d.", neededSpace, m_bufferSize);
		return;
	}
	
	// Copy vertices
	mem::copy8(reinterpret_cast<BufferVtxType*>(m_buffer) + 
	           (getVerticesPerPrimitive() * p_startIndexDestination),
	           p_source, (p_sourceVertexCount * sizeofBufferVtxType));
}


// Namespace end
}
}
}

