#if !defined(INC_TT_AUDIO_PLAYER_OGGPLAYER_H)
#define INC_TT_AUDIO_PLAYER_OGGPLAYER_H

#include <tt/audio/codec/Decoder.h>
#include <tt/audio/player/MusicPlayer.h>
#include <tt/fs/types.h>
#include <tt/snd/StreamSource.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace player {

class OggPlayer : public MusicPlayer, public snd::StreamSource
{
public:
	explicit OggPlayer(fs::identifier p_fs, snd::identifier p_snd = 0);
	virtual ~OggPlayer();
	
	
	// Playback functions
	virtual bool play(const std::string& p_song, bool p_looping);
	virtual bool stop();
	virtual bool pause();
	virtual bool resume();
	virtual void update();
	
	// Status functions
	virtual const std::string& getCurrentSong() const;
	virtual bool isPlaying() const;
	virtual bool isPaused()  const;
	virtual bool isLooping() const;
	
	// Loading functions
	virtual bool preload( const std::string& p_song);
	virtual bool unload(  const std::string& p_song);
	virtual bool isLoaded(const std::string& p_song) { return m_song == p_song; }
	
	// StreamSource functions
	virtual snd::size_type fillBuffer(snd::size_type  p_frames,
	                                  snd::size_type  p_channels,
	                                  void**          p_buffer,
	                                  snd::size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT);
	virtual snd::size_type fillBufferInterleaved(snd::size_type  p_frames,
	                                             snd::size_type  p_channels,
	                                             void*           p_buffer,
	                                             snd::size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT);
	virtual snd::size_type getSampleSize()   const;
	virtual snd::size_type getBufferSize()   const;
	virtual snd::size_type getFramerate()    const;
	virtual snd::size_type getChannelCount() const;
	
private:
	OggPlayer(const OggPlayer&);
	const OggPlayer& operator=(const OggPlayer&);
	
	/*! \brief Updates the hardware volume, called after setVolume and setFade.
	    \param p_volume The volume to set, fade has already been applied. Range [0.0 - 1.0].*/
	virtual void updateVolume(real p_volume);
	
	bool loadSong(const std::string& p_song);
	
	void destroy();
	
	
	std::string m_song;    //!< Currently playing song.
	bool        m_playing; //!< Whether song is playing.
	bool        m_looping; //!< Whether song is looping.
	bool        m_paused;  //!< Whether song is paused.
	bool        m_currentSongIsPreloaded;
	bool        m_streamHasStarted; //!< Whether stream has actually started playing (after play command was issued)
	
	fs::identifier  m_fs;  //!< Filesystem to use.
	snd::identifier m_snd; //!< Soundsystem to use.
	
	snd::StreamPtr m_stream; //!< Stream for audio output.
	
	codec::Decoder* m_decoder; //!< Decoder.
	
	// StreamSource internals
	snd::size_type m_channelCount; //!< Number of channels.
	snd::size_type m_framerate;    //!< Frames per second.
};


// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_PLAYER_OGGPLAYER_H)
