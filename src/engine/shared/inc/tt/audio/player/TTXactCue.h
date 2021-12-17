#if !defined(INC_TT_AUDIO_PLAYER_TTXACTCUE_H)
#define INC_TT_AUDIO_PLAYER_TTXACTCUE_H


#include <string>

#include <tt/audio/player/SoundCue.h>
#include <tt/audio/xact/CueInstance.h>


namespace tt {
namespace audio {
namespace player {

class TTXactPlayer;

class TTXactCue : public SoundCue
{
public:
	virtual ~TTXactCue();
	
	virtual bool play();
	virtual bool stop(bool p_immediately = false);
	virtual bool pause();
	virtual bool resume();
	
	virtual bool setVariable(const std::string& p_name, real  p_value);
	virtual bool getVariable(const std::string& p_name, real* p_value_OUT) const;
	
	virtual bool setPosition(const math::Vector3& p_position);
	virtual bool setRadius  (real p_inner, real p_outer);
	
	virtual bool setReverbVolume(real p_normalizedVolume);
	
	virtual State getState() const;
	
private:
	TTXactCue(xact::SoundBank*   p_soundBank,
	          s32                p_cueIndex,
	          bool               p_positional,
	          const std::string& p_cueName);
	
	
	xact::SoundBank*         m_soundBank;
	s32                      m_cueIndex;
	SoundCue::State          m_state;
	
	// Created at play()
	xact::CueInstanceWeakPtr m_cue;
	
#if !defined(TT_BUILD_FINAL)
	std::string              m_cueName;
#endif
	
	friend class TTXactPlayer;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_TTXACTCUE_H)
