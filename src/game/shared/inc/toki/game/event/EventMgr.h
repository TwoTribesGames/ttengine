#if !defined(INC_TOKI_GAME_EVENT_EVENTMGR_H)
#define INC_TOKI_GAME_EVENT_EVENTMGR_H

#include <set>

#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>

#include <toki/game/event/helpers/SoundChecker.h>
#include <toki/game/event/fwd.h>


namespace toki {
namespace game {
namespace event {

class EventMgr
{
public:
	EventMgr();
	
	void update(real p_deltatime);
	
	void registerEvent(const Event& p_event);
	void unregisterEvent(const Event& p_event);
	
	s32 getSignalCount() const { return m_signalCount; }
	s32 getEventCount() const { return m_eventCount; }
	
private:
	// No copying
	EventMgr(const EventMgr&);
	EventMgr& operator=(const EventMgr&);
	
	typedef std::set<Event> Events;
	Events m_events[2];
	s32 m_curEventsIndex;
	
	// Used for processing sound events
	helpers::SoundChecker m_soundChecker;
	
	// Debug stats
	s32 m_signalCount;
	s32 m_eventCount;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_GAME_EVENT_EVENTMGR_H)
