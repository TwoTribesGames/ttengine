#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/code/BitMask.h>
#include <tt/code/helpers.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/pres/PresentationCache.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>

#include <toki/game/CheckPointMgr.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/Game.h>
#include <toki/input/Recorder.h>
#include <toki/level/skin/SkinConfig.h>
#include <toki/loc/Loc.h>
#include <toki/main/loadstate/LoadStatePrecache.h>
#include <toki/main/loadstate/LoadStateShoeboxPrecache.h>
#include <toki/pres/TriggerFactory.h>
#include <toki/script/ScriptMgr.h>
#include <toki/serialization/utils.h>
#include <toki/utils/utils.h>
#include <toki/AppGlobal.h>

#if defined(TT_PLATFORM_WIN)
#pragma warning(disable : 4592)
#endif

namespace toki {

enum CmdLineFlag
{
	CmdLineFlag_DeveloperMode,
	CmdLineFlag_DemoMode,
	CmdLineFlag_LevelEditorMode,
	CmdLineFlag_AudioSilentMode,  // no audio at all (no sfx or music)
	CmdLineFlag_NoMusic,          // just no music
	CmdLineFlag_NoRumble,         // no rumble
	CmdLineFlag_DisablePrecache,
	CmdLineFlag_PrecacheAllPresentation, // Load all presentation files (DEBUG ONLY!)
	CmdLineFlag_DisableShutdownRestore,
	CmdLineFlag_ShowFps,
	CmdLineFlag_SkipGPUCheck,
	
#if !defined(TT_BUILD_FINAL)
	CmdLineFlag_CompileSquirrel,
#endif
	
