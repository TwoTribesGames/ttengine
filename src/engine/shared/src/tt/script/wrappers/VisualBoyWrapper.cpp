#include <visualboy/VisualBoy.h>

#include <tt/fs/utils/utils.h>

#include <tt/script/helpers.h>
#include <tt/script/wrappers/VisualBoyWrapper.h>


namespace tt{
namespace script {
namespace wrappers {


void VisualBoyWrapper::load(const std::string& p_rom)
{
	VisualBoy::load(p_rom);
}


void VisualBoyWrapper::setSoundVolume(real p_volume)
{
	VisualBoy::setSoundVolume(p_volume);
}


void VisualBoyWrapper::setFilter(const std::string& p_filter)
{
	VisualBoy::setFilter(p_filter);
}


void VisualBoyWrapper::pause()
{
	VisualBoy::pause();
}

bool VisualBoyWrapper::isPaused()
{
	return VisualBoy::isPaused();
}


void VisualBoyWrapper::unpause()
{
	VisualBoy::resume();
}


void VisualBoyWrapper::unload()
{
	VisualBoy::unload();
}


void VisualBoyWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(VisualBoyWrapper, "VisualBoy");
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, load);
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, setSoundVolume);
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, setFilter);
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, pause);
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, isPaused);
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, unpause);
	TT_SQBIND_STATIC_METHOD(VisualBoyWrapper, unload);
}

// Namespace end
}
}
}
