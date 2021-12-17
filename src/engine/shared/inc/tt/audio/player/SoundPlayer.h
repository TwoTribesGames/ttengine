#if !defined(INC_TT_AUDIO_PLAYER_SOUNDPLAYER_H)
#define INC_TT_AUDIO_PLAYER_SOUNDPLAYER_H


#include <string>

#include <tt/audio/player/fwd.h>
#include <tt/platform/tt_types.h>
#include <tt/math/fwd.h>


namespace tt {
namespace audio {
namespace player {

class SoundPlayer
{
public:
	SoundPlayer();
	virtual ~SoundPlayer();
	
	
	/*! \brief Update the sound player, call this once per frame.*/
	virtual void update(real p_deltaTime) = 0;
	
	
	// Sound functions
	
	virtual SoundCuePtr createCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional = false) = 0;
	virtual bool        play     (const std::string& p_soundbank, const std::string& p_name) = 0;
	virtual SoundCuePtr playCue  (const std::string& p_soundbank, const std::string& p_name, bool p_positional = false) = 0;
	virtual bool        stop     (const std::string& p_soundbank, const std::string& p_name) = 0;
	
	// Category functions
	
	/*! \brief Returns whether a category exists.
	    \param p_category The name of the category to check.
	    \return Whether the category exists.*/
	virtual bool hasCategory(const std::string& p_category) = 0;
	
	/*! \brief Stops all sounds of a category.
	    \param p_category The category to stop.
	    \return true on success, false on fail or when category does not exist.*/
	virtual bool stopCategory(const std::string& p_category) = 0;
	
	/*! \brief Pauses a category.
	    \param p_category The category to pause.
	    \return true on success or when already paused, false on fail or when category does not exist.*/
	virtual bool pauseCategory(const std::string& p_category) = 0;
	
	/*! \brief Resumes a currently paused category.
	    \param p_category The category to resume.
	    \return true on success or when already resumed, false on fail or when category does not exist.*/
	virtual bool resumeCategory(const std::string& p_category) = 0;
	
	/*! \brief Sets the volume for a category.
	    \param p_category The category to set the volume of.
	    \param p_normalizedVolume The volume to set [0.0 - 1.0].
	    \return Whether the operation succeeded.*/
	virtual bool setCategoryVolume(const std::string& p_category, real p_normalizedVolume) = 0;

	/*! \brief Enables the use of positional audio (overrides panning settings)
	    \param p_enabled Whether 3D audio should be enabled or disabled
		\return Whether the operation succeeded.*/
	virtual bool set3DAudioEnabled(bool p_enabled);

	/*! \brief Sets the position of the global listener (for positional audio)
	    \param p_position The new position for the global listener.
	    \return Whether the operation succeeded.*/
	virtual bool setListenerPosition(const math::Vector3& p_position);
	
	
	// Global RPC variables:
	
	virtual bool setGlobalVariable(const std::string& p_name, real  p_value);
	virtual bool getGlobalVariable(const std::string& p_name, real* p_value_OUT) const;
	
	
	// Audio post-processing effects:
	
	/*! \brief Sets the mixing volume for the currently active reverb effect in the SoundSystem,
	           for an entire category.
	    \param p_category The category to operate on.
	    \param p_normalizedVolume The reverb mixing volume to set, in normalized range (0 - 1). */
	virtual bool setReverbVolumeForCategory(const std::string& p_category, real p_normalizedVolume);
	
private:
	// No copying
	SoundPlayer(const SoundPlayer&);
	const SoundPlayer& operator=(const SoundPlayer&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_SOUNDPLAYER_H)
