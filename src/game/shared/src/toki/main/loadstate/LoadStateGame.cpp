#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/script/Registry.h>
#include <toki/game/Game.h>
#include <toki/game/CheckPointMgr.h>
#include <toki/input/fwd.h>
#include <toki/main/loadstate/LoadStateGame.h>
#include <toki/savedata/utils.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/serialization/utils.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {

#if !defined(TT_BUILD_FINAL)
static bool g_performedGameLoadStep = false;
#endif


//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateGame::doLoadStep()
{
#if !defined(TT_BUILD_FINAL)
	// Check if we already did this load step. (Can be because -> see show_loading_screen cmd arg in isDone function.)
	if (g_performedGameLoadStep)
	{
		return;
	}
#endif
	
	game::StartInfo startInfo;
	startInfo.resetToDefaultLevel();
	
	bool loadedPersistentData = false;
	
	// NOTE: Using write mode here, because we want to delete the shutdown state after it is loaded
	const bool mountOk = savedata::mountSaveVolumeForWriting();
	serialization::PersistentDataLoadResult loadResult = serialization::PersistentDataLoadResult_Fail;
	
	if (mountOk &&
	    AppGlobal::isInLevelEditorMode() == false &&
	    AppGlobal::shouldRestoreShutdownData())
	{
		// Load the persistent data
		toki::serialization::PersistentData persistentData;
		loadResult = serialization::loadPersistentData(&persistentData);
		if (serialization::isPersistentDataLoadResultSuccessful(loadResult))
		{
			loadedPersistentData = true;
		}
	}
	
	// --userlevel and --userrecording should be available in final builds as well
	const tt::args::CmdLine unfilteredCmdLine(tt::args::CmdLine::getApplicationCmdLine());
	if (unfilteredCmdLine.exists("userlevel"))
	{
		// Start a user level with full path from the command line
		startInfo.setUserLevel(unfilteredCmdLine.getString("userlevel"));
	}
#if ENABLE_RECORDER
	else if (unfilteredCmdLine.exists("userrecording"))
	{
		// Play a user recording with full path from the command line
		startInfo.setUserRecording(unfilteredCmdLine.getString("userrecording"));
	}
#endif
	else if (AppGlobal::isInLevelEditorMode())
	{
		// Behaving as a user level editor: switch start info to "user level mode"
		startInfo.setToUserLevelMode();
		
#if !defined(TT_BUILD_FINAL)
		const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
		if (cmdLine.exists("mission"))
		{
			startInfo.initMission(cmdLine.getString("mission"));
		}
		else if (cmdLine.exists("level"))
		{
			// Load the user level that was specified on the command line
			startInfo.setUserLevelName(cmdLine.getString("level"));
			if (tt::fs::fileExists(startInfo.getLevelFilePath()) == false)
			{
				TT_PANIC("User level '%s' (from command line) doesn't exist. Using empty level.\nFile path: %s",
				         startInfo.getLevelName().c_str(), startInfo.getLevelFilePath().c_str());
				startInfo.setUserLevelName("");
			}
		}
		else
#endif
		{
			// Start without a level (there is no sensible default level that we can load)
			startInfo.setUserLevelName("");
		}
	}
	else
	{
#if !defined(TT_BUILD_FINAL)
		const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
		if (cmdLine.exists("mission"))
		{
			// Init a mission
			startInfo.initMission(cmdLine.getString("mission"));
		}
		else if (cmdLine.exists("level"))
		{
			// Load a built-in level (only level name specified)
			startInfo.setLevel(cmdLine.getString("level"));
		}
#endif
		
		if (mountOk && AppGlobal::shouldRestoreShutdownData())
		{
			u32 dataVersion = 0;
			serialization::loadShutdownState(&dataVersion, loadResult == serialization::PersistentDataLoadResult_SuccessWithDemoFile);
			
#if defined(TT_BUILD_FINAL) && defined(TT_STEAM_BUILD)
			if (dataVersion > 0 && dataVersion < 83) // Detect shutdown state from Steam beta
			{
				// Reject the presistent data.
				// Remove all presistent data changes.
				
				startInfo.resetToDefaultLevel();
				AppGlobal::setRestoreFailureLevelName("");
				
				for (s32 progType = 0; progType < ProgressType_Count; ++progType)
				{
					ProgressType type = static_cast<ProgressType>(progType);
					game::script::getRegistry(type).clear();
					game::script::getRegistry(type).clearPersistent();
					
					AppGlobal::getCheckPointMgr(type).resetAllCheckPoints();
				}
				
				loadedPersistentData = false;
			}
#endif
		}
	}
	
	AppGlobal::setGameStartInfo(startInfo);
	
	// Let loadscreen graphics know which level we're starting.
	if (AppGlobal::isInLevelEditorMode() == false)
	{
		// FIXME: When starting in level editor mode, use a neutral graphic, since there won't be a graphic for user levels
		AppGlobal::getSharedGraphics().setLevelLoadTexture(loadedPersistentData == false,
		                                                   startInfo.getLevelName());
	}
	
	if (mountOk)
	{
		savedata::unmountSaveVolume();
	}
	
#if !defined(TT_BUILD_FINAL)
	g_performedGameLoadStep = true;
#endif
}


bool LoadStateGame::isDone() const
{
#if !defined(TT_BUILD_FINAL)
	const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
	if (cmdLine.exists("show_loading_screen"))
	{
		return false;
	}
#endif
	// This state only has one step: done right away
	return true;
}

// Namespace end
}
}
}
