#ifndef INC_TT_AUDIO_XACT_PITCHEVENTINSTANCE_H
#define INC_TT_AUDIO_XACT_PITCHEVENTINSTANCE_H


#include <tt/audio/xact/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace xact {

class PitchEventInstance
{
public:
	PitchEventInstance(PitchEvent* p_pitchEvent, TrackInstance* p_track);
	~PitchEventInstance();
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	
	void update(real p_time);
	
private:
	PitchEventInstance(const PitchEventInstance&);
	PitchEventInstance& operator=(const PitchEventInstance&);
	
	real getRandomPitch() const;
	real getTimeStamp() const;
	
	PitchEvent*    m_pitchEvent;
	TrackInstance* m_track;
	real           m_nextStart;
	int            m_loopCount;
	bool           m_paused;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_PITCHEVENTINSTANCE_H
