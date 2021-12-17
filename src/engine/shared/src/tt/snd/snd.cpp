#include <map>

#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/snd/Buffer.h>
#include <tt/snd/snd.h>
#include <tt/snd/SoundSystem.h>
#include <tt/snd/Stream.h>
#include <tt/snd/Voice.h>


namespace tt {
namespace snd {

typedef std::map<identifier, SoundSystem*> SoundSystems;

static SoundSystems ms_soundSystems;


/*! \brief Internal helper function to retrieve a sound system pointer based on an identifier.
    \return Pointer to the sound system with specified identifier. Null if no such identifier exists.
    \note This function is for tt::snd internal usage only! */
inline SoundSystem* getSoundSystem(identifier p_identifier)
{
	SoundSystems::iterator it = ms_soundSystems.find(p_identifier);
	if (it == ms_soundSystems.end())
	{
		TT_PANIC("No soundsystem with identifier %d registered.", p_identifier);
		return 0;
	}
	return (*it).second;
}


bool registerSoundSystem(SoundSystem* p_soundSystem, identifier p_identifier)
{
	if (p_soundSystem == 0)
	{
		TT_PANIC("No soundsystem specified.");
		return false;
	}
	
	if (ms_soundSystems.find(p_identifier) != ms_soundSystems.end())
	{
		TT_PANIC("Soundsystem with identifier %d already registered.", p_identifier);
		return false;
	}
	
	ms_soundSystems.insert(SoundSystems::value_type(p_identifier, p_soundSystem));
	p_soundSystem->m_registered = true;
	return true;
}


bool unregisterSoundSystem(identifier p_identifier)
{
	SoundSystems::iterator it = ms_soundSystems.find(p_identifier);
	
	if (it == ms_soundSystems.end())
	{
		TT_PANIC("No soundsystem with identifier %d registered.", p_identifier);
		return false;
	}
	
	ms_soundSystems.erase(it);
	return true;
}


bool hasSoundSystem(identifier p_identifier)
{
	return ms_soundSystems.find(p_identifier) != ms_soundSystems.end();
}


bool supportsStereo(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return sys != 0 ? sys->supportsStereo() : false;
}


bool supportsSuspending(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return sys != 0 ? sys->supportsSuspending() : false;
}


real getMasterVolume(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return sys != 0 ? sys->getMasterVolume() : real();
}


void setMasterVolume(identifier p_identifier, real p_volume)
{
	if (p_volume < 0.0f || p_volume > 1.0f)
	{
		TT_PANIC("Master volume %f out of range [0.0 - 1.0].", realToFloat(p_volume));
		return;
	}
	
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys != 0)
	{
		sys->setMasterVolume(p_volume);
	}
}


real getSecondaryMasterVolume(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return sys != 0 ? sys->getSecondaryMasterVolume() : real();
}


void setSecondaryMasterVolume(identifier p_identifier, real p_volume)
{
	if (p_volume < 0.0f || p_volume > 1.0f)
	{
		TT_PANIC("Master volume %f out of range [0.0 - 1.0].", realToFloat(p_volume));
		return;
	}
	
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys != 0)
	{
		sys->setSecondaryMasterVolume(p_volume);
	}
}


bool suspend(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return sys != 0 ? sys->suspend() : false;
}


bool resume(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return sys != 0 ? sys->resume() : false;
}


// Voice creation functions

VoicePtr openVoice(size_type p_priority, identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return VoicePtr();
	}
	
	VoicePtr ret(new Voice(p_identifier), Voice::deleteVoice);
	ret->m_this = ret;
	
	if (sys->openVoice(ret, p_priority) == false)
	{
		return VoicePtr();
	}
	
	return ret;
}


bool closeVoice(Voice* p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->closeVoice(p_voice) : false;
}


// Voice Playback functions

bool playVoice(const VoicePtr& p_voice, bool p_loop)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->playVoice(p_voice, p_loop) : false;
}


bool stopVoice(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->stopVoice(p_voice) : false;
}


bool pauseVoice(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->pauseVoice(p_voice) : false;
}


bool resumeVoice(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->resumeVoice(p_voice) : false;
}


// Voice Status functions

bool isVoicePlaying(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->isVoicePlaying(p_voice) : false;
}


bool isVoicePaused(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->isVoicePaused(p_voice) : false;
}


