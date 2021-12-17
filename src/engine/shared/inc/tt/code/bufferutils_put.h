#if !defined(INC_TT_CODE_BUFFERUTILS_PUT_H)
#define INC_TT_CODE_BUFFERUTILS_PUT_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/code/Handle.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/interpolation.h>
#include <tt/math/Point2.h>
#include <tt/math/Point3.h>
#include <tt/math/Rect.h>
#include <tt/math/TimedLinearInterpolation.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_compile_time_error.h>
#include <tt/platform/tt_endianess.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {
namespace bufferutils {

//--------------------------------------------------------------------------------------------------
// Buffer utils functions for writing data to a buffer in little endian format

inline void put(const u8* p_rawData, size_t p_rawDataSize, u8*& p_buffer, size_t& p_remainingBytes)
{
	if (p_remainingBytes < p_rawDataSize)
	{
		TT_PANIC("Can't store bytes in buffer, because not enough bytes are remaining.\n"
		         "Remaining bytes: %d, size of data to store: %d",
		         p_remainingBytes, p_rawDataSize);
		
		p_remainingBytes = 0;
		return;
	}
	
	memcpy(p_buffer, p_rawData, p_rawDataSize);
	p_remainingBytes -= p_rawDataSize;
	p_buffer         += p_rawDataSize;
}


inline void put(const u8* p_rawData, size_t p_rawDataSize, BufferWriteContext* p_context)
{
	if (p_rawDataSize < static_cast<size_t>(p_context->end - p_context->cursor))
	{
		// Early out
		memcpy(p_context->cursor, p_rawData, p_rawDataSize);
		p_context->cursor += p_rawDataSize;
		return;
	}
	
	while (p_rawDataSize > 0)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			p_context->statusCode = 1;
			return;
		}
		
		// Calculate bytes remaining in current context
		const size_t bytesRemaining(p_context->end - p_context->cursor);
		const size_t bytesToCopy(std::min(bytesRemaining, p_rawDataSize));
		
		memcpy(p_context->cursor, p_rawData, bytesToCopy);
		p_rawDataSize     -= bytesToCopy;
		p_rawData         += bytesToCopy;
		p_context->cursor += bytesToCopy;
	}
}


/*! \brief Puts a number in a buffer in little endian.
    \param p_value Number to put in buffer.
    \param p_buffer Pointer to the buffer,
                    gets incremented after successful write.
    \param p_remainingBytes Number of bytes remaining in buffer,
                            gets decremented after successful write.*/
template<typename T>
inline void put(T p_value, u8*& p_buffer, size_t& p_remainingBytes)
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
	
#if TT_PLATFORM_LITTLE_ENDIAN
	memcpy(p_buffer, &p_value, sizeof(T));
#else
	// write in little endian (Intel)
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		p_buffer[i] = static_cast<u8>((p_value >> (i * 8)) & 0xFF);
	}
#endif
	p_buffer += sizeof(T);
}


template<typename T>
inline void put(T p_value, BufferWriteContext* p_context)
{
#if TT_PLATFORM_LITTLE_ENDIAN
	u8* rawData = reinterpret_cast<u8*>(&p_value);
	size_t rawDataSize = sizeof(T);
	if (rawDataSize < static_cast<size_t>(p_context->end - p_context->cursor))
	{
		// Early out
		memcpy(p_context->cursor, rawData, rawDataSize);
		p_context->cursor += rawDataSize;
		return;
	}
	
	while (rawDataSize > 0)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			p_context->statusCode = 1;
			return;
		}
		
		// Calculate bytes remaining in current context
		const size_t bytesRemaining(p_context->end - p_context->cursor);
		const size_t bytesToCopy(std::min(bytesRemaining, rawDataSize));
		
		memcpy(p_context->cursor, rawData, bytesToCopy);
		rawDataSize       -= bytesToCopy;
		rawData           += bytesToCopy;
		p_context->cursor += bytesToCopy;
	}
#else
	// write in little endian (Intel)
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			p_context->statusCode = 1;
			return;
		}
		
		*(p_context->cursor) = static_cast<u8>((p_value >> (i * 8)) & 0xFF);
		++(p_context->cursor);
	}
