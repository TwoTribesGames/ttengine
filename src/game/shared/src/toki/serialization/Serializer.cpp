#include <tt/code/bufferutils.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>

#include <toki/serialization/Serializer.h>


namespace toki {
namespace serialization {

const tt::code::Buffer::size_type Serializer::ms_defaultInitialSize = 1024 * 1024;
const tt::code::Buffer::size_type Serializer::ms_defaultGrowBy      = 32 * 1024;


//--------------------------------------------------------------------------------------------------
// Public member functions

Serializer::Serializer()
:
m_buffer(tt::code::AutoGrowBuffer::create(ms_defaultInitialSize, ms_defaultGrowBy))
{
}


tt::code::BufferWriteContext Serializer::getAppendContext()
{
	return m_buffer->getAppendContext();
}


tt::code::BufferReadContext Serializer::getReadContext() const
{
	return m_buffer->getReadContext();
}


u32 Serializer::getSize() const
{
	return static_cast<u32>(m_buffer->getUsedSize());
}


bool Serializer::saveToBuffer(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(getSize(), p_context);
	
	// Save all data blocks
	const s32 blockCount = m_buffer->getBlockCount();
	for (s32 i = 0; i < blockCount; ++i)
	{
		const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(m_buffer->getBlockSize(i));
		const u8* data = reinterpret_cast<const u8*>(m_buffer->getBlock(i));
		bu::put(data, blockSize, p_context);
	}
	
	return p_context->statusCode == 0;
}


SerializerPtr Serializer::clone() const
{
	return SerializerPtr(new Serializer(*this));
}


SerializerPtr Serializer::createFromBuffer(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	u32 size = bu::get<u32>(p_context);
	
	if (size == 0)
	{
		// Special case for size zero. This section is empty, return null pointer. (client code is expected to check for this.)
		return SerializerPtr();
	}
	
	tt::code::AutoGrowBufferPtr buffer = tt::code::AutoGrowBuffer::create(
			static_cast<tt::code::Buffer::size_type>(size),
			ms_defaultGrowBy);
	
	if (buffer == 0)
	{
		TT_PANIC("Could not create new serializer section (of %u bytes) from serialization data.", size);
		return SerializerPtr();
	}
	
	// Write data from read context to the autogrowbuffer's writecontext
	tt::code::BufferWriteContext writeContext = buffer->getAppendContext();
	bu::copyRaw(p_context, size, &writeContext);
	if (writeContext.flush() != 0)
	{
		TT_PANIC("Cannot flush write context");
		return SerializerPtr();
	}
	
	return SerializerPtr(new Serializer(buffer));
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Serializer::Serializer(const tt::code::AutoGrowBufferPtr& p_buffer)
:
m_buffer(p_buffer)
{
}


Serializer::Serializer(const Serializer& p_rhs)
:
m_buffer(tt::code::AutoGrowBuffer::create(
		(p_rhs.m_buffer->getUsedSize() == 0) ? ms_defaultInitialSize : p_rhs.m_buffer->getUsedSize(),
		ms_defaultGrowBy))
{
	TT_NULL_ASSERT(m_buffer);
	
	const tt::code::Buffer::size_type sourceSize = p_rhs.m_buffer->getUsedSize();
	if (sourceSize > 0)
	{
		namespace bu = tt::code::bufferutils;
		tt::code::BufferWriteContext writeContext  = m_buffer->getAppendContext();
		tt::code::BufferReadContext  sourceContext = p_rhs.m_buffer->getReadContext();
		bu::copyRaw(&sourceContext, sourceSize, &writeContext);
		writeContext.flush();
	}
}

// Namespace end
}
}
