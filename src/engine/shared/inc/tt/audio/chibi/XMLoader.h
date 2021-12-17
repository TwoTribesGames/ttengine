#ifndef INC_TT_AUDIO_CHIBI_XMLOADER_H
#define INC_TT_AUDIO_CHIBI_XMLOADER_H

#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace chibi {

// forward declarations
class XMFileIO;
struct XMSong;
class XMPlayer;

class XMLoader
{
public:
	enum Error
	{
		Error_Ok,
		Error_Unconfigured, // FileIO/Mixer/MemoryIO was not set
		Error_FileIOInUse,
		Error_FileCantOpen,
		Error_FileUnrecognized,
		Error_OutOfMemory,
		Error_FileCorrupt
	};
	
	XMLoader();
	
	inline void setFileIO(XMFileIO* p_fileIO) { m_fileIO = p_fileIO; }
	
	Error openSong(const char* p_fileName, XMSong* p_song);
	Error openMusic(const char* p_fileName, XMSong* p_song); // Load only header/patterns
	Error openInstruments(const char* p_fileName, XMSong* p_song); // Load only instruments/samples
	
	Error openCustomMusic(const char* p_fileName, XMSong* p_song); // Load only pattern data
	Error openCustomInstruments(const char* p_fileName, XMSong* p_song); // Load only pattern data
	
	void freeSong(XMSong* p_song); // Free all song, p_song is NOT freed, use xm_song_free
	void freeMusic(XMSong* p_song); // Free patterns, p_song is NOT freed, use xm_song_free
	void freeInstruments(XMSong* p_song); // free instruments/samples
	
	inline void      setPlayer(XMPlayer* p_player) { m_player = p_player; }
	inline XMPlayer* getPlayer()                   { return m_player;     }
	
	static const char* getErrorDescription(Error p_error);
	
private:
	Error openSongCustom(const char* p_fileName, XMSong* p_song, bool p_loadMusic, bool p_loadInstruments);
	u32 recompressPattern(u16 p_rows, u8 p_channels, void* p_dst_data);
	
	// No copying
	XMLoader(const XMLoader&);
	XMLoader& operator=(const XMLoader&);
	
	
	XMFileIO* m_fileIO;
	XMPlayer* m_player;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMLOADER_H