	CmdLineFlag_Count,
	CmdLineFlag_Invalid
};
typedef tt::code::BitMask<CmdLineFlag, CmdLineFlag_Count> CmdLineFlags;


inline const char* const getCmdLineFlagName(CmdLineFlag p_flag)
{
	switch (p_flag)
	{
	case CmdLineFlag_DeveloperMode:           return "developer";
	case CmdLineFlag_DemoMode:                return "demo";
	case CmdLineFlag_LevelEditorMode:         return "editor";
	case CmdLineFlag_AudioSilentMode:         return "silent";
	case CmdLineFlag_NoMusic:                 return "no_music";
	case CmdLineFlag_NoRumble:                return "no_rumble";
	case CmdLineFlag_DisablePrecache:         return "no_precache";
	case CmdLineFlag_PrecacheAllPresentation: return "precache_all_presentation";
	case CmdLineFlag_DisableShutdownRestore:  return "no_shutdown_restore";
	case CmdLineFlag_ShowFps:                 return "show_fps";
	case CmdLineFlag_SkipGPUCheck:            return "no_gpu_check";
#if !defined(TT_BUILD_FINAL)
	case CmdLineFlag_CompileSquirrel:         return "compile_squirrel";
#endif
		
	default:
		TT_PANIC("Invalid CmdLineFlag: %d", p_flag);
		return "";
	}
}


inline bool isAvailableInFinalBuilds(CmdLineFlag p_flag)
{
	return p_flag == CmdLineFlag_LevelEditorMode ||
	       p_flag == CmdLineFlag_ShowFps         ||
	       p_flag == CmdLineFlag_SkipGPUCheck;
}


static CmdLineFlags g_cmdLineFlags; // Flags that were set based on command line arguments

real                           AppGlobal::ms_fixedDeltaTimeScale = 1.0f;
game::StartInfo                AppGlobal::ms_startInfo;
ProgressType                   AppGlobal::ms_nextLevelProgressTypeOverride = ProgressType_Invalid;
input::Controller              AppGlobal::ms_controllers[SupportedControllerCount];
input::RecorderPtr             AppGlobal::ms_inputRecorder;
game::Game*                    AppGlobal::ms_game = 0;
game::CheckPointMgrPtr         AppGlobal::ms_checkPointMgr[ProgressType_Count];
game::ShutdownDataMgr          AppGlobal::ms_shutdownDataMgr;
game::DemoMgr                  AppGlobal::ms_demoMgr;
DebugRenderMask                AppGlobal::ms_debugRenderMask;
game::script::EntityScriptMgr  AppGlobal::ms_entityScriptMgr;
game::entity::EntityLibrary    AppGlobal::ms_entityLibrary;
level::MetaDataGenerator       AppGlobal::ms_metaDataGenerator;
level::TileRegistrationMgrPtr  AppGlobal::ms_tileRegistrationMgr;
AppGlobal::TextureSet          AppGlobal::ms_texturePrecache;
tt::pres::PresentationMgrPtr   AppGlobal::ms_presentationMgr;
AppGlobal::PresentationSet     AppGlobal::ms_presentationPrecache;
AppGlobal::MovementSets        AppGlobal::ms_movementSetPrecache;
#if !defined(TT_BUILD_FINAL)
s32                            AppGlobal::ms_texturePrecacheMemUsage      = 0;
AppGlobal::TextureSet          AppGlobal::ms_presentationPrecacheTextures;
s32                            AppGlobal::ms_presentationPrecacheMemUsage = 0;
#endif
tt::engine::renderer::EngineIDToTextures AppGlobal::ms_shoeboxTextures;
tt::engine::renderer::EngineIDToTextures AppGlobal::ms_lightmaskShoeboxTextures;
tt::str::Strings               AppGlobal::ms_presentationNames;
tt::str::Strings               AppGlobal::ms_levelNames;
tt::str::Strings               AppGlobal::ms_particleEffectsNames;
tt::str::Strings               AppGlobal::ms_entityNames;
tt::str::Strings               AppGlobal::ms_colorGradingNames;
tt::str::Strings               AppGlobal::ms_lightNames;
tt::str::Strings               AppGlobal::ms_shoeboxIncludeNames;
bool                           AppGlobal::ms_busyLoading = false;
bool                           AppGlobal::ms_doubleUpdateMode = false;
bool                           AppGlobal::ms_badPerformanceDetected = false;
SharedGraphics                 AppGlobal::ms_sharedGraphics;
level::skin::SkinConfigPtr     AppGlobal::ms_skinConfig[level::skin::SkinConfigType_Count];
u32                            AppGlobal::ms_updateFrameCount = 0;
real64                         AppGlobal::ms_appTime = 0.0f;
bool                           AppGlobal::ms_showFPS = false;
bool                           AppGlobal::ms_frameCounterAllowed = true;
u32                            AppGlobal::ms_loadTimeLevel = 0;
u32                            AppGlobal::ms_loadTimeApp   = 0;
std::string                    AppGlobal::ms_restoreFailureLevelName;
bool                           AppGlobal::ms_shouldStartupFailSafeLevel = false;
ScreenshotSettings             AppGlobal::ms_screenshotSettings;


//--------------------------------------------------------------------------------------------------
// Public member functions

void AppGlobal::createInputRecorder()
{
	ms_inputRecorder.reset(new input::Recorder);
}


void AppGlobal::destroyInputRecorder()
{
	ms_inputRecorder.reset();
}


bool AppGlobal::hasGameAndEntityMgr()
{
	return hasGame() && getGame()->hasEntityMgr();
}


loc::Loc& AppGlobal::getLoc()
{
	static loc::Loc locInstance;
	return locInstance;
}


void AppGlobal::setCheckPointMgr(const game::CheckPointMgrPtr& p_checkpointMgr, ProgressType p_progressType)
{
	if (isValidProgressType(p_progressType) == false)
	{
		TT_ASSERT(isValidProgressType(p_progressType));
		p_progressType = ProgressType_Main;
	}
	ms_checkPointMgr[p_progressType] = p_checkpointMgr;
}


game::CheckPointMgr& AppGlobal::getCheckPointMgr(ProgressType p_progressType)
{
	if (isValidProgressType(p_progressType) == false)
	{
		TT_ASSERT(isValidProgressType(p_progressType));
		p_progressType = ProgressType_Main;
	}
	TT_NULL_ASSERT(ms_checkPointMgr[p_progressType]);
	return *ms_checkPointMgr[p_progressType];
}


ProgressType AppGlobal::getCurrentProgressType()
{
	if (hasGame() && isValidProgressType(getGame()->getProgressOverride()))
	{
		return getGame()->getProgressOverride();
	}
	
	const game::StartInfo& startInfo = (hasGame()) ? getGame()->getStartInfo() : ms_startInfo;
	
	switch (startInfo.getType())
	{
	case game::StartInfo::Type_NormalLevel: return ProgressType_Main;
	case game::StartInfo::Type_UserLevel:   return ProgressType_UserLevel;
	case game::StartInfo::Type_UserRecording:
		TT_PANIC("Can't decide progress type when startinfo is Type_UserRecording, it first needs to switch to the actual level!");
		return ProgressType_Main;
	default:
		TT_PANIC("Unknown StartInfo Type: %d. (hasGame(): %d)\n", startInfo.getType(), hasGame());
		return ProgressType_Main;
	}
}


game::DemoMgr& AppGlobal::getDemoMgr()
{
	return ms_demoMgr;
}


void AppGlobal::clearPrecache()
{
	// FIXME: ms_texturePrecache isn't required anymore since TextureCache also keeps the textures
	tt::code::helpers::freeContainer(ms_texturePrecache);
	tt::code::helpers::freeContainer(ms_presentationPrecache);
	tt::code::helpers::freeContainer(ms_movementSetPrecache);
	
	tt::engine::renderer::TextureCache::clear();
	
#if !defined(TT_BUILD_FINAL)
	ms_texturePrecacheMemUsage      = 0;
	tt::code::helpers::freeContainer(ms_presentationPrecacheTextures);
	ms_presentationPrecacheMemUsage = 0;
#endif
}


void AppGlobal::loadPrecache()
{
	if (shouldDoPrecache() == false)
	{
		return;
	}
	
	main::loadstate::LoadStatePtr loadState = main::loadstate::LoadStatePrecache::create();
	while (loadState->isDone() == false)
	{
		loadState->doLoadStep();
	}
	
	loadState = main::loadstate::LoadStateShoeboxPrecache::create();
	while (loadState->isDone() == false)
	{
		loadState->doLoadStep();
	}
}


bool AppGlobal::allowLevelCreatorDebugFeaturesInGame()
{
	const bool inPlayTestMode = (hasGame()) ? getGame()->isDoingPlayTest() : false;
	return (getInputRecorder()->isActive() == false) && 
		(isInDeveloperMode() || (isInLevelEditorMode() && inPlayTestMode == false));
}


bool AppGlobal::allowEditorFeatures()
{
	const game::StartInfo& startInfo = (hasGame()) ? getGame()->getStartInfo() : ms_startInfo;
	return isInDeveloperMode() || (isInLevelEditorMode() && startInfo.isUserLevel());
}


void AppGlobal::parseCommandLineFlags()
{
	g_cmdLineFlags.resetAllFlags();
	
#if defined(TT_BUILD_FINAL)
	const bool finalBuild = true;
#else
	const bool finalBuild = false;
#endif
	
	const tt::args::CmdLine unfilteredCmdLine(tt::args::CmdLine::getApplicationCmdLine());
	
	for (s32 i = 0; i < CmdLineFlag_Count; ++i)
	{
		const CmdLineFlag flag = static_cast<CmdLineFlag>(i);
		
		if ((finalBuild == false || isAvailableInFinalBuilds(flag)) &&
		    unfilteredCmdLine.exists(getCmdLineFlagName(flag)))
		{
			g_cmdLineFlags.setFlag(flag);
		}
	}
	
#if !defined(TT_BUILD_FINAL)
	if (g_cmdLineFlags.checkFlag(CmdLineFlag_CompileSquirrel))
	{
		tt::platform::error::turnHeadlessModeOn();
		
		g_cmdLineFlags.setFlag(CmdLineFlag_AudioSilentMode);
		g_cmdLineFlags.setFlag(CmdLineFlag_DisablePrecache);
		g_cmdLineFlags.setFlag(CmdLineFlag_DisableShutdownRestore);
	}
	
	
	//if (unfilteredCmdLine.exists("mission") || unfilteredCmdLine.exists("level"))
	//{
	//	g_cmdLineFlags.setFlag(CmdLineFlag_DisableShutdownRestore);
	//}
#endif
	
#if TT_DEMO_BUILD
	g_cmdLineFlags.resetFlag(CmdLineFlag_LevelEditorMode);
#endif
}


bool AppGlobal::isInDeveloperMode()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_DeveloperMode);
}


