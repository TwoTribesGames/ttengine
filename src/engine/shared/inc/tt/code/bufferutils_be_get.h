#if !defined(INC_TT_CODE_BUFFERUTILS_BE_GET_H)
#define INC_TT_CODE_BUFFERUTILS_BE_GET_H


#include <string>

#include <tt/code/BufferReadContext.h>
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
// Buffer utils functions for reading data from a buffer in big endian format


/*! \brief Gets a number from a buffer in big endian.
    \param p_buffer Pointer to the buffer,
                    gets incremented after successful read.
    \param p_remainingBytes Number of bytes remaining in buffer,
                            gets decremented after successful read.
    \return Number read from buffer, 0 when buffer is too small.*/
template<typename T>
inline T be_get(const u8*& p_buffer, size_t& p_remainingBytes)
{
	if (p_remainingBytes < sizeof(T))
	{
		TT_PANIC("Can't get type from buffer because not enough bytes are remaining.\n"
		         "Remaining bytes: %u, size of type: %u",
		         p_remainingBytes, sizeof(T));
		
		p_remainingBytes = 0;
		return T(0);
	}
	p_remainingBytes -= sizeof(T);
	
	// read in big endian format (Motorola)
	T ret = T(0);
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		ret += static_cast<T>(p_buffer[sizeof(T) - (i + 1)]) << (i * 8);
	}
	
	p_buffer += sizeof(T);
	
	return ret;
}


template<typename T>
inline T be_get(BufferReadContext* p_context)
{
	// read in big endian format (Motorola)
	T ret = T(0);
	const s32 typeByteSize = static_cast<s32>(sizeof(T));
	for (s32 i = typeByteSize - 1; i >= 0; --i)
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
}


template<>
inline bool be_get<bool>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	return be_get<u8>(p_buffer, p_remainingBytes) != 0;
}


template<>
inline bool be_get<bool>(BufferReadContext* p_context)
{
	return be_get<u8>(p_context) != 0;
}


template<>
inline real be_get<real>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const u32 ret      = be_get<u32>(p_buffer, p_remainingBytes);
	float     retFloat = 0.0f;
	
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(float));
	mem::copy8(&retFloat, &ret, sizeof(float));
	
	return real(retFloat);
}


template<>
inline real be_get<real>(BufferReadContext* p_context)
{
	const u32 ret      = be_get<u32>(p_context);
	float     retFloat = 0.0f;
	
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(float));
	mem::copy8(&retFloat, &ret, sizeof(float));
	
	return real(retFloat);
}


template<>
inline real64 be_get<real64>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const u64 ret       = be_get<u64>(p_buffer, p_remainingBytes);
	double    retDouble = 0.0;
	
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(double));
	mem::copy8(&retDouble, &ret, sizeof(double));
	
	return real64(retDouble);
}


template<>
inline real64 be_get<real64>(BufferReadContext* p_context)
{
	const u64 ret       = be_get<u64>(p_context);
	double    retDouble = 0.0;
	
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(double));
	mem::copy8(&retDouble, &ret, sizeof(double));
	
	return real64(retDouble);
}


template<>
inline std::string be_get<std::string>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	// Read the length of the string from the buffer
	const u16 len = be_get<u16>(p_buffer, p_remainingBytes);
	
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
		str += static_cast<std::string::value_type>(be_get<u8>(p_buffer, p_remainingBytes));
	}
	
	return str;
}


template<>
inline std::string be_get<std::string>(BufferReadContext* p_context)
{
	// Read the length of the string from the buffer
	const u16 len = be_get<u16>(p_context);
	
	// Read the characters that make up the string from the buffer
	std::string str;
	str.reserve(static_cast<std::string::size_type>(len));
	for (u16 i = 0; i < len; ++i)
	{
		str += static_cast<std::string::value_type>(be_get<u8>(p_context));
	}
	
	return str;
}


template<>
inline std::wstring be_get<std::wstring>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	// Read the length of the string (in bytes) from the buffer
	u16 len = be_get<u16>(p_buffer, p_remainingBytes);
	
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
		str += static_cast<std::wstring::value_type>(be_get<u16>(p_buffer, p_remainingBytes));
	}
	
	return str;
}


