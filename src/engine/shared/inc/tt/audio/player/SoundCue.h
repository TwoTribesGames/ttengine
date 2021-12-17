#if !defined(INC_TT_AUDIO_PLAYER_SOUNDCUE_H)
#define INC_TT_AUDIO_PLAYER_SOUNDCUE_H


#include <map>
#include <string>

#include <tt/audio/player/fwd.h>
#include <tt/audio/player/SoundCueSettings.h>
#include <tt/code/BitMask.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace player {

class SoundCue
{
public:
	enum State
	{
		State_None,
		State_Created,
		State_Playing,
		State_Stopped,
		State_Paused
	};
	
	
	virtual ~SoundCue() { }
	
	/*! \brief Plays a sound effect.
	    \return true on success, false on fail.*/
	virtual bool play() = 0;
	
	/*! \brief Stops all instances of a sound.
	    \param p_immediately Whether to stop playing immediately, or allow the cue to stop
	                         according to "release phase" and/or transition.
	    \return true on success, false on fail.*/
	virtual bool stop(bool p_immediately = false) = 0;
	
	/*! \brief Plays a sound effect.
	    \return true on success, false on fail.*/
	virtual bool pause() = 0;
	
	/*! \brief Plays a sound effect.
	    \return true on success, false on fail.*/
	virtual bool resume() = 0;
	
	/*! \return The current state of the cue. */
	virtual State getState() const = 0;
	
	virtual bool getVariable(const std::string& /*p_name*/, real* /*p_value_OUT*/) const { TT_PANIC("Not implemented"); return false; }
	
	// Setters
	
	inline virtual bool setVariable(const std::string& p_name, real p_value)
	{
		if (m_settings != 0) m_settings->setVariable(p_name, p_value);
		return true;
	}
	
	inline virtual bool setPosition(const math::Vector3& p_position)
	{
		if (m_settings != 0) m_settings->setPosition(p_position);
		return true;
	}
	
	/*! \brief Sets the inner and outer radius of a positional sound cue (only available for positional cues).
	    \param p_cue The sound cue used as the emitter.
	    \param p_inner The inner radius for the sound emitter.
	    \param p_outer The outer radius for the sound emitter.
	    \return Whether the operation succeeded.*/
	inline virtual bool setRadius(real p_inner, real p_outer)
	{
		if (m_settings != 0) m_settings->setRadius(p_inner, p_outer);
		return true;
	}
	
	inline virtual bool getRadius(real* p_inner_OUT, real* p_outer_OUT) const
	{
		return m_settings != 0 && m_settings->getRadius(p_inner_OUT, p_outer_OUT);
	}
	
	// Audio post-processing effects:
	
	/*! \brief Sets the mixing volume for the currently active reverb effect in the SoundSystem.
	    \param p_normalizedVolume The reverb mixing volume to set, in normalized range (0 - 1). */
	inline virtual bool setReverbVolume(real p_normalizedVolume)
	{
		if (m_settings != 0) m_settings->setReverbVolume(p_normalizedVolume);
		return true;
	}
	
	/*! \brief Indicates whether this cue is currently playing. */
	inline bool isPlaying() const { return getState() == State_Playing; }
	
	/*! \brief Indicates whether this cue is currently paused. */
	inline bool isPaused() const { return getState() == State_Paused; }
	
	inline bool isPositional() const { return m_isPositional; }
	
protected:
	explicit inline SoundCue(bool p_positional)
	:
	m_isPositional(p_positional),
	m_settings(new SoundCueSettings())	// FIXME: Only create settings if sound is looping
	{ }
	
	inline void applySettings()
	{
		if (m_settings != 0)
		{
			m_settings->applyToSoundCue(*this);
		}
	}
	
private:
	// No copying
	SoundCue(const SoundCue&);
	const SoundCue& operator=(const SoundCue&);
	
	bool                m_isPositional;
	SoundCueSettingsPtr m_settings;      // only used to be able to stop and start looping soundcues
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_SOUNDCUE_H)
