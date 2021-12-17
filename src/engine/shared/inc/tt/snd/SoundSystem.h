#if !defined(INC_TT_SND_SOUNDSYSTEM_H)
#define INC_TT_SND_SOUNDSYSTEM_H


#include <tt/math/fwd.h>
#include <tt/snd/types.h>


namespace tt {

namespace audio {
namespace xact {
class RPCCurve;
}
}

namespace snd {

class SoundSystem
{
public:
	SoundSystem(identifier p_id);
	virtual ~SoundSystem();
	
	// Capability functions
	
	/*! \brief Checks whether stereo sound is supported.
	    \return Whether stereo is supported.*/
	virtual bool supportsStereo();
	
	/*! \brief Checks whether sound system suspending is supported.
	    \return Whether sound system suspending is supported.*/
	virtual bool supportsSuspending();
	
	
	// Master volume functions
	
	/*! \brief Gets the master volume
	    \return The master volume (0.0 - 1.0).*/
	virtual real getMasterVolume();
	
	/*! \brief Sets the master volume
	    \param p_volume The master volume (0.0 - 1.0).*/
	virtual void setMasterVolume(real p_volume);
	
	/*! \brief Gets the master volume for a secondary output device.
	    \return The master volume (0.0 - 1.0).*/
	virtual real getSecondaryMasterVolume();
	
	/*! \brief Sets the master volume for a secondary output device.
	    \param p_volume The master volume (0.0 - 1.0).*/
	virtual void setSecondaryMasterVolume(real p_volume);
	
	
	// Master suspend functions
	
	/*! \brief Suspends the audio system.
	    \return Suspending succeeded.*/
	virtual bool suspend();
	
	/*! \brief Resumes the audio system.
	    \return Resuming succeeded.*/
	virtual bool resume();
	
	
	// Voice Creation functions
	
	/*! \brief Opens a voice.
	    \param p_voice The Voice object to use.
	    \param p_priority The priority for the voice.
	    \return null ptr on fail, pointer to voice object on success.*/
	virtual bool openVoice(const VoicePtr& p_voice, size_type p_priority) = 0;
	
	/*! \brief Closes a voice.
	    \param p_voice The voice to close.
	    \return false on fail, true on success.*/
	virtual bool closeVoice(Voice* p_voice) = 0;
	
	// Voice Playback functions
	
	/*! \brief Plays a voice.
	    \param p_voice The voice to play.
	    \param p_loop Whether the voice loops.
	    \return false on fail, true on success.*/
	virtual bool playVoice(const VoicePtr& p_voice, bool p_loop) = 0;
	
	/*! \brief Stops a voice.
	    \param p_voice The voice to stop.
	    \return false on fail, true on success.*/
	virtual bool stopVoice(const VoicePtr& p_voice) = 0;
	
	/*! \brief Pauses a voice.
	    \param p_voice The voice to pause.
	    \return false on fail, true on success.*/
	virtual bool pauseVoice(const VoicePtr& p_voice);
	
	/*! \brief Resumes a voice.
	    \param p_voice The voice to resume.
	    \return false on fail, true on success.*/
	virtual bool resumeVoice(const VoicePtr& p_voice);
	
	
	// Voice Status functions
	
	/*! \brief Returns whether a voice is playing.
	    \param p_voice The voice to get the play status of.
	    \return false when stopped, true when playing.*/
	virtual bool isVoicePlaying(const VoicePtr& p_voice) = 0;
	
	/*! \brief Returns whether a voice is paused.
	    \param p_voice The voice to get the pause status of.
	    \return false when not paused, true when paused.*/
	virtual bool isVoicePaused(const VoicePtr& p_voice);
	
	
	// Voice Parameter functions
	
	/*! \brief Gets the buffer assigned to the voice.
	    \param p_voice The voice to get the loop start of.
	    \return The buffer.*/
	virtual BufferPtr getVoiceBuffer(const VoicePtr& p_voice) = 0;
	
