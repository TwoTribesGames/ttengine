#if !defined(INC_TT_CODE_BUFFERUTILS_GET_H)
#define INC_TT_CODE_BUFFERUTILS_GET_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/Handle.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Point2.h>
#include <tt/math/Point3.h>
#include <tt/math/Rect.h>
#include <tt/math/interpolation.h>
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
// Buffer utils functions for reading data from a buffer in little endian format

inline void get(u8* p_rawData, size_t p_rawDataSize, const u8*& p_buffer, size_t& p_remainingBytes)
{
	if (p_remainingBytes < p_rawDataSize)
	{
		TT_PANIC("Can't get bytes from buffer, because not enough bytes are remaining.\n"
		         "Remaining bytes: %d, size of data to get: %d",
		         p_remainingBytes, p_rawDataSize);
		
		p_remainingBytes = 0;
		return;
	}
	
	memcpy(p_rawData, p_buffer, static_cast<mem::size_type>(p_rawDataSize));
	p_remainingBytes -= p_rawDataSize;
	p_buffer         += p_rawDataSize;
}


inline void get(u8* p_rawData, size_t p_rawDataSize, BufferReadContext* p_context)
{
	if (p_rawDataSize < static_cast<size_t>(p_context->end - p_context->cursor))
	{
		// Early out
		memcpy(p_rawData, p_context->cursor, p_rawDataSize);
		p_context->cursor += p_rawDataSize;
		return;
	}
	
	while (p_rawDataSize > 0)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			TT_PANIC("Can't get raw data from buffer because not enough bytes are remaining "
			         "(or refilling the read buffer failed).\nNumber of raw bytes to read: %u",
			         p_rawDataSize);
			p_context->statusCode = 1;
			return;
		}
		
		// Calculate bytes remaining in current context
		const size_t bytesRemaining(p_context->end - p_context->cursor);
		const size_t bytesToCopy(std::min(bytesRemaining, p_rawDataSize));
		
		memcpy(p_rawData, p_context->cursor, static_cast<mem::size_type>(bytesToCopy));
		p_rawDataSize     -= bytesToCopy;
		p_rawData         += bytesToCopy;
		p_context->cursor += bytesToCopy;
	}
}


/*! \brief Gets a number from a buffer in little endian.
    \param p_buffer Pointer to the buffer,
                    gets incremented after successful read.
    \param p_remainingBytes Number of bytes remaining in buffer,
                            gets decremented after successful read.
    \return Number read from buffer, 0 when buffer is too small.*/
template<typename T>
inline T get(const u8*& p_buffer, size_t& p_remainingBytes)
{
	if (p_remainingBytes < sizeof(T))
	{
		TT_PANIC("Can't get type from buffer because not enough bytes are remaining.\n"
		         "Remaining bytes: %u, size of type to read: %u",
		         p_remainingBytes, sizeof(T));
		
		p_remainingBytes = 0;
		return T(0);
	}
	p_remainingBytes -= sizeof(T);
	
#if TT_PLATFORM_LITTLE_ENDIAN
	T ret;
	memcpy(&ret, p_buffer, sizeof(T));
#else
	// read in little endian format (Intel)
	T ret = T(0);
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		ret += static_cast<T>(p_buffer[i]) << (i * 8);
	}
#endif
	
	p_buffer += sizeof(T);
	
	return ret;
}


template<typename T>
inline T get(BufferReadContext* p_context)
{
#if TT_PLATFORM_LITTLE_ENDIAN
	T ret;
	u8* rawData = reinterpret_cast<u8*>(&ret);
	size_t rawDataSize = sizeof(T);
	if (rawDataSize < static_cast<size_t>(p_context->end - p_context->cursor))
	{
		// Early out
		memcpy(rawData, p_context->cursor, rawDataSize);
		p_context->cursor += rawDataSize;
		return ret;
	}
	
	while (rawDataSize > 0)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			TT_PANIC("Can't get raw data from buffer because not enough bytes are remaining "
			         "(or refilling the read buffer failed).\nNumber of raw bytes to read: %u",
			         rawDataSize);
			p_context->statusCode = 1;
			return T(0);
		}
		
		// Calculate bytes remaining in current context
		const size_t bytesRemaining(p_context->end - p_context->cursor);
		const size_t bytesToCopy(std::min(bytesRemaining, rawDataSize));
		
		memcpy(rawData, p_context->cursor, bytesToCopy);
		rawDataSize       -= bytesToCopy;
		rawData           += bytesToCopy;
		p_context->cursor += bytesToCopy;
	}
	return ret;
