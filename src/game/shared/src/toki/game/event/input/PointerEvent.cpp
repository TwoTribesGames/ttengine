#include <tt/code/bufferutils.h>

#include <toki/game/event/input/PointerEvent.h>


namespace toki {
namespace game {
namespace event {
namespace input {

//--------------------------------------------------------------------------------------------------
// Public member functions

PointerEvent::PointerEvent(const tt::math::Vector2& p_pos, real p_pressDuration, bool p_onEntity)
:
m_position(p_pos),
m_pressDuration(p_pressDuration),
m_onEntity(p_onEntity)
{
}


void PointerEvent::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_position,      p_context);
	bu::put(m_pressDuration, p_context);
	bu::put(m_onEntity,      p_context);
}


PointerEvent PointerEvent::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	tt::math::Vector2 position      = bu::get<tt::math::Vector2>(p_context);
	real              pressDuration = bu::get<real             >(p_context);
	bool              onEntity      = bu::get<bool             >(p_context);
	
	return PointerEvent(position, pressDuration, onEntity);
}

// Namespace end
}
}
}
}
