#if !defined(INC_TT_AUDIO_PLAYER_XMMUSICPLAYER_H)
#define INC_TT_AUDIO_PLAYER_XMMUSICPLAYER_H

#include <map>
#include <vector>

#include <tt/audio/chibi/XMFileIO.h>
#include <tt/audio/chibi/XMLoader.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/audio/chibi/XMMixer.h>
#include <tt/audio/chibi/XMPlayer.h>
#include <tt/audio/chibi/XMSong.h>
#include <tt/audio/player/MusicJinglePlayer.h>
#include <tt/fs/types.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace player {

class XMMusicPlayer : public MusicJinglePlayer
{
public:
	XMMusicPlayer(fs::identifier     p_fs,
	              snd::identifier    p_snd,
	              const std::string& p_instruments,
	              int                p_samplerate = 44100,
	              bool               p_hardware   = false);
	virtual ~XMMusicPlayer();
	
	
	// Playback functions
	
	/*! \brief Starts playing a song.
	    \param p_song Song to start playing.
	    \param p_looping Whether the song should loop.
	    \return true on success, false on fail or when a song was already playing.*/
	virtual bool play(const std::string& p_song, bool p_looping);
	
	/*! \brief Stops the current song.
	    \return true on success or when no song was playing, false on fail.*/
	virtual bool stop();
	
	/*! \brief Pauses the current song.
	    \return true on success or when already paused, false on fail or when no song was playing.*/
	virtual bool pause();
	
	/*! \brief Resumes the currently paused song.
	    \return true on success or when already resumed, false on fail or when no song was playing.*/
	virtual bool resume();
	
	
	virtual bool playJingle(const std::string& p_jingle);
	
	
	/*! \brief Updates the music player, call this once per frame.*/
	virtual void update();
	
	
	// Status functions
	
	/*! \brief Retrieves the name of the currently playing song.
	    \return The name of the currently playing song, empty string when no song is playing.*/
	virtual const std::string& getCurrentSong() const;
	
	/*! \brief Retrieves whether or not a song is currently playing.
	    \return True when a song is playing (even if it's paused), false when there's no song.*/
	virtual bool isPlaying() const;
	
	/*! \brief Retrieves whether or not a song is currently paused.
	    \return True when a song is paused, false when it's playing or when there is no song.*/
	virtual bool isPaused() const;
	
	/*! \brief Retrieves whether the currently playing in a looping fashion.
	    \return True when a song is playing and it's looping, false there is no song or it's not looping.*/
	virtual bool isLooping() const;
	
	
	// Loading functions
	
	/*! \brief Preloads a song.
	    \param p_song Song to preload.
	    \return true on success or when already loaded, false on failure.*/
	virtual bool preload(const std::string& p_song);
	
	/*! \brief Unloads a song.
	    \param p_song Song to unload.
	    \return true on success or when not loaded, false on failure or when song is currently playing.*/
	virtual bool unload(const std::string& p_song);
	
	/*! \brief Retrieves whether or not a song has been preloaded.
	    \param p_song Song to check.
	    \return true when loaded, false when not.*/
	virtual bool isLoaded(const std::string& p_song);
	
	
private:
	XMMusicPlayer(const XMMusicPlayer&);
	const XMMusicPlayer& operator=(const XMMusicPlayer&);
	
	/*! \brief Updates the hardware volume, called after setVolume and setFade.
	    \param p_volume The volume to set, fade has already been applied. Range [0.0 - 1.0].*/
	virtual void updateVolume(real p_volume);
	
	std::string getFilename(const std::string& p_song) const;
	u8 getOrder(const std::string& p_song) const;
	bool playImpl(const std::string& p_song, bool p_looping, u8 p_resumePosition);
	
	typedef std::map<std::string, chibi::XMSong*> Patterns;
	
	bool        m_paused;
	bool        m_looping;
	bool        m_currentLoaded;
	std::string m_currentSong;
	
	chibi::XMMemoryManager* m_memMgr;
	chibi::XMPlayer*        m_player;
	chibi::XMMixer*         m_mixer;
	chibi::XMSong*          m_song;
	chibi::XMLoader*        m_loader;
	chibi::XMFileIO*        m_fileIO;
	Patterns                m_patterns;
	
	// Jingle variables (+ song restore after jingle is done.)
	bool        m_isPlayingJingle;
	u8          m_songResumePosition;
	std::string m_songResumeName;
	bool        m_songResumeLooping;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_AUDIO_PLAYER_XMMUSICPLAYER_H)
