#ifndef INC_TT_AUDIO_CHIBI_TTMEMORYMANAGER_H
#define INC_TT_AUDIO_CHIBI_TTMEMORYMANAGER_H

#include <cstring>

#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/mem/mem.h>


namespace tt {
namespace audio {
namespace chibi {

/** MEM-IO **/

/* Each type of allocation is detailed by the loader, this way,
   the host implementation can manage the optimum memory allocation scheme
   for music if necesary */

class TTMemoryManager : public XMMemoryManager
{
public:
	virtual ~TTMemoryManager() {}
	
	inline virtual void* alloc(u32 p_size, AllocType p_allocType)
	{
		u32 alignment = (p_allocType == AllocType_Sample) ? u32(32) : u32(4);
		return mem::alloc(p_size, alignment, 0, 1);
	}
	
	inline virtual void free(void* p_mem, AllocType /*p_freeType*/)
	{
		mem::free(p_mem);
	}
	
	inline virtual void zeroMem(void* p_mem, u32 p_size)
	{
		std::memset(p_mem, 0, static_cast<size_t>(p_size));
	}
};


} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_TTMEMORYMANAGER_H
