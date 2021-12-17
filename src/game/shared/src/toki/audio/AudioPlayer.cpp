#include <algorithm>

#include <json/json.h>

#include <tt/audio/helpers.h>
#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/fs/fs.h>
#include <tt/math/math.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/pres/PresentationObject.h>
#include <tt/snd/snd.h>
#include <tt/str/str.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/audio/PositionalEffect.h>
#include <toki/game/entity/Entity.h>
#include <toki/savedata/utils.h>
#include <toki/constants.h>


namespace toki {
namespace audio {

AudioPlayer* AudioPlayer::ms_instance                              = 0;
bool         AudioPlayer::ms_createOnMainThreadNow                 = false;
real         AudioPlayer::ms_musicVolumeFromSettings               = 1.0f;
real         AudioPlayer::ms_sfxVolumeFromSettings[Category_Count] = { 1.0f, 1.0f, 1.0f, 1.0f };


//--------------------------------------------------------------------------------------------------
// Public member functions

void AudioPlayer::destroyInstance()
{
	delete ms_instance;
	ms_instance = 0;
}


void AudioPlayer::playEffect(const std::string& p_soundbank, const std::string& p_name)
{
	if (m_sound != 0)
	{
		m_sound->play(p_soundbank, p_name);
	}
}


tt::audio::player::SoundCuePtr AudioPlayer::playEffectCue(const std::string& p_soundbank, const std::string& p_name)
{
	return (m_sound != 0) ? m_sound->playCue(p_soundbank, p_name, false) : tt::audio::player::SoundCuePtr();
}


tt::audio::player::SoundCuePtr AudioPlayer::playPositionalEffectCue(
	const std::string&                     p_soundbank,
	const std::string&                     p_name,
	const tt::pres::PresentationObjectPtr& p_presentationToFollow)
{
	PositionalEffectPtr effect(PositionalEffect::create(*this, p_soundbank, p_name, p_presentationToFollow));
	if (effect == 0)
	{
		return tt::audio::player::SoundCuePtr();
	}
	tt::thread::CriticalSection critSec(&m_containerMutex);
	m_positionalEffects.push_back(effect);
	return effect->getCue();
}


tt::audio::player::SoundCuePtr AudioPlayer::playPositionalEffectCue(
	const std::string&                p_soundbank,
	const std::string&                p_name,
	const game::entity::EntityHandle& p_entityToFollow)
{
	PositionalEffectPtr effect(PositionalEffect::create(*this, p_soundbank, p_name, p_entityToFollow));
	if (effect == 0)
	{
		return tt::audio::player::SoundCuePtr();
	}
	tt::thread::CriticalSection critSec(&m_containerMutex);
	m_positionalEffects.push_back(effect);
	return effect->getCue();
}


tt::audio::player::SoundCuePtr AudioPlayer::playPositionalEffectCue(
	const std::string&       p_soundbank,
	const std::string&       p_name,
	const tt::math::Vector3& p_staticPosition)
{
	PositionalEffectPtr effect(PositionalEffect::create(*this, p_soundbank, p_name, p_staticPosition));
	if (effect == 0)
	{
		return tt::audio::player::SoundCuePtr();
	}
	tt::thread::CriticalSection critSec(&m_containerMutex);
	m_positionalEffects.push_back(effect);
	return effect->getCue();
}


tt::audio::player::SoundCuePtr AudioPlayer::playPositionalEffectCue(
	const std::string&               p_soundbank,
	const std::string&               p_name,
	const AudioPositionInterfacePtr& p_audioPositionInterface)
{
	PositionalEffectPtr effect(PositionalEffect::create(*this, p_soundbank, p_name, p_audioPositionInterface));
	if (effect == 0)
	{
		return tt::audio::player::SoundCuePtr();
	}
	tt::thread::CriticalSection critSec(&m_containerMutex);
	m_positionalEffects.push_back(effect);
	return effect->getCue();
}


void AudioPlayer::registerSoundCueForUpdate(const SoundCueWeakPtr& p_soundCueWeakPtr)
{
	const SoundCuePtr cuePtr = p_soundCueWeakPtr.lock();
	if (cuePtr == 0)
	{
		return;
	}
	
	SoundCue* cueRawPtr = cuePtr.get();
	
	tt::thread::CriticalSection critSec(&m_containerMutex);
	// Check if this soundcue is already registered
	if (m_registeredSoundCues.find(cueRawPtr) != m_registeredSoundCues.end())
	{
		// Already registered
		return;
	}
	
	m_registeredSoundCues[cueRawPtr] = p_soundCueWeakPtr;
}


void AudioPlayer::keepCueAliveUntilDonePlaying(const SoundCuePtr& p_soundCue)
{
	TT_NULL_ASSERT(p_soundCue);
	if (p_soundCue != 0)
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		m_soundCuesToKeepAliveUntilDonePlaying.push_back(p_soundCue);
	}
}


void AudioPlayer::fadeOutAndKeepAliveUntilDone(const tt::audio::player::SoundCuePtr& p_soundCue,
                                               real p_duration)
{
	if (p_soundCue != 0)
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		m_fadeOutAndKeepAliveUntilDone.push_back(AudioCueWithFade(p_soundCue, p_duration));
	}
}


