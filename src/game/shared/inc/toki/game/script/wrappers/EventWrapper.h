#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_EVENTWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_EVENTWRAPPER_H


#include <tt/math/Vector2.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/event/Event.h>
#include <toki/game/script/wrappers/fwd.h>
#include <toki/game/script/fwd.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


/*! \brief 'Event' in Squirrel. */
class EventWrapper
{
public:
	EventWrapper() {}
	explicit EventWrapper(const toki::game::event::Event& p_event);
	inline ~EventWrapper() { }
	
	// bindings
	
	/*! \brief Returns the world position (Vector2) where the event occurred. */
	const tt::math::Vector2& getPosition() const;
	
	/*! \brief Returns the entity from which the event originated. Can be null if the event did not come from an entity. */
	EntityBase*              getSource()   const;
	
	/*! \brief Returns the radius of the event. */
	real                     getRadius() const;
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	toki::game::event::Event m_event;
};


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_EVENTWRAPPER_H)