#endif
}


template<>
inline void put<bool>(bool p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(static_cast<u8>(p_value), p_buffer, p_remainingBytes);
}


template<>
inline void put<bool>(bool p_value, BufferWriteContext* p_context)
{
	put(static_cast<u8>(p_value), p_context);
}


template<>
inline void put<real>(real p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(real));
	union
	{
		u32  intval;
		real floatval;
	};
	floatval = p_value;
	put(intval, p_buffer, p_remainingBytes);
}


template<>
inline void put<real>(real p_value, BufferWriteContext* p_context)
{
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(real));
	union
	{
		u32  intval;
		real floatval;
	};
	floatval = p_value;
	put(intval, p_context);
}


template<>
inline void put<real64>(real64 p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(real64));
	union
	{
		u64    intval;
		real64 floatval;
	};
	floatval = p_value;
	put(intval, p_buffer, p_remainingBytes);
}


template<>
inline void put<real64>(real64 p_value, BufferWriteContext* p_context)
{
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(real64));
	union
	{
		u64    intval;
		real64 floatval;
	};
	floatval = p_value;
	put(intval, p_context);
}


template<typename SizeType, typename EnumType>
inline void putEnum(EnumType p_value, BufferWriteContext* p_context)
{
	// Make sure we can put the max of SizeType in EnumType.
	TT_STATIC_ASSERT(sizeof(EnumType) >= sizeof(SizeType));
	
	TT_ASSERTMSG(p_value == static_cast<EnumType>(static_cast<SizeType>(p_value)),
	             "Enum value %d is changed after casting it to SizeType: %d. "
	             "(Maybe trying to store a negative number in an unsigned SizeType.)",
	             p_value, static_cast<EnumType>(static_cast<SizeType>(p_value)));
	
	return put(static_cast<SizeType>(p_value), p_context);
}


template<typename HandleType>
inline void putHandle(const code::Handle<HandleType>& p_value, BufferWriteContext* p_context)
{
	return put(p_value.getValue(), p_context);
}


template < class FlagEnum, s32 FlagCount >
inline void putBitMask(const BitMask<FlagEnum, FlagCount>& p_value, BufferWriteContext* p_context)
{
	return put(p_value.getFlags(), p_context);
}



inline void put(const std::string& p_string, u8*& p_buffer, size_t& p_remainingBytes)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_string.length());
	put(stringByteLength, p_buffer, p_remainingBytes);
	put(reinterpret_cast<const u8*>(p_string.c_str()), stringByteLength, p_buffer, p_remainingBytes);
}


inline void put(const std::string& p_string, BufferWriteContext* p_context)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_string.length());
	put(stringByteLength, p_context);
	put(reinterpret_cast<const u8*>(p_string.c_str()), stringByteLength, p_context);
}


template<>
inline void put<const char*>(const char* p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(std::string(p_value), p_buffer, p_remainingBytes);
}


template<>
inline void put<const char*>(const char* p_value, BufferWriteContext* p_context)
{
	put(std::string(p_value), p_context);
}


inline void put(const std::wstring& p_string, u8*& p_buffer, size_t& p_remainingBytes)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_string.length() * 2);
	put(stringByteLength, p_buffer, p_remainingBytes);
	
	// Write out the individual characters of the string
	for (std::wstring::const_iterator it = p_string.begin(); it != p_string.end(); ++it)
	{
		put(static_cast<u16>(*it), p_buffer, p_remainingBytes);
	}
}


inline void put(const std::wstring& p_string, BufferWriteContext* p_context)
{
	// Write out the string length (in bytes, without null terminator)
	const u16 stringByteLength = static_cast<u16>(p_string.length() * 2);
	put(stringByteLength, p_context);
	
	// Write out the individual characters of the string
	for (std::wstring::const_iterator it = p_string.begin(); it != p_string.end(); ++it)
	{
		put(static_cast<u16>(*it), p_context);
	}
}


template<>
inline void put<const wchar_t*>(const wchar_t* p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(std::wstring(p_value), p_buffer, p_remainingBytes);
}