void AudioPlayer::pauseAllAudio()
{
	// Pause all sound effect categories
	for (s32 i = 0; i < Category_Count; ++i)
	{
		pause(static_cast<Category>(i));
	}
	
	// Pause all music tracks
	m_musicTrackMgr.pauseAllTracks();
}


void AudioPlayer::resumeAllAudio()
{
	// Resume all sound effect categories
	for (s32 i = 0; i < Category_Count; ++i)
	{
		resume(static_cast<Category>(i));
	}
	
	// Resume all music tracks
	m_musicTrackMgr.resumeAllTracks();
}


void AudioPlayer::stopAllAudio()
{
	tt::thread::CriticalSection critSec(&m_containerMutex);
	
	tt::code::helpers::freeContainer(m_registeredSoundCues);
	
	for (PositionalEffects::iterator it = m_positionalEffects.begin();
	     it != m_positionalEffects.end(); ++it)
	{
		(*it)->stop();
	}
	m_positionalEffects.clear();
	
	for (s32 i = 0; i < Category_Count; ++i)
	{
		stop(static_cast<Category>(i));
	}
	
	m_musicTrackMgr.resetAll();
}


#if !defined(TT_BUILD_FINAL)
std::string AudioPlayer::getDebugPositionalSoundNames() const
{
	typedef std::map<std::string, s32> CueMap;
	CueMap cues;
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		for (PositionalEffects::const_iterator it = m_positionalEffects.begin();
		     it != m_positionalEffects.end(); ++it)
		{
			std::pair<CueMap::iterator, bool> const& r =
				cues.insert(std::make_pair((*it)->getName() + " [" + (*it)->getSoundBank() + "]", 1));
			if (r.second == false)
			{
				++r.first->second;
			}
		}
	}
	
	char buf[256];
	sprintf(buf, "Positional Sounds (%d):\n", static_cast<s32>(m_positionalEffects.size()));
	std::string result(buf);
	for (CueMap::const_iterator it = cues.begin(); it != cues.end(); ++it)
	{
		sprintf(buf, "%5d %s\n", it->second, it->first.c_str());
		result += buf;
	}
	return result;
}
#endif


void AudioPlayer::stopEffect(const std::string& p_soundbank, const std::string& p_name)
{
	if (m_sound != 0)
	{
		m_sound->stop(p_soundbank, p_name);
	}
}


void AudioPlayer::pause(Category p_category)
{
	TT_ASSERTMSG(isValidCategory(p_category),
	             "Unsupported category: %d. Cannot pause it.", p_category);
	
	if (m_sound != 0 && isValidCategory(p_category))
	{
		m_sound->pauseCategory(getCategoryName(p_category));
	}
}


void AudioPlayer::resume(Category p_category)
{
	TT_ASSERTMSG(isValidCategory(p_category),
	             "Unsupported category: %d. Cannot resume it.", p_category);
	
	if (m_sound != 0 && isValidCategory(p_category))
	{
		m_sound->resumeCategory(getCategoryName(p_category));
	}
}


void AudioPlayer::stop(Category p_category)
{
	TT_ASSERTMSG(isValidCategory(p_category),
	             "Unsupported category: %d. Cannot stop it.", p_category);
	
	if (m_sound != 0 && isValidCategory(p_category))
	{
		m_sound->stopCategory(getCategoryName(p_category));
	}
}


void AudioPlayer::setVolume(Category p_category, real p_volume)
{
	if (isValidCategory(p_category) == false)
	{
		TT_PANIC("Unsupported category: %d. Cannot set volume.", p_category);
		return;
	}
	
	if (m_loadingLevel)
	{
		// Schedule volume change for after loading completes
		m_postLevelLoadSettings.categoryVolume[p_category] = p_volume;
		return;
	}
	
	m_categoryVolume[p_category] = p_volume;
	applyCategoryVolume(p_category);
	
	// If we already have an audio player, instantly apply the new setting
	if (ms_instance != 0 && p_category == Category_Music)
	{
		ms_instance->m_musicTrackMgr.handleMusicVolumeSettingChanged();
	}
}


