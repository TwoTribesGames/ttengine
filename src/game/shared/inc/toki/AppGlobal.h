#if !defined(INC_TOKI_APPGLOBAL_H)
#define INC_TOKI_APPGLOBAL_H


#include <tt/engine/renderer/fwd.h>
#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/input/ControllerIndex.h>
#include <tt/pres/fwd.h>

#include <toki/game/entity/EntityLibrary.h>
#include <toki/game/movement/fwd.h>
#include <toki/game/script/fwd.h>
#include <toki/game/DemoMgr.h>
#include <toki/game/ShutdownDataMgr.h>
#include <toki/game/fwd.h>
#include <toki/game/StartInfo.h>
#include <toki/level/skin/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/MetaDataGenerator.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/loc/fwd.h>
#include <toki/input/Controller.h>
#include <toki/input/fwd.h>
#include <toki/constants.h>
#include <toki/ScreenshotSettings.h>
#include <toki/SharedGraphics.h>


namespace toki {

class AppGlobal
{
public:
	static inline void setGameStartInfo(const game::StartInfo& p_info) { ms_startInfo = p_info; }
	static inline const game::StartInfo& getGameStartInfo()            { return ms_startInfo;   }
	
	// Controller/input specific methods
	static inline input::Controller& getController(tt::input::ControllerIndex p_index) { TT_ASSERT(p_index < static_cast<int>(SupportedControllerCount)); return ms_controllers[p_index]; }
	
	// Recorder
	static inline input::RecorderPtr getInputRecorder() { return ms_inputRecorder; }
	static void createInputRecorder();
	static void destroyInputRecorder();
	
	static inline const DebugRenderMask& getDebugRenderMask()    { return ms_debugRenderMask; }
	static inline       DebugRenderMask& modifyDebugRenderMask() { return ms_debugRenderMask; }
	
	/*! \note AppGlobal does not take ownership of this object! */
	static inline void        setGame(game::Game* p_game) { ms_game = p_game;                        }
	static inline game::Game* getGame()                   { TT_NULL_ASSERT(ms_game); return ms_game; }
	static inline bool        hasGame()                   { return ms_game != 0;                     }
	
	static bool hasGameAndEntityMgr();
	
	static loc::Loc& getLoc();
	
	static void setCheckPointMgr(const game::CheckPointMgrPtr& p_checkpointMgr, ProgressType p_progressType);
	static game::CheckPointMgr& getCheckPointMgr(ProgressType p_progressType = getCurrentProgressType());
	
	static ProgressType getCurrentProgressType();
	
	static void setNextLevelOverrideProgressType(ProgressType p_progressType) { ms_nextLevelProgressTypeOverride = p_progressType; }
	static ProgressType getNextLevelOverrideProgressType() { return ms_nextLevelProgressTypeOverride; }
	
	static inline game::ShutdownDataMgr& getShutdownDataMgr() { return ms_shutdownDataMgr; }
	
	static game::DemoMgr& getDemoMgr();
	
	static inline const char* const getShutdownStateFilename (bool p_demoMode) { return p_demoMode ?  "shutdown_state_demo.ttsds" :  "shutdown_state.ttsds"; }
	static inline const char* const getPersistentDataFilename(bool p_demoMode) { return p_demoMode ? "persistent_data_demo.ttdat" : "persistent_data.ttdat"; }
	
	static inline game::script::EntityScriptMgr& getEntityScriptMgr() { return ms_entityScriptMgr; }
	static inline game::entity::EntityLibrary&   getEntityLibrary()   { return ms_entityLibrary;   }
	
	static inline level::MetaDataGenerator&      getMetaDataGenerator  () { return ms_metaDataGenerator;   }
	static inline level::TileRegistrationMgrPtr  getTileRegistrationMgr() { return ms_tileRegistrationMgr; }
	
	static inline void createTileRegistrationMgr() { ms_tileRegistrationMgr.reset(new level::TileRegistrationMgr()); }
	
	static void clearPrecache();
	static void loadPrecache();  // Loads the precache in one go, blocking until complete
	
#if !defined(TT_BUILD_FINAL)
	static s32  getPrecacheTextureMemUsage()      { return ms_texturePrecacheMemUsage;      }
	static s32  getPrecachePresentationMemUsage() { return ms_presentationPrecacheMemUsage; }
#endif
	
	static void addTexturesToPrecache(const tt::engine::renderer::TextureContainer& p_textures);
	static void addPresentationToPrecache(const std::string& p_filename);
	static void addMovementSetToPrecache(const game::movement::MovementSetPtr& p_movementSet);
	
