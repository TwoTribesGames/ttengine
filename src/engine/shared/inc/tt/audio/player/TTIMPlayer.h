#if !defined(INC_TT_AUDIO_PLAYER_TTIMPLAYER_H)
#define INC_TT_AUDIO_PLAYER_TTIMPLAYER_H


#include <map>
#include <vector>

#include <tt/audio/codec/Decoder.h>
#include <tt/audio/player/MusicPlayer.h>
#include <tt/fs/types.h>
#include <tt/snd/StreamSource.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace player {

/*! \brief Plays music sequences defined in .ttim files
           (supports Ogg Vorbis, ttadpcm and wav files). */
class TTIMPlayer : public MusicPlayer, public snd::StreamSource
{
public:
	explicit TTIMPlayer(fs::identifier p_fs, snd::identifier p_snd = 0);
	virtual ~TTIMPlayer();
	
	// Playback functions
	virtual bool play(const std::string& p_song, bool p_looping);
	virtual bool stop();
	virtual bool pause();
	virtual bool resume();
	virtual void update();
	
	// Status functions
	virtual const std::string& getCurrentSong() const;
	virtual bool               isPlaying()      const;
	virtual bool               isPaused()       const;
	virtual bool               isLooping()      const;
	
	// Loading functions
	virtual bool preload (const std::string& p_song);
	virtual bool unload  (const std::string& p_song);
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
	virtual void           onStreamReachedNotificationFrame();
	
	// TTIM specific functions
	bool preloadBlock(const std::string& p_song, s32 p_block);
	inline s32 getCurrentBlock() const { return m_currentBlock == -1 ? 0 : m_currentBlock; }
	
private:
	TTIMPlayer(const TTIMPlayer&);
	const TTIMPlayer& operator=(const TTIMPlayer&);
	
	/*! \brief Updates the hardware volume, called after setVolume and setFade.
	    \param p_volume The volume to set, fade has already been applied. Range [0.0 - 1.0].*/
	virtual void updateVolume(real p_volume);
	
	bool preloadCurrentBlock();
	bool loadSong(const std::string& p_song);
	
	void destroy();
	
	codec::Decoder* getNextDecoder();
	codec::Decoder* findDecoder(const std::string& p_label);
	
	typedef std::map<std::string, std::string>         Properties;
	typedef std::vector<std::string>                   Musicblocks;
	typedef std::vector<codec::Decoder*>               Decoders;
	typedef std::vector<Musicblocks::size_type>        Playlist;
	typedef std::map<Playlist::size_type, std::string> Gotos;
	typedef std::map<std::string, Playlist::size_type> Labels;
	
	
	std::string m_song;    //!< Currently playing song.
	bool        m_looping; //!< Whether song is looping.
	bool        m_playing; //!< Whether song is playing.
	bool        m_paused;  //!< Whether song is paused.
	bool        m_currentBlockIsPreloaded;
	bool        m_streamHasStarted;  //!< Whether stream has actually started playing (after play command was issued)
	
	fs::identifier  m_fs;  //!< Filesystem to use.
	snd::identifier m_snd; //!< Soundsystem to use.
	
	snd::StreamPtr m_stream; //!< Stream for audio output.
	
	// TTIM internals
	Properties          m_properties;   //!< Global song properties.
	Labels              m_labels;       //!< Labels within song.
	Gotos               m_gotos;        //!< Jumps within song.
	Playlist            m_playlist;     //!< Play order.
	Musicblocks         m_musicblocks;  //!< Song parts.
	Decoders            m_decoders;     //!< Music decoders.
	real                m_songVolume;   //!< Song volume.
	codec::Decoder*     m_decoder;      //!< Current block decoder.
	s32                 m_currentBlock; //!< Index to current block.
	
	// StreamSource internals
	snd::size_type m_channelCount; //!< Number of channels.
	snd::size_type m_framerate;    //!< Frames per second.
	
	// Loop notification support
	bool m_notifyClientAboutLoopNextUpdate;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_TTIMPLAYER_H)
