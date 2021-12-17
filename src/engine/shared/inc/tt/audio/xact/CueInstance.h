#if !defined(INC_TT_AUDIO_XACT_CUEINSTANCE_H)
#define INC_TT_AUDIO_XACT_CUEINSTANCE_H


#include <utility>
#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/math/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace xact {

class CueInstance
{
public:
	CueInstance(Cue* p_cue, Sound* p_sound);
	~CueInstance();
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	void update(real p_time);
	void updateVolume();
	
	bool belongsToCategory(const Category* p_category) const;
	
	bool setPosition(const math::Vector3& p_position);
	bool setEmitterRadius(real p_inner, real p_outer);
	
	void setReverbVolume(real p_volumeInDB);
	
	bool setVariable(const std::string& p_name, real  p_value);
	bool getVariable(const std::string& p_name, real* p_value_OUT);
	
	bool isDone() const;
	
private:
	CueInstance(const CueInstance&);
	CueInstance& operator=(const CueInstance&);
	
	Cue*           m_cue;
	SoundInstance* m_sound;
	real           m_time;
	bool           m_paused;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_XACT_CUEINSTANCE_H)