real AudioPlayer::getVolume(Category p_category) const
{
	if (isValidCategory(p_category) == false)
	{
		TT_PANIC("Unsupported category: %d. Cannot get volume.", p_category);
		return 0.0f;
	}
	
	if (m_loadingLevel)
	{
		// Volume change scheduled; return that
		return m_postLevelLoadSettings.categoryVolume[p_category];
	}
	
	return m_categoryVolume[p_category];
}


void AudioPlayer::setListenerPosition(const tt::math::Vector3& p_position)
{
	m_listenerPosition = p_position;
	if (m_sound != 0)
	{
		m_sound->setListenerPosition(p_position);
	}
}


bool AudioPlayer::isInAudibleRange(const tt::math::Vector3& p_position,
                                   real p_radius) const
{
	return (m_listenerPosition - p_position).length() <= p_radius;
}


void AudioPlayer::setReverbEffect(ReverbEffect p_effect)
{
	if (isValidReverbEffect(p_effect) == false)
	{
		TT_PANIC("Invalid reverb effect: %d", p_effect);
		return;
	}
	
	// Only set anything if the effect changed
	if (p_effect == m_activeReverbEffect)
	{
		return;
	}
	
#if !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX) // reverb disabled on OS X for now (doesn't sound right)
	if (tt::snd::hasSoundSystem(0))
	{
		tt::snd::ReverbPreset preset = tt::snd::ReverbPreset_None;
		switch (p_effect)
		{
		case ReverbEffect_None:   preset = tt::snd::ReverbPreset_None;      break;
		case ReverbEffect_Forest: preset = tt::snd::ReverbPreset_SmallCave; break;
		case ReverbEffect_Cave:   preset = tt::snd::ReverbPreset_LargeCave; break;
			
		default:
			TT_PANIC("Unsupported reverb effect: %d", p_effect);
			return;
		}
		
		tt::snd::setReverbEffect(preset);
	}
#endif
	
#if defined(TT_PLATFORM_WIN)
	// HACK: Enable/disable overall reverb by changing the global reverb level
	//       (affects all cues assigned to this effect)
	// Martijn: disabled hack: didn't work
	//if (m_sound != 0)
	//{
	//	m_sound->setGlobalVariable("GlobalReverbLevel", (p_effect != ReverbEffect_Cave) ? 0.0f : 100.0f);
	//}
#endif
	
	m_activeReverbEffect = p_effect;
}


void AudioPlayer::setReverbVolumeForCategory(Category p_category, real p_normalizedVolume)
{
	if (isValidCategory(p_category) == false)
	{
		TT_PANIC("Invalid category: %d", p_category);
		return;
	}
	
	m_categoryReverbVolume[p_category] = p_normalizedVolume;
	if (m_sound != 0)
	{
		m_sound->setReverbVolumeForCategory(getCategoryName(p_category), p_normalizedVolume);
	}
}


void AudioPlayer::resetReverbSettings()
{
	setReverbEffect(ReverbEffect_None);
	
	for (s32 i = 0; i < Category_Count; ++i)
	{
		setReverbVolumeForCategory(static_cast<Category>(i), 0.0f);
	}
}


void AudioPlayer::setOverallVolume(Device p_device, real p_normalizedVolume, real p_duration,
                                   real p_normalizedFadeStartVolume)
{
	if (isValidDevice(p_device) == false)
	{
		TT_PANIC("Invalid audio output device: %d", p_device);
		return;
	}
	
	real clampedVolume = p_normalizedVolume;
	if (tt::math::clamp(clampedVolume, 0.0f, 1.0f))
	{
		TT_NONFATAL_PANIC("Overall TV volume %f is out of range 0 - 1. It has been clamped to %f",
		                  p_normalizedVolume, clampedVolume);
	}
	
	real clampedStartVolume = p_normalizedFadeStartVolume;
	if (clampedStartVolume >= 0.0f && tt::math::clamp(clampedStartVolume, 0.0f, 1.0f))
	{
		TT_NONFATAL_PANIC("Overall TV fade start volume %f is out of range 0 - 1. It has been clamped to %f",
		                  p_normalizedFadeStartVolume, clampedStartVolume);
	}
	
	if (m_loadingLevel)
	{
		// Schedule volume change for after loading completes
		if (p_duration <= 0.0f)
		{
			m_postLevelLoadSettings.overallVolume[p_device] = RealTLI(clampedVolume);
		}
		else
		{
			if (p_normalizedFadeStartVolume >= 0.0f)
			{
				m_postLevelLoadSettings.overallVolume[p_device] = RealTLI(clampedStartVolume);
			}
			m_postLevelLoadSettings.overallVolume[p_device].startNewInterpolation(clampedVolume, p_duration);
		}
		return;
	}
	
	if (p_duration <= 0.0f)
	{
		// Set new volume instantly
		m_deviceSettings[p_device].overallVolume = RealTLI(clampedVolume);
		applyOverallVolume(p_device);
	}
	else
	{
		if (p_normalizedFadeStartVolume >= 0.0f)
		{
			m_deviceSettings[p_device].overallVolume = RealTLI(clampedStartVolume);
			applyOverallVolume(p_device);
		}
		m_deviceSettings[p_device].overallVolume.startNewInterpolation(clampedVolume, p_duration);
	}
}


