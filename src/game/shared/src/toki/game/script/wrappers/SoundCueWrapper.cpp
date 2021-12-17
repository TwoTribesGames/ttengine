#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/audio/SoundCue.h>
#include <toki/game/script/wrappers/SoundCueWrapper.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

void SoundCueWrapper::stop()
{
	if (m_cue != 0)
	{
		m_cue->stop();
		m_cue.reset();
	}
}


void SoundCueWrapper::pause()
{
	if (m_cue != 0)
	{
		m_cue->pause();
	}
}


void SoundCueWrapper::resume()
{
	if (m_cue != 0)
	{
		m_cue->resume();
	}
}


bool SoundCueWrapper::isPlaying()
{
	return (m_cue != 0) ? m_cue->isPlaying() : false;
}


bool SoundCueWrapper::isPaused()
{
	return (m_cue != 0) ? m_cue->isPaused() : false;
}


bool SoundCueWrapper::setVariable(const std::string& p_name, real p_value)
{
	if (m_cue == 0)
	{
		return false;
	}
	
	m_cue->setVariable(p_name, p_value);
	return true;
}


bool SoundCueWrapper::setFadingVariable(const std::string& p_name, real p_value, real p_duration)
{
	if (m_cue == 0)
	{
		return false;
	}
	
	m_cue->setFadingVariable(p_name, p_value, p_duration, false);
	return true;
}


bool SoundCueWrapper::fadeAndStop(const std::string& p_name, real p_value, real p_duration)
{
	if (m_cue == 0)
	{
		return false;
	}
	
	// Fade variable and tell the cue to stop afterwards
	m_cue->setFadingVariable(p_name, p_value, p_duration, true);
	
	// To prevent hard-stopping this cue if lifetime of this script wrapper ends,
	// leave cue lifetime management to the AudioPlayer from this point on
	audio::AudioPlayer::getInstance()->keepCueAliveUntilDonePlaying(m_cue);
	m_cue.reset();
	return true;
}


bool SoundCueWrapper::setRadius(real p_inner, real p_outer)
{
	return (m_cue != 0) ? m_cue->setRadius(p_inner, p_outer) : false;
}


void SoundCueWrapper::setReverbVolume(real p_volumePercentage)
{
	// Translate percentage to normalized volume
	p_volumePercentage /= 100.0f;
	tt::math::clamp(p_volumePercentage, 0.0f, 1.0f);
	
	m_cue->setReverbVolume(p_volumePercentage);
}


void SoundCueWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bool wrapperValid = (m_cue != 0);
	
	bu::put(wrapperValid, p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to save
		return;
	}
	
	m_cue->serialize(p_context);
}


void SoundCueWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	// Start with defaults
	*this = SoundCueWrapper();
	
	const bool wrapperValid = bu::get<bool>(p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to load
		return;
	}
	
	// Create empty cue
	m_cue = audio::SoundCue::create();
	
	m_cue->unserialize(p_context);
}


void SoundCueWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(SoundCueWrapper, "SoundCue");
	TT_SQBIND_METHOD(SoundCueWrapper, stop);
	TT_SQBIND_METHOD(SoundCueWrapper, pause);
	TT_SQBIND_METHOD(SoundCueWrapper, resume);
	TT_SQBIND_METHOD(SoundCueWrapper, isPlaying);
	TT_SQBIND_METHOD(SoundCueWrapper, isPaused);
	TT_SQBIND_METHOD(SoundCueWrapper, setVariable);
	TT_SQBIND_METHOD(SoundCueWrapper, setFadingVariable);
	TT_SQBIND_METHOD(SoundCueWrapper, fadeAndStop);
	TT_SQBIND_METHOD(SoundCueWrapper, setRadius);
	TT_SQBIND_METHOD(SoundCueWrapper, setReverbVolume);
}

// Namespace end
}
}
}
}