bool AppGlobal::isInDemoMode()
{
#if TT_DEMO_BUILD
	return true;
#else
	return g_cmdLineFlags.checkFlag(CmdLineFlag_DemoMode);
#endif
}


bool AppGlobal::isInLevelEditorMode()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_LevelEditorMode);
}


bool AppGlobal::isAudioInSilentMode()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_AudioSilentMode);
}


bool AppGlobal::isMusicEnabled()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_NoMusic) == false;
}


bool AppGlobal::isRumbleEnabled()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_NoRumble) == false;
}


void AppGlobal::setShowFps(bool p_enabled)
{
	ms_showFPS = p_enabled;
}


bool AppGlobal::getShowFps()
{
	return ms_showFPS;
}


bool AppGlobal::shouldShowFps()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_ShowFps) || ms_showFPS;
}


bool AppGlobal::shouldDoPrecache()
{
	return ( shouldDoAllPresentationPrecache() || g_cmdLineFlags.checkFlag(CmdLineFlag_DisablePrecache) == false)
	       &&
	       isInLevelEditorMode() == false;  // never precache when starting as a level editor
}


bool AppGlobal::shouldDoAllPresentationPrecache()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_PrecacheAllPresentation);
}


bool AppGlobal::shouldRestoreShutdownData()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_DisableShutdownRestore) == false;
}


