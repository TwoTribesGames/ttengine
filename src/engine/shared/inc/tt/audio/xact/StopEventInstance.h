#ifndef INC_TT_AUDIO_XACT_STOPEVENTINSTANCE_H
#define INC_TT_AUDIO_XACT_STOPEVENTINSTANCE_H


#include <tt/audio/xact/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class StopEventInstance
{
public:
	StopEventInstance(StopEvent* p_stopEvent, TrackInstance* p_track);
	~StopEventInstance();
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	
	void update(real p_time);
	
private:
	StopEventInstance(const StopEventInstance&);
	StopEventInstance& operator=(const StopEventInstance&);
	
	real getTimeStamp() const;
	
	StopEvent*     m_stopEvent;
	TrackInstance* m_track;
	real           m_startEvent;
	bool           m_paused;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_STOPEVENTINSTANCE_H