template<>
inline std::wstring be_get<std::wstring>(BufferReadContext* p_context)
{
	// Read the length of the string (in bytes) from the buffer
	u16 len = be_get<u16>(p_context);
	
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
		str += static_cast<std::wstring::value_type>(be_get<u16>(p_context));
	}
	
	return str;
}


template<>
inline math::Vector2 be_get<math::Vector2>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const real x = be_get<real>(p_buffer, p_remainingBytes);
	const real y = be_get<real>(p_buffer, p_remainingBytes);
	return math::Vector2(x, y);
}


template<>
inline math::Vector2 be_get<math::Vector2>(BufferReadContext* p_context)
{
	const real x = be_get<real>(p_context);
	const real y = be_get<real>(p_context);
	return math::Vector2(x, y);
}


template<>
inline math::Vector3 be_get<math::Vector3>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const real x = be_get<real>(p_buffer, p_remainingBytes);
	const real y = be_get<real>(p_buffer, p_remainingBytes);
	const real z = be_get<real>(p_buffer, p_remainingBytes);
	return math::Vector3(x, y, z);
}


template<>
inline math::Vector3 be_get<math::Vector3>(BufferReadContext* p_context)
{
	const real x = be_get<real>(p_context);
	const real y = be_get<real>(p_context);
	const real z = be_get<real>(p_context);
	return math::Vector3(x, y, z);
}


template<>
inline math::Point2 be_get<math::Point2>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const s32 x = be_get<s32>(p_buffer, p_remainingBytes);
	const s32 y = be_get<s32>(p_buffer, p_remainingBytes);
	return math::Point2(x, y);
}


template<>
inline math::Point2 be_get<math::Point2>(BufferReadContext* p_context)
{
	const s32 x = be_get<s32>(p_context);
	const s32 y = be_get<s32>(p_context);
	return math::Point2(x, y);
}


template<>
inline math::Point3 be_get<math::Point3>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const s32 x = be_get<s32>(p_buffer, p_remainingBytes);
	const s32 y = be_get<s32>(p_buffer, p_remainingBytes);
	const s32 z = be_get<s32>(p_buffer, p_remainingBytes);
	return math::Point3(x, y, z);
}


template<>
inline math::Point3 be_get<math::Point3>(BufferReadContext* p_context)
{
	const s32 x = be_get<s32>(p_context);
	const s32 y = be_get<s32>(p_context);
	const s32 z = be_get<s32>(p_context);
	return math::Point3(x, y, z);
}


template<>
inline math::VectorRect be_get<math::VectorRect>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const math::Vector2 pos(     be_get<math::Vector2>(p_buffer, p_remainingBytes));
	const real          width  = be_get<real         >(p_buffer, p_remainingBytes);
	const real          height = be_get<real         >(p_buffer, p_remainingBytes);
	
	return math::VectorRect(pos, width, height);
}


template<>
inline math::VectorRect be_get<math::VectorRect>(BufferReadContext* p_context)
{
	const math::Vector2 pos(     be_get<math::Vector2>(p_context));
	const real          width  = be_get<real         >(p_context);
	const real          height = be_get<real         >(p_context);
	
	return math::VectorRect(pos, width, height);
}


template<>
inline math::PointRect be_get<math::PointRect>(const u8*& p_buffer, size_t& p_remainingBytes)
{
	const math::Point2 pos(     be_get<math::Point2>(p_buffer, p_remainingBytes));
	const s32          width  = be_get<s32         >(p_buffer, p_remainingBytes);
	const s32          height = be_get<s32         >(p_buffer, p_remainingBytes);
	
	return math::PointRect(pos, width, height);
}


template<>
inline math::PointRect be_get<math::PointRect>(BufferReadContext* p_context)
{
	const math::Point2 pos(     be_get<math::Point2>(p_context));
	const s32          width  = be_get<s32         >(p_context);
	const s32          height = be_get<s32         >(p_context);
	
	return math::PointRect(pos, width, height);
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_CODE_BUFFERUTILS_BE_GET_H)
