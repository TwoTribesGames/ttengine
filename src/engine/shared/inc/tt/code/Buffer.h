#ifndef INC_TT_CODE_BUFFER_H
#define INC_TT_CODE_BUFFER_H


#include <tt/code/fwd.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

class Buffer
{
public:
	typedef int size_type;
	
	/*! \brief Creates a new buffer of specified size. */
	Buffer(size_type p_size, s32 p_alignment = 4, s32 p_allocType = 0);
	
	/*! \brief Points to an existing piece of memory. WARNING: Does not take ownership! */
	Buffer(void* p_data, size_type p_size);
	~Buffer();
	
	inline size_type getSize() const { return m_size; }
	inline void setSize(size_type p_size)
	{
		TT_ASSERT(p_size <= m_size);
		m_size = p_size;
	}
	
	inline void* getData() { return m_data; }
	inline const void* getData() const { return m_data; }
	
	/*! \brief Returns a context to read from the beginning of the buffer. */
	BufferReadContext getReadContext() const;
	
	/*! \brief Returns a context to write to the beginning of the buffer. */
	BufferWriteContext getWriteContext() const;
	
	/*! \brief Clones the buffer. */
	BufferPtrForCreator clone() const;
	
private:
	// Copy constructor used for clone
	Buffer(const Buffer& p_rhs);
	
	// No assignment
	Buffer& operator=(const Buffer&);
	
	bool allocate(size_type p_size, s32 p_alignment, s32 p_allocType);
	
	void*      m_data;
	size_type  m_size;
	const bool m_freeData;
	const s32  m_alignment;
	const s32  m_allocType;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_BUFFER_H)
