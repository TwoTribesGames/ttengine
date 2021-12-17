#include <tt/code/bufferutils.h>
#include <tt/script/helpers.h>

#include <toki/game/script/wrappers/PresentationStartSettingsWrapper.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

void PresentationStartSettingsWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	m_settings.serialize(p_context);
}


void PresentationStartSettingsWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	*this = PresentationStartSettingsWrapper();
	
	m_settings.unserialize(p_context);
}


void PresentationStartSettingsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(PresentationStartSettingsWrapper, "PresentationStartSettings");
	TT_SQBIND_METHOD(PresentationStartSettingsWrapper, setEndPos);
	TT_SQBIND_METHOD(PresentationStartSettingsWrapper, setEndCallbackName);
}



// Namespace end
}
}
}
}
