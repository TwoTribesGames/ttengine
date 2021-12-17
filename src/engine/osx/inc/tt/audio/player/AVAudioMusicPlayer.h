#if !defined(INC_TT_AUDIO_PLAYER_AVAUDIOMUSICPLAYER_H)
#define INC_TT_AUDIO_PLAYER_AVAUDIOMUSICPLAYER_H

#if defined(TT_PLATFORM_OSX_IPHONE)


#include <tt/audio/player/MusicJinglePlayer.h>
//#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
//#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace player {

class AVAudioMusicPlayer : public MusicJinglePlayer
{
public:
	AVAudioMusicPlayer();
	virtual ~AVAudioMusicPlayer();
	
	virtual bool play(const std::string& p_song, bool p_looping);
	virtual bool stop();
	virtual bool pause();
	virtual bool resume();
	virtual bool playJingle(const std::string& p_jingle);
	virtual void update();
	
	// Status functions
	virtual const std::string& getCurrentSong() const;
	virtual bool isPlaying() const;
	virtual bool isPaused() const;
	virtual bool isLooping() const;
	
	// Loading functions
	
	//virtual bool preload(const std::string& p_song);
	//virtual bool unload(const std::string& p_song);
	//virtual bool isLoaded(const std::string& p_song);
	
private:
	AVAudioMusicPlayer(const AVAudioMusicPlayer&);
	const AVAudioMusicPlayer& operator=(const AVAudioMusicPlayer&);
	
	virtual void updateVolume(real p_volume);
	
	bool playImpl(const std::string& p_song, bool p_looping, u8 p_resumePosition);
	
	
	void*       m_player; //!< Objective C class 'AVAudioPlayer'.
	void*       m_playerDelegate; //!< Objective C class 'TTdevObjCAVAudioMusicPlayerDelegate'.
	
	bool        m_paused;
	bool        m_looping;
	std::string m_currentSong;
	
	// Jingle variables (+ song restore after jingle is done.)
	bool        m_isPlayingJingle;
	u8          m_songResumePosition;
	std::string m_songResumeName;
	bool        m_songResumeLooping;
};


// End namespace
}
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)

#endif  // !defined(INC_TT_AUDIO_PLAYER_AVAUDIOMUSICPLAYER_H)
