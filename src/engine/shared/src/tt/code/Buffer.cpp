#include <tt/code/Buffer.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace code {

//--------------------------------------------------------------------------------------------------
// Public member functions

Buffer::Buffer(size_type p_size, s32 p_alignment, s32 p_allocType)
:
m_data(0),
m_size(0),
m_freeData(true),
m_alignment(p_alignment),
m_allocType(p_allocType)
{
	allocate(p_size, p_alignment, p_allocType);
}


Buffer::Buffer(void* p_data, size_type p_size)
:
m_data(p_data),
m_size(p_size),
m_freeData(false),
m_alignment(-1),
m_allocType(-1)
{
}


Buffer::~Buffer()
{
	if (m_freeData)
	{
		mem::free(m_data);
	}
}


BufferReadContext Buffer::getReadContext() const
{
	return BufferReadContext::createForRawBuffer(reinterpret_cast<const u8*>(m_data),
	                                             static_cast<size_t>(m_size));
}


BufferWriteContext Buffer::getWriteContext() const
{
	return BufferWriteContext::createForRawBuffer(reinterpret_cast<u8*>(m_data),
	                                              static_cast<size_t>(m_size));
}


BufferPtrForCreator Buffer::clone() const
{
	return BufferPtrForCreator(new Buffer(*this));
}

//--------------------------------------------------------------------------------------------------
// Private member functions

Buffer::Buffer(const Buffer& p_rhs)
:
m_data     (0),  // NOTE: Will be set in the constructor body
m_size     (0),  // NOTE: Will be set in the constructor body
m_freeData (p_rhs.m_freeData),
m_alignment(p_rhs.m_alignment),
m_allocType(p_rhs.m_allocType)
{
	if (m_freeData == false)
	{
		// Simply copy pointer
		m_data = p_rhs.m_data;
		return;
	}
	
	// Allocate and copy data
	if (allocate(p_rhs.m_size, m_alignment, m_allocType))
	{
		mem::copy8(m_data, p_rhs.m_data, p_rhs.m_size);
	}
}


bool Buffer::allocate(size_type p_size, s32 p_alignment, s32 p_allocType)
{
	TT_ASSERT(m_data == 0);
	
	m_size = 0;
	
	if (p_size == 0)
	{
		return true;
	}
	
	m_data = mem::alloc(static_cast<mem::size_type>(p_size),
	                    static_cast<mem::size_type>(p_alignment),
	                    0,
	                    p_allocType);
	
	TT_ASSERTMSG(m_data != 0, "Unable to allocate %d bytes.", p_size);
	if (m_data != 0)
	{
		m_size = p_size;
		return true;
	}
	
	return false;
}

// Namespace end
}
}
