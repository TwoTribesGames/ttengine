#ifndef TT_AUDIO_CHIBI_STREAMMIXER_H
#define TT_AUDIO_CHIBI_STREAMMIXER_H

#include <tt/audio/chibi/XMSoftwareMixer.h>
#include <tt/snd/StreamSource.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace chibi {

class StreamMixer : public XMSoftwareMixer, public snd::StreamSource
{
public:
	/*! \brief Instantiates StreamMixer.
	    \param p_sampleRate Sample rate at which the mixer should mix.
	    \param p_bufferSize Amount of samples that should be buffered.
	    \param p_snd Sound system to use.*/
	StreamMixer(int p_sampleRate, int p_bufferSize, snd::identifier p_snd = 0);
	virtual ~StreamMixer();
	
	
	// Playback functions
	
	/*! \brief Stops the StreamMixer.*/
	virtual void stop();
	
	/*! \brief Starts the StreamMixer.*/
	virtual void play();
	
	virtual void pause();
	virtual void resume();
	
	virtual bool update();
	
	/*! \brief Sets the volume.
	    \param p_volume The new volume [0.0 - 1.0].*/
	virtual void setVolume(real p_volume);
	
	
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
	// No copying
	StreamMixer(const StreamMixer&);
	StreamMixer& operator=(const StreamMixer&);
	
	
	snd::identifier m_snd; //!< Soundsystem to use.
	
	snd::StreamPtr m_stream; //!< Stream for audio output.
	
	// StreamSource internals
	snd::size_type m_channelCount; //!< Number of channels.
	snd::size_type m_framerate;    //!< Frames per second.
	snd::size_type m_bufferSize;   //!< Number of frames for all buffers.
	
	real m_volume;
	s32* m_xmBuffer;
};

} // namespace end
}
}

#endif // TT_AUDIO_CHIBI_STREAMMIXER_H
