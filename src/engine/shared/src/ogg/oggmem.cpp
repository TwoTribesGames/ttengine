#include <ogg/os_types.h>
#include <tt/mem/Heap.h>

using tt::mem::Heap;

void* oggMalloc(size_t p_size)
{
	return Heap::allocFromHeap(p_size);
}


void* oggCalloc(size_t p_num, size_t p_size)
{
	return Heap::callocFromHeap(p_num, p_size);
}


void* oggRealloc(void* p_block, size_t p_size)
{
	return Heap::reallocFromHeap(p_block, p_size);
}


void oggFree(void* p_block)
{
	Heap::freeToHeap(p_block);
}