	/*! \brief Assigns a buffer to a voice.
	    \param p_voice The voice to set the buffer of.
	    \param p_buffer The buffer to assign to the voice.
	    \return Whether the operation succeeded.*/
	virtual bool setVoiceBuffer(const VoicePtr& p_voice, const BufferPtr& p_buffer) = 0;
	
	/*! \brief Gets the voice loop start in samples.
	    \param p_voice The voice to get the loop start of.
	    \return The loop start.*/
	virtual size_type getVoiceLoopStart(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice loop start in samples.
	    \param p_voice The voice to set the loop start of.
	    \param p_length The loop start of the voice in samples.
	    \return Whether the operation succeeded.*/
	virtual bool setVoiceLoopStart(const VoicePtr& p_voice, size_type p_loopStart);
	
	/*! \brief Gets the voice playback ratio.
	    \param p_voice The voice to get the playback ratio of.
	    \return The playback ratio.*/
	virtual real getVoicePlaybackRatio(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice playback ratio.
	    \param p_voice The voice to set the playback ratio of.
	    \param p_playbackRatio The playback ratio of the voice.
	    \return Whether the operation succeeded.*/
	virtual bool setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio);
	
	/*! \brief Gets the voice volume ratio.
	    \param p_voice The voice to get the volume ratio of.
	    \return The volume ratio.*/
	virtual real getVoiceVolumeRatio(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice volume ratio.
	    \param p_voice The voice to set the volume ratio of.
	    \param p_volumeRatio The volume ratio of the voice.
	    \return Whether the operation succeeded.*/
	virtual bool setVoiceVolumeRatio(const VoicePtr& p_voice, real p_volumeRatio);
	
	/*! \brief Gets the voice volume in dB.
	    \param p_voice The voice to get the volume of.
	    \return The volume in dB.*/
	virtual real getVoiceVolume(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice volume in dB.
	    \param p_voice The voice to set the volume of.
	    \param p_volume The volume of the voice in dB.
	    \return Whether the operation succeeded.*/
	virtual bool setVoiceVolume(const VoicePtr& p_voice, real p_volume);
	
	/*! \brief Gets the voice panning.
	    \param p_voice The voice to get the panning of.
	    \return The panning.*/
	virtual real getVoicePanning(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice panning.
	    \param p_voice The voice to set the panning of.
	    \param p_panning The panning of the voice: -1.0f is hard left, 0.0f is center, +1.0f is hard right
	    \return Whether the operation succeeded.*/
	virtual bool setVoicePanning(const VoicePtr& p_voice, real p_panning);
	
	/*! \brief Gets the voice panning in degrees.
	    \param p_voice The voice to get the panning of.
	    \return The panning in degrees.*/
	virtual real getVoice360Panning(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice panning in degrees.
	    \param p_voice The voice to set the panning of.
	    \param p_direction The panning of the voice in degrees.
	    \return Whether the operation succeeded.*/
	virtual bool setVoice360Panning(const VoicePtr& p_voice, real p_panning);
	
	/*! \brief Gets the voice's priority.
	    \param p_voice The voice to get the priority of.
	    \return The priority.*/
	virtual size_type getVoicePriority(const VoicePtr& p_voice);
	
	/*! \brief Sets the voice priority.
	    \param p_voice The voice to set the priority of.
	    \param p_priority The priority of the voice.
	    \return Whether the operation succeeded.*/
	virtual bool setVoicePriority(const VoicePtr& p_voice, size_type p_priority);
	
	/*! \brief Sets the voice position (positional audio).
	    \param p_voice The voice to set the position of.
	    \param p_position The position of the voice.
	    \return Whether the operation succeeded.*/
	virtual bool setVoicePosition(const VoicePtr& p_voice, const math::Vector3& p_position);
	
	/*! \brief Sets the voice radius (positional audio).
	    \param p_voice The voice to set the radius of.
	    \param p_inner The inner radius of the voice (distance at which it can be heard fully).
	    \param p_outer The outer radius of the voice (distance at which it can no longer be heard).
	    \return Whether the operation succeeded.*/
	virtual bool setVoiceRadius(const VoicePtr& p_voice, real p_inner, real p_outer);

	/*! \brief Set the volume of the reverb effect
	    \param p_voice The voice to set the reverb volume on
	    \param p_volume Volume to set in dB
	    \return Whether the operation succeeded.*/
	virtual bool setVoiceReverbVolume(const VoicePtr& p_voice, real p_volume);

	/*! \brief Turn on reverb effect
	    \param p_preset Available preset of reverb settings
	    \return Whether the operation succeeded.*/
	virtual bool setReverbEffect(ReverbPreset p_preset);

	/*! \brief Turn on/off low pass filter
		\param p_enabled Whether the low pass filter should be enabled
		\return Whether the operation succeeded.*/
	virtual bool setVoiceLowPassFilterEnabled(const VoicePtr& p_voice, bool p_enabled);

	/*! \brief Set rolloff frequency for the low pass filter
		\param p_frequency Maximum frequency that is still audible
		\return Whether the operation succeeded.*/
	virtual bool setVoiceLowPassFilterFrequency(const VoicePtr& p_voice, size_type p_frequency);
	
	
	// Buffer creation functions
	
	/*! \brief Opens a Buffer.
	    \param p_buffer The buffer object to open.
	    \return false on fail, true on success.*/
	virtual bool openBuffer(const BufferPtr& p_buffer) = 0;
	
	/*! \brief Closes a buffer.
	    \param p_buffer The buffer to close.
	    \return false on fail, true on success.*/
	virtual bool closeBuffer(Buffer* p_buffer) = 0;
	
	
	// Buffer Parameter functions
	
	/*! \brief Assigns data to a buffer.
	    \param p_buffer The buffer to assign data to.
	    \param p_data The raw data to assign.
	    \param p_frames The number of audio frames within p_data.
	    \param p_channels The number of channels in p_data.
	    \param p_sampleSize The size of the samples in bits.
	    \param p_sampleRate The native samplerate of the data in Hz.
	    \param p_ownership Whether the buffer should take ownership of p_data.
	    \return Whether the operation succeeded.*/
	virtual bool setBufferData(const BufferPtr& p_buffer,
	                           const void*      p_data,
	                           size_type        p_frames,
	                           size_type        p_channels,
	                           size_type        p_sampleSize,
	                           size_type        p_sampleRate,
	                           bool             p_ownership) = 0;
	
	/*! \brief Gets the buffer length in frames.
	    \param p_buffer The buffer to get the length of.
	    \return The number of frames in the buffer.*/
	virtual size_type getBufferLength(const BufferPtr& p_buffer) = 0;
	
	/*! \brief Gets the number of channels in the buffer.
	    \param p_buffer The buffer to get the channel count.
	    \return The number of channels.*/
	virtual size_type getBufferChannelCount(const BufferPtr& p_buffer) = 0;
	
	/*! \brief Gets the buffer sample size in bits.
	    \param p_buffer The buffer to get the sample size of.
	    \return The sample size.*/
	virtual size_type getBufferSampleSize(const BufferPtr& p_buffer) = 0;
	
	/*! \brief Gets the buffer sample rate in Hz.
	    \param p_buffer The buffer to get the sample rate of.
	    \return The sample rate.*/
	virtual size_type getBufferSampleRate(const BufferPtr& p_buffer) = 0;
	
	
	// Stream functions
	
	/*! \brief Opens a stream.
	    \param p_stream The Stream object to use.
	    \return null ptr on fail, pointer to sound object on success.*/
	virtual bool openStream(const StreamPtr& p_stream);
	
	/*! \brief Closes a stream.
	    \param p_stream The stream to close.
	    \return false on fail, true on success.*/
	virtual bool closeStream(Stream* p_stream);
	
	
	// Playback functions
	
	/*! \brief Plays a stream.
	    \param p_stream The stream to play.
	    \return false on fail, true on success.*/
	virtual bool playStream(const StreamPtr& p_stream);
	
	/*! \brief Stops a stream.
	    \param p_stream The stream to stop.
	    \return false on fail, true on success.*/
	virtual bool stopStream(const StreamPtr& p_stream);
	
	/*! \brief Pauses a stream.
	    \param p_stream The stream to pause.
	    \return false on fail, true on success.*/
	virtual bool pauseStream(const StreamPtr& p_stream);
	
	/*! \brief Resumes a stream.
	    \param p_stream The stream to resume.
	    \return false on fail, true on success.*/
	virtual bool resumeStream(const StreamPtr& p_stream);
	
	/*! \brief Updates a stream.
	    \param p_stream The stream to update.
	    \return false on fail, true on success.*/
	virtual bool updateStream(const StreamPtr& p_stream);
	
	
	// Status functions
	
	/*! \brief Returns whether a stream is playing.
	    \param p_stream The stream to get the play status of.
	    \return false when stopped, true when playing.*/
	virtual bool isStreamPlaying(const StreamPtr& p_stream) = 0;
	
	/*! \brief Returns whether a stream is paused.
	    \param p_stream The stream to get the pause status of.
	    \return false when not paused, true when paused.*/
	virtual bool isStreamPaused(const StreamPtr& p_stream) = 0;
	
	
	// Parameter functions
	
	/*! \brief Gets the stream volume ratio.
	    \param p_stream The stream to get the volume ratio of.
	    \return The volume ratio.*/
	virtual real getStreamVolumeRatio(const StreamPtr& p_stream);
	
	/*! \brief Sets the stream volume ratio.
	    \param p_stream The stream to set the volume ratio of.
	    \param p_volumeRatio The volume ratio of the stream.
	    \return Whether the operation succeeded.*/
	virtual bool setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio);
	
	/*! \brief Gets the stream volume in dB.
	    \param p_stream The stream to get the volume of.
	    \return The volume in dB.*/
	virtual real getStreamVolume(const StreamPtr& p_stream);
	
	/*! \brief Sets the stream volume in dB.
	    \param p_stream The stream to set the volume of.
	    \param p_volume The volume of the stream in dB.
	    \return Whether the operation succeeded.*/
	virtual bool setStreamVolume(const StreamPtr& p_stream, real p_volume);
	
	
	// Misc functions
	
	inline identifier getIdentifier() const { return m_identifier; }


	// Positional Audio

	/*! \brief Enables the use of positional audio (overrides panning settings)
	    \param p_enabled Whether 3D audio should be enabled or disabled
		\return Whether the operation succeeded.*/
	virtual bool set3DAudioEnabled(bool p_enabled) { m_3DAudioEnabled = p_enabled; return true; }

	virtual bool is3DAudioEnabled() { return m_3DAudioEnabled; }

	/*! \brief Sets the position of the global listener (for positional audio)
	    \param p_position The new position for the global listener. */
	virtual bool setListenerPosition(const math::Vector3& p_position);

	virtual const math::Vector3& getListenerPosition() const;
	
	/*! \brief Set the distance model for positional audio.
	    \param p_curve The curve to use for distance fading
	    \return Whether the operation succeeded.*/
	virtual bool setPositionalAudioModel(const VoicePtr& p_voice, const audio::xact::RPCCurve* p_curve);

	virtual void updateProfileInfo() {};
	virtual void renderProfileInfo() {};
	virtual void resetMaxProfile  () {};
	
	
private:
	SoundSystem(const SoundSystem& p_rhs);
	SoundSystem& operator=(const SoundSystem& p_rhs);
	
	identifier m_identifier;
	bool       m_registered;
	bool       m_3DAudioEnabled;
	
	friend bool registerSoundSystem(SoundSystem*, identifier);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_SND_SOUNDSYSTEM_H)
