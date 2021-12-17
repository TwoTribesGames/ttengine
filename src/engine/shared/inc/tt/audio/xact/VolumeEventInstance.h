#ifndef INC_TT_AUDIO_XACT_VOLUMEEVENTINSTANCE_H
#define INC_TT_AUDIO_XACT_VOLUMEEVENTINSTANCE_H


#include <tt/audio/xact/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace xact {

class VolumeEventInstance
{
public:
	VolumeEventInstance(VolumeEvent* p_volumeEvent, TrackInstance* p_track);
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	void update(real p_time);
	
private:
	VolumeEventInstance(const VolumeEventInstance&);
	VolumeEventInstance& operator=(const VolumeEventInstance&);
	
	real getRandomVolume() const;
	real getTimeStamp()    const;
	
	
	VolumeEvent*   m_volumeEvent;
	TrackInstance* m_track;
	real           m_nextStart;
	int            m_loopCount;
	bool           m_paused;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_VOLUMEEVENTINSTANCE_H
