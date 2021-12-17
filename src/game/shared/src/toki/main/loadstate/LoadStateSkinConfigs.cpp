#include <toki/main/loadstate/LoadStateSkinConfigs.h>
#include <toki/level/skin/SkinConfig.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateSkinConfigs::doLoadStep()
{
	AppGlobal::createSkinConfigs();
}


bool LoadStateSkinConfigs::isDone() const
{
	// This state only has one step: done right away
	return true;
}

// Namespace end
}
}
}
