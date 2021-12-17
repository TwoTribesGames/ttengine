#ifndef INC_TT_AUDIO_CHIBI_XMSONG_H
#define INC_TT_AUDIO_CHIBI_XMSONG_H

#include <cstddef>

#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace chibi {

// forward declarations
struct XMInstrument;
class XMPlayer;


struct XMSong
{
	enum Flag
	{
		Flag_LinearPeriods    = (1 << 7),
		Flag_ChannelsUsedMask = (1 << 5) - 1 /** flags&XM_SONG_FLAGS_MASK_CHANNELS_USED + 1 to obtain**/
	};
	
	XMSong();
	~XMSong();
	
	static void* operator new(std::size_t p_size);
	static void* operator new[](std::size_t p_size);
	static void  operator delete(void* p_block);
	static void  operator delete[](void* p_block);
	
	void sharePatternData(XMSong* p_rhs);
	
	void freeSong();
	void freeMusic();
	void freeInstruments();
	
	void clear(s32 p_pattern_count, s32 p_instrument_count);
	
	inline void      setPlayer(XMPlayer* p_player) { m_player = p_player; }
	inline XMPlayer* getPlayer() const             { return m_player;     }
	
	
	char name[21];
	u8 restartPos;
	u8 orderCount;
	u8 flags; /* flags, defined in Flags (including channels used) */
	
	u16 patternCount;
	u16 instrumentCount;
	u16 tempo;
	u16 speed;
	
	u8**           patternData;    /* array of pointers to pattern data,using xm packing, NULL means empty pattern */
	bool           sharedPatternData;
	XMInstrument** instrumentData;
	
	u8 orderList[256];
	
private:
	// No copying
	XMSong(const XMSong&);
	XMSong& operator=(const XMSong&);
	
	
	XMPlayer* m_player;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMSONG_H
