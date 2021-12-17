#if !defined(INC_TT_CODE_BUFFERUTILS_H)
#define INC_TT_CODE_BUFFERUTILS_H


#include <tt/code/bufferutils_be_get.h>
#include <tt/code/bufferutils_be_put.h>
#include <tt/code/bufferutils_get.h>
#include <tt/code/bufferutils_put.h>


/**
 * These are type safe utility functions to get a specific type from/to a buffer.
 * It will also check for buffer overflows. ( By asserting in debug code and returning 0 in release code. )
 * 
 * 
 * 
 * Example code for writing to a buffer.
 * 
 * \brief Write header information to specified buffer.
 * \param p_buffer Pointer to buffer in which the header must be write
 * \param p_buffer_size The size of the buffer specified in p_buffer
 * \param p_receiver, p_type, p_length Information which should be writing into the buffer.
 * \return The size of data which was writen.
size_t createHeader(u8* p_buffer, 
                    size_t p_buffer_size,
                    msg_receiver p_receiver,
                    msg_type p_type,
                    int p_length) const
{
	u8* buf_ptr = p_buffer;
	size_t bytes_left = p_buffer_size;
	size_t header_size_count = 0;
	
	
	TT_ASSERTMSG(p_receiver == 0 || (p_receiver & 0xFFFFFF00) == 0, "p_receiver doesn't fit in a s8");
	put<s8>(p_receiver, buf_ptr, bytes_left, header_size_count);
	TT_ASSERTMSG(p_type == 0 || (p_type & 0xFFFFFF00) == 0, "p_type doesn't fit in a s8");
	put<s8>( p_type,    buf_ptr, bytes_left, header_size_count);
	put<int>(p_length,  buf_ptr, bytes_left, header_size_count);
	
	return header_size_count;
}


 * Example code for reading from a buffer.
 * 
 * \brief Read header information from specified buffer.
 * \param p_buffer Pointer to buffer from which to read the header data.
 * \param p_buffer_size The size of the buffer specified in p_buffer.
 * \param p_receiver, p_type, p_length References to where header information should be writen to.
void parseHeader(const u8* p_buffer, 
                 size_t p_buffer_size,
                 msg_receiver& p_receiver, 
                 msg_type& p_type,
                 int& p_length) const
{
	const u8* buf_ptr = p_buffer;
	size_t bytes_left = p_buffer_size;
	
	p_receiver = static_cast<msg_receiver>(get<s8>(buf_ptr, bytes_left));
	p_type     = static_cast<msg_type>(    get<s8>(buf_ptr, bytes_left));
	p_length   = get<int>(buf_ptr, bytes_left);
}

*/


namespace tt {
namespace code {
namespace bufferutils {

//--------------------------------------------------------------------------------------------------
// Legacy functions (kept for backwards compatibility, but implemented in terms of the new functions):


/*! \brief Reads a wide character string from the specified buffer.
           The string length is first read from the buffer, as a 16-bit unsigned integer in big endian format.
           The string is read after that, as 16-bit wide characters in big endian format.
    \param p_buffer The buffer to read from. Pointer will point to one past the string after it has been read.
    \param p_remainingBytes The remaining number of bytes in the buffer. Will be updated.
    \return The string that was read from the buffer. */
inline std::wstring getWideString(const u8*& p_buffer, size_t& p_remainingBytes)
{
	return be_get<std::wstring>(p_buffer, p_remainingBytes);
}


inline std::wstring getWideString(BufferReadContext* p_context)
{
	return be_get<std::wstring>(p_context);
}


/*! \brief Writes a wide character string to the specified buffer.
           The string length is first written to the buffer, as a 16-bit unsigned integer in big endian format.
           The string is written after that, as 16-bit wide characters in big endian format (without null terminator).
    \param p_value The string to write.
    \param p_buffer The buffer to write to.
    \param p_remainingBytes The remaining number of bytes in the buffer (will be updated). */
inline void putWideString(const std::wstring& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value, p_buffer, p_remainingBytes);
}


inline void putWideString(const std::wstring& p_value, BufferWriteContext* p_context)
{
	be_put(p_value, p_context);
}


/*! \brief Reads a narrow character string from the specified buffer.
           The string length is first read from the buffer, as a 16-bit unsigned integer in big endian format.
           The string is read after that, as 8-bit characters.
    \param p_buffer The buffer to read from. Pointer will point to one past the string after it has been read.
    \param p_remainingBytes The remaining number of bytes in the buffer. Will be updated.
    \return The string that was read from the buffer. */
inline std::string getNarrowString(const u8*& p_buffer, size_t& p_remainingBytes)
{
	return be_get<std::string>(p_buffer, p_remainingBytes);
}


inline std::string getNarrowString(BufferReadContext* p_context)
{
	return be_get<std::string>(p_context);
}


/*! \brief Writes a narrow character string to the specified buffer.
           The string length is first written to the buffer, as a 16-bit unsigned integer in big endian format.
           The string is written after that, as 8-bit characters (without null terminator).
    \param p_value The string to write.
    \param p_buffer The buffer to write to.
    \param p_remainingBytes The remaining number of bytes in the buffer (will be updated). */
inline void putNarrowString(const std::string& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value, p_buffer, p_remainingBytes);
}


inline void putNarrowString(const std::string& p_value, BufferWriteContext* p_context)
{
	be_put(p_value, p_context);
}


/*! \brief Copies a number of bytes from a BufferReadContext to a BufferWriteContext
    \param p_srcContext The BufferReadContext to read from.
    \param p_dataSize The number of bytes to copy.
    \param p_dstContext The BufferWriteContext to write to.*/
inline void copyRaw(BufferReadContext* p_srcContext, size_t p_dataSize, BufferWriteContext* p_dstContext)
{
	while (p_dataSize > 0)
	{
		if (p_dstContext->needsRefill() && p_dstContext->refill() != 0)
		{
			p_dstContext->statusCode = 1;
			return;
		}
		
		if (p_srcContext->needsRefill() && p_srcContext->refill() != 0)
		{
			TT_PANIC("Can't get data from read buffer because not enough bytes are remaining "
			         "(or refilling the read buffer failed).\nNumber of bytes to read: %u",
			         p_dataSize);
			p_srcContext->statusCode = 1;
			return;
		}
		
		// Calculate bytes remaining in current context
		const size_t writeBytesRemaining(p_dstContext->end - p_dstContext->cursor);
		const size_t readBytesRemaining (p_srcContext->end - p_srcContext->cursor);
		const size_t bytesRemaining(std::min(writeBytesRemaining, readBytesRemaining));
		const size_t bytesToCopy(std::min(bytesRemaining, p_dataSize));
		
		mem::copy8(p_dstContext->cursor, p_srcContext->cursor, static_cast<mem::size_type>(bytesToCopy));
		p_dataSize           -= bytesToCopy;
		p_srcContext->cursor += bytesToCopy;
		p_dstContext->cursor += bytesToCopy;
	}
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_CODE_BUFFERUTILS_H)
