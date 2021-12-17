#if !defined(INC_TT_SND_SND_H)
#define INC_TT_SND_SND_H


#include <tt/math/fwd.h>
#include <tt/snd/types.h>


namespace tt {

namespace audio {
namespace xact {
class RPCCurve;
}
}

namespace snd {

// Soundsystem functions

/*! \brief Registers a soundsystem.
    \param p_soundSystem The SoundSystem to register.
    \param p_identifier The identifier to assign to the soundsystem.
    \return false on fail, true on success.*/
bool registerSoundSystem(SoundSystem* p_soundSystem, identifier p_identifier);

/*! \brief Unregisters a soundsystem.
    \param p_identifier The identifier of the soundsystem to unregister.
    \return false on fail, true on success.*/
bool unregisterSoundSystem(identifier p_identifier);

/*! \brief Checks whether a soundsystem with the specified identifier exists.
    \param p_identifier The identifier of the soundsystem to check.
    \return Whether the soundsystem exists.*/
bool hasSoundSystem(identifier p_identifier);


// Capability functions

/*! \brief Checks whether stereo sound is supported.
    \param p_identifier soundsystem to check.
    \return Whether stereo is supported.*/
bool supportsStereo(identifier p_identifier);

/*! \brief Checks whether Master Suspending is supported.
    \param p_identifier soundsystem to check.
    \return Whether suspending is supported.*/
bool supportsSuspending(identifier p_identifier);


// Master volume functions

/*! \brief Gets the master volume
    \param p_identifier Soundsystem to get the volume of.
    \return The master volume (0.0 - 1.0).*/
real getMasterVolume(identifier p_identifier);

/*! \brief Sets the master volume
    \param p_identifier Soundsystem to set the volume of.
    \param p_volume The master volume (0.0 - 1.0).*/
void setMasterVolume(identifier p_identifier, real p_volume);

/*! \brief Gets the master volume for a secondary output device.
    \param p_identifier Soundsystem to get the volume of.
    \return The master volume (0.0 - 1.0).*/
real getSecondaryMasterVolume(identifier p_identifier);

/*! \brief Sets the master volume for a secondary output device.
    \param p_identifier Soundsystem to set the volume of.
    \param p_volume The master volume (0.0 - 1.0).*/
void setSecondaryMasterVolume(identifier p_identifier, real p_volume);


// Master suspend functions

/*! \brief Suspends the audio system.
    \param p_identifier soundsystem to suspend.
    \return Suspending succeeded.*/
bool suspend(identifier p_identifier);

/*! \brief Resumes the audio system.
    \param p_identifier soundsystem to resume.
    \return Resuming succeeded.*/
bool resume(identifier p_identifier);


// Voice creation functions

/*! \brief Opens a voice.
    \param p_priority The priority for the voice.
    \param p_identifier soundsystem to use.
    \return null ptr on fail, pointer to voice object on success.*/
VoicePtr openVoice(size_type p_priority, identifier p_identifier = 0);

/*! \brief Closes a voice.
    \param p_voice The voice to close.
    \return false on fail, true on success.*/
bool closeVoice(Voice* p_voice);


// Voice Playback functions

/*! \brief Plays a voice.
    \param p_voice The voice to play.
    \param p_loop Whether the voice loops.
    \return false on fail, true on success.*/
bool playVoice(const VoicePtr& p_voice, bool p_loop);

/*! \brief Stops a voice.
    \param p_voice The voice to stop.
    \return false on fail, true on success.*/
bool stopVoice(const VoicePtr& p_voice);

/*! \brief Pauses a voice.
    \param p_voice The voice to pause.
    \return false on fail, true on success.*/
bool pauseVoice(const VoicePtr& p_voice);

/*! \brief Resumes a voice.
    \param p_voice The voice to resume.
    \return false on fail, true on success.*/
bool resumeVoice(const VoicePtr& p_voice);


// Voice Status functions

/*! \brief Returns whether a voice is playing.
    \param p_voice The voice to get the play status of.
    \return false when stopped, true when playing.*/
bool isVoicePlaying(const VoicePtr& p_voice);

/*! \brief Returns whether a voice is paused.
    \param p_voice The voice to get the pause status of.
    \return false when not paused, true when paused.*/
bool isVoicePaused(const VoicePtr& p_voice);


// Voice Parameter functions

/*! \brief Gets the buffer assigned to a voice.
    \param p_voice The voice to get the buffer of.
    \return Pointer to the buffer.*/
BufferPtr getVoiceBuffer(const VoicePtr& p_voice);

/*! \brief Assigns a buffer to a voice.
    \param p_voice The voice to set the buffer of.
    \param p_buffer The buffer to assign to the voice.
    \return Whether the operation succeeded.*/
bool setVoiceBuffer(const VoicePtr& p_voice, const BufferPtr& p_buffer);

/*! \brief Gets the voice loop start in samples.
    \param p_voice The voice to get the loop start of.
    \return The loop start.*/
size_type getVoiceLoopStart(const VoicePtr& p_voice);

/*! \brief Sets the voice loop start in samples.
    \param p_voice The voice to set the loop start of.
    \param p_length The loop start of the voice in samples.
    \return Whether the operation succeeded.*/
bool setVoiceLoopStart(const VoicePtr& p_voice, size_type p_loopStart);

/*! \brief Gets the voice playback ratio.
    \param p_voice The voice to get the playback ratio of.
    \return The playback ratio.*/
real getVoicePlaybackRatio(const VoicePtr& p_voice);

/*! \brief Sets the voice playback ratio.
    \param p_voice The voice to set the playback ratio of.
    \param p_playbackRatio The playback ratio of the voice.
    \return Whether the operation succeeded.*/
bool setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio);

/*! \brief Gets the voice volume ratio.
    \param p_voice The voice to get the volume ratio of.
    \return The volume ratio.*/
real getVoiceVolumeRatio(const VoicePtr& p_voice);

/*! \brief Sets the voice volume ratio.
    \param p_voice The voice to set the volume ratio of.
    \param p_volumeRatio The volume ratio of the voice.
    \return Whether the operation succeeded.*/
bool setVoiceVolumeRatio(const VoicePtr& p_voice, real p_volumeRatio);

/*! \brief Gets the voice volume in dB.
    \param p_voice The voice to get the volume of.
    \return The volume in dB.*/
real getVoiceVolume(const VoicePtr& p_voice);

/*! \brief Sets the voice volume in dB.
    \param p_voice The voice to set the volume of.
    \param p_volume The volume of the voice in dB.
    \return Whether the operation succeeded.*/
bool setVoiceVolume(const VoicePtr& p_voice, real p_volume);

/*! \brief Gets the voice panning.
    \param p_voice The voice to get the panning of.
    \return The panning.*/
real getVoicePanning(const VoicePtr& p_voice);

/*! \brief Sets the voice panning.
    \param p_voice The voice to set the panning of.
    \param p_panning The panning of the voice.
    \return Whether the operation succeeded.*/
bool setVoicePanning(const VoicePtr& p_voice, real p_panning);

/*! \brief Gets the voice panning in degrees.
    \param p_voice The voice to get the panning of.
    \return The panning in degrees.*/
real getVoice360Panning(const VoicePtr& p_voice);

/*! \brief Sets the voice panning in degrees.
    \param p_voice The voice to set the panning of.
    \param p_direction The panning of the voice in degrees.
    \return Whether the operation succeeded.*/
bool setVoice360Panning(const VoicePtr& p_voice, real p_panning);

/*! \brief Gets the voice's priority.
    \param p_voice The voice to get the priority of.
    \return The priority.*/
size_type getVoicePriority(const VoicePtr& p_voice);

/*! \brief Sets the voice priority.
    \param p_voice The voice to set the priority of.
    \param p_priority The priority of the voice.
    \return Whether the operation succeeded.*/
bool setVoicePriority(const VoicePtr& p_voice, size_type p_priority);

/*! \brief Sets the voice position (positional audio).
    \param p_voice The voice to set the position of.
    \param p_position The position of the voice.
    \return Whether the operation succeeded.*/
bool setVoicePosition(const VoicePtr& p_voice, const math::Vector3& p_position);

/*! \brief Sets the voice radius (positional audio).
    \param p_voice The voice to set the radius of.
    \param p_inner The inner radius of the voice (distance at which it can be heard fully).
    \param p_outer The outer radius of the voice (distance at which it can no longer be heard).
    \return Whether the operation succeeded.*/
bool setVoiceRadius(const VoicePtr& p_voice, real p_inner, real p_outer);

/*! \brief Set volume of the reverb effect
    \param p_voice The voice to set the reverb volume on
    \param p_preset Available preset of reverb settings
    \return Whether the operation succeeded.*/
bool setVoiceReverbVolume(const VoicePtr& p_voice, real p_volume);

/*! \brief Turn on reverb effect
    \param p_preset Available preset of reverb settings
    \return Whether the operation succeeded.*/
bool setReverbEffect(ReverbPreset p_preset, identifier p_identifier = 0);

/*! \brief Turn on/off low pass filter
    \param p_enabled Whether the low pass filter should be enabled
    \return Whether the operation succeeded.*/
bool setVoiceLowPassFilterEnabled(const VoicePtr& p_voice, bool p_enabled);

/*! \brief Set rolloff frequency for the low pass filter
    \param p_frequency Maximum frequency that is still audible
    \return Whether the operation succeeded.*/
bool setVoiceLowPassFilterFrequency(const VoicePtr& p_voice, size_type p_frequency);

// Buffer creation functions

/*! \brief Opens a Buffer.
    \param p_identifier soundsystem to use.
    \return null ptr on fail, pointer to buffer object on success.*/
BufferPtr openBuffer(identifier p_identifier = 0);

/*! \brief Closes a buffer.
    \param p_buffer The buffer to close.
    \return false on fail, true on success.*/
bool closeBuffer(Buffer* p_buffer);


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
bool setBufferData(const BufferPtr& p_buffer,
                   const void*      p_data,
                   size_type        p_frames,
                   size_type        p_channels,
                   size_type        p_sampleSize,
                   size_type        p_sampleRate,
                   bool             p_ownership);

/*! \brief Gets the buffer length in frames.
    \param p_buffer The buffer to get the length of.
    \return The number of frames in the buffer.*/
size_type getBufferLength(const BufferPtr& p_buffer);

/*! \brief Gets the number of channels in the buffer.
    \param p_buffer The buffer to get the channel count.
    \return The number of channels.*/
size_type getBufferChannelCount(const BufferPtr& p_buffer);

/*! \brief Gets the buffer sample size in bits.
    \param p_buffer The buffer to get the sample size of.
    \return The sample size.*/
size_type getBufferSampleSize(const BufferPtr& p_buffer);

/*! \brief Gets the buffer sample rate in Hz.
    \param p_buffer The buffer to get the sample rate of.
    \return The sample rate.*/
size_type getBufferSampleRate(const BufferPtr& p_buffer);


// Stream functions

/*! \brief Opens a stream.
    \param p_source The source for the stream.
    \param p_identifier soundsystem to use.
    \return null ptr on fail, pointer to sound object on success.*/
StreamPtr openStream(StreamSource* p_source, identifier p_identifier = 0);

/*! \brief Closes a stream.
    \param p_stream The stream to close.
    \return false on fail, true on success.*/
bool closeStream(Stream* p_stream);


// Playback functions

/*! \brief Plays a stream.
    \param p_stream The stream to play.
    \return false on fail, true on success.*/
bool playStream(const StreamPtr& p_stream);

/*! \brief Stops a stream.
    \param p_stream The stream to stop.
    \return false on fail, true on success.*/
bool stopStream(const StreamPtr& p_stream);

/*! \brief Pauses a stream.
    \param p_stream The stream to pause.
    \return false on fail, true on success.*/
bool pauseStream(const StreamPtr& p_stream);

/*! \brief Resumes a stream.
    \param p_stream The stream to resume.
    \return false on fail, true on success.*/
bool resumeStream(const StreamPtr& p_stream);

/*! \brief Updates a stream.
    \param p_stream The stream to update.
    \return false on fail, true on success.*/
bool updateStream(const StreamPtr& p_stream);


// Status functions

/*! \brief Returns whether a stream is playing.
    \param p_stream The stream to get the play status of.
    \return false when stopped, true when playing.*/
bool isStreamPlaying(const StreamPtr& p_stream);

/*! \brief Returns whether a stream is paused.
    \param p_stream The stream to get the pause status of.
    \return false when not paused, true when paused.*/
bool isStreamPaused(const StreamPtr& p_stream);


// Parameter functions

/*! \brief Gets the stream's volume ratio.
    \param p_stream The stream to get the volume ratio of.
    \return The volume ratio.*/
real getStreamVolumeRatio(const StreamPtr& p_stream);

/*! \brief Sets the stream's volume ratio.
    \param p_stream The stream to set the volume ratio of.
    \param p_volumeRatio The volume ratio of the sound.
    \return Whether the operation succeeded.*/
bool setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio);