void AudioPlayer::resetOverallVolume()
{
	// Instantly set the overall volumes to their defaults
	for (s32 i = 0; i < Device_Count; ++i)
	{
		setOverallVolume(static_cast<Device>(i), 1.0f, 0.0f);
	}
}


void AudioPlayer::setAudioEnabled(Device p_device, bool p_enabled)
{
	if (isValidDevice(p_device) == false)
	{
		TT_PANIC("Invalid audio output device: %d", p_device);
		return;
	}
	
	if (m_loadingLevel)
	{
		// Schedule state change for after loading completes
		m_postLevelLoadSettings.audioEnabled[p_device] = p_enabled;
		return;
	}
	
	static const real enableFadeDuration = 0.25f;
	m_deviceSettings[p_device].audioEnableFade.startNewInterpolation(p_enabled ? 1.0f : 0.0f, enableFadeDuration);
}


void AudioPlayer::loadVolumeSettings()
{
	real settingVolumeMusic     = 1.0f;
	real settingVolumeEffects   = 1.0f;
	real settingVolumeAmbient   = 1.0f;
	real settingVolumeVoiceOver = 1.0f;
	
	tt::fs::FilePtr file = savedata::openSaveFile("audiosettings.json");
	if (file != 0 && file->getLength() > 0)
	{
		tt::code::BufferPtr fileContent = file->getContent();
		if (fileContent != 0)
		{
			const char* jsonDataBegin = reinterpret_cast<const char*>(fileContent->getData());
			const char* jsonDataEnd   = jsonDataBegin + fileContent->getSize();
			
			Json::Value  rootNode;
			Json::Reader reader;
			if (reader.parse(jsonDataBegin, jsonDataEnd, rootNode, false))
			{
				if (rootNode.isMember("musicVolume") &&
				    rootNode["musicVolume"].isConvertibleTo(Json::realValue))
				{
					settingVolumeMusic = static_cast<real>(rootNode["musicVolume"].asDouble());
				}
				
				if (tt::math::clamp(settingVolumeMusic, 0.0f, 1.0f))
				{
					TT_PANIC("Music volume from audio settings file was invalid. Volume was clamped to %f", settingVolumeMusic);
				}
				
				if (rootNode.isMember("effectsVolume") &&
				    rootNode["effectsVolume"].isConvertibleTo(Json::realValue))
				{
					settingVolumeEffects = static_cast<real>(rootNode["effectsVolume"].asDouble());
				}
				
				if (tt::math::clamp(settingVolumeEffects, 0.0f, 1.0f))
				{
					TT_PANIC("Effect volume from audio settings file was invalid. Volume was clamped to %f", settingVolumeEffects);
				}
				
				if (rootNode.isMember("ambientVolume") &&
				    rootNode["ambientVolume"].isConvertibleTo(Json::realValue))
				{
					settingVolumeAmbient = static_cast<real>(rootNode["ambientVolume"].asDouble());
				}
				
				if (tt::math::clamp(settingVolumeAmbient, 0.0f, 1.0f))
				{
					TT_PANIC("Ambient volume from audio settings file was invalid. Volume was clamped to %f", settingVolumeAmbient);
				}
				
				if (rootNode.isMember("voiceOverVolume") &&
				    rootNode["voiceOverVolume"].isConvertibleTo(Json::realValue))
				{
					settingVolumeVoiceOver = static_cast<real>(rootNode["voiceOverVolume"].asDouble());
				}
				
				if (tt::math::clamp(settingVolumeVoiceOver, 0.0f, 1.0f))
				{
					TT_PANIC("VoiceOver volume from audio settings file was invalid. Volume was clamped to %f", settingVolumeVoiceOver);
				}
			}
			else
			{
				TT_PANIC("Audio settings file could not be parsed as JSON.");
			}
		}
	}
	
	setMusicVolumeFromSettings(settingVolumeMusic);
	setSfxVolumeFromSettings  (Category_Effects,   settingVolumeEffects);
	setSfxVolumeFromSettings  (Category_Ambient,   settingVolumeAmbient);
	setSfxVolumeFromSettings  (Category_VoiceOver, settingVolumeVoiceOver);
}


