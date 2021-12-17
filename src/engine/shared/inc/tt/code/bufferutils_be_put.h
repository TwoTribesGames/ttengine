#if !defined(INC_TT_CODE_BUFFERUTILS_BE_PUT_H)
#define INC_TT_CODE_BUFFERUTILS_BE_PUT_H


#include <string>

#include <tt/code/BufferWriteContext.h>
#include <tt/math/Point2.h>
#include <tt/math/Point3.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_compile_time_error.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {
namespace bufferutils {

//--------------------------------------------------------------------------------------------------
// Buffer utils functions for writing data to a buffer in big endian format


/*! \brief Puts a number in a buffer in big endian.
    \param p_value Number to put in buffer.
    \param p_buffer Pointer to the buffer,
                    gets incremented after successful write.
    \param p_remainingBytes Number of bytes remaining in buffer,
                            gets decremented after successful write.*/
template<typename T>
inline void be_put(T p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	if (p_remainingBytes < sizeof(T))
	{
		TT_PANIC("Can't place type in buffer because not enough bytes are remaining.\n"
		         "Remaining bytes: %u, size of type: %u",
		         p_remainingBytes, sizeof(T));
		
		p_remainingBytes = 0;
		return;
	}
	p_remainingBytes -= sizeof(T);
	
	// write in big endian (Motorola)
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		p_buffer[sizeof(T) - (i + 1)] = static_cast<u8>((p_value >> (i * 8)) & 0xFF);
	}
	
	p_buffer += sizeof(T);
}


template<typename T>
inline void be_put(T p_value, BufferWriteContext* p_context)
{
	// write in big endian (Motorola)
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			p_context->statusCode = 1;
			return;
		}
		
		*(p_context->cursor) = static_cast<u8>((p_value >> ((sizeof(T) - (i + 1)) * 8)) & 0xFF);
		++(p_context->cursor);
	}
}


template<>
inline void be_put<bool>(bool p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(static_cast<u8>(p_value ? 1 : 0), p_buffer, p_remainingBytes);
}


template<>
inline void be_put<bool>(bool p_value, BufferWriteContext* p_context)
{
	be_put(static_cast<u8>(p_value ? 1 : 0), p_context);
}


template<>
inline void be_put<real>(real p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	float f     = realToFloat(p_value);
	u32   value = 0;
	
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(float));
	mem::copy8(&value, &f, sizeof(u32));
	
	be_put(value, p_buffer, p_remainingBytes);
}


template<>
inline void be_put<real>(real p_value, BufferWriteContext* p_context)
{
	float f     = realToFloat(p_value);
	u32   value = 0;
	
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(float));
	mem::copy8(&value, &f, sizeof(u32));
	
	be_put(value, p_context);
}


template<>
inline void be_put<real64>(real64 p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	double f     = realToFloat(p_value);
	u64    value = 0;
	
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(double));
	mem::copy8(&value, &f, sizeof(u64));
	
	be_put(value, p_buffer, p_remainingBytes);
}


template<>
inline void be_put<real64>(real64 p_value, BufferWriteContext* p_context)
{
	double f     = realToFloat(p_value);
	u64    value = 0;
	
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(double));
	mem::copy8(&value, &f, sizeof(u64));
	
	be_put(value, p_context);
}


inline void be_put(const std::string& p_string, u8*& p_buffer, size_t& p_remainingBytes)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_string.length());
	be_put(stringByteLength, p_buffer, p_remainingBytes);
	
	// Write out the individual characters of the string
	for (std::string::const_iterator it = p_string.begin(); it != p_string.end(); ++it)
	{
		be_put(static_cast<u8>(*it), p_buffer, p_remainingBytes);
	}
}


inline void be_put(const std::string& p_string, BufferWriteContext* p_context)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_string.length());
	be_put(stringByteLength, p_context);
	
	// Write out the individual characters of the string
	for (std::string::const_iterator it = p_string.begin(); it != p_string.end(); ++it)
	{
		be_put(static_cast<u8>(*it), p_context);
	}
}


