#if !defined(INC_TOKI_GAME_EVENT_INPUT_POINTEREVENT_H)
#define INC_TOKI_GAME_EVENT_INPUT_POINTEREVENT_H


#include <tt/code/fwd.h>
#include <tt/math/Vector2.h>


namespace toki {
namespace game {
namespace event {
namespace input {

class PointerEvent
{
public:
	PointerEvent(const tt::math::Vector2& p_pos, real p_pressDuration, bool p_onEntity);
	
	inline const tt::math::Vector2& getPosition()      const { return m_position;      }
	inline real                     getPressDuration() const { return m_pressDuration; }
	inline bool                     isOnEntity()       const { return m_onEntity;      }
	
	void serialize(tt::code::BufferWriteContext* p_context) const;
	static PointerEvent unserialize(tt::code::BufferReadContext* p_context);
	
private:
	tt::math::Vector2 m_position;
	real              m_pressDuration;
	bool              m_onEntity;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EVENT_INPUT_POINTEREVENT_H)
