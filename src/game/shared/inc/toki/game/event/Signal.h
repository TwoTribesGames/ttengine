#if !defined(INC_TOKI_GAME_EVENT_SIGNAL_H)
#define INC_TOKI_GAME_EVENT_SIGNAL_H

#include <toki/game/entity/fwd.h>
#include <toki/game/event/fwd.h>

namespace toki {
namespace game {
namespace event {


class Signal
{
public:
	Signal(entity::EntityHandle p_target, const Event* p_event)
	:
	m_target(p_target),
	m_event(p_event)
	{}
	
	inline entity::EntityHandle getTarget() const { return m_target; }
	inline const Event*         getEvent () const { return m_event;  }
	
private:
	entity::EntityHandle m_target;
	const Event*         m_event;
};


inline bool operator==(const Signal& p_lhs, const Signal& p_rhs)
{
	return p_lhs.getTarget() == p_rhs.getTarget() &&
	       p_lhs.getEvent()  == p_rhs.getEvent();
}


inline bool operator!=(const Signal& p_lhs, const Signal& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


inline bool operator <(const Signal& p_lhs, const Signal& p_rhs)
{
	if (p_lhs.getTarget() != p_rhs.getTarget())
	{
		return p_lhs.getTarget() < p_rhs.getTarget();
	}
	
	return p_lhs.getEvent() < p_rhs.getEvent();
}


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_GAME_EVENT_SIGNAL_H)