#else
	T ret = 0;
	// read in little endian format (Intel)
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		if (p_context->needsRefill() && p_context->refill() != 0)
		{
			TT_PANIC("Can't get type from buffer because not enough bytes are remaining "
			         "(or refilling the read buffer failed).\nSize of type to read: %u",
			         sizeof(T));
			p_context->statusCode = 1;
			return T(0);
		}
		
		ret += static_cast<T>(*(p_context->cursor)) << (i * 8);
		++(p_context->cursor);
	}
	return ret;
#endif
}


template<>
inline bool get<bool>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	return get<u8>(p_buffer, p_remainingBytes) != 0;
}


template<>
inline bool get<bool>(BufferReadContext* p_context)
{
	return get<u8>(p_context) != 0;
}


template<>
inline real get<real>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(real));
	union
	{
		u32  intval;
		real floatval;
	};
	intval = get<u32>(p_buffer, p_remainingBytes);
	return floatval;
}


template<>
inline real get<real>(BufferReadContext* p_context)
{
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(real));
	union
	{
		u32  intval;
		real floatval;
	};
	intval = get<u32>(p_context);
	return floatval;
}


template<>
inline real64 get<real64>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(real64));
	union
	{
		u64    intval;
		real64 floatval;
	};
	intval = get<u64>(p_buffer, p_remainingBytes);
	return floatval;
}


template<>
inline real64 get<real64>(BufferReadContext* p_context)
{
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(real64));
	union
	{
		u64    intval;
		real64 floatval;
	};
	intval = get<u64>(p_context);
	return floatval;
}


template<typename SizeType, typename EnumType>
inline EnumType getEnum(BufferReadContext* p_context)
{
	SizeType value = get<SizeType>(p_context);
	return static_cast<EnumType>(value);
}


template<typename HandleType>
inline code::Handle<HandleType> getHandle(BufferReadContext* p_context)
{
	return code::Handle<HandleType>::createFromRawValue(get<u32>(p_context));
}


template < class FlagEnum, s32 FlagCountParam >
inline BitMask<FlagEnum, FlagCountParam> getBitMask(BufferReadContext* p_context)
{
	typedef BitMask<FlagEnum, FlagCountParam> BitMaskType;
	return BitMaskType(get<typename BitMaskType::ValueType>(p_context));
}


template<>
inline std::string get<std::string>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	// Read the length of the string from the buffer
	const u16 len = get<u16>(p_buffer, p_remainingBytes);
	
	// Make sure enough bytes remain in the buffer for the string itself
	if (len > p_remainingBytes)
	{
		TT_PANIC("String length (%u) is longer than the number of remaining bytes in the buffer (%u).",
		         len, p_remainingBytes);
		p_buffer        += p_remainingBytes;
		p_remainingBytes = 0;
		return std::string();
	}
	
	// Read the characters that make up the string from the buffer
	std::string str;
	str.reserve(static_cast<std::string::size_type>(len));
	for (u16 i = 0; i < len; ++i)
	{
		str += static_cast<std::string::value_type>(get<u8>(p_buffer, p_remainingBytes));
	}
	
	return str;
}


template<>
inline std::string get<std::string>(BufferReadContext* p_context)
{
	// Read the length of the string from the buffer
	const u16 len = get<u16>(p_context);
	
	// Read the characters that make up the string from the buffer
	std::string str;
	str.reserve(static_cast<std::string::size_type>(len));
	for (u16 i = 0; i < len; ++i)
	{
		str += static_cast<std::string::value_type>(get<u8>(p_context));
	}
	
	return str;
}