// Voice Parameter functions

BufferPtr getVoiceBuffer(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return BufferPtr();
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoiceBuffer(p_voice) : BufferPtr();
}


bool setVoiceBuffer(const VoicePtr& p_voice, const BufferPtr& p_buffer)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoiceBuffer(p_voice, p_buffer) : false;
}


size_type getVoiceLoopStart(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return size_type();
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoiceLoopStart(p_voice) : size_type();
}


bool setVoiceLoopStart(const VoicePtr& p_voice, size_type p_loopStart)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoiceLoopStart(p_voice, p_loopStart) : false;
}


real getVoicePlaybackRatio(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoicePlaybackRatio(p_voice) : real();
}


bool setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoicePlaybackRatio(p_voice, p_playbackRatio) : false;
}


real getVoiceVolumeRatio(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoiceVolumeRatio(p_voice) : real();
}


bool setVoiceVolumeRatio(const VoicePtr& p_voice, real p_volumeRatio)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoiceVolumeRatio(p_voice, p_volumeRatio) : false;
}


real getVoiceVolume(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoiceVolume(p_voice) : real();
}


bool setVoiceVolume(const VoicePtr& p_voice, real p_volume)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoiceVolume(p_voice, p_volume) : false;
}


real getVoicePanning(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoicePanning(p_voice) : real();
}


bool setVoicePanning(const VoicePtr& p_voice, real p_panning)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoicePanning(p_voice, p_panning) : false;
}


real getVoice360Panning(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoice360Panning(p_voice) : 0.0f;
}


bool setVoice360Panning(const VoicePtr& p_voice, real p_panning)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoice360Panning(p_voice, p_panning) : false;
}


size_type getVoicePriority(const VoicePtr& p_voice)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return size_type();
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->getVoicePriority(p_voice) : size_type();
}


bool setVoicePriority(const VoicePtr& p_voice, size_type p_priority)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return sys != 0 ? sys->setVoicePriority(p_voice, p_priority) : false;
}


bool setVoicePosition(const VoicePtr& p_voice, const math::Vector3& p_position)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return (sys != 0) ? sys->setVoicePosition(p_voice, p_position) : false;
}


bool setVoiceRadius(const VoicePtr& p_voice, real p_inner, real p_outer)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return (sys != 0) ? sys->setVoiceRadius(p_voice, p_inner, p_outer) : false;
}


bool setVoiceReverbVolume(const VoicePtr& p_voice, real p_volume)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return (sys != 0) ? sys->setVoiceReverbVolume(p_voice, p_volume) : false;
}


bool setReverbEffect(ReverbPreset p_preset, identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	return (sys != 0) ? sys->setReverbEffect(p_preset) : false;
}


bool setVoiceLowPassFilterEnabled(const VoicePtr& p_voice, bool p_enabled)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return (sys != 0) ? sys->setVoiceLowPassFilterEnabled(p_voice, p_enabled) : false;
}


bool setVoiceLowPassFilterFrequency(const VoicePtr& p_voice, size_type p_frequency)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());
	return (sys != 0) ? sys->setVoiceLowPassFilterFrequency(p_voice, p_frequency) : false;
}


// Buffer creation functions

BufferPtr openBuffer(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return BufferPtr();
	}
	
	BufferPtr ret(new Buffer(p_identifier), Buffer::deleteBuffer);
	ret->m_this = ret;
	
	if (sys->openBuffer(ret) == false)
	{
		TT_PANIC("Failed to open buffer.");
		return BufferPtr();
	}
	
	return ret;
}


bool closeBuffer(Buffer* p_buffer)
{
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_buffer->getSoundSystem());
	return sys != 0 ? sys->closeBuffer(p_buffer) : false;
}


// Buffer Parameter functions

bool setBufferData(const BufferPtr& p_buffer,
                   const void*      p_data,
                   size_type        p_frames,
                   size_type        p_channels,
                   size_type        p_sampleSize,
                   size_type        p_sampleRate,
                   bool             p_ownership)
{
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return false;
	}
	
	if (p_data == 0)
	{
		TT_PANIC("No data specified.");
		return false;
	}
	
	if (p_frames <= 0)
	{
		TT_PANIC("Invalid amount of frames (%d) specified.", p_frames);
		return false;
	}
	
	if (p_channels <= 0)
	{
		TT_PANIC("Invalid amount of channels (%d) specified.", p_channels);
		return false;
	}
	
	if (p_sampleSize <= 0)
	{
		TT_PANIC("Invalid sample size (%d) specified.", p_sampleSize);
		return false;
	}
	
	if (p_sampleRate <= 0)
	{
		TT_PANIC("Invalid sample rate (%d) specified.", p_sampleRate);
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_buffer->getSoundSystem());
	return sys != 0 ?
		sys->setBufferData(p_buffer, p_data, p_frames, p_channels, p_sampleSize, p_sampleRate, p_ownership)
		: false;
}


