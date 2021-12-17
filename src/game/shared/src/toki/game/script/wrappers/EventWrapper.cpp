#include <tt/script/ScriptEngine.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/event/Event.h>
#include <toki/game/script/wrappers/EventWrapper.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

EventWrapper::EventWrapper(const toki::game::event::Event& p_event)
:
m_event(p_event)
{
}


const tt::math::Vector2& EventWrapper::getPosition() const
{
	TT_ASSERT(m_event.getType() != toki::game::event::EventType_None);
	return m_event.getPosition();
}


EntityBase* EventWrapper::getSource() const
{
	TT_ASSERT(m_event.getType() != toki::game::event::EventType_None);
	entity::Entity* entity = m_event.getSource().getPtr();
	return (entity != 0) ? entity->getEntityScript().get() : 0;
}


real EventWrapper::getRadius() const
{
	TT_ASSERT(m_event.getType() != toki::game::event::EventType_None);
	return m_event.getRadius();
}


void EventWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace event;
	TT_SQBIND_CONSTANT(EventType_Sound);
	TT_SQBIND_CONSTANT(EventType_Vibration);
	
	TT_SQBIND_INIT_NAME(EventWrapper, "Event");
	TT_SQBIND_METHOD(EventWrapper, getSource);
	TT_SQBIND_METHOD(EventWrapper, getPosition);
	TT_SQBIND_METHOD(EventWrapper, getRadius);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