void AudioPlayer::saveVolumeSettings()
{
	tt::fs::FilePtr file = savedata::createSaveFile("audiosettings.json");
	
	if (file == 0)
	{
		// Either not allowed to save or creating the save file failed
		// (underlying code will have triggered an appropriate panic message if so)
		return;
	}
	
	Json::Value rootNode(Json::objectValue);
	
	rootNode["musicVolume"]     = ms_musicVolumeFromSettings;
	rootNode["effectsVolume"]   = ms_sfxVolumeFromSettings[Category_Effects];
	rootNode["ambientVolume"]   = ms_sfxVolumeFromSettings[Category_Ambient];
	rootNode["voiceOverVolume"] = ms_sfxVolumeFromSettings[Category_VoiceOver];
	
	// Write the settings data as nicely formatted JSON
	const std::string       jsonText = Json::StyledWriter().write(rootNode);
	const tt::fs::size_type bytesToWrite = static_cast<tt::fs::size_type>(jsonText.length());
	
	file->write(jsonText.c_str(), bytesToWrite);
}


void AudioPlayer::setMusicVolumeFromSettings(real p_normalizedVolume)
{
	if (tt::math::clamp(p_normalizedVolume, 0.0f, 1.0f))
	{
		TT_WARN("Invalid normalized music volume passed. Clamped to %f", p_normalizedVolume);
	}
	
	//TT_Printf("SETTING MUSIC VOLUME TO %.2f (%d%%)\n", p_normalizedVolume, static_cast<s32>(p_normalizedVolume * 100));
	ms_musicVolumeFromSettings = p_normalizedVolume;
	
	// If we already have an audio player, instantly apply the new setting
	if (ms_instance != 0)
	{
		ms_instance->m_musicTrackMgr.handleMusicVolumeSettingChanged();
	}
}


void AudioPlayer::setSfxVolumeFromSettings(Category p_category, real p_normalizedVolume)
{
	if (tt::math::clamp(p_normalizedVolume, 0.0f, 1.0f))
	{
		TT_WARN("Invalid normalized sfx volume passed. Clamped to %f", p_normalizedVolume);
	}
	
	//TT_Printf("SETTING SFX VOLUME TO %.2f (%d%%)\n", p_normalizedVolume, static_cast<s32>(p_normalizedVolume * 100));
	ms_sfxVolumeFromSettings[p_category] = p_normalizedVolume;
	
	// If we already have an audio player, instantly apply the new setting
	if (ms_instance != 0)
	{
		ms_instance->applyCategoryVolume(p_category);
	}
}


real AudioPlayer::getSfxVolumeFromSettings(Category p_category)
{
	TT_ASSERT(p_category >= 0 && p_category < Category_Count);
	return ms_sfxVolumeFromSettings[p_category];
}


void AudioPlayer::update(real p_deltaTime)
{
	// Update overall volume per output device
	if (m_loadingLevel == false)
	{
		for (s32 deviceIndex = 0; deviceIndex < Device_Count; ++deviceIndex)
		{
			DeviceSettings& settings(m_deviceSettings[deviceIndex]);
			if (settings.overallVolume.isDone()   == false ||
			    settings.audioEnableFade.isDone() == false)
			{
				settings.overallVolume  .update(p_deltaTime);
				settings.audioEnableFade.update(p_deltaTime);
				applyOverallVolume(static_cast<Device>(deviceIndex));
			}
		}
	}
	
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		// Update registered sound cues
		for (RegisteredSoundCues::iterator it = m_registeredSoundCues.begin(); it != m_registeredSoundCues.end();)
		{
			SoundCuePtr ptr((*it).second.lock());
			if (ptr != 0 && ptr->hasVariables())
			{
				ptr->update(p_deltaTime);
				++it;
			}
			else
			{
				it = m_registeredSoundCues.erase(it);
			}
		}
	}
	
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		// Check if any of the "keep alive until done playing" cues are done playing
		// (if so, no longer store their smart pointer, thereby no longer keeping it alive)
		for (SoundCues::iterator it = m_soundCuesToKeepAliveUntilDonePlaying.begin();
		     it != m_soundCuesToKeepAliveUntilDonePlaying.end(); )
		{
			if ((*it)->isPlaying() == false)
			{
				it = tt::code::helpers::unorderedErase(m_soundCuesToKeepAliveUntilDonePlaying, it);
			}
			else
			{
				++it;
			}
		}
	}
	
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		for (TTSoundCues::iterator it = m_fadeOutAndKeepAliveUntilDone.begin();
		     it != m_fadeOutAndKeepAliveUntilDone.end();)
		{
			AudioCueWithFade& cueFade = (*it);
			TT_NULL_ASSERT(cueFade.cue);
			
			cueFade.time += p_deltaTime;
			if (cueFade.time >= cueFade.duration ||
			    cueFade.cue->isPlaying() == false)
			{
				it = m_fadeOutAndKeepAliveUntilDone.erase(it);
			}
			else
			{
				using namespace tt::math::interpolation;
				// Linear fade from 100 to 0.
				const real fadeValue = ((-100.0f * cueFade.time) / cueFade.duration) + 100.0f;
				cueFade.cue->setVariable("Volume", fadeValue);
				++it;
			}
		}
	}
	
	// Effects
	if (m_sound != 0)
	{
		{
			tt::thread::CriticalSection critSec(&m_containerMutex);
			
			// Update the positions of all positional effects
			for (PositionalEffects::iterator it = m_positionalEffects.begin();
			     it != m_positionalEffects.end(); )
			{
				PositionalEffectPtr posEffectPtr = (*it);
				if (posEffectPtr != 0 && posEffectPtr->update(*this))
				{
					++it;
				}
				else
				{
					// Positional effect is no longer valid: remove it
					it = m_positionalEffects.erase(it);
				}
			}
		}
		
		m_sound->update(p_deltaTime);
	}
	
	if (m_loadingLevel == false)
	{
		tt::thread::CriticalSection critSec(&m_containerMutex);
		m_musicTrackMgr.update(p_deltaTime);
	}
}


