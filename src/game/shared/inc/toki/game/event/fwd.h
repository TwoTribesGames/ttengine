#if !defined(INC_TOKI_GAME_EVENT_FWD_H)
#define INC_TOKI_GAME_EVENT_FWD_H

#include <set>

#include <tt/platform/tt_types.h>


namespace toki  /*! */ {
namespace game  /*! */ {
namespace event /*! */ {

/*! \brief Event type*/
enum EventType
{
	EventType_None,
	
	EventType_Sound,    //!< Sound
	EventType_Vibration //!< Vibration
};

class Event;
class Signal;
typedef std::set<Signal> SignalSet;

class EventMgr;
typedef tt_ptr<EventMgr>::shared EventMgrPtr;

class SoundGraphicsMgr;
typedef tt_ptr<SoundGraphicsMgr>::shared SoundGraphicsMgrPtr;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_EVENT_FWD_H)
