#ifndef INC_TT_AUDIO_XACT_STOPEVENT_H
#define INC_TT_AUDIO_XACT_STOPEVENT_H


#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class StopEvent
{
public:
	enum StopBehavior
	{
		Behavior_Immediate,
		Behavior_WithRelease // not supported
	};
	
	enum StopObject
	{
		Object_EntireCue,
		Object_TrackOnly
	};
	
	StopEvent();
	~StopEvent();
	
	void setTimeStamp(real p_time);
	inline real getTimeStamp() const { return m_timeStamp; }
	
	void setRandomOffset(real p_offset);
	inline real getRandomOffset() const { return m_randomOffset; }
	
	inline void setStopBehavior(StopBehavior p_behavior) { m_behavior = p_behavior; }
	inline StopBehavior getStopBehavior() const { return m_behavior; }
	
	inline void setStopObject(StopObject p_object) { m_object = p_object; }
	inline StopObject getStopObject() const { return m_object; }
	
	static StopEvent* createStopEvent(const xml::XmlNode* p_node);
	
	StopEventInstance* instantiate(TrackInstance* p_track);
	
private:
	StopEvent(const StopEvent&);
	StopEvent& operator=(const StopEvent&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	
	real m_timeStamp;    // The earliest time at which this event can occur, in seconds.
	real m_randomOffset; // Randomly delay playback of the event by up to this much time, in seconds.
	
	StopBehavior m_behavior;
	StopObject   m_object;
	
	friend class Track;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_STOPEVENT_H