template<>
inline std::wstring get<std::wstring>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	// Read the length of the string (in bytes) from the buffer
	u16 len = get<u16>(p_buffer, p_remainingBytes);
	
	// Make sure enough bytes remain in the buffer for the string itself
	if (len > p_remainingBytes)
	{
		TT_PANIC("String length (%u) is longer than the number of remaining bytes in the buffer (%u).",
		         len, p_remainingBytes);
		p_buffer        += p_remainingBytes;
		p_remainingBytes = 0;
		return std::wstring();
	}
	
	if ((len % 2) != 0)
	{
		TT_PANIC("String length (%u) is not an even number (but must be).", len);
		p_buffer         += len;
		p_remainingBytes -= len;
		return std::wstring();
	}
	
	// Transform the byte length into a character count
	len /= 2;
	
	// Read the characters that make up the string from the buffer
	std::wstring str;
	str.reserve(static_cast<std::wstring::size_type>(len));
	for (u16 i = 0; i < len; ++i)
	{
		str += static_cast<std::wstring::value_type>(get<u16>(p_buffer, p_remainingBytes));
	}
	
	return str;
}


template<>
inline std::wstring get<std::wstring>(BufferReadContext* p_context)
{
	// Read the length of the string (in bytes) from the buffer
	u16 len = get<u16>(p_context);
	
	if ((len % 2) != 0)
	{
		TT_PANIC("String length (%u) is not an even number (but must be).", len);
		return std::wstring();
	}
	
	// Transform the byte length into a character count
	len /= 2;
	
	// Read the characters that make up the string from the buffer
	std::wstring str;
	str.reserve(static_cast<std::wstring::size_type>(len));
	for (u16 i = 0; i < len; ++i)
	{
		str += static_cast<std::wstring::value_type>(get<u16>(p_context));
	}
	
	return str;
}


template<>
inline engine::renderer::ColorRGB get<engine::renderer::ColorRGB>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const u8 r = get<u8>(p_buffer, p_remainingBytes);
	const u8 g = get<u8>(p_buffer, p_remainingBytes);
	const u8 b = get<u8>(p_buffer, p_remainingBytes);
	return engine::renderer::ColorRGB(r, g, b);
}


template<>
inline engine::renderer::ColorRGB get<engine::renderer::ColorRGB>(BufferReadContext* p_context)
{
	const u8 r = get<u8>(p_context);
	const u8 g = get<u8>(p_context);
	const u8 b = get<u8>(p_context);
	return engine::renderer::ColorRGB(r, g, b);
}


template<>
inline engine::renderer::ColorRGBA get<engine::renderer::ColorRGBA>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	engine::renderer::ColorRGBA color;
	color.r = get<u8>(p_buffer, p_remainingBytes);
	color.g = get<u8>(p_buffer, p_remainingBytes);
	color.b = get<u8>(p_buffer, p_remainingBytes);
	color.a = get<u8>(p_buffer, p_remainingBytes);
	return color;
}


template<>
inline engine::renderer::ColorRGBA get<engine::renderer::ColorRGBA>(BufferReadContext* p_context)
{
	engine::renderer::ColorRGBA color;
	color.r = get<u8>(p_context);
	color.g = get<u8>(p_context);
	color.b = get<u8>(p_context);
	color.a = get<u8>(p_context);
	return color;
}


template<>
inline math::Vector2 get<math::Vector2>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const real x = get<real>(p_buffer, p_remainingBytes);
	const real y = get<real>(p_buffer, p_remainingBytes);
	return math::Vector2(x, y);
}


template<>
inline math::Vector2 get<math::Vector2>(BufferReadContext* p_context)
{
	const real x = get<real>(p_context);
	const real y = get<real>(p_context);
	return math::Vector2(x, y);
}


