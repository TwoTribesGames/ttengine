#include <toki/game/light/Darkness.h>
#include <toki/game/light/DarknessMgr.h>
#include <toki/game/script/wrappers/DarknessWrapper.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

DarknessWrapper::DarknessWrapper(const light::DarknessHandle& p_darknessHandle)
:
m_darknessHandle(p_darknessHandle)
{
}


bool DarknessWrapper::isEnabled() const
{
	const light::Darkness* darkness = m_darknessHandle.getPtr();
	return (darkness != 0) ? darkness->isEnabled() : false;
}


void DarknessWrapper::setEnabled(bool p_enabled)
{
	light::Darkness* darkness = m_darknessHandle.getPtr();
	if (darkness != 0)
	{
		darkness->setEnabled(p_enabled);
	}
}


real DarknessWrapper::getWidth() const
{
	const light::Darkness* darkness = m_darknessHandle.getPtr();
	return (darkness != 0) ? darkness->getWidth() : 0.0f;
}


real DarknessWrapper::getHeight() const
{
	const light::Darkness* darkness = m_darknessHandle.getPtr();
	return (darkness != 0) ? darkness->getHeight() : 0.0f;
}


void DarknessWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(DarknessWrapper, "Darkness");
	TT_SQBIND_METHOD(DarknessWrapper, isEnabled);
	TT_SQBIND_METHOD(DarknessWrapper, setEnabled);
	TT_SQBIND_METHOD(DarknessWrapper, getWidth);
	TT_SQBIND_METHOD(DarknessWrapper, getHeight);
	TT_SQBIND_METHOD(DarknessWrapper, equals);
	TT_SQBIND_METHOD(DarknessWrapper, getHandleValue);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