const tt::str::StringSet& AudioPlayer::getCueNames(const std::string& p_soundBankName) const
{
	CueNames::const_iterator bankIt = m_cueNames.find(p_soundBankName);
	if (bankIt == m_cueNames.end())
	{
		TT_PANIC("Sound bank '%s' does not exist in cue name list.", p_soundBankName.c_str());
		static tt::str::StringSet emptySet;
		return emptySet;
	}
	
	return (*bankIt).second;
}


tt::str::StringSet AudioPlayer::getAllCueNames() const
{
	tt::str::StringSet allNames;
	
	for (CueNames::const_iterator bankIt = m_cueNames.begin(); bankIt != m_cueNames.end(); ++bankIt)
	{
		allNames.insert((*bankIt).second.begin(), (*bankIt).second.end());
	}
	
	return allNames;
}


bool AudioPlayer::isCueLooping(const std::string& p_soundbank, const std::string& p_name) const
{
	// FIXME: Martijn: add support for different soundbanks
	(void)p_soundbank;
	CueMetaDataCollection::const_iterator it = m_cueMetaDataCollection.find(p_name);
	if (it != m_cueMetaDataCollection.end())
	{
		return (*it).second.isLooping;
	}
	
	return false;
}


void AudioPlayer::reloadCueMetaData()
{
	m_cueNames.clear();
	loadCueMetaData("audio/rewind.xcl");
}


void AudioPlayer::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	tt::thread::CriticalSection critSec(&m_containerMutex);
	
	TT_ASSERTMSG(m_loadingLevel == false,
	             "Serializing AudioPlayer while still loading a level: this will serialize incorrect values!");
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_listenerPosition, p_context);
	
	m_musicTrackMgr.serialize(p_context);
	
	bu::putEnum<u8>(m_activeReverbEffect, p_context);
	
	const s32 categoryCount = static_cast<s32>(Category_Count);
	bu::put(categoryCount, p_context);
	for (s32 i = 0; i < categoryCount; ++i)
	{
		bu::put(m_categoryVolume[i],       p_context);
		bu::put(m_categoryReverbVolume[i], p_context);
	}
}


void AudioPlayer::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	stopAllAudio();
	
	// Martijn addition: make sure to reset the overall volume as it can be set to 0 from Presentation
	resetOverallVolume();
	
	// NOTE: Locking mutex after the stopAllAudio call to prevent recursive locking, because it also locks the mutex
	tt::thread::CriticalSection critSec(&m_containerMutex);
	
	namespace bu = tt::code::bufferutils;
	
	const tt::math::Vector3 listenerPosition = bu::get<tt::math::Vector3>(p_context);
	setListenerPosition(listenerPosition);
	
	m_musicTrackMgr.unserialize(p_context);
	
	// Load reverb effect
	ReverbEffect reverbEffect = bu::getEnum<u8, ReverbEffect>(p_context);
	if (isValidReverbEffect(reverbEffect) == false)
	{
		TT_PANIC("Serialization data contains invalid reverb effect: %d. Disabling reverb effect.",
		         reverbEffect);
		reverbEffect = ReverbEffect_None;
	}
	setReverbEffect(reverbEffect);
	
	// Load category volumes (reverb and normal)
	const s32 categoryCount = bu::get<s32>(p_context);
	if (categoryCount != Category_Count)
	{
		// Cannot trust the category volume settings from data: reset to defaults
		TT_PANIC("Serialization data contains a different number of categories (%d) than what "
		         "AudioPlayer expects (%d). Were categories changed without incrementing the "
		         "serialization version number?", categoryCount, Category_Count);
		for (s32 i = 0; i < Category_Count; ++i)
		{
			const Category cat = static_cast<Category>(i);
			setVolume                 (cat, 1.0f);
			setReverbVolumeForCategory(cat, 0.0f);
		}
	}
	else
	{
		for (s32 i = 0; i < categoryCount; ++i)
		{
			const Category cat = static_cast<Category>(i);
			
			const real volume       = bu::get<real>(p_context);
			const real reverbVolume = bu::get<real>(p_context);
			
			setVolume                 (cat, volume);
			setReverbVolumeForCategory(cat, reverbVolume);
		}
	}
}