bool AppGlobal::shouldDoGpuCheck()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_SkipGPUCheck) == false;
}


#if !defined(TT_BUILD_FINAL)
bool AppGlobal::shouldCompileSquirrel()
{
	return g_cmdLineFlags.checkFlag(CmdLineFlag_CompileSquirrel);
}
#endif


void AppGlobal::addTexturesToPrecache(const tt::engine::renderer::TextureContainer& p_textures)
{
#if !defined(TT_BUILD_FINAL)
	// Add the textures individually so we know if it is a new texture. (if so add mem size to total.)
	for (tt::engine::renderer::TextureContainer::const_iterator it = p_textures.begin();
	     it != p_textures.end(); ++it)
	{
		const tt::engine::renderer::TexturePtr& texture = (*it);
		std::pair<TextureSet::iterator, bool> result = ms_texturePrecache.insert(texture);
		if (result.second && texture != 0)
		{
			ms_texturePrecacheMemUsage += texture->getMemSize();
		}
	}
#	if defined(TT_BUILD_DEV) && 0 // Extra check to see if above code works correct
	s32 totalSize = 0;
	for (TextureSet::const_iterator it = ms_texturePrecache.begin(); it != ms_texturePrecache.end(); ++it)
	{
		totalSize += (*it)->getMemSize();
	}
	TT_ASSERT(ms_texturePrecacheMemUsage == totalSize);
#	endif
#else
	ms_texturePrecache.insert(p_textures.begin(), p_textures.end());
#endif
}


void AppGlobal::addPresentationToPrecache(const std::string& p_filename)
{
	if (ms_presentationMgr == nullptr)
	{
		ms_presentationMgr = tt::pres::PresentationMgr::create(
			tt::pres::Tags(),
			tt::pres::TriggerFactoryInterfacePtr(new pres::TriggerFactory));
		
		ms_presentationMgr->setUsedForPrecache(true);
		
		// Map particle layer names (used in presentation files) to render group numbers
		// (which will be rendered by the game)
		// FIXME: This is code duplication from Game::createPresentationMgr!
		for (s32 i = 0; i < game::ParticleLayer_Count; ++i)
		{
			const game::ParticleLayer particleLayer = static_cast<game::ParticleLayer>(i);
			ms_presentationMgr->addParticleLayerMapping(game::getParticleLayerName(particleLayer),
			                                           game::getParticleLayerRenderGroup(particleLayer));
		}
	}
	
	tt::pres::PresentationObjectPtr presentation(tt::pres::PresentationCache::get(p_filename, tt::pres::Tags(), ms_presentationMgr.get()));
	if (presentation == nullptr)
	{
		return;
	}
	ms_presentationPrecache.insert(presentation);
	
#if !defined(TT_BUILD_FINAL)
	tt::engine::renderer::TextureContainer textures = presentation->getAndLoadAllUsedTextures();
	
	for (tt::engine::renderer::TextureContainer::const_iterator it = textures.begin();
	     it != textures.end(); ++it)
	{
		const tt::engine::renderer::TexturePtr& texture = (*it);
		std::pair<TextureSet::iterator, bool> result = ms_presentationPrecacheTextures.insert(texture);
		if (result.second)
		{
			ms_presentationPrecacheMemUsage += texture->getMemSize();
		}
	}
	
#	if defined(TT_BUILD_DEV) && 0 // Extra check to see if above code works correct
	TextureSet allPresTextures;
	for (PresentationSet::const_iterator it = ms_presentationPrecache.begin();
	     it != ms_presentationPrecache.end(); ++it)
	{
		tt::engine::renderer::TextureContainer textures = (*it)->getAndLoadAllUsedTextures();
		allPresTextures.insert(textures.begin(), textures.end());
	}
	
	s32 totalSize = 0;
	for (TextureSet::const_iterator it = allPresTextures.begin(); it != allPresTextures.end(); ++it)
	{
		totalSize += (*it)->getMemSize();
	}
	TT_ASSERT(ms_presentationPrecacheMemUsage == totalSize);
#	endif
#endif
}


