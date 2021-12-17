//#include <tt/fs/utils/utils.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/audio/SoundCue.h>
#include <toki/game/script/wrappers/AudioWrapper.h>
#include <toki/game/script/sqbind_bindings.h>

//#include <toki/game/Game.h>
//#include <toki/input/Recorder.h>
//#include <toki/level/LevelData.h>
//#include <toki/steam/Workshop.h>
//#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


wrappers::SoundCueWrapper AudioWrapper::playGlobalSoundEffect(const std::string& p_soundbank, const std::string& p_effectName)
{
	audio::SoundCue::PlayInfo playInfo;
	playInfo.soundbank    = p_soundbank;
	playInfo.name         = p_effectName;
	playInfo.isPositional = false;
	
	audio::SoundCuePtr cue = audio::SoundCue::create(playInfo);
	if (cue != 0)
	{
		// Lifetime is managed by code; not script
		audio::AudioPlayer::getInstance()->keepCueAliveUntilDonePlaying(cue);
	}
	
	return wrappers::SoundCueWrapper(cue);
}


void AudioWrapper::stopCategory(const std::string& p_categoryName)
{
	audio::Category cat = getCategory(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		return;
	}
	
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->stop(cat);
	}
}


void AudioWrapper::pauseCategory(const std::string& p_categoryName)
{
	audio::Category cat = getCategory(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		return;
	}
	
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->pause(cat);
	}
}


void AudioWrapper::resumeCategory(const std::string& p_categoryName)
{
	audio::Category cat = getCategory(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		return;
	}
	
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->resume(cat);
	}
}


void AudioWrapper::setReverbEffect(const std::string& p_effectName)
{
	audio::ReverbEffect effect = audio::getReverbEffectFromName(p_effectName);
	if (audio::isValidReverbEffect(effect) == false)
	{
		TT_PANIC("Invalid reverb effect: '%s'", p_effectName.c_str());
		return;
	}
	
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->setReverbEffect(effect);
	}
}


void AudioWrapper::setReverbVolumeForCategory(const std::string& p_categoryName,
                                          real               p_volumePercentage)
{
	audio::Category cat = getCategory(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		return;
	}
	
	// Translate percentage to normalized volume
	p_volumePercentage /= 100.0f;
	tt::math::clamp(p_volumePercentage, 0.0f, 1.0f);
	
	audio::AudioPlayer::getInstance()->setReverbVolumeForCategory(cat, p_volumePercentage);
}


void AudioWrapper::setVolumeForCategory(const std::string& p_categoryName, real p_volumePercentage)
{
	audio::Category cat = getCategory(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		return;
	}
	
	// Translate percentage to normalized volume
	p_volumePercentage /= 100.0f;
	tt::math::clamp(p_volumePercentage, 0.0f, 1.0f);
	
	audio::AudioPlayer::getInstance()->setVolume(cat, p_volumePercentage);
}


real AudioWrapper::getVolumeForCategory(const std::string& p_categoryName)
{
	audio::Category cat = getCategory(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		return 0.0f;
	}
	
	// Internal value is normalized; to script we expose percentages
	return audio::AudioPlayer::getInstance()->getVolume(cat) * 100.0f;
}


void AudioWrapper::setTVOverallVolume(real p_volumePercentage, real p_duration)
{
	setOverallVolume(audio::Device_TV, p_volumePercentage, p_duration);
}


void AudioWrapper::setDRCOverallVolume(real p_volumePercentage, real p_duration)
{
	setOverallVolume(audio::Device_DRC, p_volumePercentage, p_duration);
}


void AudioWrapper::setTVAudioEnabled(bool p_enabled)
{
	setAudioEnabled(audio::Device_TV, p_enabled);
}


void AudioWrapper::setDRCAudioEnabled(bool p_enabled)
{
	setAudioEnabled(audio::Device_DRC, p_enabled);
}


void AudioWrapper::setUserSettingVolumeMusic(real p_volumePercentage)
{
	// Translate percentage to normalized volume
	p_volumePercentage /= 100.0f;
	tt::math::clamp(p_volumePercentage, 0.0f, 1.0f);
	
	audio::AudioPlayer::setMusicVolumeFromSettings(p_volumePercentage);
}


void AudioWrapper::setUserSettingVolumeSfx(const std::string& p_categoryName, real p_volumePercentage)
{
	// Translate percentage to normalized volume
	p_volumePercentage /= 100.0f;
	tt::math::clamp(p_volumePercentage, 0.0f, 1.0f);
	
	const audio::Category category = audio::getCategoryFromName(p_categoryName);
	if (category != audio::Category_Music)
	{
		audio::AudioPlayer::setSfxVolumeFromSettings(category, p_volumePercentage);
	}
	else
	{
		TT_PANIC("Cannot set Category_Music in setUserSettingVolumeSfx. Use setUserSettingVolumeMusic instead.");
	}
}


real AudioWrapper::getUserSettingVolumeMusic()
{
	// Internal value is normalized; to script we expose percentages
	return audio::AudioPlayer::getMusicVolumeFromSettings() * 100.0f;
}


real AudioWrapper::getUserSettingVolumeSfx(const std::string& p_categoryName)
{
	const audio::Category category = audio::getCategoryFromName(p_categoryName);
	if (category != audio::Category_Music)
	{
		// Internal value is normalized; to script we expose percentages
		return audio::AudioPlayer::getSfxVolumeFromSettings(category) * 100.0f;
	}
	return getUserSettingVolumeMusic();
}


void AudioWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(AudioWrapper, "Audio");
	TT_SQBIND_STATIC_METHOD(AudioWrapper, playGlobalSoundEffect);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, stopCategory);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, pauseCategory);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, resumeCategory);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setReverbEffect);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setReverbVolumeForCategory);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setVolumeForCategory);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, getVolumeForCategory);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setTVOverallVolume);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setDRCOverallVolume);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setTVAudioEnabled);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setDRCAudioEnabled);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setUserSettingVolumeMusic);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, setUserSettingVolumeSfx);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, getUserSettingVolumeMusic);
	TT_SQBIND_STATIC_METHOD(AudioWrapper, getUserSettingVolumeSfx);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

audio::Category AudioWrapper::getCategory(const std::string& p_categoryName)
{
	audio::Category cat = audio::getCategoryFromName(p_categoryName);
	if (audio::isValidCategory(cat) == false)
	{
		TT_PANIC("Invalid category: '%s'. Cannot set reverb volume for it.", p_categoryName.c_str());
		return audio::Category_Invalid;
	}
	
	if (audio::AudioPlayer::hasInstance() == false)
	{
		// No player: return invalid category
		return audio::Category_Invalid;
	}
	
	return cat;
}


void AudioWrapper::setOverallVolume(audio::Device p_device, real p_volumePercentage, real p_duration)
{
	if (audio::AudioPlayer::hasInstance())
	{
		// Translate percentage to normalized volume
		p_volumePercentage /= 100.0f;
		tt::math::clamp(p_volumePercentage, 0.0f, 1.0f);
		
		audio::AudioPlayer::getInstance()->setOverallVolume(p_device, p_volumePercentage, p_duration);
	}
}


void AudioWrapper::setAudioEnabled(audio::Device p_device, bool p_enabled)
{
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->setAudioEnabled(p_device, p_enabled);
	}
}

// Namespace end
}
}
}
}