template<>
inline void be_put<const char*>(const char* p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(std::string(p_value), p_buffer, p_remainingBytes);
}


template<>
inline void be_put<const char*>(const char* p_value, BufferWriteContext* p_context)
{
	be_put(std::string(p_value), p_context);
}


inline void be_put(const std::wstring& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_value.length() * 2);
	be_put(stringByteLength, p_buffer, p_remainingBytes);
	
	// Write out the individual characters of the string
	for (std::wstring::const_iterator it = p_value.begin(); it != p_value.end(); ++it)
	{
		be_put(static_cast<u16>(*it), p_buffer, p_remainingBytes);
	}
}


inline void be_put(const std::wstring& p_value, BufferWriteContext* p_context)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_value.length() * 2);
	be_put(stringByteLength, p_context);
	
	// Write out the individual characters of the string
	for (std::wstring::const_iterator it = p_value.begin(); it != p_value.end(); ++it)
	{
		be_put(static_cast<u16>(*it), p_context);
	}
}


template<>
inline void be_put<const wchar_t*>(const wchar_t* p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(std::wstring(p_value), p_buffer, p_remainingBytes);
}


template<>
inline void be_put<const wchar_t*>(const wchar_t* p_value, BufferWriteContext* p_context)
{
	be_put(std::wstring(p_value), p_context);
}


inline void be_put(const math::Vector2& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value.x, p_buffer, p_remainingBytes);
	be_put(p_value.y, p_buffer, p_remainingBytes);
}


inline void be_put(const math::Vector2& p_value, BufferWriteContext* p_context)
{
	be_put(p_value.x, p_context);
	be_put(p_value.y, p_context);
}


inline void be_put(const math::Vector3& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value.x, p_buffer, p_remainingBytes);
	be_put(p_value.y, p_buffer, p_remainingBytes);
	be_put(p_value.z, p_buffer, p_remainingBytes);
}


inline void be_put(const math::Vector3& p_value, BufferWriteContext* p_context)
{
	be_put(p_value.x, p_context);
	be_put(p_value.y, p_context);
	be_put(p_value.z, p_context);
}


inline void be_put(const math::Point2& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value.x, p_buffer, p_remainingBytes);
	be_put(p_value.y, p_buffer, p_remainingBytes);
}


inline void be_put(const math::Point2& p_value, BufferWriteContext* p_context)
{
	be_put(p_value.x, p_context);
	be_put(p_value.y, p_context);
}


inline void be_put(const math::Point3& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value.x, p_buffer, p_remainingBytes);
	be_put(p_value.y, p_buffer, p_remainingBytes);
	be_put(p_value.z, p_buffer, p_remainingBytes);
}


inline void be_put(const math::Point3& p_value, BufferWriteContext* p_context)
{
	be_put(p_value.x, p_context);
	be_put(p_value.y, p_context);
	be_put(p_value.z, p_context);
}


template< typename InternalRectType, s32 inclusiveCorrection>
inline void be_put(const math::Rect<InternalRectType, inclusiveCorrection>& p_value,
                   u8*& p_buffer, size_t& p_remainingBytes)
{
	be_put(p_value.getPosition(), p_buffer, p_remainingBytes);
	be_put(p_value.getWidth(),    p_buffer, p_remainingBytes);
	be_put(p_value.getHeight(),   p_buffer, p_remainingBytes);
}


template< typename InternalRectType, s32 inclusiveCorrection>
inline void be_put(const math::Rect<InternalRectType, inclusiveCorrection>& p_value, BufferWriteContext* p_context)
{
	be_put(p_value.getPosition(), p_context);
	be_put(p_value.getWidth(),    p_context);
	be_put(p_value.getHeight(),   p_context);
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_CODE_BUFFERUTILS_BE_PUT_H)
