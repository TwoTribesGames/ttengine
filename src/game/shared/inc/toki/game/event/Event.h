#if !defined(INC_TOKI_GAME_EVENT_EVENT_H)
#define INC_TOKI_GAME_EVENT_EVENT_H

#include <string>

#include <tt/math/Point2.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/event/helpers/SoundChecker.h>
#include <toki/game/event/fwd.h>


namespace toki {
namespace game {
namespace event {

class Event
{
public:
	Event();
	Event(EventType p_type, const tt::math::Point2& p_tilePosition, real p_radius,
	      entity::EntityHandle p_source = entity::EntityHandle());
	
	Event(EventType p_type, const tt::math::Vector2& p_worldPosition, real p_radius,
	      entity::EntityHandle p_source = entity::EntityHandle());
	
	inline EventType                getType()     const { return m_type;     }
	inline const tt::math::Vector2& getPosition() const { return m_position; }
	inline entity::EntityHandle     getSource()   const { return m_source;   }
	inline real                     getRadius()   const { return m_radius;   }
	
	SignalSet process(helpers::SoundChecker& p_soundChecker) const;
	
	inline void setCallback(const std::string& p_userParam)
	{
		m_callbackSource    = true;
		m_callbackUserParam = p_userParam;
	}
	
private:
	entity::EntityHandleSet processSound(helpers::SoundChecker& p_soundChecker) const;
	entity::EntityHandleSet processVibration() const;
	
	EventType            m_type;
	tt::math::Vector2    m_position;
	entity::EntityHandle m_source;
	real                 m_radius;
	
	bool                 m_callbackSource;
	std::string          m_callbackUserParam;
};


bool operator==(const Event& p_lhs, const Event& p_rhs);
bool operator!=(const Event& p_lhs, const Event& p_rhs);
bool operator< (const Event& p_lhs, const Event& p_rhs);


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_GAME_EVENT_EVENTMGR_H)