template<>
inline void put<const wchar_t*>(const wchar_t* p_value, BufferWriteContext* p_context)
{
	put(std::wstring(p_value), p_context);
}


inline void put(const engine::renderer::ColorRGB& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.r, p_buffer, p_remainingBytes);
	put(p_value.g, p_buffer, p_remainingBytes);
	put(p_value.b, p_buffer, p_remainingBytes);
}


inline void put(const engine::renderer::ColorRGB& p_value, BufferWriteContext* p_context)
{
	put(p_value.r, p_context);
	put(p_value.g, p_context);
	put(p_value.b, p_context);
}


inline void put(const engine::renderer::ColorRGBA& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.r, p_buffer, p_remainingBytes);
	put(p_value.g, p_buffer, p_remainingBytes);
	put(p_value.b, p_buffer, p_remainingBytes);
	put(p_value.a, p_buffer, p_remainingBytes);
}


inline void put(const engine::renderer::ColorRGBA& p_value, BufferWriteContext* p_context)
{
	put(p_value.r, p_context);
	put(p_value.g, p_context);
	put(p_value.b, p_context);
	put(p_value.a, p_context);
}


inline void put(const math::Vector2& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.x, p_buffer, p_remainingBytes);
	put(p_value.y, p_buffer, p_remainingBytes);
}


inline void put(const math::Vector2& p_value, BufferWriteContext* p_context)
{
	put(p_value.x, p_context);
	put(p_value.y, p_context);
}


inline void put(const math::Vector3& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.x, p_buffer, p_remainingBytes);
	put(p_value.y, p_buffer, p_remainingBytes);
	put(p_value.z, p_buffer, p_remainingBytes);
}


inline void put(const math::Vector3& p_value, BufferWriteContext* p_context)
{
	put(p_value.x, p_context);
	put(p_value.y, p_context);
	put(p_value.z, p_context);
}


inline void put(const math::Point2& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.x, p_buffer, p_remainingBytes);
	put(p_value.y, p_buffer, p_remainingBytes);
}


inline void put(const math::Point2& p_value, BufferWriteContext* p_context)
{
	put(p_value.x, p_context);
	put(p_value.y, p_context);
}


inline void put(const math::Point3& p_value, u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.x, p_buffer, p_remainingBytes);
	put(p_value.y, p_buffer, p_remainingBytes);
	put(p_value.z, p_buffer, p_remainingBytes);
}


inline void put(const math::Point3& p_value, BufferWriteContext* p_context)
{
	put(p_value.x, p_context);
	put(p_value.y, p_context);
	put(p_value.z, p_context);
}


template< typename InternalRectType, s32 inclusiveCorrection>
inline void put(const math::Rect<InternalRectType, inclusiveCorrection>& p_value,
                u8*& p_buffer, size_t& p_remainingBytes)
{
	put(p_value.getPosition(), p_buffer, p_remainingBytes);
	put(p_value.getWidth(),    p_buffer, p_remainingBytes);
	put(p_value.getHeight(),   p_buffer, p_remainingBytes);
}


template< typename InternalRectType, s32 inclusiveCorrection>
inline void put(const math::Rect<InternalRectType, inclusiveCorrection>& p_value, BufferWriteContext* p_context)
{
	put(p_value.getPosition(),    p_context);
	put(p_value.getWidth(),  p_context);
	put(p_value.getHeight(), p_context);
}


template<typename T>
inline void putTLI(const math::TimedLinearInterpolation<T>& p_value, BufferWriteContext* p_context)
{
	put(p_value.getStartValue(),  p_context);
	put(p_value.getEndValue(),    p_context);
	put(p_value.getCurrentTime(), p_context);
	put(p_value.getDuration(),    p_context);
}


template<typename T>
inline void putEasing(const math::interpolation::Easing<T>& p_value, BufferWriteContext* p_context)
{
	put(p_value.getBeginValue(),   p_context);
	put(p_value.getEndValue(),     p_context);
	put(p_value.getDuration(),     p_context);
	put(p_value.getTime(),         p_context);
	putEnum<u8>(p_value.getType(), p_context);
}


// Namespace end
}
}
}


#endif  // !defined(INC_TT_CODE_BUFFERUTILS_PUT_H)
