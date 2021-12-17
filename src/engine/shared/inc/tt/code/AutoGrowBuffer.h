#if !defined(INC_TT_CODE_AUTOGROWBUFFER_H)
#define INC_TT_CODE_AUTOGROWBUFFER_H


#include <vector>

#include <tt/code/Buffer.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

class AutoGrowBuffer;
typedef tt_ptr<AutoGrowBuffer>::shared AutoGrowBufferPtr;


class AutoGrowBuffer
{
public:
	static AutoGrowBufferPtr create(Buffer::size_type p_initialSize, Buffer::size_type p_growBy,
	                                s32 p_align = 4, s32 p_allocType = 0);
	
	/*! \brief Creates a buffer pre-populated with data.
	    \param p_existingData     Data to pre-populate the buffer with (data will be copied).
	    \param p_existingDataSize Size of the p_existingData buffer. */
	static AutoGrowBufferPtr createPrePopulated(const u8* p_existingData,
	                                            Buffer::size_type p_existingDataSize,
	                                            Buffer::size_type p_growBy,
	                                            s32 p_align = 4, s32 p_allocType = 0);
	
	~AutoGrowBuffer();
	
	/*! \brief Returns a buffer context with which data can be appended to the end of this buffer. */
	BufferWriteContext getAppendContext();
	
	/*! \brief Returns a buffer context with which data can be read from this buffer.
	           Always starts at the beginning of the buffer. */
	BufferReadContext getReadContext() const;
	
	s32               getBlockCount()           const;
	const void*       getBlock(s32 p_index)     const;
	Buffer::size_type getBlockSize(s32 p_index) const;
	
	/*! \return Total number of bytes written to this buffer. */
	inline Buffer::size_type getUsedSize() const { return m_usedSize; }
	
	AutoGrowBufferPtr clone() const;
	
	void clear();
	
	bool isEqual(const AutoGrowBufferPtr& p_rhs) const;
	
private:
	typedef std::vector<BufferPtrForCreator> Blocks;
	
	
	AutoGrowBuffer(Buffer::size_type p_initialSize, Buffer::size_type p_growBy,
	               s32 p_align = 4, s32 p_allocType = 0);
	AutoGrowBuffer(const AutoGrowBuffer& p_rhs);
	
	static BufferWriteContext::StatusCode flushBuffer(BufferWriteContext* p_context);
	static BufferWriteContext::StatusCode growBuffer (BufferWriteContext* p_context);
	
	static BufferReadContext::StatusCode getNextReadBuffer(BufferReadContext* p_context);
	
	// No assignment
	AutoGrowBuffer& operator=(const AutoGrowBuffer&);
	
	
	Blocks                  m_blocks;
	const Buffer::size_type m_growBy; //!< Size of new buffers to create when the buffer needs to be expanded.
	const s32               m_alignment;
	const s32               m_allocType;
	Buffer::size_type       m_usedSize; //!< Total used byte count of all buffer blocks.
	Buffer::size_type       m_posInLastBlock;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_AUTOGROWBUFFER_H)