void AppGlobal::addMovementSetToPrecache(const game::movement::MovementSetPtr& p_movementSet)
{
	ms_movementSetPrecache.insert(p_movementSet);
}


void AppGlobal::updateShoeboxTextures(const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_data,
                                      const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_skinData,
                                      const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_scriptDataOne,
                                      const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_scriptDataTwo,
                                      const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_levelBackgroundData)
{
	if (shouldStoreShoeboxTexturesForThisLevel() == false)
	{
		return;
	}
	using namespace tt::engine::scene2d::shoebox;
	
	// Get engineIDs of textures which will be used.
	EngineIDValues engineIds;
	getAllUsedEngineIDs(p_data               , engineIds, true);
	getAllUsedEngineIDs(p_skinData           , engineIds, true);
	getAllUsedEngineIDs(p_scriptDataOne      , engineIds, true);
	getAllUsedEngineIDs(p_scriptDataTwo      , engineIds, true);
	getAllUsedEngineIDs(p_levelBackgroundData, engineIds, true);
	
	// Get all textures which match engine ids
	using tt::engine::renderer::EngineIDToTextures;
	EngineIDToTextures remainingTextures = getTexturesWithID(ms_shoeboxTextures, engineIds);
	
	ms_shoeboxTextures.clear(); // Drop all other textures
	
	// Load new textures
	getAllUsedTextures(p_data               , ms_shoeboxTextures, true);
	getAllUsedTextures(p_skinData           , ms_shoeboxTextures, true);
	getAllUsedTextures(p_scriptDataOne      , ms_shoeboxTextures, true);
	getAllUsedTextures(p_scriptDataTwo      , ms_shoeboxTextures, true);
	getAllUsedTextures(p_levelBackgroundData, ms_shoeboxTextures, true);
}


void AppGlobal::updateLightmaskShoeboxTextures(const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_data)
{
	if (shouldStoreShoeboxTexturesForThisLevel() == false)
	{
		return;
	}
	using namespace tt::engine::scene2d::shoebox;
	
	// Get engineIDs of textures which will be used.
	EngineIDValues engineIds;
	getAllUsedEngineIDs(p_data, engineIds, true);
	
	// Get all textures which match engine ids
	using tt::engine::renderer::EngineIDToTextures;
	EngineIDToTextures remainingTextures = getTexturesWithID(ms_lightmaskShoeboxTextures, engineIds);
	
	ms_lightmaskShoeboxTextures.clear(); // Drop all other textures
	
	// Load new textures
	getAllUsedTextures(p_data, ms_lightmaskShoeboxTextures, true);
}


void AppGlobal::clearShoeboxTextures()
{
	ms_shoeboxTextures.clear();
	ms_lightmaskShoeboxTextures.clear();
}


bool AppGlobal::shouldStoreShoeboxTexturesForThisLevel()
{
	// Do nothing for menu levels.
	return ms_game == 0 ||
	       tt::str::startsWith(ms_game->getStartInfo().getLevelName(), "menu_") == false;
}


