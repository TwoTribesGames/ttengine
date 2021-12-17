#include <tt/script/helpers.h>

#include <toki/game/script/wrappers/PointerEventWrapper.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

PointerEventWrapper::PointerEventWrapper()
:
m_event(tt::math::Vector2::zero, 0.0f, false)
{
	// NOTE: The event when default-constructed does not need to be valid:
	//       SqBind default-constructs an instance and immediately assigns a different copy to it
}


PointerEventWrapper::PointerEventWrapper(const toki::game::event::input::PointerEvent& p_event)
:
m_event(p_event)
{
}


const tt::math::Vector2& PointerEventWrapper::getPosition() const
{
	return m_event.getPosition();
}


real PointerEventWrapper::getPressDuration() const
{
	return m_event.getPressDuration();
}


bool PointerEventWrapper::isOnEntity() const
{
	return m_event.isOnEntity();
}


void PointerEventWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(PointerEventWrapper, "PointerEvent");
	TT_SQBIND_METHOD(PointerEventWrapper, getPosition);
	TT_SQBIND_METHOD(PointerEventWrapper, getPressDuration);
	TT_SQBIND_METHOD(PointerEventWrapper, isOnEntity);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
