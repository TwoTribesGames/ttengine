#if !defined(INC_TT_AUDIO_PLAYER_WMPPLAYER_H)
#define INC_TT_AUDIO_PLAYER_WMPPLAYER_H

#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <atlbase.h>
#include <atlwin.h>
#include <wmp.h>

#include <tt/audio/player/MusicJinglePlayer.h>


namespace tt {
namespace audio {
namespace player {

/*! \brief Music playback using the Windows Media Player COM service. */
class WMPPlayer : public MusicJinglePlayer
{
public:
	static WMPPlayer* create();
	virtual ~WMPPlayer();
	
	// ==== MusicPlayer interface
	// Playback functions
	virtual bool play(const std::string& p_song, bool p_looping);
	virtual bool stop();
	virtual bool pause();
	virtual bool resume();
	virtual void update();
	
	// Status functions
	virtual const std::string& getCurrentSong() const { return m_currentSong; }
	virtual bool isPlaying() const;
	virtual bool isPaused() const;
	virtual bool isLooping() const;
	
	// Loading functions (NOTE: Not supported in WMPPlayer)
	virtual bool preload(const std::string& p_song);
	virtual bool unload(const std::string& p_song);
	virtual bool isLoaded(const std::string& p_song);
	
	// ==== MusicJinglePlayer interface
	virtual bool playJingle(const std::string& p_jingle);
	
private:
	WMPPlayer(const CComPtr<IWMPPlayer>&   p_wmpPlayer,
	          const CComPtr<IWMPCore>&     p_wmpCore,
	          const CComPtr<IWMPCore3>&    p_wmpCore3,
	          const CComPtr<IWMPControls>& p_wmpControls,
	          const CComPtr<IWMPPlaylist>& p_wmpPlaylist,
	          const CComPtr<IWMPSettings>& p_wmpSettings);
	
	virtual void updateVolume(real p_volume); // from MusicPlayer interface
	
	bool playImpl(const std::string& p_song, bool p_looping, double p_resumePosition);
	
	// No copying
	WMPPlayer(const WMPPlayer&);
	const WMPPlayer& operator=(const WMPPlayer&);
	
	
	std::string m_currentSong;
	bool        m_currentSongLooping;
	
	// COM access to WMP services
	CComPtr<IWMPPlayer>   m_wmpPlayer;
	CComPtr<IWMPCore>     m_wmpCore;
	CComPtr<IWMPCore3>    m_wmpCore3;
	CComPtr<IWMPControls> m_wmpControls;
	CComPtr<IWMPPlaylist> m_wmpPlaylist;
	CComPtr<IWMPSettings> m_wmpSettings;
	
	// Jingle playback information
	bool        m_isPlayingJingle;
	double      m_songResumePosition;
	std::string m_songResumeName;
	bool        m_songResumeLooping;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_WMPPLAYER_H)