void AppGlobal::loadScriptLists()
{
	loadFilenamesSorted("presentation/"           , "pres"   , ms_startInfo.isUserLevel() == false , true , &ms_presentationNames   );
	loadFilenamesSorted("levels/"                 , "ttlvl"  , true                                , true , &ms_levelNames          );
	loadFilenamesSorted("particles/"              , "trigger", true                                , true , &ms_particleEffectsNames);
	loadFilenamesSorted("scripts/entities/"       , "nut"    , true                                , false, &ms_entityNames         );
	loadFilenamesSorted("scripts/entities/"       , "bnut"   , true                                , false, &ms_entityNames         );
	loadFilenamesSorted("levels/shoebox_includes/", "shoebox", true                                , true,  &ms_shoeboxIncludeNames );
	
	loadTextureNamesSorted("color_grading/"  , true, &ms_colorGradingNames);
	loadTextureNamesSorted("textures/lights/", true, &ms_lightNames       , ".img");
	
	// Now that the list of level names is available, verify that the restore failure level actually exists
	{
		const std::string restoreFailureLevel(cfg()->getStringDirect("toki.startup.restore_failure_level"));
		TT_ASSERTMSG(findInLevelNames(restoreFailureLevel),
		             "restore_failure_level in config.xml is set to a level that doesn't exist ('%s')! Fix immediately!",
		             restoreFailureLevel.c_str());
	}
}


void AppGlobal::clearScriptLists()
{
	tt::code::helpers::freeContainer(ms_presentationNames);
	tt::code::helpers::freeContainer(ms_levelNames);
	tt::code::helpers::freeContainer(ms_particleEffectsNames);
	tt::code::helpers::freeContainer(ms_entityNames);
	tt::code::helpers::freeContainer(ms_colorGradingNames);
	tt::code::helpers::freeContainer(ms_lightNames);
	tt::code::helpers::freeContainer(ms_shoeboxIncludeNames);
}


bool AppGlobal::findInLevelNames(const std::string& p_name)
{
	// TODO: We could use a faster find because ms_levelNames is sorted (and unique).
	return std::find(ms_levelNames.begin(), ms_levelNames.end(), p_name) != ms_levelNames.end();
}


void AppGlobal::addLevelToLevelNames(const std::string& p_name)
{
	if (findInLevelNames(p_name) == false)
	{
		ms_levelNames.push_back(p_name);
		std::sort(ms_levelNames.begin(), ms_levelNames.end());
	}
}


void AppGlobal::setDoubleUpdateMode(bool p_active)
{
	tt::app::getApplication()->setTargetFPS( (p_active) ? 30 : 60 );
	
	ms_doubleUpdateMode = p_active;
}


bool AppGlobal::hasDoubleUpdateMode()
{
	return ms_doubleUpdateMode;
}


s32 AppGlobal::getTargetFPS()
{
	if (ms_doubleUpdateMode)
	{
		return 60;
	}
	else
	{
		return tt::app::getApplication()->getTargetFPS();
	}
}


void AppGlobal::setFixedDeltaTimeScale(real p_scale)
{
	if (p_scale < 0.0f)
	{
		TT_PANIC("fixedDeltaTime scale should be >= 0.0");
		return;
	}
	ms_fixedDeltaTimeScale = p_scale;
}


real AppGlobal::getFixedDeltaTime()
{
	const u32 targetFPS = getTargetFPS();
	const real fixedDeltaTime = (targetFPS == 0) ? (1.0f / 60) : (1.0f / targetFPS);
	return fixedDeltaTime * ms_fixedDeltaTimeScale;
}


void AppGlobal::createSkinConfigs()
{
	static const char* const filenames[level::skin::SkinConfigType_Count] =
	{
		"config/skin_config.xml",
	};
	
	for (s32 i = 0; i < level::skin::SkinConfigType_Count; ++i)
	{
		const level::skin::SkinConfigType type = static_cast<level::skin::SkinConfigType>(i);
		
		TT_ASSERT(ms_skinConfig[i] == 0);
		
		if (filenames[i] == 0)
		{
			TT_PANIC("No filename specified for skin config for %s",
			         level::skin::getSkinConfigTypeName(type));
			continue;
		}
		
		ms_skinConfig[i].reset(new level::skin::SkinConfig);
		const bool loadOk = ms_skinConfig[i]->load(filenames[i]);
		TT_ASSERTMSG(loadOk, "Loading skin config for %s failed (filename '%s').",
		             level::skin::getSkinConfigTypeName(type), filenames[i]);
	}
}


