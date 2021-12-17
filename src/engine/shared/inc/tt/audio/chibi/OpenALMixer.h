#ifndef TT_AUDIO_CHIBI_OPENALMIXER_H
#define TT_AUDIO_CHIBI_OPENALMIXER_H


#if defined(TT_PLATFORM_OSX) && !defined(TT_FORCE_OPENAL_SOFT)
#include <OpenAL/al.h>
#else
#include <al/al.h>
#endif

#include <tt/audio/chibi/XMSoftwareMixer.h>


namespace tt {
namespace audio {
namespace chibi {

class OpenALMixer : public XMSoftwareMixer
{
public:
	/*! \brief Instantiates OpenALMixer.
	    \param p_sampleRate Sample rate at which the mixer should mix.
	    \param p_ups The minimum amount of updates the mixer will receive per second.
	    \param p_buffers. The amount of buffers to use.*/
	OpenALMixer(int p_sampleRate, int p_ups, int p_buffers);
	
	/*! \brief Destroys OpenALMixer.*/
	virtual ~OpenALMixer();
	
	
	// Playback functions
	
	/*! \brief Stops the OpenALMixer.*/
	virtual void stop();
	
	/*! \brief Starts the OpenALMixer.*/
	virtual void play();
	
	virtual void pause();
	virtual void resume();
	
	
	// Misc functions
	
	/*! \brief Updates the OpenALMixer, call this function periodically.
	    \return false when no data remains, true on success.*/
	virtual bool update();
	
	/*! \brief Sets the volume.
	    \param p_volume The new volume [0.0 - 1.0].*/
	virtual void setVolume(real p_volume);
	
private:
	bool playing();
	
	bool stream(ALuint buffer);
	void empty();
	
	bool initializeBuffers(int p_bufferCount);
	bool clearBuffers();
	
	bool initializeSource();
	bool clearSource();
	
	// No copying
	OpenALMixer(const OpenALMixer&);
	OpenALMixer& operator=(const OpenALMixer&);
	
	
	enum
	{
		ChannelCount = 2
	};
	
	ALuint* m_buffers;
	ALuint  m_source;
	int     m_bufferCount;
	
	int m_sampleRate;
	int m_bufferFrames;
	int m_bufferSize;
	
	bool m_active;
	
	s16* m_pcmBuffer;
	s32* m_xmBuffer;
	real m_volume;
};

} // namespace end
}
}

#endif // TT_AUDIO_CHIBI_OPENALMIXER_H
