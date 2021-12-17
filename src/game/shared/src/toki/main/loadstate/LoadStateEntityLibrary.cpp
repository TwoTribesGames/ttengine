#include <toki/main/loadstate/LoadStateEntityLibrary.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateEntityLibrary::doLoadStep()
{
	// Populate the entity library
	AppGlobal::getEntityLibrary().rescan();
}


bool LoadStateEntityLibrary::isDone() const
{
	// This state only has one step: done right away
	return true;
}

// Namespace end
}
}
}
