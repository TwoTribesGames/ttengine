#ifndef INC_TT_SND_VOICE_H
#define INC_TT_SND_VOICE_H

#include <tt/snd/snd.h>
#include <tt/snd/types.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace snd {

class Voice
{
public:
	// Playback functions
	
	/*! \brief Assigns a buffer to a voice.
	    \param p_buffer Buffer to assign to this voice.
	    \return false on fail, true on success.*/
	inline bool setBuffer(const BufferPtr& p_buffer)
	{
		return snd::setVoiceBuffer(m_this.lock(), p_buffer);
	}
	
	/*! \brief Gets the buffer assigned to this voice.
	    \return The buffer assigned to this voice.*/
	inline BufferPtr getBuffer() const
	{
		return snd::getVoiceBuffer(m_this.lock());
	}
	
	/*! \brief Plays a sound.
	    \param p_loop Whether the sound loops.
	    \return false on fail, true on success.*/
	inline bool play(bool p_loop)
	{
		return snd::playVoice(m_this.lock(), p_loop);
	}
	
	/*! \brief Stops a sound.
	    \return false on fail, true on success.*/
	inline bool stop()
	{
		return snd::stopVoice(m_this.lock());
	}
	
	/*! \brief Pauses a sound.
	    \return false on fail, true on success.*/
	inline bool pause()
	{
		return snd::pauseVoice(m_this.lock());
	}
	
	/*! \brief Resumes a sound.
	    \return false on fail, true on success.*/
	inline bool resume()
	{
		return snd::resumeVoice(m_this.lock());
	}
	
	
	// Status functions
	
	/*! \brief Returns whether a sound is playing.
	    \return false when stopped, true when playing.*/
	inline bool isPlaying()
	{
		return snd::isVoicePlaying(m_this.lock());
	}
	
	/*! \brief Returns whether a sound is paused.
	    \return false when not paused, true when paused.*/
	inline bool isPaused()
	{
		return snd::isVoicePaused(m_this.lock());
	}
	
	
	// Parameter functions
	
	/*! \brief Gets the sound loop start in samples.
	    \return The loop start.*/
	inline size_type getLoopStart()
	{
		return snd::getVoiceLoopStart(m_this.lock());
	}
	
	/*! \brief Sets the sound loop start in samples.
	    \param p_length The loop start of the sound in samples.
	    \return Whether the operation succeeded.*/
	inline bool setLoopStart(size_type p_loopStart)
	{
		return snd::setVoiceLoopStart(m_this.lock(), p_loopStart);
	}
	
	/*! \brief Gets the sound playback ratio.
	    \return The playback ratio.*/
	inline real getPlaybackRatio()
	{
		return snd::getVoicePlaybackRatio(m_this.lock());
	}
	
	/*! \brief Sets the sound playback ratio.
	    \param p_playbackRatio The playback ratio of the sound.
	    \return Whether the operation succeeded.*/
	inline bool setPlaybackRatio(real p_playbackRatio)
	{
		return snd::setVoicePlaybackRatio(m_this.lock(), p_playbackRatio);
	}
	
	/*! \brief Gets the sound volume ratio.
	    \return The volume ratio.*/
	inline real getVolumeRatio()
	{
		return snd::getVoiceVolumeRatio(m_this.lock());
	}
	
	/*! \brief Sets the sound volume ratio.
	    \param p_volumeRatio The volume ratio of the sound.
	    \return Whether the operation succeeded.*/
	inline bool setVolumeRatio(real p_volumeRatio)
	{
		return snd::setVoiceVolumeRatio(m_this.lock(), p_volumeRatio);
	}
	
	/*! \brief Gets the sound volume in dB.
	    \return The volume in dB.*/
	inline real getVolume()
	{
		return snd::getVoiceVolume(m_this.lock());
	}
	
	/*! \brief Sets the sound volume in dB.
	    \param p_volume The volume of the sound in dB.
	    \return Whether the operation succeeded.*/
	inline bool setVolume(real p_volume)
	{
		return snd::setVoiceVolume(m_this.lock(), p_volume);
	}
	
	/*! \brief Gets the sound panning.
	    \return The panning.*/
	inline real getPanning()
	{
		return snd::getVoicePanning(m_this.lock());
	}
	
	/*! \brief Sets the sound panning.
	    \param p_panning The panning of the sound.
	    \return Whether the operation succeeded.*/
	inline bool setPanning(real p_panning)
	{
		return snd::setVoicePanning(m_this.lock(), p_panning);
	}
	
	/*! \brief Gets the sound panning in degrees.
	    \return The panning in degrees.*/
	inline real get360Panning()
	{
		return snd::getVoice360Panning(m_this.lock());
	}
	
	/*! \brief Sets the sound panning in degrees.
	    \param p_direction The panning of the sound in degrees.
	    \return Whether the operation succeeded.*/
	inline bool set360Panning(real p_panning)
	{
		return snd::setVoice360Panning(m_this.lock(), p_panning);
	}
	
	/*! \brief Gets the sound's priority.
	    \return The priority.*/
	inline size_type getPriority()
	{
		return snd::getVoicePriority(m_this.lock());
	}
	
	/*! \brief Sets the sound priority.
	    \param p_priority The priority of the sound.
	    \return Whether the operation succeeded.*/
	inline bool setPriority(size_type p_priority)
	{
		return snd::setVoicePriority(m_this.lock(), p_priority);
	}
	
	
	inline bool setPosition(const math::Vector3& p_position)
	{
		return snd::setVoicePosition(m_this.lock(), p_position);
	}
	
	inline bool setRadius(real p_inner, real p_outer)
	{
		return snd::setVoiceRadius(m_this.lock(), p_inner, p_outer);
	}

	inline bool setReverbVolume(real p_volume)
	{
		return snd::setVoiceReverbVolume(m_this.lock(), p_volume);
	}

	inline bool setLowPassFilterEnabled(bool p_enabled)
	{
		return snd::setVoiceLowPassFilterEnabled(m_this.lock(), p_enabled);
	}

	inline bool setLowPassFilterFrequency(size_type p_frequency)
	{
		return snd::setVoiceLowPassFilterFrequency(m_this.lock(), p_frequency);
	}
	
	// Misc functions
	
	/*! \brief Gets the sound's internal data.
	    \return The sound's internal data.*/
	inline void* getData() const { return m_data; }
	
	/*! \brief Sets the sound's internal data.
	    \param p_data The data to set. */
	inline void setData(void* p_data) { m_data = p_data; }
	
	/*! \brief Gets the sound's soundsystem identifier.
	    \return The sound's soundsystem identifier.*/
	inline identifier getSoundSystem() const { return m_identifier; }
	
private:
	/*! \brief Constructs a sound
	    \param p_identifier The soundsystem assigned to the sound.*/
	inline Voice(identifier p_identifier)
	:
	m_identifier(p_identifier),
	m_data(0),
	m_this()
	{
	}
	
	/*! \brief Destructs a file. */
	inline ~Voice()
	{
		snd::closeVoice(this);
		if (m_data != 0)
		{
			TT_PANIC("Cleanup failed for sound");
		}
	}
	
	Voice(const Voice& p_rhs);
	Voice& operator=(const Voice& p_rhs);
	
	static void deleteVoice(Voice* Voice);
	
	friend VoicePtr openVoice(size_type, identifier);
	friend bool closeVoice(Voice*);
	
	identifier  m_identifier;
	void*       m_data;
	
	tt_ptr<Voice>::weak m_this;
	
};

} // namespace end
}

#endif // INC_TT_SND_VOICE_H