size_type getBufferLength(const BufferPtr& p_buffer)
{
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	SoundSystem* sys = getSoundSystem(p_buffer->getSoundSystem());
	return sys != 0 ? sys->getBufferLength(p_buffer) : size_type();
}


size_type getBufferChannelCount(const BufferPtr& p_buffer)
{
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	SoundSystem* sys = getSoundSystem(p_buffer->getSoundSystem());
	return sys != 0 ? sys->getBufferChannelCount(p_buffer) : size_type();
}


size_type getBufferSampleSize(const BufferPtr& p_buffer)
{
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	SoundSystem* sys = getSoundSystem(p_buffer->getSoundSystem());
	return sys != 0 ? sys->getBufferSampleSize(p_buffer) : size_type();
}


size_type getBufferSampleRate(const BufferPtr& p_buffer)
{
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	SoundSystem* sys = getSoundSystem(p_buffer->getSoundSystem());
	return sys != 0 ? sys->getBufferSampleRate(p_buffer) : size_type();
}


// Stream functions

StreamPtr openStream(StreamSource* p_source, identifier p_identifier)
{
	if (p_source == 0)
	{
		TT_PANIC("Stream source should not be null.");
		return StreamPtr();
	}
	
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return StreamPtr();
	}
	
	StreamPtr ret(new Stream(p_identifier, p_source), Stream::deleteStream);
	ret->m_this = ret;
	
	if (sys->openStream(ret) == false)
	{
		TT_PANIC("Failed to open stream.");
		return StreamPtr();
	}
	
	return ret;
}


bool closeStream(Stream* p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->closeStream(p_stream) : false;
}


bool playStream(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->playStream(p_stream) : false;
}


bool stopStream(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->stopStream(p_stream) : false;
}


bool pauseStream(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->pauseStream(p_stream) : false;
}


bool resumeStream(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->resumeStream(p_stream) : false;
}


bool updateStream(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->updateStream(p_stream) : false;
}


bool isStreamPlaying(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->isStreamPlaying(p_stream) : false;
}


bool isStreamPaused(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->isStreamPaused(p_stream) : false;
}


real getStreamVolumeRatio(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->getStreamVolumeRatio(p_stream) : real();
}


bool setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->setStreamVolumeRatio(p_stream, p_volumeRatio) : false;
}


real getStreamVolume(const StreamPtr& p_stream)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return 0.0f;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->getStreamVolume(p_stream) : real();
}


bool setStreamVolume(const StreamPtr& p_stream, real p_volume)
{
	if (p_stream == 0)
	{
		TT_PANIC("No stream specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_stream->getSoundSystem());
	return sys != 0 ? sys->setStreamVolume(p_stream, p_volume) : false;
}


bool set3DAudioEnabled(bool p_enabled, identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return false;
	}

	return sys->set3DAudioEnabled(p_enabled);
}


bool is3DAudioEnabled(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return false;
	}

	return sys->is3DAudioEnabled();
}

bool setListenerPosition(const math::Vector3& p_position, identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return false;
	}

	return sys->setListenerPosition(p_position);
}


math::Vector3 getListenerPosition(identifier p_identifier)
{
	SoundSystem* sys = getSoundSystem(p_identifier);
	if (sys == 0)
	{
		return math::Vector3::zero;
	}

	return sys->getListenerPosition();
}


bool setPositionalAudioModel(const VoicePtr& p_voice, const audio::xact::RPCCurve* p_curve)
{
	if (p_voice == 0)
	{
		TT_PANIC("No voice specified.");
		return false;
	}
	
	SoundSystem* sys = getSoundSystem(p_voice->getSoundSystem());

	return sys->setPositionalAudioModel(p_voice, p_curve);
}


// Namespace end
}
}
