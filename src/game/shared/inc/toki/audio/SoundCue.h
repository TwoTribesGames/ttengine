#if !defined(INC_TOKI_AUDIO_SOUNDCUE_H)
#define INC_TOKI_AUDIO_SOUNDCUE_H


#include <map>
#include <string>
#include <vector>

#include <tt/audio/player/SoundCue.h>
#include <tt/code/fwd.h>
#include <tt/math/TimedLinearInterpolation.h>
#include <tt/platform/tt_types.h>

#include <toki/audio/fwd.h>
#include <toki/game/entity/fwd.h>


namespace toki {
namespace audio {

class SoundCue
{
public:
	struct PlayInfo
	{
		std::string                soundbank;                // The name of the soundbank the effect is in
		std::string                name;                     // The name of the sound effect that was started
		bool                       isPositional;             // Whether the sound effect is started as positional audio
		game::entity::EntityHandle entityForPositionalSound; // Which entity a positional sound effect should follow
		
		
		inline PlayInfo()
		:
		soundbank(),
		name(),
		isPositional(false),
		entityForPositionalSound()
		{
		}
	};
	
	static SoundCuePtr create();
	static SoundCuePtr create(const PlayInfo& p_playInfo);
	
	void play();
	void stop();
	void pause();
	void resume();
	bool isPlaying() const;
	bool isPaused() const;
	
	inline void setVariable(const std::string& p_name, real p_value)
	{
		setFadingVariable(p_name, p_value, 0.0f, false);
	}
	void setFadingVariable(const std::string& p_name,
	                       real               p_value,
	                       real               p_duration,
	                       bool               p_stopCueAfterFade);
	
	inline bool hasVariables() const { return m_variables.empty() == false; }
	
	bool setRadius(real p_inner, real p_outer);
	
	void setReverbVolume(real p_normalizedVolume);
	
	void update(real p_deltaTime);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	typedef tt::math::TimedLinearInterpolation<real> TLI;
	struct FadingVariable
	{
		std::string name;
		TLI         interpolation;
		bool        stopCueAfterFade;
		
		
		inline FadingVariable()
		:
		name(),
		interpolation(),
		stopCueAfterFade(false)
		{
		}
	};
	typedef std::vector<FadingVariable> FadingVariables;
	typedef std::map<std::string, real> NonFadingVariables;
	
	static const s32 reserveCount = 5;
	
	
	SoundCue();
	SoundCue(const PlayInfo& p_playInfo);
	
	void reset();
	
	// No assignment
	const SoundCue& operator=(const SoundCue&);
	
	
	tt::audio::player::SoundCuePtr m_cue;
	FadingVariables                m_variables;
	NonFadingVariables             m_nonFadingVariables;
	PlayInfo                       m_playInfo;
	real                           m_emitterConeInnerRadius;  // negative number if not set
	real                           m_emitterConeOuterRadius;  // negative number if not set
	real                           m_reverbNormalizedVolume;  // negative number if not set
	SoundCueWeakPtr                m_this;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_SOUNDCUE_H)
