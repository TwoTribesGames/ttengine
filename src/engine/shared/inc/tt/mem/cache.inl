#if !defined(INC_TT_MEM_CACHE_INL)
#define INC_TT_MEM_CACHE_INL

#include <tt/mem/types.h>


namespace tt {
namespace mem {

// default implementation of cache functions

inline void flush(void* p_addr, size_type p_size)
{
	(void)p_addr;
	(void)p_size;
}


inline void flushAsync(void* p_addr, size_type p_size)
{
	(void)p_addr;
	(void)p_size;
}


inline void store(void* p_addr, size_type p_size)
{
	(void)p_addr;
	(void)p_size;
}


inline void storeAsync(void * p_addr, size_type p_size)
{
	(void)p_addr;
	(void)p_size;
}


inline void invalidate(void* p_addr, size_type p_size)
{
	(void)p_addr;
	(void)p_size;
}


inline void sync()
{
}

}
}


#endif // !defined(INC_TT_MEM_CACHE_INL)
