#include <tt/code/AutoGrowBuffer.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>


namespace tt {
namespace code {

//--------------------------------------------------------------------------------------------------
// Public member functions

AutoGrowBufferPtr AutoGrowBuffer::create(Buffer::size_type p_initialSize,
                                         Buffer::size_type p_growBy,
                                         s32               p_align,
                                         s32               p_allocType)
{
	if (p_initialSize <= 0)
	{
		TT_PANIC("Invalid initial buffer size specified: %d (must be at least 1).", p_initialSize);
		return AutoGrowBufferPtr();
	}
	
	if (p_growBy <= 0)
	{
		TT_PANIC("Invalid size specified to grow the buffer by: %d (must be at least 1).", p_growBy);
		return AutoGrowBufferPtr();
	}
	
	return AutoGrowBufferPtr(new AutoGrowBuffer(p_initialSize, p_growBy, p_align, p_allocType));
}


AutoGrowBufferPtr AutoGrowBuffer::createPrePopulated(const u8*         p_existingData,
                                                     Buffer::size_type p_existingDataSize,
                                                     Buffer::size_type p_growBy,
                                                     s32               p_align,
                                                     s32               p_allocType)
{
	TT_NULL_ASSERT(p_existingData);
	if (p_existingData == 0)
	{
		return AutoGrowBufferPtr();
	}
	
	AutoGrowBufferPtr buffer = create(p_existingDataSize, p_growBy, p_align, p_allocType);
	if (buffer == 0)
	{
		return AutoGrowBufferPtr();
	}
	
	TT_ASSERT(buffer->m_blocks.empty() == false);
	
	BufferPtrForCreator firstBlock = buffer->m_blocks.front();
	TT_NULL_ASSERT(firstBlock);
	TT_ASSERT(firstBlock->getSize() >= p_existingDataSize);
	
	mem::copy8(firstBlock->getData(), p_existingData,
	           static_cast<tt::mem::size_type>(p_existingDataSize));
	
	buffer->m_usedSize       = p_existingDataSize;
	buffer->m_posInLastBlock = p_existingDataSize;
	
	return buffer;
}


AutoGrowBuffer::~AutoGrowBuffer()
{
}


BufferWriteContext AutoGrowBuffer::getAppendContext()
{
	BufferPtrForCreator lastBlock(m_blocks.back());
	
	u8* blockStart = reinterpret_cast<u8*>(lastBlock->getData());
	
	BufferWriteContext context;
	context.start       = blockStart + m_posInLastBlock;
	context.end         = blockStart + lastBlock->getSize();
	context.cursor      = context.start;
	context.statusCode  = 0;
	context.flushFunc   = AutoGrowBuffer::flushBuffer;
	context.refillFunc  = AutoGrowBuffer::growBuffer;
	context.userContext = this;
	
	return context;
}


BufferReadContext AutoGrowBuffer::getReadContext() const
{
	BufferPtrForCreator firstBlock(m_blocks.front());
	
	const u8* blockStart = reinterpret_cast<const u8*>(firstBlock->getData());
	
	BufferReadContext context;
	context.start       = blockStart;
	context.end         = blockStart + getBlockSize(0);
	context.cursor      = context.start;
	context.statusCode  = 0;
	context.refillFunc  = AutoGrowBuffer::getNextReadBuffer;
	context.userContext = (void*)this;
	
	return context;
}


s32 AutoGrowBuffer::getBlockCount() const
{
	return static_cast<s32>(m_blocks.size());
}


const void* AutoGrowBuffer::getBlock(s32 p_index) const
{
	if (p_index < 0 || p_index >= getBlockCount())
	{
		TT_PANIC("Invalid block index specified: %d. Must be in range [0 - %d).",
		         p_index, getBlockCount());
		return 0;
	}
	
	return m_blocks.at(static_cast<Blocks::size_type>(p_index))->getData();
}


Buffer::size_type AutoGrowBuffer::getBlockSize(s32 p_index) const
{
	const s32 blockCount = getBlockCount();
	if (p_index < 0 || p_index >= blockCount)
	{
		TT_PANIC("Invalid block index specified: %d. Must be in range [0 - %d).",
		         p_index, blockCount);
		return 0;
	}
	
	// Special case for last block: only report the used bytes
	if (p_index == blockCount - 1)
	{
		return m_posInLastBlock;
	}
	
	return m_blocks.at(static_cast<Blocks::size_type>(p_index))->getSize();
}


AutoGrowBufferPtr AutoGrowBuffer::clone() const
{
	AutoGrowBufferPtr result(new AutoGrowBuffer(*this));
	TT_ASSERT(isEqual(result));
	return result;
}


void AutoGrowBuffer::clear()
{
	m_usedSize = 0;
	m_posInLastBlock = 0;
	m_blocks.resize(1);
}


bool AutoGrowBuffer::isEqual(const AutoGrowBufferPtr& p_rhs) const
{
	if (m_usedSize       != p_rhs->m_usedSize)       return false;
	if (m_growBy         != p_rhs->m_growBy)         return false;
	if (m_alignment      != p_rhs->m_alignment)      return false;
	if (m_allocType      != p_rhs->m_allocType)      return false;
	if (m_posInLastBlock != p_rhs->m_posInLastBlock) return false;
	
	const s32 blockCount = getBlockCount();
	if (blockCount != p_rhs->getBlockCount()) return false;
	
	// All member are equal, compare the contents of the blocks
	for (s32 i = 0; i < blockCount; ++i)
	{
		Buffer::size_type blockSize(getBlockSize(i));
		if (blockSize != p_rhs->getBlockSize(i))
		{
			return false;
		}
		
		// Size equal, compare actual data
		const void* block1 = getBlock(i);
		const void* block2 = p_rhs->getBlock(i);
		if (std::memcmp(block1, block2, blockSize) != 0)
		{
			return false;
		}
	}
	
	return true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

AutoGrowBuffer::AutoGrowBuffer(Buffer::size_type p_initialSize, Buffer::size_type p_growBy,
                               s32 p_align, s32 p_allocType)
:
m_blocks(),
m_growBy(p_growBy),
m_alignment(p_align),
m_allocType(p_allocType),
m_usedSize(0),
m_posInLastBlock(0)
{
	m_blocks.emplace_back(new Buffer(p_initialSize, m_alignment, m_allocType));
}


AutoGrowBuffer::AutoGrowBuffer(const AutoGrowBuffer& p_rhs)
:
m_blocks        (),  // NOTE: Will be copied in the constructor body
m_growBy        (p_rhs.m_growBy),
m_alignment     (p_rhs.m_alignment),
m_allocType     (p_rhs.m_allocType),
m_usedSize      (p_rhs.m_usedSize),
m_posInLastBlock(p_rhs.m_posInLastBlock)
{
	// Clone all blocks
	m_blocks.reserve(p_rhs.m_blocks.size());
	for (Blocks::const_iterator it = p_rhs.m_blocks.begin(); it != p_rhs.m_blocks.end(); ++it)
	{
		m_blocks.emplace_back((*it)->clone());
	}
}


BufferWriteContext::StatusCode AutoGrowBuffer::flushBuffer(BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	if (p_context == 0)
	{
		return 1;
	}
	
	TT_NULL_ASSERT(p_context->userContext);
	if (p_context->userContext == 0)
	{
		return 1;
	}
	
	AutoGrowBuffer* buffer = reinterpret_cast<AutoGrowBuffer*>(p_context->userContext);
	
	// Update the "used bytes" count
	Buffer::size_type capacity = 0;
	for (Blocks::iterator it = buffer->m_blocks.begin(); it != buffer->m_blocks.end(); ++it)
	{
		capacity += (*it)->getSize();
	}
	buffer->m_usedSize = static_cast<Buffer::size_type>(capacity - (p_context->end - p_context->cursor));
	
	buffer->m_posInLastBlock = static_cast<Buffer::size_type>(p_context->cursor - p_context->start);
	
	return 0;
}


BufferWriteContext::StatusCode AutoGrowBuffer::growBuffer(BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	if (p_context == 0)
	{
		return 1;
	}
	
	TT_NULL_ASSERT(p_context->userContext);
	if (p_context->userContext == 0)
	{
		return 1;
	}
	
	TT_ASSERTMSG(p_context->cursor == p_context->end, "Should only refill if buffer is full.");
	
	AutoGrowBuffer* buffer = reinterpret_cast<AutoGrowBuffer*>(p_context->userContext);
	
	buffer->m_blocks.emplace_back(new Buffer(buffer->m_growBy, buffer->m_alignment, buffer->m_allocType));
	Buffer* block(buffer->m_blocks.back().get());
	
	p_context->start  = reinterpret_cast<u8*>(block->getData());
	p_context->cursor = p_context->start;
	p_context->end    = p_context->start + block->getSize();
	
	buffer->m_posInLastBlock = 0;
	
	return 0;
}


BufferReadContext::StatusCode AutoGrowBuffer::getNextReadBuffer(BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	if (p_context == 0)
	{
		return 1;
	}
	
	TT_NULL_ASSERT(p_context->userContext);
	if (p_context->userContext == 0)
	{
		return 1;
	}
	
	AutoGrowBuffer* autoGrowBuffer = reinterpret_cast<AutoGrowBuffer*>(p_context->userContext);
	
	// Find the current block based on the data address
	Blocks::const_iterator blockIt;
	s32                    blockIndex = 0;
	for (blockIt = autoGrowBuffer->m_blocks.begin();
	     blockIt != autoGrowBuffer->m_blocks.end(); ++blockIt, ++blockIndex)
	{
		if ((*blockIt)->getData() == p_context->start)
		{
			// Found the current block
			break;
		}
	}
	
	if (blockIt == autoGrowBuffer->m_blocks.end())
	{
		// Block for current buffer pointer wasn't found
		return 1;
	}
	
	// Move on to the next block
	++blockIt;
	++blockIndex;
	if (blockIt == autoGrowBuffer->m_blocks.end())
	{
		// There is no next block
		return 1;
	}
	
	// Set up the context pointers for the next buffer block
	const BufferPtrForCreator& block(*blockIt);
	const u8*                  blockStart = reinterpret_cast<const u8*>(block->getData());
	
	p_context->start  = blockStart;
	p_context->end    = blockStart + autoGrowBuffer->getBlockSize(blockIndex);
	p_context->cursor = p_context->start;
	
	return 0;
}

// Namespace end
}
}
