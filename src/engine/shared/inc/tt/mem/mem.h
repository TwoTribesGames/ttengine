#ifndef INC_TT_MEM_MEM_H
#define INC_TT_MEM_MEM_H

#include <tt/mem/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace mem {
	
	/*! \brief Initialize the memory allocation system.
	    \p_extended Whether to use extended memory or not.*/
	void init(bool p_extended = false);
	
	/*! \brief Creates a unit heap.
	    \param p_blockSize Size of the unit heap's blocks in bytes.
	    \param p_blocks Amount of blocks in the heap.
	    \param p_alignment Alignment of blocks in the heap in bytes.
	    \param p_type The allocation type for the heap.*/
	void createUnitHeap(size_type p_blockSize,
	                    size_type p_blocks,
	                    size_type p_alignment = 4,
	                    s32       p_type = 0);
	
	/*! \brief Destroys a unit heap.
	    \param p_type The allocation type of the heap.*/
	void destroyUnitHeap(s32 p_type = 0);
	
	/*! \brief Checks whether a unit heap exists or not.
	    \param p_type The allocation type of the heap.
	    \return Whether a unit heap for the specified type exists or not.*/
	bool hasUnitHeap(s32 p_type = 0);
	
	
	// Allocation functions
	
	/*! \brief Allocate memory.
	    \param p_size The number of bytes to allocate.
	    \param p_alignment The number of bytes it should be aligned to.
	    \param p_creator The address of the calling code.
	    \param p_type The type of allocation (for multi heap systems).
	    \return Pointer to an allocated block of at least p_size bytes,
	            aligned to p_alignment, or 0 when the allocation failed.*/
	void* alloc(size_type p_size,
	            size_type p_alignment = 4,
	            void*     p_creator = 0,
	            s32       p_type = 0);
	
	/*! \brief Allocate a zeroed array of objects.
	    \param p_num The number of objects to allocate.
	    \param p_size The size of an object in bytes.
	    \param p_alignment The number of bytes it should be aligned to.
	    \param p_creator The address of the calling code.
	    \param p_type The type of allocation (for multi heap systems).
	    \return Pointer to a zeroed, allocated block of at least p_size * p_num bytes,
	            aligned to p_alignment, or 0 when the allocation failed.*/
	void* calloc(size_type p_num,
	             size_type p_size,
	             size_type p_alignment = 4,
	             void*     p_creator = 0,
	             s32       p_type = 0);
	
	/*! \brief Frees allocated memory.
	    \param p_block The block of memory to free, or 0.
	    \param p_destroyer The address of the calling code.*/
	void free(void* p_block, void* p_destroyer = 0);
	
	/*! \brief Changes the size of previously allocated memory.
	    \param p_block The block of memory to reallocate, or 0 (realloc will act like alloc).
	    \param p_size The new size in bytes.
	    \param p_alignment The alignment of the new memory block in bytes.
	    \param p_creator The address of the calling code.
	    \param p_type The type of allocation (for multi heap systems).
	    \return 0 on failure (the old data is not deleted), pointer to the new block on success.*/
	void* realloc(void*     p_block,
	              size_type p_size,
	              size_type p_alignment = 4,
	              void*     p_creator = 0,
	              s32       p_type = 0);
	
	
	// Status functions
	
	/*! \brief Get the starting address of the allocation system for a type.
	    \param p_type The allocation type to get the starting address of.
	    \return The starting address of the allocation system.*/
	void* getStart(s32 p_type = 0);
	
	/*! \brief Get the size of the allocation system for a type.
	    \param p_type The allocation type to get the size of.
	    \return The size of the allocation system.*/
	size_type getSize(s32 p_type = 0);
	
	/*! \brief Gets the total amount of free space for an allocation type.
	    \param p_type The allocation type to get the amount of free space of.
	    \return The amount of free space.*/
	size_type getTotalFreeSize(s32 p_type = 0);
	
	/*! \brief Gets the maximum amount of allocatable space for an allocation type.
	    \param p_type The allocation type to get the max. amount of allocatable space of.
	    \return The maximum amount of allocatable space.*/
	size_type getLargestFreeSize(s32 p_type = 0);
	
	/*! \brief Gets the total amount of allocated space for an allocation type.
	    \param p_includeHeader Whether internal 
	    \param p_type The allocation type to get the total amount of allocated space of.
	    \return The maximum amount of allocatable space.*/
	size_type getTotalAllocatedSize(bool p_includeHeader, s32 p_type = 0);
	
	
	// Debugging functions
	
	typedef void (*AllocCallback)(size_type p_size,
	                              size_type p_alignment,
	                              void*     p_creator,
	                              s32       p_type,
	                              void*     p_preBlock,  size_type p_preSize,
	                              void*     p_userBlock, size_type p_userSize,
	                              void*     p_postBlock, size_type p_postSize);
	
	typedef void (*FreeCallback)(void* p_destroyer,
	                             void* p_preBlock,  size_type p_preSize,
	                             void* p_userBlock, size_type p_userSize,
	                             void* p_postBlock, size_type p_postSize);
	
	/*! \brief Sets debugging callbacks, CALL THIS BEFORE INIT.
	    \param p_preSize Minimum amount of bytes of debug data required before the userdata.
	    \param p_postSize Minimum amount of bytes of debug data required behind the userdata.
	    \param p_alloc Callback function called after allocations.
	    \param p_free Callback function called before frees.*/
	void setDebugCallbacks(size_type     p_preSize,
	                       size_type     p_postSize,
	                       AllocCallback p_alloc,
	                       FreeCallback  p_free);
	
// namespace end
}
}

#endif // INC_TT_MEM_MEM_H
