#if !defined(INC_TT_AUDIO_PLAYER_TTXACTPLAYER_H)
#define INC_TT_AUDIO_PLAYER_TTXACTPLAYER_H


/*
#define NOMINMAX
#include <xact3.h>
*/

#include <map>

#include <tt/audio/player/SoundPlayer.h>
#include <tt/code/Buffer.h>
#include <tt/fs/types.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace audio {
namespace player {

/*! \brief SoundPlayer front-end for tt::audio::xact::AudioTT XACT clone. */
class TTXactPlayer : public SoundPlayer
{
public:
	TTXactPlayer();
	virtual ~TTXactPlayer();
	
	
	// Initialization functions
	
	/*! \brief Initialize the TT XACT engine and load project data.
	    \param p_projectFile The XACT project file (.xap) to load.
	    \param p_autoLoad Whether to automatically load all wave banks in the project file.
	    \return True on success, false on failure or when already initialized.*/
	bool init(const std::string& p_projectFile, bool p_autoLoad = true);
	
	/*! \brief Uninitialize the TT XACT engine.
	    \return True on success, false on failure or when not initialized.*/
	bool uninit();
	
	/*! \brief Returns whether the player is initialized or not.
	    \return Whether the player is initialized or not.*/
	inline bool isInitialized() const { return m_initialized; }
	
	virtual void update(real p_deltaTime);
	
	
	// Sound functions
	
	virtual SoundCuePtr createCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional = false);
	virtual bool        play     (const std::string& p_soundbank, const std::string& p_name);
	virtual SoundCuePtr playCue  (const std::string& p_soundbank, const std::string& p_name, bool p_positional = false);
	virtual bool        stop     (const std::string& p_soundbank, const std::string& p_name);
	
	
	// Category functions
	
	virtual bool hasCategory      (const std::string& p_category);
	virtual bool stopCategory     (const std::string& p_category);
	virtual bool pauseCategory    (const std::string& p_category);
	virtual bool resumeCategory   (const std::string& p_category);
	virtual bool setCategoryVolume(const std::string& p_category, real p_normalizedVolume);
	
	// Positional Audio
	virtual bool set3DAudioEnabled(bool p_enabled);
	virtual bool setListenerPosition(const math::Vector3& p_position);
	
	// Audio post-processing effects
	virtual bool setReverbVolumeForCategory(const std::string& p_category, real p_normalizedVolume);
	
private:
	// No copying
	TTXactPlayer(const TTXactPlayer&);
	const TTXactPlayer& operator=(const TTXactPlayer&);
	
	
	bool          m_initialized;
	math::Vector3 m_listenerPosition;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_AUDIO_PLAYER_TTXACTPLAYER_H)