/*! \brief Gets the stream's volume in dB.
    \param p_stream The stream to get the volume of.
    \return The volume in dB.*/
real getStreamVolume(const StreamPtr& p_stream);

/*! \brief Sets the stream's volume in dB.
    \param p_stream The stream to set the volume of.
    \param p_volume The volume of the sound in dB.
    \return Whether the operation succeeded.*/
bool setStreamVolume(const StreamPtr& p_stream, real p_volume);


// Positional Audio

/*! \brief Enables the use of positional audio (overrides panning settings)
    \param p_enabled Whether 3D audio should be enabled or disabled
	\return Whether the operation succeeded.*/
bool set3DAudioEnabled(bool p_enabled, identifier p_identifier = 0);

bool is3DAudioEnabled(identifier p_identifier = 0);

/*! \brief Sets the position of the global listener (for positional audio)
    \param p_position The new position for the global listener.
    \param p_identifier Soundsystem to set the listener of.
	\return Whether the operation succeeded.*/
bool setListenerPosition(const math::Vector3& p_position, identifier p_identifier = 0);

/*! \brief Gets the position of the global listener (for positional audio) */
math::Vector3 getListenerPosition(identifier p_identifier = 0);

/*! \brief Set the distance model for positional audio.
    \param p_curve The curve to use for distance fading
    \param p_identifier Soundsystem to set the listener of.
    \return Whether the operation succeeded.*/
bool setPositionalAudioModel(const VoicePtr& p_voice, const audio::xact::RPCCurve* p_curve);

// Namespace end
}
}


#endif  // !defined(INC_TT_SND_SND_H)
