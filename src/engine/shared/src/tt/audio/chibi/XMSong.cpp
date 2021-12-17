#include <cstring>

#include <tt/audio/chibi/types.h>
#include <tt/audio/chibi/XMInstrument.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/audio/chibi/XMMixer.h>
#include <tt/audio/chibi/XMPlayer.h>
#include <tt/audio/chibi/XMSong.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace chibi {


XMSong::XMSong()
:
restartPos(0),
orderCount(0),
flags(0),
patternCount(0),
instrumentCount(0),
tempo(125),
speed(6),
patternData(0),
sharedPatternData(false),
instrumentData(0),
m_player(0)
{
	for (s32 i = 0; i < 21; ++i)
	{
		name[i] = 0;
	}
	for (s32 i = 0; i < 256; ++i)
	{
		orderList[i] = 0;
	}
}


XMSong::~XMSong()
{
	freeSong();
}


void* XMSong::operator new(std::size_t p_size)
{
	void* mem = XMUtil::getMemoryManager()->alloc(static_cast<u32>(p_size), XMMemoryManager::AllocType_SongHeader);
	if ( mem != 0 )
	{
		XMUtil::getMemoryManager()->zeroMem(mem, static_cast<u32>(p_size));
	}
	return mem;
}


void* XMSong::operator new[](std::size_t p_size)
{
	void* mem = XMUtil::getMemoryManager()->alloc(static_cast<u32>(p_size), XMMemoryManager::AllocType_SongHeader);
	if ( mem != 0 )
	{
		XMUtil::getMemoryManager()->zeroMem(mem, static_cast<u32>(p_size));
	}
	return mem;
}


void XMSong::operator delete(void* p_mem)
{
	XMUtil::getMemoryManager()->free(p_mem, XMMemoryManager::AllocType_SongHeader);
}


void XMSong::operator delete[](void* p_mem)
{
	XMUtil::getMemoryManager()->free(p_mem, XMMemoryManager::AllocType_SongHeader);
}


void XMSong::sharePatternData(XMSong* p_rhs)
{
	if (sharedPatternData == false)
	{
		// first, clean up pattern data
		if (patternData != 0)
		{
			for (s32 i = patternCount - 1; i >= 0; --i)
			{
				if (patternData[i] != 0)
				{
					XMUtil::getMemoryManager()->free(patternData[i], XMMemoryManager::AllocType_Pattern);
				}
			}
			
			if (patternCount != 0)
			{
				XMUtil::getMemoryManager()->free(patternData, XMMemoryManager::AllocType_Pattern);
				patternData = 0;
			}
		}
	}
	
	// copy members
	restartPos   = p_rhs->restartPos;
	orderCount   = p_rhs->orderCount;
	flags        = p_rhs->flags;
	patternCount = p_rhs->patternCount;
	tempo        = p_rhs->tempo;
	speed        = p_rhs->speed;
	
	// share pattern data
	patternData       = p_rhs->patternData;
	sharedPatternData = true;
	
	// copy order count
	std::memcpy(orderList, p_rhs->orderList, 256);
}

void XMSong::freeSong()
{
	clear(patternCount, instrumentCount);
}


void XMSong::freeMusic()
{
	clear(patternCount, -1);
}


void XMSong::freeInstruments()
{
	clear(-1, instrumentCount);
}


void XMSong::clear(s32 p_pattern_count, s32 p_instrument_count)
{
	/**
	  * Erasing must be done in the opposite way as creating.
	  * This way, allocating memory and deallocating memory from a song can work like a stack,
	  * to simplify the memory allocator in some implementations.
	  */
	
	/* Instruments first */
	
	for ( int i = (p_instrument_count - 1); i >= 0; --i )
	{
		if ( i >= instrumentCount )
		{
			TT_WARN("Invalid clear instrument amount specified.");
			continue;
		}
		
		if ( instrumentData[i] != 0 )
		{
			XMInstrument* ins = instrumentData[i];
			
			if (m_player != 0)
			{
				for ( int j = (ins->sampleCount - 1); j >= 0; --j )
				{
					if (ins->samples[j].sampleId != XMConstant_InvalidSampleID)
					{
						m_player->getMixer()->sampleUnregister(ins->samples[j].sampleId);
					}
				}
			}
			
			if ( ins->samples != 0 )
			{
				XMUtil::getMemoryManager()->free(ins->samples, XMMemoryManager::AllocType_Instrument);
			}
			XMUtil::getMemoryManager()->free(instrumentData[i], XMMemoryManager::AllocType_Instrument);
		}
	}
	
	if ( instrumentData != 0 && p_instrument_count >= 0 )
	{
		XMUtil::getMemoryManager()->free(instrumentData, XMMemoryManager::AllocType_Instrument);
		instrumentData = 0;
	}
	
	/* Patterns Last */
	if (sharedPatternData == false)
	{
		for ( int i = (p_pattern_count - 1); i >= 0; --i )
		{
			if ( i >= patternCount )
			{
				TT_WARN("Invalid clear pattern amount specified.");
				continue;
			}
			
			if ( patternData[i] != 0 )
			{
				XMUtil::getMemoryManager()->free(patternData[i], XMMemoryManager::AllocType_Pattern);
			}
		}
		
		if ( patternData != 0 && p_pattern_count != 0 )
		{
			XMUtil::getMemoryManager()->free(patternData, XMMemoryManager::AllocType_Pattern);
			patternData = 0;
		}
	}
}

} // namespace end
}
}