void AudioPlayer::setLoadingLevel(bool p_loading)
{
	if (p_loading == m_loadingLevel)
	{
		return;
	}
	
	if (p_loading)
	{
		// Save current settings as defaults to be applied after load complete
		for (s32 i = 0; i < Category_Count; ++i)
		{
			m_postLevelLoadSettings.categoryVolume[i] = m_categoryVolume[i];
			
			// Also mute all sound effect categories, so that any sounds played during loading won't be heard yet
			setVolume(static_cast<Category>(i), 0.0f);
		}
		
		for (s32 i = 0; i < Device_Count; ++i)
		{
			m_postLevelLoadSettings.audioEnabled[i] =
					tt::math::realEqual(m_deviceSettings[i].audioEnableFade.getEndValue(), 1.0f);
			m_postLevelLoadSettings.overallVolume[i] = m_deviceSettings[i].overallVolume.getEndValue();
		}
		
		// Enable flag last, so that any changes performed above are still applied immediately
		m_loadingLevel = true;
	}
	else
	{
		// Disable flag first, so that all AudioPlayer calls will immediately apply changes
		m_loadingLevel = false;
		
		// Apply all settings scheduled during loading
		for (s32 i = 0; i < Category_Count; ++i)
		{
			setVolume(static_cast<Category>(i), m_postLevelLoadSettings.categoryVolume[i]);
		}
		
		for (s32 i = 0; i < Device_Count; ++i)
		{
			const Device device = static_cast<Device>(i);
			
			setAudioEnabled(device, m_postLevelLoadSettings.audioEnabled[device]);
			m_deviceSettings[device].overallVolume = m_postLevelLoadSettings.overallVolume[device];
			applyOverallVolume(device);
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

AudioPlayer::AudioPlayer(tt::audio::player::SoundPlayer* p_soundPlayer)
:
m_sound(p_soundPlayer),
m_activeReverbEffect(ReverbEffect_None),
m_containerMutex(),
m_positionalEffects(),
m_listenerPosition(tt::math::Vector3::zero),
m_cueNames(),
m_registeredSoundCues(),
m_soundCuesToKeepAliveUntilDonePlaying(),
m_fadeOutAndKeepAliveUntilDone(),
m_musicTrackMgr(15),
m_loadingLevel(false)
{
	for (s32 i = 0; i < Category_Count; ++i)
	{
		m_categoryVolume[i]         = 1.0f;
		m_categoryReverbVolume[i]   = 1.0f;
		
		setVolume(static_cast<Category>(i), m_categoryVolume[i]);
	}
	/*
	m_categoryVolume[Category_Music] = 0.75f;
	
	setVolume(Category_Music, m_categoryVolume[Category_Music]);
	*/
	
	reloadCueMetaData();
	
	// Enable positional audio
	if (m_sound != 0)
	{
		m_sound->set3DAudioEnabled(true);
	}
	
	resetOverallVolume();
	
	m_activeReverbEffect = static_cast<ReverbEffect>(-1); // invalidate to force a set of reverb effect
	setReverbEffect(ReverbEffect_None);
}


AudioPlayer::~AudioPlayer()
{
	tt::thread::CriticalSection critSec(&m_containerMutex);
	
	// Clean up all sound effects before destroying the player
	m_positionalEffects.clear();
	m_soundCuesToKeepAliveUntilDonePlaying.clear();
	m_fadeOutAndKeepAliveUntilDone.clear();
	
	// Clean up players
	if (m_sound != 0)
	{
		m_sound->stopCategory(getCategoryName(Category_Effects));
		delete m_sound;
	}
}


void AudioPlayer::applyOverallVolume(Device p_device)
{
	switch (p_device)
	{
	case Device_TV:
		if (tt::snd::hasSoundSystem(0))
		{
			tt::snd::setMasterVolume(0,
					m_deviceSettings[p_device].overallVolume.getValue() *
					m_deviceSettings[p_device].audioEnableFade.getValue());
		}
		
#if defined(TT_PLATFORM_WIN)
		// NOTE: On Windows, our sound effects are implemented using Microsoft's XACT.
		//       This does not use one of our SoundSystems, so setting master volume has no effect on sound effects.
		//       Change the overall volume for sound effects by setting the volume for all XACT categories
		if (m_sound != 0)
		{
			for (s32 i = 0; i < Category_Count; ++i)
			{
				applyCategoryVolume(static_cast<Category>(i));
			}
		}
#endif
		break;
		
	case Device_DRC:
		if (tt::snd::hasSoundSystem(0))
		{
			tt::snd::setSecondaryMasterVolume(0,
					m_deviceSettings[p_device].overallVolume.getValue() *
					m_deviceSettings[p_device].audioEnableFade.getValue());
		}
		break;
		
	default:
		TT_PANIC("Unsupported audio output device: %d", p_device);
		break;
	}
}


void AudioPlayer::applyCategoryVolume(Category p_category)
{
	TT_ASSERT(isValidCategory(p_category));
	if (m_sound != 0)
	{
		real combinedVolume = getSfxVolumeFromSettings(p_category) * m_categoryVolume[p_category];
		
#if defined(TT_PLATFORM_WIN)
		// NOTE: On Windows, our sound effects are implemented using Microsoft's XACT.
		//       This does not use one of our SoundSystems, so setting master volume has no effect on sound effects.
		//       Change the overall volume for sound effects by setting the volume for all XACT categories
		combinedVolume *=
				m_deviceSettings[Device_TV].overallVolume.getValue() *
				m_deviceSettings[Device_TV].audioEnableFade.getValue();
#endif
		
		m_sound->setCategoryVolume(getCategoryName(p_category), combinedVolume);
	}
}


void AudioPlayer::loadCueMetaData(const std::string& p_filename)
{
	if (tt::fs::fileExists(p_filename) == false)
	{
		TT_PANIC("Cannot load XACT cue metadata: file '%s' does not exist.", p_filename.c_str());
		return;
	}
	
	tt::xml::XmlDocument doc(p_filename);
	const tt::xml::XmlNode* rootNode = doc.getRootNode();
	if (rootNode == 0 || rootNode->getName() != "xact_cue_list")
	{
		TT_PANIC("Cannot load XACT cue metadata: file is not valid XML or root element is not 'xact_cue_list'\n"
		         "Root element: <%s>\nFilename: '%s'",
		         (rootNode == 0) ? "" : rootNode->getName().c_str(), p_filename.c_str());
		return;
	}
	
	TT_ASSERT(m_cueNames.empty());
	
	for (const tt::xml::XmlNode* soundBank = rootNode->getChild();
	     soundBank != 0; soundBank = soundBank->getSibling())
	{
		if (soundBank->getName() != "soundbank")
		{
			TT_PANIC("Invalid XACT cue list data: <xact_cue_list> element has unsupported child <%s>. "
			         "Only <soundbank> is supported.\nFilename: '%s'",
			         soundBank->getName().c_str(), p_filename.c_str());
			continue;
		}
		
		const std::string& soundBankName(soundBank->getAttribute("name"));
		if (soundBankName.empty())
		{
			TT_PANIC("Invalid XACT cue list data: <soundbank> element is missing required attribute 'name'.\n"
			         "Filename: '%s'", p_filename.c_str());
			continue;
		}
		
		for (const tt::xml::XmlNode* cue = soundBank->getChild(); cue != 0; cue = cue->getSibling())
		{
			if (cue->getName() != "cue")
			{
				TT_PANIC("Invalid XACT cue list data: <soundbank> element has unsupported child <%s>. "
				         "Only <cue> is supported.\nFilename: '%s'",
				         cue->getName().c_str(), p_filename.c_str());
				continue;
			}
			
			const std::string& cueName(cue->getAttribute("name"));
			if (cueName.empty())
			{
				TT_PANIC("Invalid XACT cue list data: <cue> element is missing required attribute 'name'.\n"
				         "Filename: '%s'", p_filename.c_str());
				continue;
			}
			
			m_cueNames[soundBankName].insert(cueName);
			
			CueMetaData metaData;
			metaData.isLooping = cue->hasAttribute("is_looping") &&
				tt::str::parseBool(cue->getAttribute("is_looping"), 0);
			m_cueMetaDataCollection[cueName] = metaData;
		}
	}
}

// Namespace end
}
}
