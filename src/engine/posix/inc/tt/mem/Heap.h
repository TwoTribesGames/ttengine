#if !defined(INC_TT_MEM_HEAP_H)
#define INC_TT_MEM_HEAP_H


#include <cstddef>

#include <tt/platform/tt_types.h>


namespace tt {
namespace mem {

class Heap
{
public:
	// Heap functions
	static void createHeap(void* p_start, std::size_t p_size);
	static void cleanHeap();

	// Allocation / free functions
	static void* allocFromHeap(std::size_t p_size,
	                           std::size_t p_alignment = 4,
	                           void*       p_creator   = 0,
	                           s32         p_type      = 0);
	
	static void* callocFromHeap(std::size_t p_num,
	                            std::size_t p_size,
	                            std::size_t p_alignment = 4,
	                            void*       p_creator   = 0,
	                            s32         p_type      = 0);
	
	static void freeToHeap(void* p_block, void* p_destroyer = 0);
	
	static void* reallocFromHeap(void*       p_block,
	                             std::size_t p_size,
	                             std::size_t p_alignment = 4,
	                             void*       p_creator   = 0,
	                             s32         p_type      = 0);
	
	
	// Free memory information functions
	static std::size_t getTotalFreeSize(s32 p_type = 0);
	static std::size_t getLargestFreeSize(s32 p_type = 0);
	
	// Used memory information functions
	static std::size_t getTotalAllocSize(bool p_includeHeader);
	
	static void*       getStart(s32) { return 0; }
	static std::size_t getSize(s32)  { return 0; }
	
	// Callback functions for debugging
	typedef void (*AllocCallback)  (std::size_t p_size, std::size_t p_alignment, void* p_creator, s32 p_type);
	typedef void (*CallocCallback) (std::size_t p_num, std::size_t p_size, std::size_t p_alignment, void* p_creator, s32 p_type);
	typedef void (*FreeCallback)   (void* p_block, void* p_destroyer);
	
	static void setAllocCallback(AllocCallback p_callback)
	{ ms_allocCallback = p_callback; }
	static void setCallocCallback(CallocCallback p_callback)
	{ ms_callocCallback = p_callback; }
	static void setFreeCallback (FreeCallback  p_callback)
	{ ms_freeCallback = p_callback; }
	
private:
	// Non-instantiable class
	Heap();
	~Heap();
	
	Heap(const Heap&);
	const Heap& operator=(const Heap&);
	
	
	static AllocCallback  ms_allocCallback;
	static CallocCallback ms_callocCallback;
	static FreeCallback   ms_freeCallback;
};

// Namespace end
}
}


#endif	// !defined(INC_TT_MEM_HEAP_H)
