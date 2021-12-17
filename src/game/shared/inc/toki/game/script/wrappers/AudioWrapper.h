#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_AUDIOWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_AUDIOWRAPPER_H


#include <tt/script/helpers.h>

#include <toki/audio/constants.h>
#include <toki/constants.h>
#include <toki/game/script/wrappers/SoundCueWrapper.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'Audio' in Squirrel. */
class AudioWrapper
{
public:
	/*! \brief Plays a global sound effect. This version is not positional (equal volume no matter the distance).
	           The scope of the returned SoundCue determines the lifetime of the sound effect.
	    \param p_soundbank The name of the soundbank
	    \param p_effectName The name of the sound effect (XACT cue) to play.
	    \return SoundCue object that can be used to control the sound effect. */
	static SoundCueWrapper playGlobalSoundEffect(const std::string& p_soundbank, const std::string& p_effectName);
	
	/*! \brief Stops a whole audio category.
	    \param p_categoryName The name of the category to pause. */
	static void stopCategory(const std::string& p_categoryName);
	
	/*! \brief Pauses a whole audio category.
	    \param p_categoryName The name of the category to pause. */
	static void pauseCategory(const std::string& p_categoryName);
	
	/*! \brief Pauses a whole audio category.
	    \param p_categoryName The name of the category to pause. */
	static void resumeCategory(const std::string& p_categoryName);
	
	/*! \brief Sets the global reverb effect to apply to the audio
	           (sound effects need to set a reverb mixing volume in order to get this effect:
	           either by setting the reverb mixing volume for an entire category or setting it per cue). */
	static void setReverbEffect(const std::string& p_effectName);
	
	/*! \brief Sets the reverb mixing volume for a whole audio category.
	    \param p_categoryName The name of the category for which to set the volume.
	    \param p_volumePercentage The volume to set, in range 0 - 100. */
	static void setReverbVolumeForCategory(const std::string& p_categoryName, real p_volumePercentage);
	
	/*! \brief Sets the audio volume for a whole audio category.
	    \param p_categoryName The name of the category for which to set the volume.
	    \param p_volumePercentage The volume to set, in range 0 - 100. */
	static void setVolumeForCategory(const std::string& p_categoryName, real p_volumePercentage);

	/*! \brief Retrieves the audio volume for a whole audio category.
	    \param p_categoryName The name of the category for which to retrieve the volume.
	    \return The volume, in range 0 - 100. */
	static real getVolumeForCategory(const std::string& p_categoryName);
	
	/*! \brief Sets the overall audio volume for the TV.
	    \param p_volumePercentage The volume to set, in range 0 - 100.
	    \param p_duration         Fade to the new volume in this many seconds (if 0 or a negative number: set instantly). */
	static void setTVOverallVolume(real p_volumePercentage, real p_duration);
	
	/*! \brief Sets the overall audio volume for the DRC (Wii U GamePad).
	    \param p_volumePercentage The volume to set, in range 0 - 100.
	    \param p_duration         Fade to the new volume in this many seconds (if 0 or a negative number: set instantly). */
	static void setDRCOverallVolume(real p_volumePercentage, real p_duration);
	
	/*! \brief Sets whether audio should play on the TV.
	    \param p_enabled Whether the audio on TV should be enabled (independent of the volume set for it). */
	static void setTVAudioEnabled(bool p_enabled);
	
	/*! \brief Sets whether audio should play on the DRC (Wii U GamePad).
	    \param p_enabled Whether the audio on DRC should be enabled (independent of the volume set for it). */
	static void setDRCAudioEnabled(bool p_enabled);
	
	/*! \brief Sets the user controlled volume of the music.
	    \param p_volumePercentage The volume to set, in range 0 - 100. */
	static void setUserSettingVolumeMusic(real p_volumePercentage);
	
	/*! \brief Sets the user controlled volume of the sound effects.
	    \param p_categoryName The name of the category for which to set the volume.
	    \param p_volumePercentage The volume to set, in range 0 - 100. */
	static void setUserSettingVolumeSfx(const std::string& p_categoryName, real p_volumePercentage);
	
	/*! \brief Retrieves the user controlled volume of the music.
	    \return The volume, in range 0 - 100. */
	static real getUserSettingVolumeMusic();
	
	/*! \brief Retrieves the user controlled volume of the sound effects.
	    \param p_categoryName The name of the category for which to retrieve the volume.
	    \return The volume, in range 0 - 100. */
	static real getUserSettingVolumeSfx(const std::string& p_categoryName);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	static audio::Category getCategory(const std::string& p_categoryName);
	static void setOverallVolume(audio::Device p_device, real p_volumePercentage, real p_duration);
	static void setAudioEnabled (audio::Device p_device, bool p_enabled);
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_AUDIOWRAPPER_H)
