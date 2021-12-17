#ifndef INC_TT_AUDIO_CHIBI_XMMEMORYMANAGER_H
#define INC_TT_AUDIO_CHIBI_XMMEMORYMANAGER_H

#include <tt/audio/chibi/types.h>


namespace tt {
namespace audio {
namespace chibi {

/** MEM-IO **/

/* Each type of allocation is detailed by the loader, this way,
   the host implementation can manage the optimum memory allocation scheme
   for music if necesary */

class XMMemoryManager
{
public:
	enum AllocType
	{
		AllocType_SongHeader,
		AllocType_Instrument,
		AllocType_Sample,
		AllocType_Pattern,
		AllocType_SWMixer, /** Only if SW Mixer is used, allocated once **/
		AllocType_Player   /** Memory needed for XM_Player, allocated once **/
	};
	
	virtual ~XMMemoryManager() {}
	
	virtual void* alloc(u32 p_size, AllocType p_allocType) = 0;
	virtual void  free(void* p_mem, AllocType p_freeType) = 0;
	virtual void  zeroMem(void* p_mem, u32 p_size) = 0;
};


} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMMEMORYMANAGER_H
