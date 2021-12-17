#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_POINTEREVENTWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_POINTEREVENTWRAPPER_H


#include <tt/script/VirtualMachine.h>

#include <toki/game/event/input/PointerEvent.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'PointerEvent' in Squirrel. Contains information about a pointer (e.g. mouse) event. */
class PointerEventWrapper
{
public:
	PointerEventWrapper();  // default construction is needed by SqBind implementation details
	explicit PointerEventWrapper(const toki::game::event::input::PointerEvent& p_event);
	
	// Bindings
	
	/*! \return World position (Vector2) where the event occurred. */
	const tt::math::Vector2& getPosition() const;
	
	/*! \return For how long the pointer has been pressed, in seconds. */
	real getPressDuration() const;
	
	/*! \brief Whether the pointer is on the entity for which the callback was called. */
	bool isOnEntity() const;
	
	
	inline const toki::game::event::input::PointerEvent& getEvent() const { return m_event; }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	toki::game::event::input::PointerEvent m_event;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_POINTEREVENTWRAPPER_H)
