#if !defined(INC_TT_MEM_CACHE_H)
#define INC_TT_MEM_CACHE_H

#include <tt/mem/types.h>


namespace tt {
namespace mem {

/*! \brief Writes cached data to memory and invalidates cache.
    \param p_addr Address from which to start flushing.
    \param p_size Size of the region to be flushed.*/
void flush(void* p_addr, size_type p_size);

/*! \brief Writes cached data to memory and invalidates cache, but does not wait for the write to finish.
    \param p_addr Address from which to start flushing.
    \param p_size Size of the region to be flushed.*/
void flushAsync(void* p_addr, size_type p_size);

/*! \brief Writes cached data to memory.
    \param p_addr Address from which to start storing.
    \param p_size Size of the region to be stored.*/
void store(void* p_addr, size_type p_size);

/*! \brief Writes cached data to memory, but does not wait for the write to finish.
    \param p_addr Address from which to start storing.
    \param p_size Size of the region to be stored.*/
void storeAsync(void* p_addr, size_type p_size);

/*! \brief Invalidates a range of cache.
    \param p_addr Address from which to start invalidating.
    \param p_size Size of the region to be invalidated.*/
void invalidate(void* p_addr, size_type p_size);

/*! \brief Waits for all cache opertions to be finished, this may stall the CPU.*/
void sync();

}
}

#include <tt/mem/cache.inl>

#endif // !defined(INC_TT_MEM_CACHE_H)