template<>
inline math::Vector3 get<math::Vector3>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const real x = get<real>(p_buffer, p_remainingBytes);
	const real y = get<real>(p_buffer, p_remainingBytes);
	const real z = get<real>(p_buffer, p_remainingBytes);
	return math::Vector3(x, y, z);
}


template<>
inline math::Vector3 get<math::Vector3>(BufferReadContext* p_context)
{
	const real x = get<real>(p_context);
	const real y = get<real>(p_context);
	const real z = get<real>(p_context);
	return math::Vector3(x, y, z);
}


template<>
inline math::Point2 get<math::Point2>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const s32 x = get<s32>(p_buffer, p_remainingBytes);
	const s32 y = get<s32>(p_buffer, p_remainingBytes);
	return math::Point2(x, y);
}


template<>
inline math::Point2 get<math::Point2>(BufferReadContext* p_context)
{
	const s32 x = get<s32>(p_context);
	const s32 y = get<s32>(p_context);
	return math::Point2(x, y);
}


template<>
inline math::Point3 get<math::Point3>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const s32 x = get<s32>(p_buffer, p_remainingBytes);
	const s32 y = get<s32>(p_buffer, p_remainingBytes);
	const s32 z = get<s32>(p_buffer, p_remainingBytes);
	return math::Point3(x, y, z);
}


template<>
inline math::Point3 get<math::Point3>(BufferReadContext* p_context)
{
	const s32 x = get<s32>(p_context);
	const s32 y = get<s32>(p_context);
	const s32 z = get<s32>(p_context);
	return math::Point3(x, y, z);
}


template<>
inline math::VectorRect get<math::VectorRect>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const math::Vector2 pos(     get<math::Vector2>(p_buffer, p_remainingBytes));
	const real          width  = get<real         >(p_buffer, p_remainingBytes);
	const real          height = get<real         >(p_buffer, p_remainingBytes);
	
	return math::VectorRect(pos, width, height);
}


template<>
inline math::VectorRect get<math::VectorRect>(BufferReadContext* p_context)
{
	const math::Vector2 pos(     get<math::Vector2>(p_context));
	const real          width  = get<real         >(p_context);
	const real          height = get<real         >(p_context);
	
	return math::VectorRect(pos, width, height);
}


template<>
inline math::PointRect get<math::PointRect>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const math::Point2 pos(     get<math::Point2>(p_buffer, p_remainingBytes));
	const s32          width  = get<s32         >(p_buffer, p_remainingBytes);
	const s32          height = get<s32         >(p_buffer, p_remainingBytes);
	
	return math::PointRect(pos, width, height);
}


template<>
inline math::PointRect get<math::PointRect>(BufferReadContext* p_context)
{
	const math::Point2 pos(     get<math::Point2>(p_context));
	const s32          width  = get<s32         >(p_context);
	const s32          height = get<s32         >(p_context);
	
	return math::PointRect(pos, width, height);
}


template<typename T>
inline math::TimedLinearInterpolation<T> getTLI(BufferReadContext* p_context)
{
	const T    startValue(get<T   >(p_context));
	const T    endValue  (get<T   >(p_context));
	const real currTime = get<real>(p_context);
	const real duration = get<real>(p_context);
	
	return math::TimedLinearInterpolation<T>::createFromData(
		startValue, endValue, currTime, duration);
}


template<typename T>
inline math::interpolation::Easing<T> getEasing(BufferReadContext* p_context)
{
	const T    beginValue(get<T   >(p_context));
	const T    endValue  (get<T   >(p_context));
	const real duration = get<real>(p_context);
	const real time     = get<real>(p_context);
	const  math::interpolation::EasingType type = getEnum<u8,  math::interpolation::EasingType>(p_context);
	
	return math::interpolation::Easing<T>::createFromData(
		beginValue, endValue, duration, time, type);
}


// Namespace end
}
}
}


#endif  // !defined(INC_TT_CODE_BUFFERUTILS_GET_H)
