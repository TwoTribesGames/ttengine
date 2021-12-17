#ifndef INC_TT_MEM_HEAP_H
#define INC_TT_MEM_HEAP_H

#include <tt/platform/tt_types.h>
#include <stddef.h>

namespace tt {
namespace mem {

/**
 * Heap static
 */
class Heap
{
public:
	// heap functions
	static void		createHeap(void* p_start, size_t p_size);
	static void		cleanHeap();

	// allocation / free functions
	static void*	allocFromHeap(size_t p_size,
	                              size_t p_alignment = 4,
	                              void*  p_creator = 0,
								  s32    p_type = 0);

	static void*	callocFromHeap(size_t p_num,
	                               size_t p_size,
								   size_t p_alignment = 4,
								   void*  p_creator = 0,
								   s32    p_type = 0);

	static void		freeToHeap(void* p_block, void* p_destroyer = 0);

	static void*	reallocFromHeap(void*  p_block,
									size_t p_size,
									size_t p_alignment = 4,
									void*  p_creator = 0,
									s32    p_type = 0);


	// free memory information functions
	static size_t   getTotalFreeSize(s32 p_type = 0);
	static size_t   getLargestFreeSize(s32 p_type = 0);

	// used memory information functions
	static size_t   getTotalAllocSize(bool p_include_header);

	static void*    getStart(s32) { return 0; }
	static size_t   getSize(s32)  { return 0; }

	// callback functions for debugging
	typedef void (*AllocCallback)  (size_t p_size, size_t p_alignment, void* p_creator, s32 p_type);
	typedef void (*CallocCallback) (size_t p_num, size_t p_size, size_t p_alignment, void* p_creator, s32 p_type);
	typedef void (*FreeCallback)   (void* p_block, void* p_destroyer);

	static void setAllocCallback(AllocCallback p_callback)   { m_allocCallback  = p_callback; }
	static void setCallocCallback(CallocCallback p_callback) { m_callocCallback = p_callback; }
	static void setFreeCallback (FreeCallback  p_callback)   { m_freeCallback   = p_callback; }

private:

	// non instantiable class
	Heap();
	~Heap();

    Heap( Heap& );
    Heap& operator=( Heap& );

	static AllocCallback  m_allocCallback;
	static CallocCallback m_callocCallback;
	static FreeCallback   m_freeCallback;
};

}
}

#endif	// INC_TT_MEM_HEAP_H
