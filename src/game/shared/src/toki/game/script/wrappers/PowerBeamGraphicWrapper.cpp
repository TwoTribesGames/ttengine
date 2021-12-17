#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>
#include <tt/script/ScriptEngine.h>

#include <toki/game/script/wrappers/PowerBeamGraphicWrapper.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

PowerBeamGraphicWrapper::PowerBeamGraphicWrapper()
:
m_powerBeamGraphic()
{
}


PowerBeamGraphicWrapper::PowerBeamGraphicWrapper(
		const entity::graphics::PowerBeamGraphicHandle& p_powerBeamGraphic)
:
m_powerBeamGraphic(p_powerBeamGraphic)
{
}


void PowerBeamGraphicWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_powerBeamGraphic, p_context);
}


void PowerBeamGraphicWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_powerBeamGraphic = bu::getHandle<entity::graphics::PowerBeamGraphic>(p_context);
}


void PowerBeamGraphicWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(PowerBeamGraphicWrapper, "PowerBeamGraphic");
}

// Namespace end
}
}
}
}
