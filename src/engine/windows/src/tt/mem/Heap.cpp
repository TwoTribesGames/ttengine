#include <tt/mem/Heap.h>
#include <tt/profiler/MemoryProfiler.h>
#include <tt/math/math.h>

#include <stdlib.h>

namespace tt {
namespace mem {

Heap::AllocCallback  Heap::m_allocCallback  = 0;
Heap::CallocCallback Heap::m_callocCallback = 0;
Heap::FreeCallback   Heap::m_freeCallback   = 0;


// Public functions

// Heap Functions

void Heap::createHeap(void*, size_t)
{
	// does nothing
}


void Heap::cleanHeap()
{
	// does nothing
}


/*---------------------------------------------------------------------------*
  Name:         allocFromHeap

  Description:  Allocates /p_size/ bytes from heap. Some additional memory
                will also be consumed from heap.

  Arguments:    p_size      : size of object to be allocated
                p_alignment : alignment for the block to be created
				p_creator   : pointer to the code that requested the block
				p_front     : wether to get this cell from the front or the
				              back of the heap

  Returns:      a null pointer or a pointer to the allocated space aligned
                with ALIGNMENT bytes boundaries
 *---------------------------------------------------------------------------*/
void* Heap::allocFromHeap(size_t p_size,
						  size_t p_alignment,
						  void*  p_creator,
						  s32    p_type)
{
	if ( m_allocCallback != 0 )
	{
		m_allocCallback(p_size, p_alignment, p_creator, p_type);
	}

#ifdef PROFILE_MEMORY
	size_t correctedSize = 
		profiler::MemoryProfiler::getCorrectedAllocSize(p_size, p_alignment);

	void *buffer = malloc(correctedSize);

	buffer = profiler::MemoryProfiler::getCorrectedAllocAddress(buffer, p_alignment);

	// do some bookkeeping for the MemoryProfiler
	profiler::MemoryProfiler::addMemBlock(buffer, p_creator, p_type, p_size, 
		                                  p_alignment, 0);

	return buffer;
#else
	void *mem = malloc(p_size);
	TT_ASSERTMSG(mem != 0, "Failed to allocate %d bytes.", p_size);
	return mem;
#endif
}


/*---------------------------------------------------------------------------*
  Name:         callocFromHeap

  Description:  Allocates /p_size/ bytes from heap. Some additional memory
                will also be consumed from heap.

  Arguments:    p_size      : size of object to be allocated
                p_alignment : alignment for the block to be created
				p_creator   : pointer to the code that requested the block
				p_front     : wether to get this cell from the front or the
				              back of the heap

  Returns:      a null pointer or a pointer to the allocated space aligned
                with ALIGNMENT bytes boundaries
 *---------------------------------------------------------------------------*/
void* Heap::callocFromHeap(size_t p_num,
                           size_t p_size,
						   size_t p_alignment,
						   void*  p_creator,
						   s32    p_type)
{
	if ( m_callocCallback != 0 )
	{
		m_callocCallback(p_num, p_size, p_alignment, p_creator, p_type);
	}

#ifdef PROFILE_MEMORY
	size_t correctedSize = 
		profiler::MemoryProfiler::getCorrectedAllocSize(p_size * p_num, p_alignment);

	void *buffer = malloc(correctedSize);

	// clear memory (calloc)
	memset(buffer, 0, correctedSize);

	buffer = profiler::MemoryProfiler::getCorrectedAllocAddress(buffer, p_alignment);

	// do some bookkeeping for the MemoryProfiler
	profiler::MemoryProfiler::addMemBlock(buffer, p_creator, p_type, p_size * p_num, 
		                        p_alignment, 0);

	return buffer;
#else
	void *mem = calloc(p_num, p_size);
	TT_ASSERTMSG(mem != 0, "Failed to allocate %d bytes", p_num * p_size);
	return mem;
#endif
}


/*---------------------------------------------------------------------------*
  Name:         freeToHeap

  Description:  Returns obj /p_block/ to heap.

  Arguments:    p_block : pointer to object previously returned from
                           allocFromHeap().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void Heap::freeToHeap(void* p_block, void* p_destroyer)
{
	if ( m_freeCallback != 0 )
	{
		m_freeCallback(p_block, p_destroyer);
	}

	// deleting null pointer should return immediately
	if ( p_block == 0 )
	{
		return;
	}

#ifdef PROFILE_MEMORY
	profiler::MemoryProfiler::delMemBlock(p_block, p_destroyer);

	p_block = profiler::MemoryProfiler::getCorrectedFreeAddress(p_block);
#endif

	free(p_block);
}


/*---------------------------------------------------------------------------*
  Name:         reallocFromHeap

  Description:  The reallocFromHeap function changes the size of the memory
                object pointed to by p_block to the size specified by p_size.
				The contents of the object will remain unchanged up to the
				lesser of the new and old sizes. If the new size of the memory
				object would require movement of the object, the space for the
				previous instantiation of the object is freed. If the new size
				is larger, the contents of the newly allocated portion of the
				object are unspecified. If size is 0 and p_block is not a null
				pointer, the object pointed to is freed.
				If the space cannot be allocated, the object remains unchanged.
                If p_block is a null pointer, reallocFromHeap behaves like
				allocFromHeap for the specified size.
                The pointer returned points to the start (lowest byte address)
				of the allocated space.
				If the space cannot be allocated, a null pointer is returned.

  Arguments:    p_block : pointer to the memory block to be reallocated
                p_size  : the new size of the memory block

  Returns:      Upon successful completion with a size not equal to 0,
                reallocFromHeap returns a pointer to the (possibly moved)
				allocated space. If size is 0, either a null pointer or a
				unique pointer that can be successfully passed to freeToHeap
				is returned. If there is not enough available memory, realloc
				returns a null pointer.
 *---------------------------------------------------------------------------*/
void* Heap::reallocFromHeap(void* p_block,
                            size_t p_size,
                            size_t p_alignment,
                            void*  p_creator,
                            s32    p_type)
{
	if(p_block == 0)
	{
		void* ret = allocFromHeap(p_size, p_alignment, p_creator, p_type);

		return ret;
	}

	if(p_size == 0)
	{
		freeToHeap(p_block, p_creator);
		return 0;
	}

	if ( m_allocCallback != 0 )
	{
		m_allocCallback(p_size, p_alignment, p_creator, p_type);
	}

	void* new_block = realloc(p_block, p_size);

	if(new_block == 0)
	{
		return 0;
	}

	if ( m_freeCallback != 0 )
	{
		m_freeCallback(p_block, p_creator);
	}

	return new_block;
}


/*---------------------------------------------------------------------------*
  Name:         getTotalAllocSize

  Description:  Get sum of allocated block size.
				Subroutine for OS_GetTotalAllocSize and OS_GetTotalOccupiedSize.

  Arguments:    id   : arena ID
                isHeadInclude : whether if including block header.

  Returns:      sum of allocated block size
 *---------------------------------------------------------------------------*/
size_t Heap::getTotalAllocSize(bool)
{
	return 0;
}


/*---------------------------------------------------------------------------*
  Name:         getTotalFreeSize

  Description:  Get sum of free block size,
                not including of block header.

  Arguments:    None.

  Returns:      sum of free block size
 *---------------------------------------------------------------------------*/
size_t Heap::getTotalFreeSize(s32 /* p_type */)
{
	return 0;
}


/*---------------------------------------------------------------------------*
  Name:         getLargestFreeSize

  Description:  Get maximum free block size

  Arguments:    None

  Returns:      maximum free block size.
 *---------------------------------------------------------------------------*/
size_t Heap::getLargestFreeSize(s32 /* p_type */)
{
	return 0;
}

// Namespace end
}
}

