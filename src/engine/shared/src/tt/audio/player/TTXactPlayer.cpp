#include <tt/audio/helpers.h>
#include <tt/audio/player/TTXactCue.h>
#include <tt/audio/player/TTXactPlayer.h>
#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Category.h>
#include <tt/audio/xact/CueInstance.h>
#include <tt/audio/xact/SoundBank.h>
#include <tt/platform/tt_error.h>
#include <tt/snd/snd.h>


namespace tt {
namespace audio {
namespace player {

//--------------------------------------------------------------------------------------------------
// Public member functions

TTXactPlayer::TTXactPlayer()
:
SoundPlayer(),
m_initialized(false)
{
}


TTXactPlayer::~TTXactPlayer()
{
	uninit();
}


bool TTXactPlayer::init(const std::string& p_projectFile, bool p_autoLoad)
{
	if (m_initialized)
	{
		TT_WARN("TTXactPlayer already initialized!");
		return false;
	}
	
	if (xact::AudioTT::load(p_projectFile, p_autoLoad) == false)
	{
		TT_PANIC("Loading XACT project via AudioTT failed.");
		return false;
	}
	
	m_initialized = true;
	
	return true;
}


bool TTXactPlayer::uninit()
{
	if (m_initialized == false)
	{
		TT_WARN("TTXactPlayer not initialized.");
		return false;
	}
	
	if (xact::AudioTT::unload())
	{
		m_initialized = false;
	}
	else
	{
		TT_PANIC("Unloading AudioTT failed.");
		return false;
	}
	
	return true;
}


void TTXactPlayer::update(real p_deltaTime)
{
	xact::AudioTT::update(p_deltaTime);
}


SoundCuePtr TTXactPlayer::createCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional)
{
	if (m_initialized == false)
	{
		TT_PANIC("TTXactPlayer has not been initialized.");
		return SoundCuePtr();
	}
	
	xact::SoundBank* bank = xact::AudioTT::getSoundBank(p_soundbank);
	if (bank == 0)
	{
		return SoundCuePtr();
	}
	
	s32 cueIndex = bank->getCueIndex(p_name);
	if (cueIndex == -1)
	{
		TT_PANIC("Cue '%s' not found in soundbank '%s'.", p_name.c_str(), p_soundbank.c_str());
		return SoundCuePtr();
	}
	
	return SoundCuePtr(new TTXactCue(bank, cueIndex, p_positional, p_name));
}


bool TTXactPlayer::play(const std::string& p_soundbank, const std::string& p_name)
{
	if (m_initialized == false)
	{
		TT_PANIC("TTXactPlayer has not been initialized.");
		return false;
	}
	
	xact::SoundBank* bank = xact::AudioTT::getSoundBank(p_soundbank);
	if (bank == 0)
	{
		return false;
	}
	
	s32 cueIndex = bank->getCueIndex(p_name);
	if (cueIndex == -1)
	{
		TT_PANIC("Cue '%s' not found in soundbank '%s'.", p_name.c_str(), p_soundbank.c_str());
		return false;
	}
	
	return bank->play(cueIndex);
}


SoundCuePtr TTXactPlayer::playCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional)
{
	SoundCuePtr cue = createCue(p_soundbank, p_name, p_positional);
	if (cue == 0 || cue->play() == false)
	{
		return SoundCuePtr();
	}
	
	return cue;
}


bool TTXactPlayer::stop(const std::string& p_soundbank, const std::string& p_name)
{
	if (m_initialized == false)
	{
		TT_PANIC("TTXactPlayer has not been initialized.");
		return false;
	}
	
	xact::SoundBank* bank = xact::AudioTT::getSoundBank(p_soundbank);
	if (bank == 0)
	{
		return false;
	}
	
	s32 cueIndex = bank->getCueIndex(p_name);
	if (cueIndex == -1)
	{
		TT_PANIC("Cue '%s' not found in soundbank '%s'.", p_name.c_str(), p_soundbank.c_str());
		return false;
	}
	
	return bank->stop(cueIndex);;
}


bool TTXactPlayer::hasCategory(const std::string& p_category)
{
	TT_ASSERTMSG(m_initialized, "TTXactPlayer has not been initialized.");
	return xact::AudioTT::getCategory(p_category) != 0;
}


bool TTXactPlayer::stopCategory(const std::string& p_category)
{
	TT_ASSERTMSG(m_initialized, "TTXactPlayer has not been initialized.");
	return xact::AudioTT::stopCategory(p_category);
}


bool TTXactPlayer::pauseCategory(const std::string& p_category)
{
	TT_ASSERTMSG(m_initialized, "TTXactPlayer has not been initialized.");
	return xact::AudioTT::pauseCategory(p_category);
}


bool TTXactPlayer::resumeCategory(const std::string& p_category)
{
	TT_ASSERTMSG(m_initialized, "TTXactPlayer has not been initialized.");
	return xact::AudioTT::resumeCategory(p_category);
}


bool TTXactPlayer::setCategoryVolume(const std::string& p_category, real p_normalizedVolume)
{
	if (m_initialized == false)
	{
		TT_PANIC("TTXactPlayer has not been initialized.");
		return false;
	}
	
	xact::Category* cat = xact::AudioTT::getCategory(p_category);
	if (cat == 0)
	{
		TT_PANIC("No category named '%s'.", p_category.c_str());
		return false;
	}
	
	cat->setVolume(helpers::ratioTodB(p_normalizedVolume));
	
	xact::AudioTT::updateVolume();
	
	return true;
}


bool TTXactPlayer::set3DAudioEnabled(bool p_enabled)
{
	return snd::set3DAudioEnabled(p_enabled, xact::AudioTT::getSoundSystem());
}


bool TTXactPlayer::setListenerPosition(const math::Vector3& p_position)
{
	m_listenerPosition = p_position;

	return snd::setListenerPosition(p_position, xact::AudioTT::getSoundSystem());
}


bool TTXactPlayer::setReverbVolumeForCategory(const std::string& p_category, real p_normalizedVolume)
{
	if (m_initialized == false)
	{
		TT_PANIC("TTXactPlayer has not been initialized.");
		return false;
	}
	
	xact::AudioTT::setReverbVolumeForCategory(p_category, helpers::ratioTodB(p_normalizedVolume));
	return true;
}

// Namespace end
}
}
}
