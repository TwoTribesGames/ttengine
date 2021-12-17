#if !defined(INC_TT_AUDIO_PLAYER_MUSICPLAYER_H)
#define INC_TT_AUDIO_PLAYER_MUSICPLAYER_H


#include <string>

#include <tt/audio/player/fwd.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace player {

class MusicPlayer
{
public:
	virtual ~MusicPlayer() {}
	
	
	// Playback functions
	
	/*! \brief Starts playing a song.
	    \param p_song Song to start playing.
	    \param p_looping Whether the song should loop.
	    \return true on success, false on fail or when a song was already playing.*/
	virtual bool play(const std::string& p_song, bool p_looping) = 0;
	
	/*! \brief Stops the current song.
	    \return true on success or when no song was playing, false on fail.*/
	virtual bool stop() = 0;
	
	/*! \brief Pauses the current song.
	    \return true on success or when already paused, false on fail or when no song was playing.*/
	virtual bool pause() = 0;
	
	/*! \brief Resumes the currently paused song.
	    \return true on success or when already resumed, false on fail or when no song was playing.*/
	virtual bool resume() = 0;
	
	/*! \brief Updates the music player, call this once per frame.*/
	virtual void update() = 0;
	
	
	// Status functions
	
	/*! \brief Retrieves the name of the currently playing song.
	    \return The name of the currently playing song, empty string when no song is playing.*/
	virtual const std::string& getCurrentSong() const = 0;
	
	/*! \brief Retrieves whether or not a song is currently playing.
	    \return True when a song is playing (even if it's paused), false when there's no song.*/
	virtual bool isPlaying() const = 0;
	
	/*! \brief Retrieves whether or not a song is currently paused.
	    \return True when a song is paused, false when it's playing or when there is no song.*/
	virtual bool isPaused() const = 0;
	
	/*! \brief Retrieves whether the currently playing in a looping fashion.
	    \return True when a song is playing and it's looping, false there is no song or it's not looping.*/
	virtual bool isLooping() const = 0;
	
	/*! \brief Sets an interface to receive callbacks from the music player. */
	inline void setCallbackInterface(const MusicPlayerCallbackInterfacePtr& p_callback)
	{ m_callbackInterface = p_callback; }
	
	
	// Volume functions
	
	/*! \brief Sets the volume of the music player.
	    \param p_volume The volume to set, range [0.0 - 1.0].*/
	inline void setVolume(real p_volume)
	{
		TT_ASSERTMSG(p_volume >= 0.0f && p_volume <= 1.0f,
		             "Volume %f out of bounds.", realToFloat(p_volume));
		m_volume = p_volume;
		updateVolume(getPlaybackVolume());
	}
	
	/*! \brief Gets the current volume.
	    \return The current volume.*/
	inline real getVolume() const { return m_volume; }
	
	/*! \brief Sets the current volume fade position.
	    \param p_fade The fade position to set, range [0.0 - 1.0].*/
	inline void setFade(real p_fade)
	{
		TT_ASSERTMSG(p_fade >= 0.0f && p_fade <= 1.0f,
		             "Fade %f out of bounds.", realToFloat(p_fade));
		m_fade = p_fade;
		updateVolume(getPlaybackVolume());
	}
	
	/*! \brief Gets the current fade position.
	    \return The current fade position.*/
	inline real getFade() const { return m_fade; }
	
	
	// Loading functions
	
	/*! \brief Preloads a song.
	    \param p_song Song to preload.
	    \return true on success or when already loaded, false on failure.*/
	virtual bool preload(const std::string& p_song)
	{
		(void)p_song;
		return false;
	}
	
	/*! \brief Unloads a song.
	    \param p_song Song to unload.
	    \return true on success or when not loaded, false on failure or when song is currently playing.*/
	virtual bool unload(const std::string& p_song)
	{
		(void)p_song;
		return false;
	}
	
	/*! \brief Retrieves whether or not a song has been preloaded.
	    \param p_song Song to check.
	    \return true when loaded, false when not.*/
	virtual bool isLoaded(const std::string& p_song)
	{
		(void)p_song;
		return false;
	}
	
protected:
	MusicPlayer()
	:
	m_volume(1.0f),
	m_fade(1.0f),
	m_callbackInterface()
	{ }
	
	inline real getPlaybackVolume() const { return m_volume * m_fade; }
	
	inline MusicPlayerCallbackInterfacePtr getCallbackInterface() { return m_callbackInterface.lock(); }
	
	/*! \brief Updates the hardware volume, called after setVolume and setFade.
	    \param p_volume The volume to set, fade has already been applied. Range [0.0 - 1.0].*/
	virtual void updateVolume(real p_volume) = 0;
	
private:
	MusicPlayer(const MusicPlayer&);
	const MusicPlayer& operator=(const MusicPlayer&);
	
	real m_volume; //!< Volume of the music player, range [0.0 - 1.0].
	real m_fade;   //!< Fade position, range [0.0 - 1.0].
	
	MusicPlayerCallbackInterfaceWeakPtr m_callbackInterface;
};


// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_PLAYER_MUSICPLAYER_H)
