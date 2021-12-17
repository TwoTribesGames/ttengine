#include <tt/audio/helpers.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/snd/snd.h>
#include <tt/snd/SoundSystem.h>


namespace tt {
namespace snd {

SoundSystem::SoundSystem(identifier p_identifier)
:
m_identifier(p_identifier),
m_registered(false),
m_3DAudioEnabled(false)
{
	
}


SoundSystem::~SoundSystem()
{
	if (m_registered)
	{
		snd::unregisterSoundSystem(m_identifier);
	}
}


// Capability functions

bool SoundSystem::supportsStereo()
{
	return true;
}


bool SoundSystem::supportsSuspending()
{
	return false;
}


bool SoundSystem::suspend()
{
	return false;
}


bool SoundSystem::resume()
{
	return false;
}


// Master volume functions

real SoundSystem::getMasterVolume()
{
	return real(1.0f);
}


void SoundSystem::setMasterVolume(real p_volume)
{
	(void)p_volume;
}


real SoundSystem::getSecondaryMasterVolume()
{
	return 1.0f;
}


void SoundSystem::setSecondaryMasterVolume(real /*p_volume*/)
{
}


// Voice Playback functions

bool SoundSystem::pauseVoice(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return false;
}


bool SoundSystem::resumeVoice(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return false;
}


// Status functions

bool SoundSystem::isVoicePaused(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return false;
}


// Parameter functions

size_type SoundSystem::getVoiceLoopStart(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return size_type();
}


bool SoundSystem::setVoiceLoopStart(const VoicePtr& p_voice, size_type p_loopStart)
{
	(void)p_voice;
	(void)p_loopStart;
	
	return false;
}


real SoundSystem::getVoicePlaybackRatio(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return real();
}


bool SoundSystem::setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio)
{
	(void)p_voice;
	(void)p_playbackRatio;
	
	return false;
}


real SoundSystem::getVoiceVolumeRatio(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return real();
}


bool SoundSystem::setVoiceVolumeRatio(const VoicePtr& p_voice, real p_volumeRatio)
{
	(void)p_voice;
	(void)p_volumeRatio;
	
	return false;
}


real SoundSystem::getVoiceVolume(const VoicePtr& p_voice)
{
	return audio::helpers::ratioTodB(getVoiceVolumeRatio(p_voice));
}


bool SoundSystem::setVoiceVolume(const VoicePtr& p_voice, real p_volume)
{
	return setVoiceVolumeRatio(p_voice, audio::helpers::ratioTodB(p_volume));
}


real SoundSystem::getVoicePanning(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return 0.0f;
}


bool SoundSystem::setVoicePanning(const VoicePtr& p_voice, real p_panning)
{
	(void)p_voice;
	(void)p_panning;
	
	return false;
}


real SoundSystem::getVoice360Panning(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return 0.0f;
}


bool SoundSystem::setVoice360Panning(const VoicePtr& p_voice, real p_panning)
{
	(void)p_voice;
	(void)p_panning;
	
	return false;
}


size_type SoundSystem::getVoicePriority(const VoicePtr& p_voice)
{
	(void)p_voice;
	
	return size_type();
}


bool SoundSystem::setVoicePriority(const VoicePtr& p_voice, size_type p_priority)
{
	(void)p_voice;
	(void)p_priority;
	
	return false;
}


bool SoundSystem::setVoicePosition(const VoicePtr&, const math::Vector3&)
{
	return false;
}


bool SoundSystem::setVoiceRadius(const VoicePtr&, real, real)
{
	return false;
}


bool SoundSystem::setVoiceReverbVolume(const VoicePtr&, real)
{
	return false;
}


bool SoundSystem::setReverbEffect(ReverbPreset)
{
	return false;
}


bool SoundSystem::setVoiceLowPassFilterEnabled(const VoicePtr&, bool)
{
	return false;
}


bool SoundSystem::setVoiceLowPassFilterFrequency(const VoicePtr&, size_type)
{
	return false;
}


bool SoundSystem::openStream(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::closeStream(Stream* p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::playStream(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::stopStream(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::pauseStream(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::resumeStream(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::updateStream(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::isStreamPlaying(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


bool SoundSystem::isStreamPaused(const StreamPtr& p_stream)
{
	(void)p_stream;
	return false;
}


real SoundSystem::getStreamVolumeRatio(const StreamPtr& p_stream)
{
	(void)p_stream;
	
	return real();
}


bool SoundSystem::setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio)
{
	(void)p_stream;
	(void)p_volumeRatio;
	
	return false;
}


real SoundSystem::getStreamVolume(const StreamPtr& p_stream)
{
	(void)p_stream;
	
	return real();
}


bool SoundSystem::setStreamVolume(const StreamPtr& p_stream, real p_volume)
{
	(void)p_stream;
	(void)p_volume;
	
	return false;
}


bool SoundSystem::setListenerPosition(const math::Vector3&)
{
	return false;
}


const math::Vector3& SoundSystem::getListenerPosition() const
{
	TT_PANIC("getListenerPosition not implemented for this sound system, implement it.");
	static math::Vector3 pos(math::Vector3::zero);
	return pos;
}


bool SoundSystem::setPositionalAudioModel(const VoicePtr&, const audio::xact::RPCCurve*)
{
	return false;
}


// namespace end
}
}
