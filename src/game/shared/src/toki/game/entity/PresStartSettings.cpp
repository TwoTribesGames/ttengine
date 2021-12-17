#include <tt/script/helpers.h>

#include <toki/game/entity/PresStartSettings.h>

namespace toki {
namespace game {
namespace entity {


void PresStartSettings::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(PresStartSettings);
	TT_SQBIND_METHOD(PresStartSettings, setEndPos);
	TT_SQBIND_METHOD(PresStartSettings, setEndCallbackName);
}


// Namespace end
}
}
}