	static void updateShoeboxTextures(         const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_data,
	                                           const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_skinData,
	                                           const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_scriptDataOne,
	                                           const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_scriptDataTwo,
	                                           const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_levelBackgroundData);
	static void updateLightmaskShoeboxTextures(const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_data);
	static void clearShoeboxTextures();
	static bool shouldStoreShoeboxTexturesForThisLevel();
	
	/*! \brief When creating a level these features help to test it in game.
	           (e.g. Allow teleport 'I', entity dragging 'ctrl+mouse', etc.) */
	static bool allowLevelCreatorDebugFeaturesInGame(); 
	
	/*! \brief Do we allow any of the features which need to open the editor?
	           (e.g. Allow 'tab', 'ctrl+O', etc.) */
	static bool allowEditorFeatures();
	
	// Based on command line arguments:
	static void parseCommandLineFlags();
	static bool isInDeveloperMode();
	static bool isInDemoMode();
	static bool isInLevelEditorMode();
	static bool isAudioInSilentMode();
	static bool isMusicEnabled();
	static bool isRumbleEnabled();
	static void setShowFps(bool p_enabled); // if commandline flag is passed, it will always show
	static bool getShowFps();
	static bool shouldShowFps();
	static bool shouldDoPrecache();
	static bool shouldDoAllPresentationPrecache();
	static bool shouldRestoreShutdownData();
	static bool shouldDoGpuCheck();
#if !defined(TT_BUILD_FINAL)
	static bool shouldCompileSquirrel();
#endif
	
	static void loadScriptLists();
	static void clearScriptLists();
	
	static inline const tt::str::Strings& getPresentationNames()   { return ms_presentationNames;    }
	static inline const tt::str::Strings& getLevelNames()          { return ms_levelNames;           }
	static bool findInLevelNames(const std::string& p_name);
	static void addLevelToLevelNames(const std::string& p_name);
	static inline const tt::str::Strings& getParticleEffectNames() { return ms_particleEffectsNames; }
	static inline const tt::str::Strings& getEntityNames()         { return ms_entityNames;          }
	static inline const tt::str::Strings& getColorGradingNames()   { return ms_colorGradingNames;    }
	static inline const tt::str::Strings& getLightNames()          { return ms_lightNames;           }
	static inline const tt::str::Strings& getShoeboxIncludeNames() { return ms_shoeboxIncludeNames;  }
	
	static inline bool isBusyLoading()             { return ms_busyLoading;   }
	static inline void setBusyLoading(bool p_busy) { ms_busyLoading = p_busy; }
	
	static inline u32  getUpdateFrameCount()       { return ms_updateFrameCount; }
	static inline void incrementUpdateFrameCount() { ++ms_updateFrameCount;      }
	
	static inline void toggleFrameCounter() { ms_frameCounterAllowed = !ms_frameCounterAllowed; }
	static inline bool isFrameCounterAllowed() { return ms_frameCounterAllowed; }
	
	static void setDoubleUpdateMode(bool p_active);
	static bool hasDoubleUpdateMode();
	static s32  getTargetFPS();
	
	static void setFixedDeltaTimeScale(real p_scale);
	static inline real getFixedDeltaTimeScale()    { return ms_fixedDeltaTimeScale; }
	
	static real getFixedDeltaTime();
	
	static inline bool isBadPerformanceDetected()  { return ms_badPerformanceDetected; }
	static inline void setBadPerformanceDetected() { ms_badPerformanceDetected = true; }
	
	static inline void            createSharedGraphics()  { ms_sharedGraphics.createGraphics();  }
	static inline void            destroySharedGraphics() { ms_sharedGraphics.destroyGraphics(); }
	static inline SharedGraphics& getSharedGraphics()     { return ms_sharedGraphics;            }
	
	static void createSkinConfigs();
	static void destroySkinConfigs();
	static inline const level::skin::SkinConfigPtr& getSkinConfig(level::skin::SkinConfigType p_type)
	{
		TT_ASSERT(level::skin::isValidSkinConfigType(p_type));
		TT_NULL_ASSERT(ms_skinConfig[p_type]);
		return ms_skinConfig[p_type];
	}
	
	static inline u32 getLoadTimeApp() { return ms_loadTimeApp; }
	static inline void setLoadTimeApp(u32 p_loadTime) { ms_loadTimeApp = p_loadTime; }
	static inline u32 getLoadTimeLevel() { return ms_loadTimeLevel; }
	static inline void setLoadTimeLevel(u32 p_loadTime) { ms_loadTimeLevel = p_loadTime; }
	
	static void updateAppTime(real p_deltaTime) { ms_appTime += p_deltaTime; }
	static inline real64 getAppTime() { return ms_appTime; }
	
	static inline const std::string& getRestoreFailureLevelName() { return ms_restoreFailureLevelName; }
	static inline void setRestoreFailureLevelName(const std::string& p_levelName)
	{
		ms_restoreFailureLevelName = p_levelName;
	}
	
