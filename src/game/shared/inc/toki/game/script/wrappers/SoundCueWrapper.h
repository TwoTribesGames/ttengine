#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_SOUNDCUEWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_SOUNDCUEWRAPPER_H


#include <string>

#include <tt/code/fwd.h>
#include <tt/script/helpers.h>

#include <toki/audio/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'SoundCue' in Squirrel. Used to control sound effects */
class SoundCueWrapper
{
public:
	inline SoundCueWrapper()
	:
	m_cue()
	{ }
	
	inline explicit SoundCueWrapper(const audio::SoundCuePtr& p_cue)
	:
	m_cue(p_cue)
	{ }
	
	/*! \brief Stop the sound effect. The sound effect will be removed and cannot be used anymore afterwards. */
	void stop();
	
	/*! \brief Pause the sound effect. */
	void pause();
	
	/*! \brief Resume a paused sound effect. */
	void resume();
	
	/*! \brief Get sound effect state.
	    \return true if the sound effect is currently playing. */
	bool isPlaying();
	
	/*! \brief Get sound effect state.
	    \return true if the sound effect is currently paused. */
	bool isPaused();
	
	/*! \brief Sets a per-cue-instance variable value. */
	bool setVariable(const std::string& p_name, real p_value);
	
	/*! \brief Sets a per-cue-instance fading variable value. */
	bool setFadingVariable(const std::string& p_name, real p_value, real p_duration);
	
	/*! \brief Fades a variable value to the specified value and stops the cue afterwards. */
	bool fadeAndStop(const std::string& p_name, real p_value, real p_duration);
	
	/*! \brief For positional audio: sets the radius of the sound effect
	           (from how far away it can be heard), in world units ('tiles').
	    \note This function can only be called on positional sound effects! */
	bool setRadius(real p_inner, real p_outer);
	
	/*! \brief Sets the reverb mixing volume for this specific cue.
	    \param p_volumePercentage The volume to set, in range 0 - 100. */
	void setReverbVolume(real p_volumePercentage);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	audio::SoundCuePtr m_cue;
};

// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_SOUNDCUEWRAPPER_H)
