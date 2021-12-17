#include <toki/main/loadstate/LoadStateScriptLists.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateScriptLists::doLoadStep()
{
	// Load script lists
	AppGlobal::loadScriptLists();
}


bool LoadStateScriptLists::isDone() const
{
	// This state only has one step: done right away
	return true;
}

// Namespace end
}
}
}
