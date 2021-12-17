#include <tt/stats/stats.h>

#include <toki/main/loadstate/LoadStateScriptMgr.h>
#include <toki/script/ScriptMgr.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateScriptMgr::doLoadStep()
{
	// Initialize the Script Manager
	script::ScriptMgr::init();
	
	// The statistic definitions come from script, so we need to create this after script was initilized.
	tt::stats::createInstance(script::ScriptMgr::getVM());
	
	/* // FIXME: Already added leaderboard setup code for future addition.
#ifndef TT_BUILD_FINAL
	// Autocreate leaderboards in non-final builds
	bool autoCreateLeaderboards = tt::app::getApplication()->getCmdLine().exists("autocreateleaderboards");
	tt::steam::Leaderboards::createInstance(g_leaderboardSaveFilename, 
		tt::app::getApplication()->getSaveFsID(), autoCreateLeaderboards, 
		k_ELeaderboardSortMethodAscending,
		k_ELeaderboardDisplayTypeTimeMilliSeconds);
#else
	tt::steam::Leaderboards::createInstance(g_leaderboardSaveFilename, 
		tt::app::getApplication()->getSaveFsID());
#endif
	*/
}


bool LoadStateScriptMgr::isDone() const
{
	// This state only has one step: done right away
	return true;
}

// Namespace end
}
}
}