	static inline bool shouldStartupFailSafeLevel() { return ms_shouldStartupFailSafeLevel; }
	static inline void setShouldStartupFailSafeLevel(bool p_shouldStartupFailSafeLevel) 
	{
		ms_shouldStartupFailSafeLevel = p_shouldStartupFailSafeLevel;
	}

	static inline bool shouldTakeLevelScreenshot() { return ms_screenshotSettings.type != ScreenshotType_None; }
	static inline void takeLevelScreenshot(const ScreenshotSettings& p_settings) { ms_screenshotSettings = p_settings; }
	static inline void stopLevelScreenshot() { ms_screenshotSettings = ScreenshotSettings(); }
	static inline const ScreenshotSettings& getLevelScreenshotSettings() { return ms_screenshotSettings; }
	
	static void resetDemo();
	
private:
	enum { SupportedControllerCount = 1 };
	
	typedef std::set<tt::engine::renderer::TexturePtr> TextureSet;
	typedef std::set<tt::pres::PresentationObjectPtr > PresentationSet;
	typedef std::set<game::movement::MovementSetPtr  > MovementSets;
	
	static void loadFilenames(const std::string& p_rootPath, const std::string& p_path, 
	                          const std::string& p_extension, bool p_recursive, bool p_addPath,
	                          tt::str::Strings* p_result_OUT);
	static void loadFilenamesSorted(const std::string& p_path, const std::string& p_extension,
	                                bool p_recursive, bool p_addPath, tt::str::Strings* p_result_OUT);
	static void loadTextureNamesSorted(const std::string& p_path, bool p_recursive, 
	                                   tt::str::Strings* p_result_OUT, const std::string& p_assetExt = ".volt");
	
	// No instantiation
	AppGlobal();
	AppGlobal(const AppGlobal&);
	~AppGlobal();
	AppGlobal& operator=(const AppGlobal&);
	
	static real                   ms_fixedDeltaTimeScale;
	
	static game::StartInfo        ms_startInfo;
	static ProgressType           ms_nextLevelProgressTypeOverride;
	static input::Controller      ms_controllers[SupportedControllerCount];
	static input::RecorderPtr     ms_inputRecorder;
	static game::Game*            ms_game;
	static game::CheckPointMgrPtr ms_checkPointMgr[ProgressType_Count];
	static game::ShutdownDataMgr  ms_shutdownDataMgr;
	static DebugRenderMask        ms_debugRenderMask;
	
	static game::script::EntityScriptMgr ms_entityScriptMgr;
	static game::entity::EntityLibrary   ms_entityLibrary;
	
	static level::MetaDataGenerator      ms_metaDataGenerator;
	static level::TileRegistrationMgrPtr ms_tileRegistrationMgr;
	
	static TextureSet                   ms_texturePrecache;
	static tt::pres::PresentationMgrPtr ms_presentationMgr;
	static PresentationSet              ms_presentationPrecache;
	static MovementSets                 ms_movementSetPrecache;
#if !defined(TT_BUILD_FINAL)
	static s32             ms_texturePrecacheMemUsage;
	static TextureSet      ms_presentationPrecacheTextures;
	static s32             ms_presentationPrecacheMemUsage;
#endif
	
	static tt::engine::renderer::EngineIDToTextures ms_shoeboxTextures;
	static tt::engine::renderer::EngineIDToTextures ms_lightmaskShoeboxTextures;
	
	static tt::str::Strings ms_presentationNames;
	static tt::str::Strings ms_levelNames;
	static tt::str::Strings ms_particleEffectsNames;
	static tt::str::Strings ms_entityNames;
	static tt::str::Strings ms_colorGradingNames;
	static tt::str::Strings ms_lightNames;
	static tt::str::Strings ms_shoeboxIncludeNames;
	
	static bool            ms_busyLoading;
	static bool            ms_doubleUpdateMode;
	static bool            ms_badPerformanceDetected;
	
	static SharedGraphics             ms_sharedGraphics;
	
	static level::skin::SkinConfigPtr ms_skinConfig[level::skin::SkinConfigType_Count];
	
	static u32 ms_updateFrameCount;  // for debugging: counter for update() frames (continually incrementing)
	static real64 ms_appTime;
	
	static bool ms_showFPS;
	static bool ms_frameCounterAllowed;
	
	static std::string ms_restoreFailureLevelName;
	
	static u32 ms_loadTimeApp;
	static u32 ms_loadTimeLevel;
	
	static bool ms_shouldStartupFailSafeLevel;
	static ScreenshotSettings ms_screenshotSettings;
	
	static game::DemoMgr ms_demoMgr;
};

// Namespace end
}


#endif  // !defined(INC_TOKI_APPGLOBAL_H)
