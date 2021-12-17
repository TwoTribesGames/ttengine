#if !defined(INC_TT_MEM_UTIL_INL)
#define INC_TT_MEM_UTIL_INL

#include <cstring>

#include <tt/mem/types.h>


namespace tt {
namespace mem {

// default implementation of memory utility functions

inline void copy8(void* p_dest, const void* p_source, size_type p_size)
{
	std::memcpy(p_dest, p_source, static_cast<size_t>(p_size));
}


inline void copy16(void* p_dest, const void* p_source, size_type p_size)
{
	std::memcpy(p_dest, p_source, static_cast<size_t>(p_size));
}


inline void copy32(void* p_dest, const void* p_source, size_type p_size)
{
	std::memcpy(p_dest, p_source, static_cast<size_t>(p_size));
}


inline void copyWithPitch(void* p_dest, const void* p_source, u32 p_srcPitch, u32 p_dstPitch, u32 p_rows)
{
	if(p_srcPitch == p_dstPitch)
	{
		if(p_srcPitch % 4 == 0)
		{
			copy32(p_dest, p_source, p_srcPitch * p_rows);
		}
		else
		{
			copy8(p_dest, p_source, p_srcPitch * p_rows);
		}
	}
	else
	{
		const u8* src = reinterpret_cast<const u8*>(p_source);
		u8* dst = reinterpret_cast<u8*>(p_dest);

		for(u32 y = 0; y < p_rows; ++y)
		{
			mem::copy8(dst, src, p_srcPitch); // Copy single row of pixels

			src += p_srcPitch;
			dst += p_dstPitch;
		}
	}
}


inline void move8(void* p_dest, const void* p_source, size_type p_size)
{
	std::memmove(p_dest, p_source, static_cast<size_t>(p_size));
}


inline void move16(void* p_dest, const void* p_source, size_type p_size)
{
	std::memmove(p_dest, p_source, static_cast<size_t>(p_size));
}


inline void move32(void* p_dest, const void* p_source, size_type p_size)
{
	std::memmove(p_dest, p_source, static_cast<size_t>(p_size));
}


inline void fill8(void* p_buffer, u8 p_value, size_type p_size)
{
	std::memset(p_buffer, static_cast<int>(p_value), static_cast<size_t>(p_size));
}


inline void fill16(void* p_buffer, u16 p_value, size_type p_size)
{
	u16* scratch = reinterpret_cast<u16*>(p_buffer);
	for (size_type i = 0; i < (p_size / 2); ++i)
	{
		scratch[i] = p_value;
	}
}


inline void fill32(void* p_buffer, u32 p_value, size_type p_size)
{
	u32* scratch = reinterpret_cast<u32*>(p_buffer);
	for (size_type i = 0; i < (p_size / 4); ++i)
	{
		scratch[i] = p_value;
	}
}


inline void zero8(void* p_buffer, size_type p_size)
{
	std::memset(p_buffer, 0, static_cast<size_t>(p_size));
}


inline void zero16(void* p_buffer, size_type p_size)
{
	std::memset(p_buffer, 0, static_cast<size_t>(p_size));
}


inline void zero32(void* p_buffer, size_type p_size)
{
	std::memset(p_buffer, 0, static_cast<size_t>(p_size));
}


}
}


#endif // !defined(INC_TT_MEM_UTIL_INL)