void AppGlobal::destroySkinConfigs()
{
	for (s32 i = 0; i < level::skin::SkinConfigType_Count; ++i)
	{
		ms_skinConfig[i].reset();
	}
}


void AppGlobal::resetDemo()
{
	if (isInDemoMode())
	{
		TT_Printf("AppGlobal::resetDemo - RESETTING DEMO.\n");
		
		// Reset persistent data.
		serialization::clearRegistries();
		
		// Reset all checkpoints
		for (s32 progType = 0; progType < ProgressType_Count; ++progType)
		{
			ProgressType type = static_cast<ProgressType>(progType);
			getCheckPointMgr(type).resetAllCheckPoints();
		}
		
		// Reset demo time.
		getDemoMgr().resetCountdown();
	}
	else
	{
		TT_Printf("AppGlobal::resetDemo - Called, but not in demo mode! This does nothing.\n");
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void AppGlobal::loadFilenames(const std::string& p_rootPath, const std::string& p_path,
                              const std::string& p_extension, bool p_recursive, bool p_addPath,
                              tt::str::Strings* p_result_OUT)
{
	TT_NULL_ASSERT(p_result_OUT);
	
	if (tt::fs::dirExists(p_path) == false)
	{
		return;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(p_path));
	if (dir == 0)
	{
		return;
	}
	
	tt::fs::DirEntry entry;
	const size_t rootPathLength = p_rootPath.size();
	while (dir->read(entry))
	{
		if (entry.isDirectory())
		{
			// Only recursively load subdirectories if requested.
			if (p_recursive && entry.getName() != "." && entry.getName() != "..")
			{
				loadFilenames(p_rootPath, p_path + entry.getName() + "/", p_extension, p_recursive, p_addPath, p_result_OUT);
			}
		}
		else if (tt::fs::utils::getExtension(entry.getName()) == p_extension)
		{
			// Strip rootpath
			const std::string path( (p_addPath) ? p_path.substr(rootPathLength) : "" );
			p_result_OUT->push_back(path + tt::fs::utils::getFileTitle(entry.getName()));
		}
	}
}


void AppGlobal::loadFilenamesSorted(const std::string& p_path, const std::string& p_extension,
                                    bool p_recursive, bool p_addPath, tt::str::Strings* p_result_OUT)
{
	TT_NULL_ASSERT(p_result_OUT);
	loadFilenames(p_path, p_path, p_extension, p_recursive, p_addPath, p_result_OUT);
	std::sort(  p_result_OUT->begin(), p_result_OUT->end());
}


void AppGlobal::loadTextureNamesSorted(const std::string& p_path, bool p_recursive, 
                                       tt::str::Strings* p_result_OUT, const std::string& p_assetExt)
{
	TT_NULL_ASSERT(p_result_OUT);
	
	// Get all filenames and get the asset id from it.
	{
		tt::str::Strings filenames;
		loadFilenames(p_path, p_path, "etx", true, p_recursive, &filenames);
		for (tt::str::Strings::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
		{
			const std::string filename = p_path + (*it) + ".etx";
			
			// Open the file
			tt::fs::FilePtr file(tt::fs::open(filename, tt::fs::OpenMode_Read));
			TT_ASSERTMSG(file != 0, "Error opening file '%s'", filename.c_str());
			
			// Get asset id from header.
			tt::engine::file::ResourceHeader header;
			
			if(file->read(&header, sizeof(header)) != sizeof(header))
			{
				TT_PANIC("Failed to read resource header.");
			}
			
			// Validate version
			if (header.checkVersion() == false)
			{
				continue;
			}
			
			// FIXME: We might want to add support for 'normal' textures. That means .img asset ext.
			const std::string assetID(header.name);
			
			if (tt::str::endsWith(assetID, p_assetExt))
			{
				// Add asset ID to result.
				p_result_OUT->push_back(assetID.substr(0, assetID.length() - p_assetExt.length()));
			}
		}
	}
	std::sort(p_result_OUT->begin(), p_result_OUT->end());
}


// Namespace end
}
