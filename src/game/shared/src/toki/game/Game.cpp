#include <iterator>
#include <visualboy/VisualBoy.h>

#include <tt/algorithms/set_helpers.h>
#include <tt/app/Application.h>
#include <tt/app/Platform.h>
#include <tt/args/CmdLine.h>
#include <tt/code/bufferutils.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/engine/scene2d/shoebox/TagMgr.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/input/KeyboardController.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/pres/PresentationCache.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/profiler/PerformanceProfiler.h>
#include <tt/stats/stats.h>
#include <tt/system/Time.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/effects/ColorGrading.h>
#include <toki/game/CheckPointMgr.h>
#include <toki/game/DemoMgr.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/entity/effect/ColorGradingEffectMgr.h>
#include <toki/game/entity/effect/FogEffectMgr.h>
#include <toki/game/entity/graphics/PowerBeamGraphic.h>
#include <toki/game/entity/movementcontroller/DirectionalMovementController.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/event/input/PointerEvent.h>
#include <toki/game/event/Event.h>
#include <toki/game/event/EventMgr.h>
#include <toki/game/event/SoundGraphicsMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/hud/DebugUI.h>
#include <toki/game/hud/WorkshopLevelPicker.h>
#include <toki/game/hud/ResolutionPicker.h>
#include <toki/game/light/DarknessMgr.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/movement/fwd.h>
#include <toki/game/movement/MoveBase.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/script/Registry.h>
#include <toki/game/AttributeDebugView.h>
#include <toki/game/Border.h>
#include <toki/game/DebugView.h>
#include <toki/game/Game.h>
#include <toki/game/Minimap.h>
#include <toki/game/StartInfo.h>
#include <toki/game/types.h>
#include <toki/input/Recorder.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/skin/SkinContext.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/Note.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/level/skin/functions.h>
#include <toki/loc/Loc.h>
#include <toki/pres/PresentationObjectMgr.h>
#include <toki/pres/TriggerFactory.h>
#include <toki/savedata/utils.h>
#include <toki/script/ScriptMgr.h>
#include <toki/statelist/statelist.h>
#include <toki/utils/GlyphSetMgr.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>
#include <toki/cfg.h>
#include <toki/constants.h>


#define USE_SHOEBOX_THREADING



namespace toki {
namespace game {

// NOTE: This script button code is in the cpp file so that the header doesn't need to include EntityBase.h.
//       EntityBase.h is needed for the function pointer typedefs in this struct.
struct ScriptButton
{
	typedef bool (script::EntityBase::*PressedFunc )()     const;
	typedef bool (script::EntityBase::*ReleasedFunc)(real) const;
	
	const tt::input::Button* controllerButton;
	PressedFunc              onPressed;
	ReleasedFunc             onReleased;
	real                     pressDuration;
	
	inline ScriptButton(const tt::input::Button* p_controllerButton             = 0,
	                    PressedFunc              p_onPressed                    = 0,
	                    ReleasedFunc             p_onReleased                   = 0)
	:
	controllerButton            (p_controllerButton),
	onPressed                   (p_onPressed),
	onReleased                  (p_onReleased),
	pressDuration               (0.0f)
	{ }
};
typedef std::vector<ScriptButton> ScriptButtons;


// NOTE: This is a global variable by necessity: the ScriptButton definition
//       would need to be in the header for this variable to be a member of Game
static ScriptButtons g_scriptButtons;

static const real g_audioListenerZDistance = 1.0f;


//--------------------------------------------------------------------------------------------------
// Public member functions

Game::Game()
:
m_cameraMgr(),
m_minimap(new Minimap),
m_startInfo(),
m_progressTypeOverride(ProgressType_Invalid),
m_levelData(),
m_attribDebugView(),
m_debugView(),
m_shadowMaskShoeboxData(),
m_shoeboxDataEnvironment(),
m_shoeboxDataLevelSkin(),
m_shoeboxDataUserLevelBackground(),
m_shoeboxDataFromScript(new tt::engine::scene2d::shoebox::ShoeboxData),
m_shoeboxDataIncludesFromScript(new tt::engine::scene2d::shoebox::ShoeboxData),
m_shadowMaskShoebox(),
m_shadowMaskShoeboxTexMemSize(0),
m_shoeboxSkinAndEnvironment(),
m_shoeboxSkinAndEnvironmentTexMemSize(0),
m_levelOverriddenThemeTiles(),
m_levelBorder(Border::create(level::tileToWorld(4),
                             tt::engine::renderer::ColorRGBA(tt::engine::renderer::ColorRGB::blue, 63))),
m_debugUI(),
#if defined(TT_STEAM_BUILD)
m_workshopLevelPicker(),
#endif
m_resolutionPicker(),
m_editor(),
m_entitiesInLevelDataOnEditorOpen(),
m_levelBackgroundOnEditorOpen(),
m_levelAttributeLayerOnEditorOpen(),
m_entityMgr(),
// Martijn: not needed for RIVE
/*
m_eventMgr(new event::EventMgr),
m_soundGraphicsMgr(new event::SoundGraphicsMgr),
// */
m_tileRegistrationMgr(AppGlobal::getTileRegistrationMgr()),
m_fluidMgr(),
m_lightMgr(),
m_darknessMgr(),
m_stopLightRenderingOnSplit(true),
m_pathMgr(),
m_colorGradingEffectMgr(new entity::effect::ColorGradingEffectMgr),
m_fogEffectMgr(new entity::effect::FogEffectMgr),
m_presentationObjectMgr(),
m_draggingEntity(),
m_draggingEntityRestoreSuspendedState(false),
m_directionalControlsEnabled(true),
m_pressingEntities(),
m_screenSpaceEntities(),
m_buttonInputListeningEntities(),
m_mouseInputListeningEntities(),
m_keyboardListeningEntities(),
m_gameTimeInSeconds(0.0),
m_spacebarScrollMode(),
m_forceReload(false),
m_reloadEntities(false),
m_scheduledScreenshot(),
m_scheduledScreenshotDelay(-1.0f),
m_serializationAction(SerializationAction_None),
m_serializationID(),
m_serializationProgressType(ProgressType_Invalid),
m_serializationRemoveAfterUnserialize(false),
m_updateSectionProfiler("Game - update"),
m_updateForRenderSectionProfiler("Game - update for render"),
#if ENABLE_RENDER_SECTIONS
m_renderSectionProfiler("Game - render"),
#endif
m_colorGrading(),
m_colorGradingAfterHud(false),
m_shoeboxThread(),
m_threadShouldExit(false),
m_deltaTime(0.0f),
m_queuedEvents(),
m_eventsMutex(),
m_assetMonitor(),
m_debugFOVDelta(0.0f)
{
	m_layersVisible.setAllFlags();                    // all game layers are visible by default
	m_layersVisible.resetFlag(GameLayer_Attributes);  // except the attribute layer
	
	setupScriptButtons();
	
	// Set up the presentation system
	for (s32 layer = 0; layer < ParticleLayer_Count; ++layer)
	{
		m_presentationMgr[layer] = createPresentationMgr();
	}
	
	// Passing dummy level size. Will get resized on level change.
	m_levelSkinContext = level::skin::SkinContext::create(1, 1);
	
#ifdef USE_SHOEBOX_THREADING
	// Start shoebox thread
	m_shoeboxThread = tt::thread::create(
		staticUpdateShoeboxThread, this, false, 0, tt::thread::priority_below_normal,
#if defined(TT_PLATFORM_XBO)
		tt::thread::Affinity_Core1, "Shoebox Update Thread");
#else
		tt::thread::Affinity_Core2, "Shoebox Update Thread");
#endif
#endif
}


Game::~Game()
{
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->stopAllAudio();
	}
	
	input::Controller& controller = AppGlobal::getController(tt::input::ControllerIndex_One);
	controller.stopRumble(true);
	controller.setRumbleEnabled(true);
	
#ifdef USE_SHOEBOX_THREADING
	m_threadShouldExit = true;
	m_startShoeboxUpdate.signal();
	tt::thread::wait(m_shoeboxThread);
#endif
	
	g_scriptButtons.clear();
	
	m_colorGrading.reset();
}


void Game::init(const StartInfo& p_startInfo, ProgressType p_progressTypeOverride)
{
	toki::script::ScriptMgr::reset();
	m_startInfo = p_startInfo;
	
	if (m_startInfo.getMissionID().empty() == false && m_startInfo.shouldStartMission())
	{
		// Do mission start magic
		tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
		std::string level;
		vmPtr->callSqFunWithReturn(&level, "onStartMission", m_startInfo.getMissionID());
		m_startInfo.setLevel(level);
		m_startInfo.setShouldStartMission(false);
		AppGlobal::setGameStartInfo(m_startInfo);
	}
	
	m_progressTypeOverride = p_progressTypeOverride;
	m_serializationAction = SerializationAction_None;
	
	// Level has changed, notify recorder
	input::RecorderPtr recorder(AppGlobal::getInputRecorder());
	TT_NULL_ASSERT(recorder);
	recorder->onGameInit(&m_startInfo);
	
	const bool restoringShutdownState = AppGlobal::getShutdownDataMgr().hasData();
	
	if (restoringShutdownState == false && m_startInfo.isUserLevel())
	{
		// A fresh start of a user level. (Reset all progress information.)
		AppGlobal::getCheckPointMgr(ProgressType_UserLevel).resetAllCheckPoints();
		script::getRegistry(ProgressType_UserLevel).clear();
		script::getRegistry(ProgressType_UserLevel).clearPersistent();
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::init: Loading level '%s'. %s.\n", m_startInfo.getLevelFilePath().c_str(),
	          restoringShutdownState ? "Restoring shutdown state" : "Regular level load");
#endif
	
	audio::AudioPlayer* audioPlayer = audio::AudioPlayer::getInstance();
	
	// Tell the audio player we're currently loading a level, so that it can delay certain operations
	audioPlayer->setLoadingLevel(true);
	
	// When loading a new level, always reset the reverb settings and overall volumes to defaults
	audioPlayer->resetReverbSettings();
	audioPlayer->resetOverallVolume();
	
	setDrcCameraEnabled(false);
	
	if (m_startInfo.getType() == StartInfo::Type_UserRecording)
	{
		recorder->loadRecordedFile(m_startInfo.getRecordingFilePath());
		m_startInfo.setLevel(recorder->getCurrentSectionName());
	}
	
	loadLevel(m_startInfo);
	
	input::Controller& controller = AppGlobal::getController(tt::input::ControllerIndex_One);
	controller.makeAllPressedDownOnly();
	controller.setRumbleEnabled(true);
	
	m_gameTimeInSeconds = 0.0;
	
#if defined(TT_PLATFORM_WIN)
	m_debugView = DebugView::create("debugview");
#endif
	
	// HACK: Before creating any entities (which could trigger sound effects), focus the camera on
	//       the first PlayerBot entity we find in the level data. This should provide more accurate
	//       sound effect culling at the start of a level.
	bool foundFocusEntity = false;
	for (s32 i = 0; i < m_levelData->getEntityCount(); ++i)
	{
		level::entity::EntityInstancePtr instance(m_levelData->getEntityByIndex(i));
		if (instance->getType() == "PlayerBot")
		{
			getCamera().setPosition(instance->getPosition() + tt::math::Vector2(0.0f, 1.0f), true);
			foundFocusEntity = true;
			break;
		}
	}
	
	if (foundFocusEntity == false)
	{
		// Focus the camera on the start point
		const tt::math::Vector2 startWorldPos(level::tileToWorld(m_levelData->getPlayerStartPosition()));
		getCamera().setPosition(startWorldPos, true);
	}
	
	// Before creating any entities, ensure the audio listener position (or at least the Z distance)
	// is correct. This is so that any sound cue "setRadius" calls will produce correct angles for the emitter cones
	audioPlayer->setListenerPosition(tt::math::Vector3(
			getCamera().getCurrentPosition().x,
			getCamera().getCurrentPosition().y,
			g_audioListenerZDistance));
	
	// Create light manager
	enum { maxLights = 100 };  // FIXME: Get the max number lights needed from somewhere else.
	m_lightMgr.reset(new light::LightMgr(maxLights, m_levelData->getAttributeLayer()));
	m_stopLightRenderingOnSplit = true;
	m_levelData->getAttributeLayer()->registerObserver(m_lightMgr);
	
	// Create darkness manager
	enum { maxDarknesses = 10 };  // FIXME: Get the max number lights needed from somewhere else.
	m_darknessMgr.reset(new light::DarknessMgr(maxDarknesses));
	
	const tt::engine::renderer::EngineIDToTextures previousTextures(createEmptyShoeboxesWithPreviousSettings());
	// Should expect any textures to be in here at this point
	TT_ASSERT(previousTextures.empty());
	
	resetParticleMgr();
	
	createLevelEntities();
	
	if (restoringShutdownState == false)
	{
		// Load and execute the level script
		loadLevelScript(m_startInfo);
		
		createShoeboxesFromDataInclSkin();
	}
	
	// On CAT, we always render separately to TV and DRC (instead of clone mode):
	// set the DRC camera up for this
	if (restoringShutdownState == false && isDrcCameraEnabled() == false)
	{
		// DRC Camera was not enabled, copy normal camera.
		getDrcCamera().syncWith(getCamera());
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::init: Level loaded and prepared in %u ms.\n", u32(loadEnd - loadStart));
	AppGlobal::setLoadTimeLevel(u32(loadEnd - loadStart));
#endif
	
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	if (AppGlobal::isInDeveloperMode())
	{
		m_assetMonitor.reset(new utils::AssetMonitor());
		TT_NULL_ASSERT(m_assetMonitor);
		if (m_assetMonitor != 0)
		{
			m_assetMonitor->start();
		}
	}
#endif
	
	resetAndDisableDebugFOV();
	
	// Call generic onGameInitialized to signal to script that the game has been initialized
	{
		tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
		vmPtr->callSqFun("onGameInitialized");
	}
}


void Game::initOnRenderThread()
{
	// NOTE: This part of initialization cannot happen in loading thread, because render APIs are used
	m_colorGrading.reset(new effects::ColorGrading());
	m_colorGrading->update();
	
#if defined(TT_PLATFORM_WIN)
	m_debugUI = hud::DebugUI::create();
#endif
	
	if ((AppGlobal::isInLevelEditorMode()
#if !defined(TT_BUILD_FINAL)
	    || tt::app::getApplication()->getCmdLine().exists("startineditor")
#endif
	    ) && AppGlobal::allowEditorFeatures())
	{
		setEditorOpen(true);
	}
}


void Game::loadLevel(const std::string& p_levelName, ProgressType p_overrideProgressType )
{
	AppGlobal::setNextLevelOverrideProgressType(p_overrideProgressType);
	
#if !defined(TT_BUILD_FINAL)
	if (AppGlobal::getGame()->m_editor != 0 && AppGlobal::getGame()->m_editor->hasUnsavedChanges())
	{
		AppGlobal::getGame()->m_editor->handleUnsavedChangesBeforeLevelLoadFromGame(p_levelName);
	}
	else
#endif
	{
		// Set new level file
		StartInfo info(AppGlobal::getGame()->getStartInfo());
		info.setLevel(p_levelName);
		AppGlobal::setGameStartInfo(info);
		
		// Force reload of level.
		AppGlobal::getGame()->forceReload();
	}
}


void Game::startMission(const std::string& p_missionID)
{
	StartInfo startInfo(AppGlobal::getGame()->getStartInfo());
	startInfo.initMission(p_missionID);
	AppGlobal::setGameStartInfo(startInfo);
	
	// Start the mission
	AppGlobal::getGame()->forceReload();
}


void Game::onLoadScreenComplete()
{
	audio::AudioPlayer::getInstance()->setLoadingLevel(false);
}


void Game::update(real p_deltaTime)
{
#if !defined(TT_BUILD_FINAL)
	/* // Automated serialization test for deterministic behavior.
	{
		static s32 updateCount = 0;
		++updateCount;
		
		if (updateCount == 39)
		{
			serializeGameState();
		}
		else if (updateCount == 60)
		{
			unserializeGameState();
		}
	}
	// */
#endif
	
	TT_NULL_ASSERT(m_entityMgr);
	
	using namespace toki::utils;
	
	PROFILE_PERFORMANCE_UPDATE();
	
	m_updateSectionProfiler.startFrameUpdate();
	
	tt::stats::update(); // This needs to be called from main thread.
	
	// Unserialize the game at the start of a frame (only if not in editor)
	if (m_serializationAction == SerializationAction_Unserialize &&
	    isEditorOpen() == false)
	{
		CheckPointMgr& cpMgr = getCheckPointMgr();
		
		if (cpMgr.hasCheckPoint(m_serializationID) == false)
		{
			TT_PANIC("Trying to unserialize Game but checkpoint '%s' was not found (ProgressType: %d). (Restarting level.)",
			         m_serializationID.c_str(), m_serializationProgressType);
			reloadGame(true, false);
		}
		else
		{
			unserializeAll(*cpMgr.getCheckPoint(m_serializationID), m_serializationID);
		}
		m_serializationAction = SerializationAction_None;
		
		if (m_serializationRemoveAfterUnserialize)
		{
			cpMgr.resetCheckPoint(m_serializationID);
			m_serializationRemoveAfterUnserialize = false;
		}
	}
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_Misc);
	
	const input::Controller::State&              inputState(AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	const input::Controller::State::EditorState& editorState(inputState.editor);
	
	Camera& inputCamera(getInputCamera());
	const tt::math::Vector2 pointerWorldPos(inputCamera.screenToWorld(inputState.pointer));
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_EditorToggle);
	
	bool shouldHandleInput = true;
	
	if (m_editorWarnings.empty() == false)
	{
		if (AppGlobal::allowEditorFeatures())
		{
			setEditorOpen(true);
		}
	}
	else if (inputState.toggleEditor.pressed && AppGlobal::allowEditorFeatures())
	{
		toggleEditor();
	}
	// NOTE: Not using "editorIsOpen" variable here, because the editor open state could change because of DebugUI update
	else if (isEditorOpen() == false)
	{
#if defined(TT_STEAM_BUILD)
		if (m_workshopLevelPicker != 0 &&
		    m_workshopLevelPicker->update(p_deltaTime))
		{
			shouldHandleInput = false;
		}
#endif
		if (m_resolutionPicker != 0 &&
		    m_resolutionPicker->update(p_deltaTime))
		{
			shouldHandleInput = false;
		}
		
		// Do not handle game input if the debug UI handled it
		if (m_debugUI != nullptr && m_debugUI->update(p_deltaTime))
		{
			shouldHandleInput = false;
		}
	}
	
	// Now that any potential editor opening/closing has been handled, determine the editor open state
	const bool editorIsOpen = isEditorOpen();
	if (editorIsOpen)
	{
		shouldHandleInput = false;
	}

	// Handle visualboy editor logic
	if (VisualBoy::isLoaded())
	{
		static bool wasEditorOpen     = false;
		static bool wasEmulatorPaused = false;
		if (editorIsOpen && wasEditorOpen == false)
		{
			wasEmulatorPaused = VisualBoy::isPaused();
			VisualBoy::pause();
		}
		else if (editorIsOpen == false && wasEditorOpen && wasEmulatorPaused == false)
		{
			VisualBoy::resume();
		}
		wasEditorOpen = editorIsOpen;
	}
	
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	if (m_assetMonitor != 0)
	{
		m_assetMonitor->lockForReload();
		{
			if (m_assetMonitor->shouldReloadAssets(utils::AssetMonitor::AssetType_Texture))
			{
				tt::engine::renderer::TextureCache::reload();
				m_assetMonitor->signalAssetReloadCompleted(utils::AssetMonitor::AssetType_Texture);
			}
			
			if (m_assetMonitor->shouldReloadAssets(utils::AssetMonitor::AssetType_Presentation))
			{
				tt::pres::PresentationCache::reload(getPresentationMgr());
				m_assetMonitor->signalAssetReloadCompleted(utils::AssetMonitor::AssetType_Presentation);
			}
		}
		m_assetMonitor->unlockFromReload();
	}
#endif
	
	if (shouldHandleInput && AppGlobal::allowLevelCreatorDebugFeaturesInGame())
	{
		// DEBUG: Ctrl+E = recreate/reload level entities (also exposed in final builds for now)
		if (editorState.keys[tt::input::Key_Control].down &&
		    editorState.keys[tt::input::Key_E      ].pressed)
		{
			reloadGame(true, editorState.keys[tt::input::Key_Shift].down == false);
		}
#if !defined(TT_BUILD_FINAL)
		else if (editorState.keys[tt::input::Key_Control].down &&
		         editorState.keys[tt::input::Key_F5     ].pressed)
		{
			// Ctrl+F5 reloads everything (same as onRequestReloadAssets), but does not restore entity position
			reloadGame(false, false);
		}
		else if (editorState.keys[tt::input::Key_Control].down &&
		         editorState.keys[tt::input::Key_F2     ].pressed)
		{
			// Toggle color table.
			if (m_colorTable == 0)
			{
				createColorTable();
			}
			else
			{
				m_colorTable.reset();
			}
		}
#endif
	}
	
	//-------------------------------------------------------------------------------------------------------
	// Game frame updates after this.
	//-------------------------------------------------------------------------------------------------------
	
	if (editorIsOpen == false)
	{
		m_gameTimeInSeconds += static_cast<real64>(p_deltaTime);
	}
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_ScriptMgr);
	toki::script::ScriptMgr::update();
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_Misc);
	
#if !defined(TT_BUILD_FINAL)
	if (editorIsOpen == false && AppGlobal::allowLevelCreatorDebugFeaturesInGame())
	{
		// Handle "spacebar scrolling"
		const tt::input::Button& spaceBar(editorState.keys[tt::input::Key_Space]);
		
		if ((editorState.pointerLeft.pressed && spaceBar.down) ||
		    editorState.pointerMiddle.pressed ||
		    inputState.panCamera.pressed)
		{
			m_spacebarScrollMode.update(true);
		}
		else if (m_spacebarScrollMode.down &&
		         (editorState.pointerLeft.down   == false &&
		          editorState.pointerMiddle.down == false &&
		          inputState.panCamera.down      == false))
		{
			m_spacebarScrollMode.update(false);
		}
		else
		{
			m_spacebarScrollMode.update(m_spacebarScrollMode.down);
		}
	}
	
	static tt::math::Vector2 levelDragStartInputPos;
	if (m_spacebarScrollMode.pressed)
	{
		//TT_Printf("Game::update: Start spacebar scrolling. (%f, %f)\n", pointerWorldPos.x, pointerWorldPos.y);
		levelDragStartInputPos = pointerWorldPos;
	}
	else if (m_spacebarScrollMode.down)
	{
		const tt::math::Vector2 distanceWorld(levelDragStartInputPos - pointerWorldPos);
		//TT_Printf("Game::udpate: spacebar scrolling down. (%f, %f) - (%f, %f) = (%f, %f)\n",
		//          levelDragStartInputPos.x, levelDragStartInputPos.y, pointerWorldPos.x, pointerWorldPos.y,
		//          distanceWorld.x, distanceWorld.y);
		inputCamera.setPosition(inputCamera.getCurrentPosition() + distanceWorld, true);
		inputCamera.setFreeScrollMode(true);
	}
	else if (m_spacebarScrollMode.released)
	{
		//TT_Printf("Game::update: Stop spacebar scrolling.\n");
		tt::math::Vector2 diff(AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer - AppGlobal::getController(tt::input::ControllerIndex_One).prev.pointer);
		if (diff.lengthSquared() > 25) // 5 pixels deadzone. (Check against 5^2). FIXME: Add to cfg.
		{
			const tt::math::Vector2 distanceWorld(levelDragStartInputPos - pointerWorldPos);
			inputCamera.initSpeedFromPos(inputCamera.getCurrentPosition() + distanceWorld);
		}
	}
	
	// Free scrolling should be disabled when direction input is given
	if (inputState.direction.lengthSquared() > 0.0f)
	{
		inputCamera.setFreeScrollMode(false);
	}
#endif
	
	// Get mouse scroll
	tt::math::Vector2 scroll(inputState.scroll);
	
	static tt::math::Point2 mouseScrollStart;
	const bool inMouseScrollMode = editorState.pointerLeft.down || editorState.pointerRight.down;
	if (editorIsOpen == false && inMouseScrollMode) // should scroll with mouse
	{
		if ((editorState.pointerLeft .pressed && editorState.pointerRight.down == false) ||
		    (editorState.pointerRight.pressed && editorState.pointerLeft .down == false))
		{
			// Started scrolling
			mouseScrollStart = editorState.pointer;
		}
		else // Continue scrolling
		{
			tt::math::Vector2 diff(editorState.pointer - mouseScrollStart);
			diff.y = -diff.y;
			const s32 screenH = tt::engine::renderer::Renderer::getInstance()->getScreenHeight();
			diff /= (screenH * 0.2f);
			diff.normalizeClamp();
			scroll += diff;
			scroll.normalizeClamp();
		}
	}
	
	const tt::math::Vector2 worldPos(inputCamera.screenToWorld(inputState.pointer));
#if !defined(TT_BUILD_FINAL)
	const tt::math::Point2  tilePos(level::worldToTile(worldPos));
#endif
	
	// Screenspace Pointer
	tt::math::Vector2 screenspacePointerPos = tt::math::Vector2(inputState.pointer);
	const tt::math::Point2 scrSize(inputCamera.getViewPortSize());
	screenspacePointerPos -= tt::math::Vector2(scrSize.x * 0.5f, scrSize.y * 0.5f); // Center of screen is (0,0)
	screenspacePointerPos /= scrSize.y; // Normalized space
	screenspacePointerPos.y = -screenspacePointerPos.y;   // Flip Y
	
	if (editorIsOpen == false)
	{
//#if !defined(TT_BUILD_FINAL)
		// Debug feature: if 'I' is pressed, move the entity followed by the camera to the position under the cursor
		if (editorState.keys[tt::input::Key_I].pressed && AppGlobal::allowLevelCreatorDebugFeaturesInGame())
		{
			entity::Entity* entity = inputCamera.getFollowEntity().getPtr();
			if (entity != 0)
			{
				entity->setPositionForced(inputCamera.screenToWorld(inputState.pointer), editorState.keys[tt::input::Key_Alt].down == false);
			}
		}
//#endif
		
		// Update keyboard key listeners (used for remappable controls)
		bool hasBlockingKeyboardListener = false;
		{
			typedef std::vector<script::EntityBase*> EntityScripts;
			EntityScripts scripts;
			scripts.reserve(m_keyboardListeningEntities.size());
			if (m_keyboardListeningEntities.size() > 0)
			{
				// Priorities are sorted in ascending order, so start at end of vector (highest prio)
				for (PrioritizedInputListeners::reverse_iterator it = m_keyboardListeningEntities.rbegin();
					    it != m_keyboardListeningEntities.rend(); ++it)
				{
					entity::Entity* entity = (*it).entity.getPtr();
					if (entity != 0)
					{
						scripts.push_back(entity->getEntityScript().get());
						if (it->isBlocking)
						{
							// Encountered a input blocking entity; stop adding more listening entities
							hasBlockingKeyboardListener = true;
							break;
						}
					}
				}
			}
			
			// Find out which keyboard key is pressed
			const tt::input::KeyboardController& kbd(tt::input::KeyboardController::getState(tt::input::ControllerIndex_One));
			s32 keyCode = -1;
			for (s32 i = 0; i < tt::input::Key_Count; ++i)
			{
				if (kbd.keys[i].released)
				{
					keyCode = i;
					break;
				}
			}
			
			if (keyCode != -1)
			{
				for (EntityScripts::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
				{
					(*it)->callSqFun("onKeyboardDown", keyCode);
				}
			}
		}
		
		// Update input listeners (Don't accept input for scripts when specific editorkeys are pressed)
#if !defined(TT_BUILD_FINAL)
		const bool editorKeyPressed = editorState.keys[tt::input::Key_Control].down ||
		                              editorState.keys[tt::input::Key_Alt].down ||
		                              editorState.keys[tt::input::Key_Space].down;
		
		if (((AppGlobal::allowLevelCreatorDebugFeaturesInGame() && editorKeyPressed) == false ||
		    AppGlobal::getInputRecorder()->isActive()) &&
#else
		if (
#endif
			hasBlockingKeyboardListener == false)
		{
			typedef std::vector<script::EntityBase*> EntityScripts;
			EntityScripts scripts;
			scripts.reserve(m_buttonInputListeningEntities.size());
			
			if (m_buttonInputListeningEntities.size() > 0)
			{
				// Priorities are sorted in ascending order, so start at end of vector (highest prio)
				for (PrioritizedInputListeners::reverse_iterator it = m_buttonInputListeningEntities.rbegin();
				     it != m_buttonInputListeningEntities.rend(); ++it)
				{
					entity::Entity* entity = (*it).entity.getPtr();
					if (entity != 0)
					{
						scripts.push_back(entity->getEntityScript().get());
						if (it->isBlocking)
						{
							// Encountered a input blocking entity; stop adding more listening entities
							break;
						}
					}
				}
			}
			
			for (ScriptButtons::iterator it = g_scriptButtons.begin(); it != g_scriptButtons.end(); ++it)
			{
				ScriptButton& scriptButton(*it);
				TT_NULL_ASSERT(scriptButton.controllerButton);
				
				if (scriptButton.controllerButton->pressed && scriptButton.onPressed != 0)
				{
					for (EntityScripts::const_iterator scriptIt = scripts.begin(); scriptIt != scripts.end(); ++scriptIt)
					{
						const bool wasHandled = ((*scriptIt)->*(scriptButton.onPressed))();
						if (wasHandled)
						{
							break;
						}
					}
					
					scriptButton.pressDuration = 0.0f;
				}
				else if (scriptButton.controllerButton->released && scriptButton.onReleased != 0)
				{
					for (EntityScripts::const_iterator scriptIt = scripts.begin(); scriptIt != scripts.end(); ++scriptIt)
					{
						const bool wasHandled = ((*scriptIt)->*(scriptButton.onReleased))(scriptButton.pressDuration);
						if (wasHandled)
						{
							break;
						}
					}
				}
				
				if (scriptButton.controllerButton->down)
				{
					scriptButton.pressDuration += p_deltaTime;
				}
			}
		}
		
		{
			m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_EntityMgr);
			const Camera& camera(isDrcCameraEnabled() ? getDrcCamera() : getCamera());
			m_entityMgr->updateEntities(p_deltaTime, camera);
		}
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_FluidMgr);
		if (m_fluidMgr != 0)
		{
			m_fluidMgr->update(p_deltaTime);
		}
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_LightMgr);
		if (m_lightMgr != 0)
		{
			m_lightMgr->update(p_deltaTime);
		}
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_PathMgr);
		m_pathMgr.update(p_deltaTime);
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_Misc);
		
		// Color Grading
		getColorGradingEffectMgr().update(p_deltaTime);
		
		if (m_colorGrading != 0)
		{
			using entity::effect::ColorGradingEffectMgr;
			const ColorGradingEffectMgr& colorGradingEffectMgr = getColorGradingEffectMgr();
			
			if (colorGradingEffectMgr.getShaderEffectTotalStrength() > 0.0f
#if !defined(TT_BUILD_FINAL)
			    && m_colorTable  == 0 // Only do color grading if color table is not active.
#endif
			   )
			{
				TT_STATIC_ASSERT(
					static_cast<s32>(effects::ColorGrading::Lookup_Count) ==
					static_cast<s32>(ColorGradingEffectMgr::EffectInShader_Count));
				
				const real strengthOne =
					colorGradingEffectMgr.getShaderEffectStrength(ColorGradingEffectMgr::EffectInShader_One);
				const real strengthTwo =
					colorGradingEffectMgr.getShaderEffectStrength(ColorGradingEffectMgr::EffectInShader_Two);
				const real totalStrength(strengthOne + strengthTwo);
				const real lerpLookupTextures(strengthOne / totalStrength);
				const real lerpWithOriginal(1.0f - totalStrength);
				
				m_colorGrading->setLerpParameters(lerpLookupTextures, lerpWithOriginal);
				
				for (s32 i = 0; i < effects::ColorGrading::Lookup_Count; ++i)
				{
					m_colorGrading->setLookupTexture(
						colorGradingEffectMgr.getShaderEffectTexture (
							static_cast<ColorGradingEffectMgr::EffectInShader>(i)),
							static_cast<effects::ColorGrading::Lookup        >(i));
				}
				m_colorGrading->setEnabled(true);
			}
			else
			{
				m_colorGrading->setEnabled(false);
			}
			
			// Enable or disable post-processing based on color-grading needs
			{
				using namespace tt::engine::renderer;
				Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
				renderer->getPP()->setActive(
					AppOptions::getInstance().postProcessing && m_colorGrading->isEnabled());
			}
		}
		
#if !defined(TT_BUILD_FINAL)
		
		if (editorState.keys[tt::input::Key_C].pressed &&
		    // Also check shift so we don't overlap with hotKeyToggleCameraFollowEntity.
		    editorState.keys[tt::input::Key_Shift].down == false)
		{
			m_cameraMgr.toggleEmulateDRC();
		}
		
		// Pressing * while pointing at an entity toggles its display of debug info
		if (editorState.keys[tt::input::Key_Multiply].pressed &&
		    inputState.editor.pointerLeft.down  == false &&
		    m_spacebarScrollMode.down == false &&
		    AppGlobal::allowLevelCreatorDebugFeaturesInGame() && 
		    getTileRegistrationMgr().contains(tilePos))
		{
			entity::EntityHandleSet foundEntities;
			getTileRegistrationMgr().findRegisteredEntityHandles(tilePos, foundEntities);
			
			if (editorState.keys[tt::input::Key_Shift].down)
			{
				for (entity::EntityHandleSet::iterator it = foundEntities.begin(); it != foundEntities.end(); ++it)
				{
					entity::Entity* entity = m_entityMgr->getEntity(*it);
					if (entity != 0)
					{
						entity->incrementDebugPresentationObjectIdx();
					}
				}
			}
			else
			{
				for (entity::EntityHandleSet::iterator it = foundEntities.begin(); it != foundEntities.end(); ++it)
				{
					entity::Entity* entity = m_entityMgr->getEntity(*it);
					if (entity != 0)
					{
						entity->setShowPresentationTags(entity->shouldShowPresentationTags() == false);
						
						TT_Printf("Game::update: %s debug info for entity 0x%08X (type '%s')\n",
								  entity->shouldShowPresentationTags() ? "Enabling" : "Disabling",
								  entity, entity->getType().c_str());
					}
				}
			}
		}
#endif
		
		if (inputState.editor.pointerLeft.pressed && shouldHandleInput && m_spacebarScrollMode.down == false)
		{
			TT_ASSERT(m_draggingEntity.isEmpty());
			
			
#if !defined(TT_BUILD_FINAL)
			if (AppGlobal::allowLevelCreatorDebugFeaturesInGame())
			{
				static const entity::EntityHandleSet empty;
				const entity::EntityHandleSet& foundEntities = (getTileRegistrationMgr().contains(tilePos)) ?
						getTileRegistrationMgr().getRegisteredEntityHandles(tilePos) : empty;
				
				for (entity::EntityHandleSet::const_iterator it = foundEntities.begin(); it != foundEntities.end(); ++it)
				{
					entity::Entity* entity = m_entityMgr->getEntity(*it);
					if (entity != 0)
					//if (entity->contains(inputState.pointer))
					{
						// Kill the entity if Alt is down
						if (editorState.keys[tt::input::Key_Alt].down)
						{
							entity->kill();
						}
						// Start dragging this entity if Ctrl is down
						else if (editorState.keys[tt::input::Key_Control].down)
						{
							startDraggingEntity(*it);
						}
						
						break;
					}
				}
			}
#endif
			
#if !defined(TT_BUILD_FINAL)
			const bool keyPressed = editorState.keys[tt::input::Key_Control].down ||
			                        editorState.keys[tt::input::Key_Alt].down ||
			                        editorState.keys[tt::input::Key_Space].down;
			
			// Send pointer pressed event to all entities under cursor
			// (only if no modifier keys are being pressed)
			if ((AppGlobal::allowLevelCreatorDebugFeaturesInGame() && keyPressed) == false ||
			    AppGlobal::getInputRecorder()->isActive())
#endif
			{
				typedef std::vector<entity::Entity*> Entities;
				Entities entities;
				entities.reserve(m_mouseInputListeningEntities.size());
				
				if (m_mouseInputListeningEntities.size() > 0)
				{
					const s32 prio = m_mouseInputListeningEntities.back().priority;
					// Priorities are sorted in ascending order, so start at end of vector (highest prio)
					for (PrioritizedInputListeners::reverse_iterator it = m_mouseInputListeningEntities.rbegin();
						 it != m_mouseInputListeningEntities.rend(); ++it)
					{
						if (it->priority < prio)
						{
							// Lower priority mouse input found; these should not be handled
							break;
						}
						
						entity::Entity* entity = (*it).entity.getPtr();
						if (entity != 0)
						{
							entities.push_back(entity);
							if (it->isBlocking)
							{
								// Encountered a input blocking entity; stop adding more listening entities
								break;
							}
						}
					}
				}
				
				for (Entities::const_iterator it = entities.begin(); it != entities.end(); ++it)
				{
					const entity::Entity* entity = (*it);
					const bool isScreenSpace = entity->isScreenSpaceEntity();
					toki::game::event::input::PointerEvent pointerEvent((isScreenSpace) ?
							screenspacePointerPos : pointerWorldPos, 0.0f, true);
					
					if (isScreenSpace ?
						entity->getWorldRect().contains(screenspacePointerPos) :
						entity->getWorldRect().contains(pointerWorldPos))
					{
						m_pressingEntities.push_back(EntityPressInfo(entity->getHandle()));
						entity->getEntityScript()->onPointerPressed(
								script::wrappers::PointerEventWrapper(pointerEvent));
					}
				}
			}
		}
	}
	
	if (inputState.editor.pointerLeft.down && m_draggingEntity.isEmpty() == false)
	{
		entity::Entity* entity = getDraggingEntity();
		if (entity != 0)
		{
			entity->setPositionForced(worldPos, editorState.keys[tt::input::Key_Alt].down == false);
		}
	}
	else
	{
		releaseDraggingEntity();
	}
	
	updatePressingEntities(p_deltaTime, screenspacePointerPos, pointerWorldPos, inputState.editor.pointerLeft.released);
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_Editor);
	
	if (m_editor != 0)
	{
		m_editor->update(p_deltaTime);
	}
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_Misc);
	
	AppGlobal::getDemoMgr().update(p_deltaTime);
	
	
#if !defined(TT_BUILD_FINAL)
	{
		//--------------------------------------------------------------------------
		// DEBUG CODE:
		
		if (isEditorOpen())
		{
			if (getCamera().isDebugFOVEnabled())
			{
				getCamera().setDebugFOVEnabled(false);
			}
		}
		else if (m_debugFOVDelta != 0.0f && getCamera().isDebugFOVEnabled() == false)
		{
			getCamera().setDebugFOVEnabled(true);
		}
		
		if (shouldHandleInput && AppGlobal::allowLevelCreatorDebugFeaturesInGame())
		{
			// Support 'zooming' (changing FOV) using mouse wheel or shortcuts
			bool fovChanged = (inputState.wheelNotches != 0);
			m_debugFOVDelta -= inputState.wheelNotches * 5.0f;
			
			if (editorState.keys[tt::input::Key_Alt  ].down == false &&
			    editorState.keys[tt::input::Key_Shift].down == false)
			{
				static const char* const fovDeltaOption = "toki.camera.game.fov_zoom_step";
				
				if (editorState.keys[tt::input::Key_Control].down)
				{
					if (editorState.keys[tt::input::Key_Plus].pressed)
					{
						m_debugFOVDelta -= cfg()->getRealDirect(fovDeltaOption);
						fovChanged = true;
					}
					if (editorState.keys[tt::input::Key_Minus].pressed)
					{
						m_debugFOVDelta += cfg()->getRealDirect(fovDeltaOption);
						fovChanged = true;
					}
				}
				else
				{
					if (editorState.keys[tt::input::Key_PageUp].pressed)
					{
						m_debugFOVDelta -= cfg()->getRealDirect(fovDeltaOption);
						fovChanged = true;
					}
					if (editorState.keys[tt::input::Key_PageDown].pressed)
					{
						m_debugFOVDelta += cfg()->getRealDirect(fovDeltaOption);
						fovChanged = true;
					}
				}
			}
			
			if (fovChanged)
			{
				getCamera().setDebugFOVEnabled(true);
				const real targetFOV = getCamera().getTargetFOV();
				getCamera().setDebugFOV(targetFOV + m_debugFOVDelta);
				// Account for clamping
				m_debugFOVDelta = getCamera().getDebugFOV() - targetFOV;
			}
			
			// Reset debug offsets when pressing F4
			if (editorState.keys[tt::input::Key_F4].pressed)
			{
				resetAndDisableDebugFOV();
			}
		}
	}
	//--------------------------------------------------------------------------
#endif  // !defined(TT_BUILD_FINAL)
	
	if (editorIsOpen == false)
	{
#ifdef USE_SHOEBOX_THREADING
		// Trigger shoebox update on update thread
		m_deltaTime = p_deltaTime;
		m_startShoeboxUpdate.signal();
#else
		updateShoebox(p_deltaTime);
#endif
		
		// Martijn: not needed for RIVE
		/*
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_EventMgr);
		m_eventMgr->update(p_deltaTime);
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_SoundGraphicsMgr);
		m_soundGraphicsMgr->update(p_deltaTime);
		// */
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_PresentationMgr);
		m_presentationObjectMgr->update(p_deltaTime);
		for(s32 layer = 0; layer < ParticleLayer_Count; ++layer)
		{
			m_presentationMgr[layer]->update(p_deltaTime);
		}
		m_presentationObjectMgr->checkAnimationEnded();
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_EntityScriptMgr);
		if (AppGlobal::isBadPerformanceDetected() &&
		    AppOptions::getInstance().lowPerfReported == false)
		{
			tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
			vmPtr->callSqFun("onBadPerformanceDetected");
		}
		AppGlobal::getEntityScriptMgr().update(p_deltaTime);
		
		getEntityMgr().callUpdateOnAllEntities(p_deltaTime); // HACK: update callback for script, each frame.
		
		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_EntityMgr);
		m_entityMgr->updateEntitiesChanges(p_deltaTime);
		
#ifdef USE_SHOEBOX_THREADING
		// Sync with shoebox thread
		m_finishedShoeboxUpdate.wait();
#endif

		m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_ParticleMgr);
		tt::engine::particles::ParticleMgr::getInstance()->update(p_deltaTime);
		
		m_cameraMgr.update(p_deltaTime,
		                   scroll,
		                   getEffectMgr().getCameraOffset(),
		                   getEffectMgr().getDrcCameraOffset(),
		                   true);
	}
	
	audio::AudioPlayer::getInstance()->setListenerPosition(tt::math::Vector3(
		m_cameraMgr.getCamera().getCurrentPositionWithEffects().x,
		m_cameraMgr.getCamera().getCurrentPositionWithEffects().y,
		g_audioListenerZDistance));
	
	m_updateSectionProfiler.startFrameUpdateSection(FrameUpdateSection_Misc);
	if (m_attribDebugView != 0)
	{
		m_attribDebugView->update();
	}
	
	if (m_debugView != 0)
	{
		m_debugView->update(p_deltaTime);
	}
	
#if !defined(TT_BUILD_FINAL)  // disable notes in final builds
	if (editorIsOpen == false)
	{
		// Only show notes for user levels (or in developer mode)
		if (m_layersVisible.checkFlag(GameLayer_Notes) &&
		    (AppGlobal::isInDeveloperMode() || m_startInfo.isUserLevel()))
		{
			// Update the note graphics
			const level::Notes& notes(m_levelData->getAllNotes());
			for (level::Notes::const_iterator it = notes.begin(); it != notes.end(); ++it)
			{
				(*it)->update(p_deltaTime);
			}
		}
	}
#endif
	
	if (editorIsOpen == false &&
	    m_scheduledScreenshotDelay > 0.0f)
	{
		m_scheduledScreenshotDelay -= p_deltaTime;
		if (m_scheduledScreenshotDelay <= 0.0f)
		{
			AppGlobal::takeLevelScreenshot(m_scheduledScreenshot);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------
	// End of game frame. No more updates after this.
	//-------------------------------------------------------------------------------------------------------
	
	// Serialize the game at the end of a frame (if serialization was requested and not in editor)
	if (m_serializationAction == SerializationAction_Serialize &&
	    editorIsOpen == false)
	{
		static serialization::SerializationMgrPtr newCheckPoint;
		if (newCheckPoint == nullptr)
		{
			newCheckPoint = serialization::SerializationMgr::createEmpty();
		}
		serializeAll(*newCheckPoint);
		getCheckPointMgr().setCheckPoint(newCheckPoint, m_serializationID);
		m_serializationAction = SerializationAction_None;
	}
	
	m_updateSectionProfiler.stopFrameUpdate();
}


void Game::updateForRender(real p_deltaTime)
{
	using namespace toki::utils;
	m_updateForRenderSectionProfiler.startFrameUpdate();
	
	const bool editorIsOpen = isEditorOpen();
	CameraMgr& cameraMgr(editorIsOpen ? m_editor->getCameraMgr() : m_cameraMgr);
	
	const bool bothCamerasHudOnly = cameraMgr.updateForRender();
	
	if (bothCamerasHudOnly)
	{
		// Both cameras are hud only.
		
		// Make sure to update presentation mgr before the particles.
		m_updateForRenderSectionProfiler.startFrameUpdateSection(FrameUpdateForRenderSection_PresentationMgr);
		if (m_presentationMgr[ParticleLayer_Hud       ] != 0)
		{
			m_presentationMgr[ParticleLayer_Hud       ]->updateForRender(p_deltaTime);
		}
		if (m_presentationMgr[ParticleLayer_HudDrcOnly] != 0)
		{
			m_presentationMgr[ParticleLayer_HudDrcOnly]->updateForRender(p_deltaTime);
		}
		if (m_presentationMgr[ParticleLayer_HudTvOnly ] != 0)
		{
			m_presentationMgr[ParticleLayer_HudTvOnly ]->updateForRender(p_deltaTime);
		}
		
		m_updateForRenderSectionProfiler.startFrameUpdateSection(FrameUpdateForRenderSection_ParticleMgr);
		tt::engine::particles::ParticleMgr::getInstance()->updateForRender(0);
		
		m_updateForRenderSectionProfiler.stopFrameUpdate();
		return;
	}
	
	// Make sure to update presentation mgr before the particles.
	m_updateForRenderSectionProfiler.startFrameUpdateSection(FrameUpdateForRenderSection_PresentationMgr);
	for (s32 i = 0; i < ParticleLayer_Count; ++i)
	{
		if (m_presentationMgr[i] != 0)
		{
			m_presentationMgr[i]->updateForRender(p_deltaTime);
		}
	}
	
	
	const tt::math::VectorRect& visibilityRect(cameraMgr.getCombinedVisibilityRect());
	m_updateForRenderSectionProfiler.startFrameUpdateSection(FrameUpdateForRenderSection_ParticleMgr);
	tt::engine::particles::ParticleMgr::getInstance()->updateForRender(&visibilityRect);
	
	if (editorIsOpen == false && m_lightMgr != 0)
	{
		m_updateForRenderSectionProfiler.startFrameUpdateSection(FrameUpdateForRenderSection_LightMgr);
		m_lightMgr->updateForRender(visibilityRect,
		                            static_cast<s32>(getEffectMgr().getLightAmbient(m_lightMgr->getLevelLightAmbient())));
	}
	
	{
		m_updateForRenderSectionProfiler.startFrameUpdateSection(FrameUpdateForRenderSection_Fog);
		
		getFogEffectMgr().update(p_deltaTime);
		
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		const bool hasFog = (getFogEffectMgr().hasFog()
#if !defined(TT_BUILD_FINAL)
				&& AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Disable_Fog) == false
#endif
		);
		
		tt::engine::renderer::ColorRGB fogColor(getEffectMgr().getFogColor(getFogEffectMgr().getCurrentDefaultColor()));
		
		// Apply fog settings
		if (hasFog)
		{
			renderer->setFogMode(tt::engine::renderer::FogMode_Linear);
			renderer->setFogSetting(tt::engine::renderer::FogSetting_Start,
			                        getEffectMgr().getFogNear(getFogEffectMgr().getCurrentDefaultNear()));
			renderer->setFogSetting(tt::engine::renderer::FogSetting_End,
			                        getEffectMgr().getFogFar(getFogEffectMgr().getCurrentDefaultFar()));
			renderer->setFogColor(fogColor);
			
			// Make sure the alpha channel is always cleared to 0 for light rendering
			renderer->setClearColor(tt::engine::renderer::ColorRGBA(fogColor, 0));
		}
		else
		{
#if !defined(TT_BUILD_FINAL)
			if (m_shoeboxDataIncludesFromScript != 0 && m_shoeboxDataIncludesFromScript->includes.empty())
			{
				// Make sure the alpha channel is always cleared to 0 for light rendering
				renderer->setClearColor(tt::engine::renderer::ColorRGBA(96, 96, 96, 0));
			}
			else
#endif
			{
				// Make sure the alpha channel is always cleared to 0 for light rendering
				renderer->setClearColor(tt::engine::renderer::ColorRGBA(0, 0, 0, 0));
			}
		}
	}
	
	if (editorIsOpen)
	{
		m_editor->updateForRender();
	}
	
	m_updateForRenderSectionProfiler.stopFrameUpdate();
}


void Game::render()
{
	using namespace toki::utils;
	
	START_RENDER_FRAME();
	START_RENDER_SECTION(FrameRenderSection_Misc);
	
	TT_NULL_ASSERT(m_entityMgr);
	TT_NULL_ASSERT(m_lightMgr);
	
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	renderer->setZBufferEnabled(false);
	renderer->setCullingEnabled(false);
	renderer->setColorMask(tt::engine::renderer::ColorMask_Color);
	
	const bool editorIsOpen    = isEditorOpen();
	
	CameraMgr& cameraMgr(editorIsOpen ? m_editor->getCameraMgr() : m_cameraMgr);
	cameraMgr.onRenderBegin();
	const CameraMgr::ViewPortState& currentViewportState(cameraMgr.getCurrentViewportState());
	const bool isRenderingToDRC = currentViewportState.renderingToDRC;
	
	const bool renderingEditor = editorIsOpen;
	
	const bool                  isRenderingMainCam = currentViewportState.renderingMainCam;
	const bool                  renderHudOnly      = currentViewportState.renderHudOnly;
	const tt::math::VectorRect& visibilityRect     = *currentViewportState.visibilityRect;
	
	const GameLayers& layersVisible(editorIsOpen ? m_editor->getGameLayersVisible() : m_layersVisible);
	
	tt::engine::particles::ParticleMgr* particleMgr = tt::engine::particles::ParticleMgr::getInstance();
	
	const tt::math::VectorRect& cullingRect = (isRenderingToDRC) ? 
		getDrcCamera().getCurrentCullingRect() : getCamera().getCurrentCullingRect();
	particleMgr->updateForRender(&cullingRect);
	
	START_RENDER_SECTION(FrameRenderSection_Shoebox);
	
	
	const bool renderFog = (getFogEffectMgr().hasFog()
#if !defined(TT_BUILD_FINAL)
			&& AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Disable_Fog) == false
			&& AppGlobal::getLevelScreenshotSettings().type != ScreenshotType_FullLevel
#endif
	                       );
	
	if (renderHudOnly && renderingEditor == false)
	{
		renderHud(isRenderingToDRC, isRenderingMainCam);
		renderDebug(isRenderingToDRC);
		return;
	}
	
	renderer->setFogEnabled(renderFog);
	
	if (layersVisible.checkFlag(GameLayer_ShoeboxBackground) && m_shoeboxSkinAndEnvironment != 0)
	{
		m_shoeboxSkinAndEnvironment->renderBackground();
	}
	
	if (editorIsOpen == false)
	{
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_BehindShoeboxZero]->render();
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_BehindShoeboxZero);
	}
	
	START_RENDER_SECTION(FrameRenderSection_Shoebox);
	const bool shouldRenderShoeboxZero = layersVisible.checkFlag(GameLayer_ShoeboxZero);
	if (shouldRenderShoeboxZero && m_shoeboxSkinAndEnvironment != 0)
	{
		m_shoeboxSkinAndEnvironment->renderBackgroundZero(visibilityRect);
	}
	
	START_RENDER_SECTION(FrameRenderSection_AttributeDebugView);
	if (layersVisible.checkFlag(GameLayer_Attributes) && m_attribDebugView != 0)
	{
		// Render attribute layer without fog if the editor is open
		if (editorIsOpen && renderFog)
		{
			renderer->setFogEnabled(false);
		}
		
		m_attribDebugView->render();
		
		if (editorIsOpen && renderFog)
		{
			renderer->setFogEnabled(true);
		}
	}
	
	if (editorIsOpen == false)
	{
#if !defined(TT_BUILD_FINAL)
		if (layersVisible.checkFlag(GameLayer_Attributes))
		{
			entity::EntityTiles::debugRenderAllInstances();
		}
#endif
		
		START_RENDER_SECTION(FrameRenderSection_LightMgr);
		m_lightMgr->startLightRender(m_shadowMaskShoebox, visibilityRect);
	}
	
	START_RENDER_SECTION(FrameRenderSection_FluidMgr);
	if (m_fluidMgr != 0)
	{
		m_fluidMgr->updateForRender(cameraMgr.getCombinedVisibilityRect());
		m_fluidMgr->render();
	}
	
	START_RENDER_SECTION(FrameRenderSection_Misc);
	if (editorIsOpen == false)
	{
#if !defined(TT_BUILD_FINAL)
		const DebugRenderMask& debugRenderMask(AppGlobal::getDebugRenderMask());
		
		if (debugRenderMask.checkFlag(DebugRender_LevelBorder))
		{
			m_levelBorder->render();
		}
#endif
		
		START_RENDER_SECTION(FrameRenderSection_FluidMgr);
		m_fluidMgr->renderBack();
		m_fluidMgr->renderBackStillWater();
		
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_BehindEntities]->render();
		
		if (isRenderingMainCam == false)
		{
			m_presentationMgr[ParticleLayer_BehindEntitiesSubOnly]->render();
		}
		
		// Render power beam graphics behind the "behind entities" particles,
		// so that the power beam particles show up in front of the beams
		START_RENDER_SECTION(FrameRenderSection_EntityMgr);

		renderer->getDebug()->startRenderGroup("Beam Graphics");
		m_entityMgr->updateForRender(visibilityRect);
		m_entityMgr->renderPowerBeamGraphics(visibilityRect);
		renderer->getDebug()->endRenderGroup();
		
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_BehindEntities);
		
		START_RENDER_SECTION(FrameRenderSection_SoundGraphics);
		renderer->getDebug()->startRenderGroup("Sound Graphics");
		
		// Martijn: not needed for RIVE
		/*
		m_soundGraphicsMgr->updateForRender();
		m_soundGraphicsMgr->render(visibilityRect);
		m_soundGraphicsMgr->renderLightmask(visibilityRect);
		// */
		
		renderer->getDebug()->endRenderGroup();
		
		START_RENDER_SECTION(FrameRenderSection_Presentation);
#if !defined(TT_BUILD_FINAL)
		if (debugRenderMask.checkFlag(DebugRender_Disable_PresentationInFrontOfEntities) == false)
#endif
		{
			m_presentationMgr[ParticleLayer_InFrontOfEntities]->render();
			if (isRenderingMainCam == false)
			{
				m_presentationMgr[ParticleLayer_InFrontOfEntitiesSubOnly]->render();
			}
		}
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_InFrontOfEntities);
		
		START_RENDER_SECTION(FrameRenderSection_FluidMgr);
		m_fluidMgr->renderFront();
		
		// Render "in front of water" layer
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_InFrontOfWater]->render();
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_InFrontOfWater);
		
		START_RENDER_SECTION(FrameRenderSection_FluidMgr);
		m_fluidMgr->renderFrontStillWater();
		
		// Render "in front of water" layer
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_InFrontOfStillWater]->render();
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_InFrontOfStillWater);
	}
	
	START_RENDER_SECTION(FrameRenderSection_Shoebox);
	if (shouldRenderShoeboxZero && m_shoeboxSkinAndEnvironment != 0)
	{
		m_shoeboxSkinAndEnvironment->renderForegroundZeroBack(visibilityRect);
	}
	
	if (editorIsOpen == false)
	{
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_InFrontOfSplit]->render();
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_InFrontOfSplit);
		
		START_RENDER_SECTION(FrameRenderSection_LightMgr);
		m_lightMgr->addLightsToRender();
		
		if (m_stopLightRenderingOnSplit)
		{
			// Stop light here
			m_lightMgr->endLightRender(m_shadowMaskShoebox, visibilityRect);
		}
		
		m_entityMgr->renderTextLabels();
		
		if (renderFog)
		{
			renderer->setFogEnabled(false);
		}
		
#if defined(TT_STEAM_BUILD)
		if (m_workshopLevelPicker != 0)
		{
			m_workshopLevelPicker->render();
		}
#endif
		if (m_resolutionPicker != 0)
		{
			m_resolutionPicker->render();
		}
		if (renderFog)
		{
			renderer->setFogEnabled(true);
		}
	}
	
	START_RENDER_SECTION(FrameRenderSection_Shoebox);
	if (shouldRenderShoeboxZero && m_shoeboxSkinAndEnvironment != 0)
	{
		m_shoeboxSkinAndEnvironment->renderForegroundZeroFront(visibilityRect);
	}
	
	if (editorIsOpen == false)
	{
#if !defined(TT_BUILD_FINAL)
		const DebugRenderMask& debugRenderMask(AppGlobal::getDebugRenderMask());
		
		if (debugRenderMask.checkFlag(DebugRender_LevelBorder))
		{
			m_levelBorder->render();
		}
#endif
		
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_InFrontOfShoeboxZeroOne]->render();
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_InFrontOfShoeboxZeroOne);
		
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_InFrontOfShoeboxZeroTwo]->render();
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_InFrontOfShoeboxZeroTwo);
		
		if (m_stopLightRenderingOnSplit == false)
		{
			m_lightMgr->endLightRender(m_shadowMaskShoebox, visibilityRect);
		}
	}

	if(editorIsOpen == false)
	{
		m_fluidMgr->renderLavaGlow();
	}

	if (editorIsOpen == false)
	{
		m_lightMgr->renderLightGlows();
	}
	
	START_RENDER_SECTION(FrameRenderSection_Shoebox);
	if (layersVisible.checkFlag(GameLayer_ShoeboxForeground) && m_shoeboxSkinAndEnvironment != 0)
	{
		m_shoeboxSkinAndEnvironment->renderForeground();
	}
	
	if (renderer->getDebug()->getCaptureMode() != tt::engine::debug::DebugRenderer::ScreenCaptureMode_None &&
		editorIsOpen == false)
	{
		m_lightMgr->restoreAlphaChannel();
	}
	
	START_RENDER_SECTION(FrameRenderSection_Misc);
	// Fog should be disabled when rendering debug graphics, the HUD and the editor
	renderer->setFogEnabled(false);
	
	START_RENDER_SECTION(FrameRenderSection_LightMgr);
	if (editorIsOpen == false)
	{
		m_lightMgr->debugRender();
	}
	
	if (editorIsOpen == false)
	{
		START_RENDER_SECTION(FrameRenderSection_EntityMgr);
		m_entityMgr->renderEntitiesDebug();
		
		if (m_colorGradingAfterHud == false)
		{
			applyColorGrading();
		}
	}
	
	START_RENDER_SECTION(FrameRenderSection_Misc);
	m_pathMgr.render();
	
	if (m_debugView != 0)
	{
		m_debugView->render();
	}
	
	if (editorIsOpen == false)
	{
		m_tileRegistrationMgr->render();
		
		START_RENDER_SECTION(FrameRenderSection_Presentation);
		m_presentationMgr[ParticleLayer_BehindHud]->render();
		
		if (isRenderingToDRC)
		{
			m_presentationMgr[ParticleLayer_BehindHud]->renderPass("drc_only");
			m_presentationMgr[ParticleLayer_BehindHudDrcOnly]->render();
		}
		else
		{
			m_presentationMgr[ParticleLayer_BehindHud]->renderPass("tv_only");
			m_presentationMgr[ParticleLayer_BehindHudTvOnly]->render();
		}
		
		if (isRenderingMainCam)
		{
			m_presentationMgr[ParticleLayer_BehindHud]->renderPass("main_only");
		}
		else
		{
			m_presentationMgr[ParticleLayer_BehindHud]->renderPass("sub_only");
		}
		
		START_RENDER_SECTION(FrameRenderSection_Particles);
		particleMgr->renderGroup(ParticleRenderGroup_BehindHud);
		if (isRenderingToDRC)
		{
			particleMgr->renderGroup(ParticleRenderGroup_BehindHudDrcOnly);
		}
		else
		{
			particleMgr->renderGroup(ParticleRenderGroup_BehindHudTvOnly);
		}
		
#if !defined(TT_BUILD_FINAL)  // disable notes in final builds
		// Render the level notes (only show notes for user levels, or in developer mode)
		if (layersVisible.checkFlag(GameLayer_Notes) &&
		    (AppGlobal::isInDeveloperMode() || m_startInfo.isUserLevel()))
		{
			const level::Notes& notes(m_levelData->getAllNotes());
			for (level::Notes::const_iterator it = notes.begin(); it != notes.end(); ++it)
			{
				(*it)->render();
			}
		}
#endif
		
		if (AppGlobal::shouldTakeLevelScreenshot() == false)
		{
			renderHud(isRenderingToDRC, isRenderingMainCam);
		}
	}
	START_RENDER_SECTION(FrameRenderSection_Misc);
	
	if (isEditorOpen() == false)
	{
		if (m_colorGradingAfterHud)
		{
			applyColorGrading();
		}
		
		if (isRenderingToDRC)
		{
			getDrcCamera().renderDebug();
		}
		else
		{
			getCamera().renderDebug();
		}
	}
	
	if (renderingEditor)
	{
		m_editor->render();
	}
	
	STOP_RENDER_FRAME();
	
	renderDebug(isRenderingToDRC);
	
	// Make sure the alpha is written when the backbuffer is cleared (OpenGL)
	renderer->setColorMask(tt::engine::renderer::ColorMask_All);
}


void Game::onResetDevice()
{
	tt::engine::renderer::Renderer::getInstance()->setZBufferEnabled(false);
	tt::engine::renderer::Renderer::getInstance()->setCullingEnabled(false);
	
	if (m_debugUI != nullptr)
	{
		m_debugUI->onResetDevice();
	}
	
	if (m_editor != 0)
	{
		m_editor->onResetDevice();
	}
	
	if (AppOptions::getInstance().setWindowed(tt::app::getApplication()->isFullScreen() == false))
	{
		// Windowed setting changed!
		if (m_entityMgr != 0)
		{
			m_entityMgr->callFunctionOnAllEntities("onFullScreenSwitched");
		}
	}
}


void Game::onRequestReloadAssets()
{
	reloadGame(false, true);
	
	if (m_colorGradingEffectMgr != 0)
	{
		m_colorGradingEffectMgr->onRequestReloadAssets();
	}
}


void Game::onPreDestroy()
{
	m_assetMonitor.reset();
	m_editor.reset();
}


void Game::handleLevelResized()
{
	m_levelBorder->fitAroundRectangle(level::tileToWorld(m_levelData->getLevelRect()));
	m_tileRegistrationMgr->handleLevelResized();
	m_fluidMgr->handleLevelResized();
	m_lightMgr->handleLevelResized();
	m_darknessMgr->handleLevelResized();
	
	level::AttributeLayerPtr attribLayer(m_levelData->getAttributeLayer());
	m_levelSkinContext->handleLevelResized(attribLayer->getWidth(), attribLayer->getHeight());
	
	m_reloadEntities = true;
}


void Game::handleLevelBackgroundPicked()
{
	TT_NULL_ASSERT(m_levelData);
	if (m_levelData == 0 ||
	    m_levelData->getLevelBackground() == m_levelBackgroundOnEditorOpen)
	{
		// No level data (which is an error!) or selection didn't change
		return;
	}
	
	loadUserLevelShoeboxData();
	createShoeboxesFromDataInclSkin();
	
	m_levelBackgroundOnEditorOpen = m_levelData->getLevelBackground();
}


void Game::showLoadLevelDialog()
{
	setEditorOpen(true);
	TT_NULL_ASSERT(m_editor);
	if (m_editor != 0)
	{
		m_editor->showLoadLevelDialog();
	}
}


void Game::onEditorWillOpen()
{
	releaseDraggingEntity(); // Release entity when opening editor.
	
	// Get a snapshot of the current entities in LevelData (for comparison when closing editor)
	m_entitiesInLevelDataOnEditorOpen = level::entity::EntityInstanceSet(
			m_levelData->getAllEntities().begin(),
			m_levelData->getAllEntities().end());
	
	m_levelBackgroundOnEditorOpen = m_levelData->getLevelBackground();
	m_levelAttributeLayerOnEditorOpen = m_levelData->getAttributeLayer()->clone();
	
	m_fluidMgr->setFluidLayerVisible(false);
	
	m_spacebarScrollMode.reset();
	
	// Set the editor camera position to the game camera position
	m_editor->getEditorCamera().setPosition(getInputCamera().getTargetPosition(), true);
}


void Game::onEditorClosed(bool p_restartLevel)
{
	// Set the game camera position to the editor camera position
	getInputCamera().setPosition(m_editor->getEditorCamera().getTargetPosition(), true);
	
	// Sync the run-time entities based on editor changes
	if (m_reloadEntities || m_editor->hasEditorWarnings() || p_restartLevel)
	{
		// We want a 'clear' start when doing a play test, so only restore position in the other cases.
		const bool restoreControllingEntityPosition = (p_restartLevel == false);
		reloadGame(true, restoreControllingEntityPosition);
		m_reloadEntities = false;
	}
	else if (m_editor->shouldAutoSyncEntityChanges())
	{
		syncRuntimeEntitiesWithLevelData();
	}
	
	// We need to regenerate level skin because tiles might have changed.
	// but we can't only reset that part of the shoebox.
	// So we'll have to recreate the whole shoebox.
	// We can restore blur layers, but not any tags stared/stopped/hidden/etc.
	callOnProgressRestoredOnAllEntities("editor");
	
	// Load new user level shoebox if the setting changed
	TT_NULL_ASSERT(m_levelData);
	if (m_levelData != 0 &&
	    m_levelData->getLevelBackground() != m_levelBackgroundOnEditorOpen)
	{
		loadUserLevelShoeboxData();
	}
	
	// Compare attrib layer to see if we need to recreate the tile caches and levelshoebox
	TT_NULL_ASSERT(m_levelAttributeLayerOnEditorOpen);
	if (m_levelData->getAttributeLayer()->equals(m_levelAttributeLayerOnEditorOpen) == false)
	{
		createShoeboxesFromDataInclSkin();
		buildPathFindingData();
	}
	
	m_fluidMgr->resetSimulation(true);
	m_fluidMgr->setFluidLayerVisible(true);
	
	m_spacebarScrollMode.reset();
	
	// Cleanup unreferences entities
	level::entity::removeUnreferencedEntityReferences(m_levelData);
	
	// Open the editor when there were warnings.
	if (m_editorWarnings.empty() == false)
	{
		setEditorOpen(true);
	}
}


void Game::onEditorReloadAll()
{
	reloadGame(false, false);
	
	if (m_colorGradingEffectMgr != 0)
	{
		m_colorGradingEffectMgr->onRequestReloadAssets();
	}
}


void Game::clearEditorWarnings()
{
	if (m_editor == 0)
	{
		return;
	}
	
	m_editorWarnings.clear(); // We should not have any pending warnings, but just in case clear them anyway.
	m_editor->clearEditorWarnings();
}


void Game::editorWarning(const entity::EntityHandle& p_sourceEntity, const std::string& p_warningStr)
{
	// Store the editor warnings untill we open the editor and pass them.
	const s32 entityId = m_entityMgr->getEntityIDByHandle(p_sourceEntity);
	TT_Printf("Game::editorWarning - ID: %d, warning: '%s'\n", entityId, p_warningStr.c_str());
	
	m_editorWarnings.push_back(EditorWarning(entityId, p_warningStr));
}


bool Game::handleUserLevelCompleted()
{
	TT_Printf("userLevelCompleted is called.\n"); // Request from Hessel to print this so he knows the function is called.
	
	// User level completion is mostly handled by script.
	// We only need to know that it was completed in editor mode
	if (AppGlobal::isInLevelEditorMode() && m_editor != 0)
	{
		TT_ASSERT(m_editor->hasEditorWarnings() == false); // If the editor has warnings it should stop playing the game and return to the editor.
		setEditorOpen(true, m_editor->isDoingPlayTest());
		return true; // We handled it.
	}
	return false;
}


bool Game::isDoingPlayTest() const
{
	return (m_editor != 0) ? m_editor->isDoingPlayTest() : false;
}


#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
bool Game::saveLevelSkinAsXML(bool p_generateSkin) const
{
	tt::engine::scene2d::shoebox::ShoeboxDataPtr skinData =
			(p_generateSkin) ? generateLevelSkinData() : m_shoeboxDataLevelSkin;
	
	if (skinData == 0)
	{
		TT_PANIC("Cannot save level skin as XML file: no skin shoebox data is available.");
		return false;
	}
	
	TT_ERR_CREATE("Save level skin shoebox as XML.");
	std::string sourcePath(tt::app::getApplication()->getAssetRootDir() +
	                       "../../source/shared/levels/skin/" + m_startInfo.getLevelName() + ".xml");
	sourcePath = tt::fs::utils::compactPath(sourcePath, "\\/");
	skinData->saveAsXML(sourcePath, true, &errStatus);
	
	TT_ERR_ASSERT_ON_ERROR();
	return errStatus.hasError() == false;
}
#endif


Minimap& Game::getMinimap()
{
	TT_NULL_ASSERT(m_minimap);
	return *m_minimap;
}


const Minimap& Game::getMinimap() const
{
	TT_NULL_ASSERT(m_minimap);
	return *m_minimap;
}


void Game::setStartInfo(const StartInfo& p_newStartInfo)
{
	m_startInfo = p_newStartInfo;
	// FIXME: Do we need to do anything to keep the current game in a sane state? The level name changed here...
}


void Game::scheduleScreenshot(const ScreenshotSettings& p_settings, real p_delay)
{
	m_scheduledScreenshot      = p_settings;
	m_scheduledScreenshotDelay = p_delay;
}


void Game::unscheduleScreenshot()
{
	m_scheduledScreenshot      = ScreenshotSettings();
	m_scheduledScreenshotDelay = -1.0f;
}


const level::AttributeLayerPtr& Game::getAttributeLayer()
{
	return m_levelData->getAttributeLayer();
}


#if defined(TT_STEAM_BUILD)

void Game::createWorkshopLevelPicker()
{
	if (m_workshopLevelPicker == 0)
	{
		m_workshopLevelPicker = hud::WorkshopLevelPicker::create();
	}
}


void Game::openWorkshopLevelPicker()
{
	createWorkshopLevelPicker();
	
	TT_NULL_ASSERT(m_workshopLevelPicker);
	if (m_workshopLevelPicker != 0)
	{
		m_workshopLevelPicker->open();
	}
}


void Game::closeWorkshopLevelPicker()
{
	if (m_workshopLevelPicker != 0)
	{
		m_workshopLevelPicker->close();
	}
}

#endif  // defined(TT_STEAM_BUILD)


void Game::createResolutionPicker()
{
	if (m_resolutionPicker == 0)
	{
		m_resolutionPicker = hud::ResolutionPicker::create();
	}
}


void Game::openResolutionPicker()
{
	createResolutionPicker();
	
	TT_NULL_ASSERT(m_resolutionPicker);
	if (m_resolutionPicker != 0)
	{
		m_resolutionPicker->open();
	}
}


void Game::closeResolutionPicker()
{
	if (m_resolutionPicker != 0)
	{
		m_resolutionPicker->close();
	}
}


entity::effect::EffectMgr& Game::getEffectMgr()
{
	return getEntityMgr().getEffectRectMgr().getEffectMgr();
}


entity::effect::ColorGradingEffectMgr& Game::getColorGradingEffectMgr()
{
	TT_NULL_ASSERT(m_colorGradingEffectMgr);
	return *m_colorGradingEffectMgr;
}


const entity::effect::ColorGradingEffectMgr& Game::getColorGradingEffectMgr() const
{
	TT_NULL_ASSERT(m_colorGradingEffectMgr);
	return *m_colorGradingEffectMgr;
}


entity::effect::FogEffectMgr& Game::getFogEffectMgr()
{
	TT_NULL_ASSERT(m_fogEffectMgr);
	return *m_fogEffectMgr;
}


const entity::effect::FogEffectMgr& Game::getFogEffectMgr() const
{
	TT_NULL_ASSERT(m_fogEffectMgr);
	return *m_fogEffectMgr;
}


void Game::addScreenSpaceEntity(const entity::EntityHandle& p_controllingEntity)
{
#if !defined(TT_BUILD_FINAL)
	{
		for (entity::EntityHandles::iterator it = m_screenSpaceEntities.begin();
		     it != m_screenSpaceEntities.end(); ++it)
		{
			if ((*it) == p_controllingEntity)
			{
				TT_NONFATAL_PANIC("Entity 0x%08X was already added as a screenspace entity.",
				                  p_controllingEntity.getValue());
			}
		}
	}
#endif
	
	m_screenSpaceEntities.push_back(p_controllingEntity);
}


void Game::removeScreenSpaceEntity(const entity::EntityHandle& p_controllingEntity)
{
	for (entity::EntityHandles::iterator it = m_screenSpaceEntities.begin();
	     it != m_screenSpaceEntities.end(); ++it)
	{
		if ((*it) == p_controllingEntity)
		{
			m_screenSpaceEntities.erase(it);
			return;
		}
	}
	
	TT_PANIC("Couldn't find entity while removing screenspace entity.");
}


void Game::addButtonInputListeningEntity(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking)
{
#if !defined(TT_BUILD_FINAL)
	{
		for (PrioritizedInputListeners::const_iterator it = m_buttonInputListeningEntities.begin();
		     it != m_buttonInputListeningEntities.end(); ++it)
		{
			if (it->entity == p_entity)
			{
				TT_NONFATAL_PANIC("Entity 0x%08X was already added as a button input listening entity.",
				                  p_entity.getValue());
			}
		}
	}
#endif
	
	m_buttonInputListeningEntities.push_back(InputListener(p_entity, p_priority, p_isBlocking));
	
	// Now sort it
	std::sort(m_buttonInputListeningEntities.begin(), m_buttonInputListeningEntities.end());
}


void Game::removeButtonInputListeningEntity(const entity::EntityHandle& p_entity)
{
	for (PrioritizedInputListeners::iterator it = m_buttonInputListeningEntities.begin();
	     it != m_buttonInputListeningEntities.end(); ++it)
	{
		if (it->entity == p_entity)
		{
			m_buttonInputListeningEntities.erase(it);
			return;
		}
	}
	
	// Do not panic here; calling code might find it easier to call removeInputListeningEntity multiple times
	TT_WARN("Couldn't find entity while removing button input listening entity.");
}


void Game::removeAllButtonInputListeningEntities()
{
	m_buttonInputListeningEntities.clear();
}


void Game::addMouseInputListeningEntity(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking)
{
#if !defined(TT_BUILD_FINAL)
	{
		for (PrioritizedInputListeners::const_iterator it = m_mouseInputListeningEntities.begin();
		     it != m_mouseInputListeningEntities.end(); ++it)
		{
			if (it->entity == p_entity)
			{
				TT_NONFATAL_PANIC("Entity 0x%08X was already added as a mouse input listening entity.",
				                  p_entity.getValue());
			}
		}
	}
#endif
	
	if (m_mouseInputListeningEntities.empty() && // If we're adding the first entity to our listening entities, and 
	    m_pressingEntities.empty() == false)   // we're still pressing entities.
	{
		// Release all currently pressing entities.
		const tt::math::Vector2 invalidPosition(std::numeric_limits<real>::max(), std::numeric_limits<real>::max());
		updatePressingEntities(0.0f, invalidPosition, invalidPosition, true);
	}
	
	m_mouseInputListeningEntities.push_back(InputListener(p_entity, p_priority, p_isBlocking));
	
	// Now sort it
	std::sort(m_mouseInputListeningEntities.begin(), m_mouseInputListeningEntities.end());
}


void Game::removeMouseInputListeningEntity(const entity::EntityHandle& p_entity)
{
	for (PrioritizedInputListeners::iterator it = m_mouseInputListeningEntities.begin();
	     it != m_mouseInputListeningEntities.end(); ++it)
	{
		if (it->entity == p_entity)
		{
			const entity::EntityHandle entityHandle = it->entity;
			m_mouseInputListeningEntities.erase(it);
			
			// Check if this entity was being pressed. (If found call released.)
			for (PressingEntities::iterator pressingIt = m_pressingEntities.begin();
			     pressingIt != m_pressingEntities.end(); ++pressingIt)
			{
				if (entityHandle == pressingIt->entityHandle)
				{
					// Send entity a "pointer released" event.
					const tt::math::Vector2 invalidPosition(std::numeric_limits<real>::max(), std::numeric_limits<real>::max());
					toki::game::event::input::PointerEvent pointerEvent(invalidPosition, (*pressingIt).pressDuration, false);
					
					entity::Entity* pressingEntity = (*pressingIt).entityHandle.getPtr();
					m_pressingEntities.erase(pressingIt);
					if (pressingEntity != 0 && pressingEntity->isInitialized())
					{
						pressingEntity->getEntityScript()->onPointerReleased(
								script::wrappers::PointerEventWrapper(pointerEvent));
					}
					break;
				}
			}
			
			return;
		}
	}
	
	// Do not panic here; calling code might find it easier to call removeMouseInputListeningEntity multiple times
	TT_WARN("Couldn't find entity while removing mouse input listening entity.");
}


void Game::removeAllMouseInputListeningEntities()
{
	m_mouseInputListeningEntities.clear();
	
	// Release all currently pressing entities.
	const tt::math::Vector2 invalidPosition(std::numeric_limits<real>::max(), std::numeric_limits<real>::max());
	updatePressingEntities(0.0f, invalidPosition, invalidPosition, true);
}


void Game::addKeyboardListeningEntity(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking)
{
#if !defined(TT_BUILD_FINAL)
	{
		for (PrioritizedInputListeners::const_iterator it = m_keyboardListeningEntities.begin();
		     it != m_keyboardListeningEntities.end(); ++it)
		{
			if (it->entity == p_entity)
			{
				TT_NONFATAL_PANIC("Entity 0x%08X was already added as a keyboard listening entity.",
				                  p_entity.getValue());
			}
		}
	}
#endif
	
	m_keyboardListeningEntities.push_back(InputListener(p_entity, p_priority, p_isBlocking));
	
	// Now sort it
	std::sort(m_keyboardListeningEntities.begin(), m_keyboardListeningEntities.end());
}


void Game::removeKeyboardListeningEntity(const entity::EntityHandle& p_entity)
{
	for (PrioritizedInputListeners::iterator it = m_keyboardListeningEntities.begin();
	     it != m_keyboardListeningEntities.end(); ++it)
	{
		if (it->entity == p_entity)
		{
			m_keyboardListeningEntities.erase(it);
			return;
		}
	}
	
	// Do not panic here; calling code might find it easier to call removeKeyboardListeningEntity multiple times
	TT_WARN("Couldn't find entity while removing keyboard listening entity.");
}


void Game::removeAllKeyboardListeningEntities()
{
	m_keyboardListeningEntities.clear();
}


tt::math::Vector3 Game::getShoeboxSpaceFromWorldPos(const tt::math::Vector2& p_worldPosition) const
{
	TT_NULL_ASSERT(m_levelData);
	const s32 levelWidth  = static_cast<s32>(level::tileToWorld(m_levelData->getLevelWidth()));
	const s32 levelHeight = static_cast<s32>(level::tileToWorld(m_levelData->getLevelHeight()));
	return tt::math::Vector3(  p_worldPosition.x - (levelWidth  * 0.5f) , 
	                         -(p_worldPosition.y - (levelHeight * 0.5f)), 
	                         0.0f);
}


level::ThemeType Game::getThemeAtTilePosition(const tt::math::Point2& p_position) const
{
	level::ThemeTiles::const_iterator it = m_levelOverriddenThemeTiles.find(p_position);
	if (it != m_levelOverriddenThemeTiles.end())
	{
		return (*it).second;
	}
	
	if (m_levelData->getLevelRect().contains(p_position) == false)
	{
		TT_PANIC("Game::getThemeAtTilePosition: position (%d, %d) lies outside level border",
			p_position.x, p_position.y);
		return level::ThemeType_Invalid;
	}
	
	const level::AttributeLayerPtr& attribs(m_levelData->getAttributeLayer());
	level::ThemeType result = attribs->getThemeType(p_position);
	
	return (result == level::ThemeType_UseLevelDefault) ? m_levelData->getLevelTheme() : result;
}


const level::skin::TileMaterial& Game::getTileMaterial(const tt::math::Point2& p_tile) const
{
	if (m_levelSkinContext != 0)
	{
		return m_levelSkinContext->tileMaterial.getTileMaterial(p_tile);
	}
	else
	{
		static const level::skin::TileMaterial empty;
		return empty;
	}
}


void Game::serializeGameState(const std::string& p_id)
{
	TT_WARNING(m_serializationAction == SerializationAction_None,
			"Serialization action '%d' already pending. Overwriting.", m_serializationAction);
	
	m_serializationAction       = SerializationAction_Serialize;
	m_serializationProgressType = ProgressType_Invalid;
	
	if(p_id.empty() == false)
	{
		m_serializationID = p_id;
	}
	else
	{
		TT_WARN("No ID for checkpoint specified!");
		m_serializationID = "Anonymous";
	}
}


void Game::unserializeGameState(const std::string& p_id, bool p_removeCheckpointAfterRestore, ProgressType p_progressType)
{
	TT_WARNING(m_serializationAction == SerializationAction_None,
			"Serialization action '%d' already pending. Overwriting.", m_serializationAction);
	
	TT_ASSERT(p_id.empty() == false);
	
	if(p_id.empty() == false)
	{
		m_serializationAction                 = SerializationAction_Unserialize;
		m_serializationRemoveAfterUnserialize = p_removeCheckpointAfterRestore;
		m_serializationID                     = p_id;
		m_serializationProgressType           = p_progressType;
	}
}


void Game::queueShoeboxTagEvent(const std::string& p_tag, const std::string& p_event, const std::string& p_param)
{
	ShoeboxTagEvent tagEvent;
	tagEvent.tag   = p_tag;
	tagEvent.event = p_event;
	tagEvent.param = p_param;

	m_eventsMutex.lock();

	m_queuedEvents.push_back(tagEvent);
	
	m_eventsMutex.unlock();
}


void Game::setMissionID(const std::string& p_missionID)
{
	m_startInfo.setMissionID(p_missionID);
}


void Game::resetAndDisableDebugFOV()
{
	m_debugFOVDelta = 0.0f;
	getCamera().setDebugFOVEnabled(false);
}


void Game::onGameStarted()
{
	TT_Printf("Game::onGameStarted()\n");
}


void Game::onGameEnded()
{
	TT_Printf("Game::onGameEnded()\n");
}


void Game::onGamePaused()
{
	TT_Printf("Game::onGamePaused()\n");
}


void Game::onGameResumed()
{
	TT_Printf("Game::onGameResumed()\n");
}


void Game::onGameProgressChanged(s32 /*p_progress*/)
{
	//TT_Printf("Game::onGameProgressChanged(%d)\n", p_progress);
}


bool Game::onGamePastIntro()
{
	TT_Printf("Game::onGamePastIntro()\n");
	return tt::app::getApplication()->onPastIntro();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void Game::setupScriptButtons()
{
	TT_ASSERT(g_scriptButtons.empty());
	
	// Set up the script callback buttons
	const input::Controller::State& inputState(AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	using script::EntityBase;
	
	// Left button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.left,                 // button to respond to
			&EntityBase::onButtonLeftPressed, // pressed callback
			&EntityBase::onButtonLeftReleased // released callback
			));
	
	// Right button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.right,                 // button to respond to
			&EntityBase::onButtonRightPressed, // pressed callback
			&EntityBase::onButtonRightReleased // released callback
			));
	
	// Up button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.up,                 // button to respond to
			&EntityBase::onButtonUpPressed, // pressed callback
			&EntityBase::onButtonUpReleased // released callback
			));
	
	// Down button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.down,                 // button to respond to
			&EntityBase::onButtonDownPressed, // pressed callback
			&EntityBase::onButtonDownReleased // released callback
			));
	
	// VirusUpload button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.virusUpload,                   // button to respond to
			&EntityBase::onButtonVirusUploadPressed,   // pressed callback
			&EntityBase::onButtonVirusUploadReleased   // released callback
			));
	
	// Menu button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.menu,                   // button to respond to
			&EntityBase::onButtonMenuPressed,   // pressed callback
			&EntityBase::onButtonMenuReleased   // released callback
			));
	
	// Screen switch button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.screenSwitch,                   // button to respond to
			&EntityBase::onButtonScreenSwitchPressed,   // pressed callback
			&EntityBase::onButtonScreenSwitchReleased   // released callback
			));
	
	// Accept button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.accept,                   // button to respond to
			&EntityBase::onButtonAcceptPressed,   // pressed callback
			&EntityBase::onButtonAcceptReleased   // released callback
			));
	
	// Cancel button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.cancel,                   // button to respond to
			&EntityBase::onButtonCancelPressed,   // pressed callback
			&EntityBase::onButtonCancelReleased   // released callback
			));
	
	// FaceUp button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.faceUp,                   // button to respond to
			&EntityBase::onButtonFaceUpPressed,   // pressed callback
			&EntityBase::onButtonFaceUpReleased   // released callback
			));
	
	// FaceLeft button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.faceLeft,                  // button to respond to
			&EntityBase::onButtonFaceLeftPressed,  // pressed callback
			&EntityBase::onButtonFaceLeftReleased  // released callback
			));
	
	// Jump button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.jump,                 // button to respond to
			&EntityBase::onButtonJumpPressed, // pressed callback
			&EntityBase::onButtonJumpReleased // released callback
			));
	
	// Primary Fire button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.primaryFire,                 // button to respond to
			&EntityBase::onButtonPrimaryFirePressed, // pressed callback
			&EntityBase::onButtonPrimaryFireReleased // released callback
			));
	
	// Secondary Fire button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.secondaryFire,                   // button to respond to
			&EntityBase::onButtonSecondaryFirePressed,   // pressed callback
			&EntityBase::onButtonSecondaryFireReleased   // released callback
			));
	
	// Select Weapon 1 button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.selectWeapon1,                   // button to respond to
			&EntityBase::onButtonSelectWeapon1Pressed,   // pressed callback
			0                                            // no released callback
			));
	
	// Select Weapon 2 button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.selectWeapon2,                   // button to respond to
			&EntityBase::onButtonSelectWeapon2Pressed,   // pressed callback
			0                                            // no released callback
			));
	
	// Select Weapon 3 button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.selectWeapon3,                   // button to respond to
			&EntityBase::onButtonSelectWeapon3Pressed,   // pressed callback
			0                                            // no released callback
			));
	
	// Select Weapon 4 button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.selectWeapon4,                   // button to respond to
			&EntityBase::onButtonSelectWeapon4Pressed,   // pressed callback
			0                                            // no released callback
			));
	
	// Toggle Weapons button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.toggleWeapons,                   // button to respond to
			&EntityBase::onButtonToggleWeaponsPressed,   // pressed callback
			0                                            // no released callback
			));
	
	// Respawn "button"
	g_scriptButtons.push_back(ScriptButton(
			&inputState.respawn,                   // button to respond to
			&EntityBase::onButtonRespawnPressed,   // pressed callback
			&EntityBase::onButtonRespawnReleased   // released callback
			));
	
// Debug buttons
#if !defined(TT_BUILD_FINAL)
	// DemoReset button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.demoReset,                 // button to respond to
			&EntityBase::onButtonDemoResetPressed, // pressed callback
			&EntityBase::onButtonDemoResetReleased // released callback
			));
	
	// Cheat button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.debugCheat,                                    // button to respond to
			&EntityBase::onButtonDebugCheatPressed,                    // pressed callback
			&EntityBase::onButtonDebugCheatReleased                    // released callback
			));
	
	// Restart button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.debugRestart,                                  // button to respond to
			&EntityBase::onButtonDebugRestartPressed,                  // pressed callback
			&EntityBase::onButtonDebugRestartReleased                  // released callback
			));
	
	// Previous spawn point
	g_scriptButtons.push_back(ScriptButton(
			&inputState.editor.keys[tt::input::Key_Comma],             // button to respond to
			&EntityBase::onButtonDebugPreviousSpawnPointPressed,       // pressed callback
			&EntityBase::onButtonDebugPreviousSpawnPointReleased       // released callback
			));
	
	// Next spawn point
	g_scriptButtons.push_back(ScriptButton(
			&inputState.editor.keys[tt::input::Key_Period],            // button to respond to
			&EntityBase::onButtonDebugNextSpawnPointPressed,           // pressed callback
			&EntityBase::onButtonDebugNextSpawnPointReleased           // released callback
			));
	
	// Last spawn point
	g_scriptButtons.push_back(ScriptButton(
			&inputState.editor.keys[tt::input::Key_SlashQuestionmark], // button to respond to
			&EntityBase::onButtonDebugLastSpawnPointPressed,           // pressed callback
			&EntityBase::onButtonDebugLastSpawnPointReleased           // released callback
			));
	
	// Generic debug button
	g_scriptButtons.push_back(ScriptButton(
			&inputState.editor.keys[tt::input::Key_U],                 // button to respond to
			&EntityBase::onButtonDebugPressed,                         // pressed callback
			&EntityBase::onButtonDebugReleased                         // released callback
			));
#endif
	
}


void Game::loadLevel(const StartInfo& p_startInfo)
{
	// Load the level data and (optionally) the corresponding shoebox
	if (p_startInfo.isUserLevel() && p_startInfo.getLevelName().empty())
	{
		// Starting (presumably) the editor without having a specific level to load:
		// start with a template level provided with the game
		// NOTE: If loading the template level failed, the normal level handling
		//       will kick in and create a fallback empty level
		m_levelData = level::LevelData::loadLevel("levels/template_userlevel.ttlvl");
		handleLevelChanged();
	}
	else
	{
		loadLevelData(p_startInfo.getLevelFilePath());
	}
	TT_NULL_ASSERT(m_levelData);
}


void Game::loadLevelData(const std::string& p_filename)
{
	std::string filename(p_filename);
	
	// If a file with the new extension exists, prefer that
	if (tt::fs::fileExists(filename) == false)
	{
		TT_PANIC("Level '%s' doesn't exist. Legacy (Ice Age / Toki 1) file extension used", filename.c_str());
		tt::str::replace(filename, ".ttlvl", ".tokilevel");
	}
	
	m_levelData = level::LevelData::loadLevel(filename);
	TT_Printf("Game::loadLevelData: Level data %s (level filename: '%s').\n",
	          (m_levelData != 0) ? "loaded successfully" : "load FAILED", filename.c_str());
	
	handleLevelChanged();
}


void Game::loadUserLevelShoeboxData()
{
	std::string shoeboxPath;
	if (m_levelData != 0 && m_levelData->getLevelBackground().empty() == false)
	{
		shoeboxPath = level::getUserLevelShoeboxPath() + m_levelData->getLevelBackground() + ".shoebox";
	}
	m_shoeboxDataUserLevelBackground = loadShoeboxData(shoeboxPath);
}


void Game::loadLevelScript(const StartInfo& p_startInfo)
{
	// FIXME: Allow for basic user level scripts?
	if (p_startInfo.isUserLevel() == false)
	{
		const std::string scriptFile(p_startInfo.getLevelPath() + p_startInfo.getLevelName());
		if (tt::fs::fileExists(scriptFile + ".nut" ) ||
		    tt::fs::fileExists(scriptFile + ".bnut") )
		{
			toki::script::ScriptMgr::initLevel(scriptFile);
		}
	}
}


void Game::resetParticleMgr()
{
	// Good indicators if something is wrong
	TT_ASSERT(m_shoeboxSkinAndEnvironmentTexMemSize == 0);
	TT_ASSERT(m_shadowMaskShoeboxTexMemSize == 0);
	TT_ASSERT(m_shoeboxSkinAndEnvironment == 0 || m_shoeboxSkinAndEnvironment->hasParticleCache() == false);
	TT_ASSERT(m_shadowMaskShoebox == 0 || m_shadowMaskShoebox->hasParticleCache() == false);
	
	// Kill all particle effects
	if (tt::engine::particles::ParticleMgr::hasInstance())
	{
		tt::engine::particles::ParticleMgr::getInstance()->reset();
	}
}


void Game::handleLevelChanged()
{
	// If loading the level data failed, create an empty level as fallback (this should not fail)
	if (m_levelData == 0)
	{
		m_levelData = level::LevelData::create(50, 20);
		TT_NULL_ASSERT(m_levelData);
		if (m_levelData != 0)
		{
			// Add PlayerBot to the fallback level, so that the player can at least still have some control
			// over the game (such as opening the menu)
			level::entity::EntityInstancePtr player = level::entity::EntityInstance::create(
					"PlayerBot",
					m_levelData->createEntityID(),
					tt::math::Vector2(m_levelData->getLevelWidth() * 0.5f, 0.0f));
			m_levelData->addEntity(player);
		}
		
		// If the level that failed isn't the "restore failure" level, load that level instead
		StartInfo restoreFailureLevel;
		restoreFailureLevel.setLevel(cfg()->getStringDirect("toki.startup.restore_failure_level"));
		if (m_startInfo != restoreFailureLevel)
		{
			AppGlobal::setGameStartInfo(restoreFailureLevel);
			forceReload();
		}
	}
	
	level::AttributeLayerPtr attribLayer(m_levelData->getAttributeLayer());
	if (attribLayer == 0)
	{
		TT_PANIC("Level doesn't contain attriblayer");
		return;
	}
	
	// Tile layer attributes should notify the tile registration manager if a change occurs
	attribLayer->registerObserver(m_tileRegistrationMgr);
	m_tileRegistrationMgr->setLevelLayer(attribLayer);
	
	// (re)create fluidmgr with correct size
#if !defined(TT_BUILD_FINAL)
	m_fluidMgr.reset();  // destroy old instance first, to force reload of textures and other cached resources
#endif
	m_fluidMgr = fluid::FluidMgr::create(attribLayer);
	attribLayer->registerObserver(m_fluidMgr);
	
	if (m_lightMgr != 0)
	{
		// Let lightMgr know about the new level.
		m_lightMgr->setAttributeLayer(attribLayer);
		attribLayer->registerObserver(m_lightMgr);
	}
	
	// Let path mgr know about the level
	//TT_Printf("Game::handleLevelChanged: Generating path finding data.\n");
	loadPathFindingData();
	
	// Now that the (new) level data is available, update all code that depends on it
	m_levelBorder->fitAroundRectangle(level::tileToWorld(m_levelData->getLevelRect()));
	
#if defined(TT_PLATFORM_WIN)
	{
		m_attribDebugView.reset();
		if (m_levelData != 0)
		{
			m_attribDebugView = AttributeDebugView::create(attribLayer,
			                                               AttributeDebugView::ViewMode_CollisionType);
		}
	}
#endif
	
	// New level data could mean the size changed as well: ensure the level skinning context knows about this
	if (m_levelSkinContext != 0)
	{
		m_levelSkinContext->handleLevelResized(attribLayer->getWidth(), attribLayer->getHeight());
	}
	
	if (m_editor != 0)
	{
		m_editor->setLevelData(m_levelData);
	}
	
	AppGlobal::getDemoMgr().setCountdownEnabled(tt::str::startsWith(getLevelData()->getLevelFilename(), "menu_") == false);
}


tt::engine::scene2d::shoebox::ShoeboxDataPtr Game::generateLevelSkinData() const
{
	TT_NULL_ASSERT(m_levelSkinContext);
	TT_NULL_ASSERT(m_levelData);
	
	tt::engine::scene2d::shoebox::ShoeboxDataPtr skinData(
			new tt::engine::scene2d::shoebox::ShoeboxData);
	
	// Generate the shoebox definition data
	level::skin::generateSkinShoebox(
			m_levelSkinContext,
			m_levelData,
			m_levelData->getLevelTheme(),
			m_levelOverriddenThemeTiles,
			skinData.get());
	
	return skinData;
}


tt::engine::renderer::EngineIDToTextures Game::createEmptyShoeboxesWithPreviousSettings()
{
	// Save blur & split priority
	using tt::engine::scene2d::BlurLayers;
	const BlurLayers backLayers = m_shoeboxSkinAndEnvironment != 0 ? m_shoeboxSkinAndEnvironment->getBackBlurLayers() : BlurLayers();
	const BlurLayers foreLayers = m_shoeboxSkinAndEnvironment != 0 ? m_shoeboxSkinAndEnvironment->getForeBlurLayers() : BlurLayers();
	const s32 splitPriority     = m_shoeboxSkinAndEnvironment != 0 ? m_shoeboxSkinAndEnvironment->getSplitPriority() :
	                              cfg()->getIntegerDirect("toki.shoebox.split_priority");
	
	// Store textures
	using tt::engine::renderer::EngineIDToTextures;
	EngineIDToTextures texUsedNow;
	
	if (m_shoeboxSkinAndEnvironment != 0)
	{
		EngineIDToTextures tex(m_shoeboxSkinAndEnvironment->getAllUsedTextures(true));
		texUsedNow.insert(tex.begin(), tex.end());
	}
	
	
	// Create empty main shoebox
	m_shoeboxSkinAndEnvironment.reset(new tt::engine::scene2d::shoebox::Shoebox(tt::pres::PresentationMgrPtr(),
	                                  splitPriority));
	m_shoeboxSkinAndEnvironmentTexMemSize = 0;
	
	if (m_shoeboxSkinAndEnvironment != 0)
	{
		// Restore blur & splitpriority
		m_shoeboxSkinAndEnvironment->setBackBlurLayers(backLayers);
		m_shoeboxSkinAndEnvironment->setForeBlurLayers(foreLayers);
		m_shoeboxSkinAndEnvironment->setSplitPriority(splitPriority);
	}
	
	// Shadow mask shoebox
	if (m_shadowMaskShoebox != 0)
	{
		EngineIDToTextures tex(m_shadowMaskShoebox->getAllUsedTextures(true));
		texUsedNow.insert(tex.begin(), tex.end());
	}
	
	// Create empty shadowmask shoebox
	m_shadowMaskShoebox.reset(new tt::engine::scene2d::shoebox::Shoebox);
	m_shadowMaskShoeboxTexMemSize  = 0;
	return texUsedNow;
}


void Game::createShoeboxesFromDataInclSkin()
{
	// Generate the shoebox definition data
	m_shoeboxDataLevelSkin = generateLevelSkinData();
	TT_NULL_ASSERT(m_shoeboxDataLevelSkin);
	
	using tt::engine::renderer::EngineIDToTextures;
	const EngineIDToTextures previousTextures(createEmptyShoeboxesWithPreviousSettings());
	
	TT_NULL_ASSERT(m_shoeboxSkinAndEnvironment);
	if (m_shoeboxSkinAndEnvironment != 0)
	{
#if !defined(TT_BUILD_FINAL)
		const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		AppGlobal::updateShoeboxTextures(m_shoeboxDataEnvironment, m_shoeboxDataLevelSkin,
		                                 m_shoeboxDataFromScript, m_shoeboxDataIncludesFromScript,
		                                 m_shoeboxDataUserLevelBackground);
		
		addDataToShoebox(m_shoeboxSkinAndEnvironment, m_shoeboxDataEnvironment,         true);
		addDataToShoebox(m_shoeboxSkinAndEnvironment, m_shoeboxDataUserLevelBackground, true);
		addDataToShoebox(m_shoeboxSkinAndEnvironment, m_shoeboxDataLevelSkin,           false);
		addDataToShoebox(m_shoeboxSkinAndEnvironment, m_shoeboxDataFromScript,          true);
		addDataToShoebox(m_shoeboxSkinAndEnvironment, m_shoeboxDataIncludesFromScript,  true);
		
#if !defined(TT_BUILD_FINAL)
		const u64 duration = tt::system::Time::getInstance()->getMilliSeconds() - startTime;
		TT_Printf("Game::createShoeboxesFromDataInclSkin shoebox creation duration: %u ms\n", static_cast<u32>(duration));
		
		EngineIDToTextures texUsedNow(m_shoeboxSkinAndEnvironment->getAllUsedTextures(true));
		
		for (EngineIDToTextures::iterator it = texUsedNow.begin();
		     it != texUsedNow.end(); ++it)
		{
			m_shoeboxSkinAndEnvironmentTexMemSize += (*it).second->getMemSize();
		}
#endif
	}
	
	// Create shadowMaskShoebox from data
	TT_NULL_ASSERT(m_shadowMaskShoebox);
	if (m_shadowMaskShoebox != 0)
	{
		// Load shoebox lightmasks that are set from script
		{
			using tt::engine::scene2d::shoebox::ShoeboxData;
			m_shadowMaskShoeboxData.reset();
			for (ShoeboxData::Includes::const_iterator it = m_shoeboxDataIncludesFromScript->includes.begin();
				 it != m_shoeboxDataIncludesFromScript->includes.end(); ++it)
			{
				if (tt::fs::fileExists((*it).filename + "_lightmask.shoebox"))
				{
					if (m_shadowMaskShoeboxData == 0)
					{
						m_shadowMaskShoeboxData.reset(new ShoeboxData());
					}
					tt::engine::scene2d::shoebox::IncludeData lightmaskInclude(*it);
					lightmaskInclude.filename += "_lightmask";
					m_shadowMaskShoeboxData->includes.push_back(lightmaskInclude);
				}
			}
		}
		
		AppGlobal::updateLightmaskShoeboxTextures(m_shadowMaskShoeboxData);
		if (m_shadowMaskShoeboxData == 0)
		{
			// No shadowmask; reset the shoeboxr
			m_shadowMaskShoebox.reset();
		}
		else
		{
			addDataToShoebox(m_shadowMaskShoebox, m_shadowMaskShoeboxData, true);
		}
		
		if (m_shadowMaskShoebox != 0)
		{
			if (m_shadowMaskShoebox->hasParticleCache())
			{
				TT_PANIC("One of the lightmask includes has particles. This is not allowed.");
				m_shadowMaskShoebox->clearParticleCache();
			}
			
#if !defined(TT_BUILD_FINAL)
			EngineIDToTextures texUsedNow(m_shadowMaskShoebox->getAllUsedTextures(true));
			
			for (EngineIDToTextures::iterator it = texUsedNow.begin();
			     it != texUsedNow.end(); ++it)
			{
				m_shadowMaskShoeboxTexMemSize += (*it).second->getMemSize();
			}
#endif
		}
	}
}


void Game::buildPathFindingData()
{
	m_pathMgr.reset();
	
	m_pathMgr.buildTileCaches(m_levelData->getAttributeLayer());
	
	// FIXME: How to handle path build failure? Can DMCs still re-acquire path agents? Should they?
	if (m_entityMgr != 0)
	{
		m_entityMgr->getMovementControllerMgr().handlePathMgrReset();
	}
	entity::EntityTiles::handlePathMgrReset();
}


void Game::loadPathFindingData()
{
	m_pathMgr.reset();
	
	if (m_levelData->hasPathfindingData())
	{
		m_pathMgr.loadTileCachesFromLevelData(m_levelData);
	}
	else
	{
		TT_NONFATAL_PANIC("No pathfinding data found in leveldata of level '%s'",
		                  m_levelData->getLevelFilename().c_str());
		m_pathMgr.buildTileCaches(m_levelData->getAttributeLayer());
	}
	
	// FIXME: How to handle path build failure? Can DMCs still re-acquire path agents? Should they?
	if (m_entityMgr != 0)
	{
		m_entityMgr->getMovementControllerMgr().handlePathMgrReset();
	}
	entity::EntityTiles::handlePathMgrReset();
}


tt::engine::scene2d::shoebox::ShoeboxDataPtr Game::loadShoeboxData(const std::string& p_filename)
{
	TT_NULL_ASSERT(m_levelData);
	if (m_levelData == 0)
	{
		return tt::engine::scene2d::shoebox::ShoeboxDataPtr();
	}
	
	tt::engine::scene2d::shoebox::ShoeboxDataPtr result;
	
	if (tt::fs::fileExists(p_filename))
	{
		TT_ERR_CREATE("Loading file '" << p_filename << "'");
		
		result = tt::engine::scene2d::shoebox::ShoeboxData::parse(p_filename, &errStatus);
		TT_ERR_ASSERT_ON_ERROR();
		if (errStatus.hasError())
		{
			return tt::engine::scene2d::shoebox::ShoeboxDataPtr();
		}
		
		// If ShoeboxData load succeeded (did not set an error in ErrorStatus),
		// the returned pointer should not be null
		TT_NULL_ASSERT(result);
	}
	
	return result;
}


void Game::addDataToShoebox(const tt::engine::scene2d::shoebox::ShoeboxPtr& p_shoebox,
                            const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_data,
                            bool p_useLevelSize)
{
	if (p_shoebox == 0 || p_data == 0)
	{
		return;
	}
	
	if (p_useLevelSize)
	{
		p_shoebox->create(*p_data,
		                  static_cast<s32>(level::tileToWorld(m_levelData->getLevelWidth())),
		                  static_cast<s32>(level::tileToWorld(m_levelData->getLevelHeight())),
		                  1.0f,
		                  tt::math::Vector3(0.0f, -level::tileToWorld(m_levelData->getLevelHeight()), 0.0f));
	}
	else
	{
		p_shoebox->create(*p_data, 0, 0);
	}
}


void Game::createLevelEntities(s32                      p_overridePositionEntityID,
                               const tt::math::Vector2& p_overridePosition,
                               bool                     p_gameReloaded)
{
	// Create presentation object manager
	// FIXME: This shouldn't be in createLevelEntities
	enum { reservedPresentationObjects = 2048 };
	m_presentationObjectMgr.reset(new pres::PresentationObjectMgr(reservedPresentationObjects));
	
	enum { maxEntities = 2048 };  // FIXME: Get the max number entities needed from somewhere else.
	
	m_entityMgr.reset(new entity::EntityMgr(maxEntities));
	m_levelOverriddenThemeTiles.clear();
	m_shoeboxDataIncludesFromScript->clear();
	
	if (m_levelData == 0)
	{
		// No level, no entities.
		return;
	}
	
	getTileRegistrationMgr().reset();
	
	AppGlobal::getEntityScriptMgr().reset();
	
	releaseDraggingEntity();
	m_pressingEntities.clear();
	m_screenSpaceEntities.clear();
	m_buttonInputListeningEntities.clear();
	m_mouseInputListeningEntities.clear();
	m_keyboardListeningEntities.clear();
	
	m_fluidMgr->resetLevel();
	m_lightMgr->resetLevel();
	m_darknessMgr->resetLevel();
	
	// FIXME: Martijn: Make sure all potential released texures are gone, to prevent double allocations
	// this 'fixes' the occasional 'Failed to allocate memory' bug when reloading level
	// although it doesn't explain why it fails, since there should be more than enough memory available
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
	tt::engine::renderer::Renderer::clearDeathRow();
#endif
	
	if (AppGlobal::getShutdownDataMgr().hasData())
	{
		ShutdownDataMgr& shutdownDataMgr(AppGlobal::getShutdownDataMgr());
		m_serializationID = shutdownDataMgr.getID();
		unserializeAll(*shutdownDataMgr.getData(), m_serializationID);
		shutdownDataMgr.reset();
	}
	else
	{
		m_entityMgr->createSpawnSections(m_levelData->getAllEntities(), m_startInfo.getMissionID(),
		                                 p_gameReloaded);
		m_entityMgr->createEntities(m_levelData->getAllEntities(), m_startInfo.getMissionID(),
		                            p_overridePositionEntityID, p_overridePosition, p_gameReloaded);
		
		m_fluidMgr->resetSimulation(true);
		getTileRegistrationMgr().update(0.0f);
		m_cameraMgr.update(0.0f);
		// Do this after the camera update
		m_entityMgr->initCullingForAllEntities(getCamera());
	}
	
	// Update the fluid manager, so that fluid pregeneration is performed before a checkpoint is saved
	// (the creation of entities has several effects: entities can have their own collision tiles,
	//  changing where fluids can flow; entities can also request that a checkpoint be saved.
	//  The fluid checkpoint data should contain the pregenerated fluids. Therefore, ensure that here).
	// FIXME: Ok, perhaps this change isn't needed. The "fluid pregeneration after restore" bug can no longer be reproduced.
	//m_fluidMgr->update(0.0f);
}


tt::pres::PresentationMgrPtr Game::createPresentationMgr() const
{
	tt::pres::PresentationMgrPtr mgr = tt::pres::PresentationMgr::create(
			tt::pres::Tags(),
			tt::pres::TriggerFactoryInterfacePtr(new pres::TriggerFactory)/*,
			particles::ParticleCategory_Game,
			tt::pres::GroupFactoryInterfacePtr(new tt::pres::GroupFactory<pres::CustomPresObjLess>)*/);
	
	// Map particle layer names (used in presentation files) to render group numbers
	// (which will be rendered by the game)
	for (s32 i = 0; i < ParticleLayer_Count; ++i)
	{
		const ParticleLayer particleLayer = static_cast<ParticleLayer>(i);
		mgr->addParticleLayerMapping(getParticleLayerName(particleLayer),
		                                                  getParticleLayerRenderGroup(particleLayer));
	}
	return mgr;
}


entity::Entity* Game::getDraggingEntity()
{
	entity::Entity* entity = m_entityMgr->getEntity(m_draggingEntity);
	if (entity != 0 && entity->isInitialized() == false)
	{
		releaseDraggingEntity();
		return 0;
	}
	else
	{
		return entity;
	}
}


void Game::releaseDraggingEntity()
{
	entity::Entity* entity = m_entityMgr->getEntity(m_draggingEntity);
	if (entity != 0 && entity->isInitialized())
	{
		if (entity->isSuspended())
		{
			entity->setSuspended(m_draggingEntityRestoreSuspendedState);
		}
		else
		{
			TT_PANIC("Expected dragging entity to be suspended on release, but it wasn't.");
		}
	}
	
	m_draggingEntity.invalidate();
}


void Game::startDraggingEntity(entity::EntityHandle p_handle)
{
	entity::Entity* entity = m_entityMgr->getEntity(p_handle);
	if (entity != 0 && entity->isInitialized())
	{
		m_draggingEntityRestoreSuspendedState = entity->isSuspended();
		entity->setSuspended(true);
		m_draggingEntity = p_handle;
	}
	else
	{
		TT_PANIC("Trying to drag an invalid or uninitalized entity!");
	}
}


void Game::updatePressingEntities(real p_deltaTime,
                                  const tt::math::Vector2& p_screenspacePointerPos,
                                  const tt::math::Vector2& p_pointerWorldPos,
                                  bool p_doRelease)
{
	// Update the press duration for all pressing entities
	// (and send them a pointer released event if pointer was released)
	for (PressingEntities::iterator it = m_pressingEntities.begin(); it != m_pressingEntities.end(); )
	{
		entity::Entity* pressingEntity = m_entityMgr->getEntity((*it).entityHandle);
		if (pressingEntity != 0)
		{
			(*it).pressDuration += p_deltaTime;
			
			if (p_doRelease)
			{
				const tt::math::Vector2& pos = (pressingEntity->isScreenSpaceEntity()) ? p_screenspacePointerPos : p_pointerWorldPos;
				const bool onEntity          = pressingEntity->getWorldRect().contains(pos);
				
				// Send entity a "pointer released" event.
				// NOTE: All entities will be removed from the "pressing entities" list in one go after this loop
				toki::game::event::input::PointerEvent pointerEvent(pos, (*it).pressDuration, onEntity);
				pressingEntity->getEntityScript()->onPointerReleased(
						script::wrappers::PointerEventWrapper(pointerEvent));
			}
			
			++it;
		}
		else
		{
			// Entity is no longer valid: remove it from the list
			it = m_pressingEntities.erase(it);
		}
	}
	
	if (p_doRelease)
	{
		// No longer pressing any entity
		// NOTE: The events will already have been sent in the loop above
		m_pressingEntities.clear();
	}
}


void Game::toggleEditor()
{
	setEditorOpen(isEditorOpen() == false);
}


void Game::setEditorOpen(bool p_open, bool p_userLevelCompleted)
{
	if (p_open == false && m_editor != 0)
	{
		m_editor->hide();
	}
	else if (p_open)
	{
		if (m_editor == 0)
		{
			m_editor = editor::Editor::create(m_levelData);
		}
		
		TT_NULL_ASSERT(m_editor);
		// HACKFIX: m_editor->show repositions the camera; effectively disabling the set position of the editorwarning
		// I fixed this by storing the warning position and setting it after the m_editor->show;
		// FIXME: this is an order problem; fix the correct order properly
		{
			tt::math::Vector2 warningPosition;
			for (EditorWarnings::const_iterator it = m_editorWarnings.begin(); it != m_editorWarnings.end(); ++it)
			{
				const EditorWarning& warning = (*it);
			
				m_editor->editorWarning(warning.id, warning.warningStr);
				
				// Part of hackfix
				level::entity::EntityInstancePtr entity = m_levelData->getEntityByID(warning.id);
				if (entity != 0)
				{
					warningPosition = entity->getPosition();
				}
				// End of hackfix
			}
			m_editor->show(p_userLevelCompleted);
			
			// Part of hackfix
			if (m_editorWarnings.empty() == false)
			{
				m_editor->getEditorCamera().setPosition(warningPosition, true);
			}
			// End of hackfix
			
			m_editorWarnings.clear();
		}
	}
}


bool Game::isEditorOpen() const
{
	return m_editor != 0 && m_editor->isActive();
}


void Game::syncRuntimeEntitiesWithLevelData()
{
	// Compare the current LevelData entities against the snapshot stored on editor open
	using level::entity::EntityInstanceSet;
	EntityInstanceSet entitiesOnEditorClose(
			m_levelData->getAllEntities().begin(), m_levelData->getAllEntities().end());
	
	tt::algorithms::SetIntersectResult<level::entity::EntityInstancePtr> result =
			tt::algorithms::intersectSet(m_entitiesInLevelDataOnEditorOpen, entitiesOnEditorClose);
	
	// Add potentially missing tilecaches
	m_pathMgr.addAndBuildMissingTileCaches(m_levelData);
	
	const EntityInstanceSet& entitiesRemoved(result.onlyInFirst);
	const EntityInstanceSet& entitiesAdded  (result.onlyInSecond);
	
	entity::EntityMgr& entityMgr(getEntityMgr());
	
	// Destroy run-time entities that were removed from the level
	for (EntityInstanceSet::const_iterator it = entitiesRemoved.begin();
	     it != entitiesRemoved.end(); ++it)
	{
		entity::EntityHandle handle(entityMgr.getEntityHandleByID((*it)->getID()));
		entity::Entity* entityToRemove = entityMgr.getEntity(handle);
		if (entityToRemove != 0 &&
		    entityToRemove->getType() == (*it)->getType())
		{
			entityToRemove->kill();
			entityMgr.removeEntityIDFromMapping((*it)->getID());
		}
		
		// Always remove the entity ID from the mapping if the run-time entity was already destroyed
		if (entityToRemove == 0)
		{
			entityMgr.removeEntityIDFromMapping((*it)->getID());
		}
	}
	
	clearEditorWarnings();
	
	// Create run-time entities for newly added entities
	if (entitiesAdded.empty() == false)
	{
		entityMgr.createEntities(level::entity::EntityInstances(entitiesAdded.begin(),
		                                                        entitiesAdded.end()),
		                         m_startInfo.getMissionID());
		
		getTileRegistrationMgr().update(0.0f);
		m_cameraMgr.update(0.0f);
	}
	else if (entitiesRemoved.empty() == false) // No entity was added, but some were removed.
	{
		// Manually call onValidateScriptState callbacks.
		// (in the if above the entityMgr.createEntity call would take care of this.)
		entityMgr.callOnValidateScriptStateOnAllEntities();
	}
}


void Game::reloadGame(bool p_entitiesOnly, bool p_restoreFollowEntityPosition)
{
#if !defined(TT_BUILD_FINAL)
	const u64 timestampStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	// FIXME: p_restoreFollowEntityPosition is used for more than just that; change this to a more sensible name
	if (m_assetMonitor != 0)
	{
		m_assetMonitor->stop();
	}
	
	// Always enable asserts
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
	tt::platform::error::resetAssertMuteMode();
#endif
	
	TT_Printf("Game::reloadGame\n");
	
	callOnReloadRequestedOnAllEntities();
	
	// Force removal of permanent cache objects; otherwise they won't get reloaded
	tt::pres::PresentationCache::setRemovePermanentObjectsEnabled(true);
	
	AppGlobal::setBusyLoading(true);
	
	setDrcCameraEnabled(false);
	m_directionalControlsEnabled = true;
	
	// Save camera follow entity ID and position (so it can be restored after the reload)
	s32               followEntityID = -1;
	tt::math::Vector2 followEntityPos(tt::math::Vector2::zero);
	if (p_restoreFollowEntityPosition)
	{
		const entity::Entity* entity = getCamera().getFollowEntity().getPtr();
		if (entity != 0)
		{
			followEntityID  = m_entityMgr->getEntityIDByHandle(getCamera().getFollowEntity());
			followEntityPos = entity->getPosition();
		}
	}
	
	// Store pre-reload FOV
	const real fov = getCamera().getCurrentFOV();
	
	// Clear the tile registration manager before removing all entities
	getTileRegistrationMgr().reset();
	
	m_pathMgr.reset();
	
	getColorGradingEffectMgr().reset();
	
	// Remove all entities
	m_entityMgr.reset();
	
	// Also ensure the audio is cleaned up
	audio::AudioPlayer::getInstance()->stopAllAudio();
	
	// FIXME: This should be handled in script now
	//        Remove this if working correctly
	//AppGlobal::getCheckPointMgr().resetAllCheckPoints();
	
	m_serializationAction = SerializationAction_None;
	
#if defined(TT_PLATFORM_WIN)
	getDebugView().clear();
#endif
	
	// Reset camera defaults
	getCamera().setFollowSpeedToDefault();
	
	TT_NULL_ASSERT(m_lightMgr);
	m_lightMgr->setDarkLevel(false); // default is disabled. (Script has to turn it on.)
	m_stopLightRenderingOnSplit = true;
	
	// Recreate localization strings
	{
		loc::Loc& loc = AppGlobal::getLoc();
		if (loc.hasLocStr(loc::SheetID_Game))
		{
			loc.destroyLocStr(loc::SheetID_Game);
			loc.createLocStr(loc::SheetID_Game);
		}
		
		if (loc.hasLocStr(loc::SheetID_Editor))
		{
			loc.destroyLocStr(loc::SheetID_Editor);
			loc.createLocStr(loc::SheetID_Editor);
		}
		
		if (loc.hasLocStr(loc::SheetID_Achievements))
		{
			loc.destroyLocStr(loc::SheetID_Achievements);
			loc.createLocStr(loc::SheetID_Achievements);
		}
	}
	
	// Clear the shoeboxes, but keep the textures and settings
	using tt::engine::renderer::EngineIDToTextures;
	EngineIDToTextures previousTexures(createEmptyShoeboxesWithPreviousSettings());
	
	if (p_entitiesOnly == false)
	{
		// For F5, also reset the camera FOV
		getCamera().setFOVToDefault();
		
		// Full reload; clear the textures as well
		previousTexures.clear();
		
		if (m_editor != 0)
		{
			m_editor->releaseResourcesForReload();
		}
		
		// Clear the old cache.
		tt::engine::particles::ParticleMgr::getInstance()->clearTriggerCache();
		
		// Reload the namespace mapping
		tt::engine::file::FileUtils::getInstance()->generateNamespaceMapping();
		
		// Reload the config
		cfg()->reload();
		
		// Reload the glyph sets
		if (p_restoreFollowEntityPosition == false) // Only reload on Ctrl + F5.
		{
			utils::GlyphSetMgr::unloadAll();
			utils::GlyphSetMgr::loadAll();
		}
		
		if (p_restoreFollowEntityPosition == false) // We want to clear the registry on Ctrl + F5.
		{
			script::getRegistry().clear();
		}
		
		tt::stats::destroyInstance();
		
		// Destroy script manager and reload scripts
		toki::script::ScriptMgr::deinit();
		tt::pres::PresentationCache::clear();
		
		if (m_entityMgr != 0)
		{
			m_entityMgr->resetAll();
		}
		if (m_presentationObjectMgr != 0)
		{
			m_presentationObjectMgr->reset();
		}
		entity::graphics::PowerBeamGraphic::reloadConfig();
		
		resetParticleMgr();
		
		toki::script::ScriptMgr::init();
		AppGlobal::getEntityLibrary().rescan();
		// FIXME: Somehow check timestamps on assets?
		
		tt::stats::createInstance(toki::script::ScriptMgr::getVM());
		
		// Reload the level data and shoebox
		if (p_restoreFollowEntityPosition == false) // We want to load the level on Ctrl + F5.
		{
			loadLevel(m_startInfo);
		}
		
		// Reload the skin config and generate a new level skin
		TT_NULL_ASSERT(m_levelSkinContext);
		AppGlobal::destroySkinConfigs();
		AppGlobal::createSkinConfigs();
		{
			level::AttributeLayerPtr attribLayer(m_levelData->getAttributeLayer());
			m_levelSkinContext->handleLevelResized(attribLayer->getWidth(), attribLayer->getHeight());
		}
		
		if (p_restoreFollowEntityPosition == false) // We want to recreate the tilecaches on Ctrl + F5.
		{
			m_pathMgr.recreateTileCaches(m_levelData);
			loadPathFindingData();
		}
		
		// ... reload the GWEN skin
		//
		
		/*
		// Kill all audio
		audio::AudioPlayer::getInstance()->stop(audio::Category_Ambient);
		audio::AudioPlayer::getInstance()->stop(audio::Category_Effects);
		// */
	}
	else
	{
		// Even when doing entities only also create shoebox based on the latest XML. (Added for designers.)
	}
	
	// Recreate the entities
	createLevelEntities(followEntityID, followEntityPos, true);
	
	// generate new level skin data and recreate shoebox.
	createShoeboxesFromDataInclSkin();
	
	if (p_entitiesOnly == false)
	{
		// Load and execute the level script
		loadLevelScript(m_startInfo);
		
		// Get a snapshot of the current entities in LevelData (for comparison when closing editor)
		m_entitiesInLevelDataOnEditorOpen = level::entity::EntityInstanceSet(
				m_levelData->getAllEntities().begin(),
				m_levelData->getAllEntities().end());
		
		// Update the level editor
		if (m_editor != 0)
		{
			m_editor->onRequestReloadAssets();
		}
	}
	
	// Restore camera follow entity handle (based on ID)
	if (followEntityID != -1)
	{
		entity::EntityHandle followHandle = m_entityMgr->getEntityHandleByID(followEntityID);
		entity::Entity*      followEntity = m_entityMgr->getEntity(followHandle);
		if (followEntity != 0)
		{
			getCamera().setFollowEntity(followHandle);
		}
	}
	
	// Restore pre-reload FOV
	if (p_restoreFollowEntityPosition)
	{
		getCamera().setFOV(fov, true);
	}
	
	// Disable removal of permanent cache objects again
	tt::pres::PresentationCache::setRemovePermanentObjectsEnabled(false);
	
	// Call generic onGameReloaded to signal to script that the game has been reloaded
	{
		tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
		vmPtr->callSqFun("onGameReloaded");
	}
	
	if (m_assetMonitor != 0)
	{
		m_assetMonitor->start();
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 timestampEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::reloadGame: took %.2f seconds.\n", (timestampEnd - timestampStart) / 1000.0f);
#endif
}


void Game::renderHud(bool p_isRenderingToDRC, bool p_isRenderingMainCam)
{
	using namespace toki::utils;
	
	START_RENDER_SECTION(FrameRenderSection_Misc);
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	tt::engine::renderer::MatrixStack* stack = tt::engine::renderer::MatrixStack::getInstance();
	
	renderer->beginHud();
	
	stack->setMode(tt::engine::renderer::MatrixStack::Mode_Position);
	stack->push();
	
	stack->translate(tt::math::Vector3( (0.5f * renderer->getScreenWidth( )),
	                                   -(0.5f * renderer->getScreenHeight()), 0.0f));
	stack->uniformScale(renderer->getScreenHeight());
		
	START_RENDER_SECTION(FrameRenderSection_Presentation);
	m_presentationMgr[ParticleLayer_Hud]->render();
	
	if (p_isRenderingToDRC)
	{
		m_presentationMgr[ParticleLayer_Hud]->renderPass("drc_only");
		m_presentationMgr[ParticleLayer_HudDrcOnly]->render();
		m_presentationMgr[ParticleLayer_HudDrcOnly]->renderPass("drc_only");
	}
	else
	{
		m_presentationMgr[ParticleLayer_Hud]->renderPass("tv_only");
		m_presentationMgr[ParticleLayer_HudTvOnly]->render();
		m_presentationMgr[ParticleLayer_HudTvOnly]->renderPass("tv_only");
	}
	
	if (p_isRenderingMainCam)
	{
		m_presentationMgr[ParticleLayer_Hud]->renderPass("main_only");
		//m_presentationMgr[ParticleLayer_HudMainOnly]->render();
		//m_presentationMgr[ParticleLayer_HudMainOnly]->renderPass("main_only");
	}
	else
	{
		m_presentationMgr[ParticleLayer_Hud]->renderPass("sub_only");
		//m_presentationMgr[ParticleLayer_HudSubOnly]->render();
		//m_presentationMgr[ParticleLayer_HudSubOnly]->renderPass("main_only");
	}
	
	START_RENDER_SECTION(FrameRenderSection_Particles);
	tt::engine::particles::ParticleMgr* particleMgr = tt::engine::particles::ParticleMgr::getInstance();
	particleMgr->renderGroup(ParticleRenderGroup_Hud);
	
	if (p_isRenderingToDRC)
	{
		particleMgr->renderGroup(ParticleRenderGroup_HudDrcOnly);
	}
	else
	{
		particleMgr->renderGroup(ParticleRenderGroup_HudTvOnly);
	}
	
#if !defined(TT_BUILD_FINAL)
	const DebugRenderMask& mask = AppGlobal::getDebugRenderMask();
	if (mask.checkFlag(DebugRender_RenderScreenspace))
	{
		START_RENDER_SECTION(FrameRenderSection_EntityMgr);
		for (entity::EntityHandles::const_iterator it = m_screenSpaceEntities.begin();
		     it != m_screenSpaceEntities.end(); ++it)
		{
			entity::Entity* entity = (*it).getPtr();
			if (entity != 0 && entity->isInitialized())
			{
				entity->renderDebug(true);
			}
		}
		renderer->getDebug()->flush(); // HACK: Flush the screenspace debug rects so they use the current matrix stack.
	}
#endif
	
	// If minimap rendering is turned on, render it drc if it's the sub camera.
	if (p_isRenderingMainCam == false && p_isRenderingToDRC       )
	{
		// FIXME: The particles are NOT positioned to a minimap position like the presentation objects are.
		//        ParticleRenderGroup_Minimap is not rendered.
		
		getMinimap().render(m_presentationMgr[ParticleLayer_Minimap]);
	}
	
	START_RENDER_SECTION(FrameRenderSection_Presentation);
	m_presentationMgr[ParticleLayer_InFrontOfHud]->render();
	
	if (p_isRenderingToDRC)
	{
		m_presentationMgr[ParticleLayer_InFrontOfHud]->renderPass("drc_only");
	}
	else
	{
		m_presentationMgr[ParticleLayer_InFrontOfHud]->renderPass("tv_only");
	}
	
	if (p_isRenderingMainCam)
	{
		m_presentationMgr[ParticleLayer_InFrontOfHud]->renderPass("main_only");
	}
	else
	{
		m_presentationMgr[ParticleLayer_InFrontOfHud]->renderPass("sub_only");
	}
	
	START_RENDER_SECTION(FrameRenderSection_Particles);
	particleMgr->renderGroup(ParticleRenderGroup_InFrontOfHud);
	
	stack->pop();
	
	renderer->endHud();
	
#if !defined(TT_BUILD_FINAL)
	if (m_colorTable != 0)
	{
		renderer->beginHud();
		m_colorTable->render();
		renderer->endHud();
	}
#endif
	
	{
		if (m_debugUI != nullptr)
		{
			m_debugUI->render();
		}
		AppGlobal::getInputRecorder()->renderGui();
	}
}


void Game::renderDebug(bool p_isRenderingToDRC) const
{
#if !defined(TT_BUILD_FINAL)
	const toki::DebugRenderMask mask = AppGlobal::getDebugRenderMask();
	if (mask.checkFlag(DebugRender_RenderParticleRects) && isEditorOpen() == false)
	{
		tt::engine::particles::ParticleMgr::getInstance()->renderDebug();
	}
	
	if (mask.checkFlag(DebugRender_SectionProfiler))
	{
		tt::engine::renderer::Renderer::getInstance()->beginHud();
		s32 xPos = 450;
		s32 yPos = 120;
		yPos = m_updateSectionProfiler.render(xPos, yPos, true);
		
		yPos += 20;
		yPos = m_updateForRenderSectionProfiler.render(xPos, yPos, false);
		
#if ENABLE_RENDER_SECTIONS
		yPos += 20;
		yPos = m_renderSectionProfiler.render(xPos, yPos, true);
#endif
		if (mask.checkFlag(DebugRender_SectionProfilerTwo))
		{
			using namespace toki::utils;
			xPos += 20 + (SectionProfiler_renderWidth + (SectionProfiler_renderBorderSize * 2));
			yPos = 120;
			yPos = m_lightMgr->renderProfiler(xPos, yPos);
			yPos += 20;
			yPos = m_fluidMgr->renderProfiler(xPos, yPos);
			yPos += 20;
			yPos = m_entityMgr->renderProfiler(xPos, yPos);
		}
		tt::engine::renderer::Renderer::getInstance()->endHud();
	}
	
	if (p_isRenderingToDRC && isEditorOpen() == false)
	{
		tt::engine::renderer::Renderer* renderer = 
			tt::engine::renderer::Renderer::getInstance();
		
		tt::engine::debug::DebugRendererPtr debug(renderer->getDebug());
		
		debug->renderText("DRC Camera", renderer->getScreenWidth() - 100, 20, 
		                  tt::engine::renderer::ColorRGB::green);
	}
#else
	(void)p_isRenderingToDRC;
#endif // #if !defined(TT_BUILD_FINAL)
}


void Game::serializeAll(toki::serialization::SerializationMgr& p_serializationMgr) const
{
#if !defined(TT_BUILD_FINAL)
	const u64 timestampBegin = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::serializeAll: Starting serialization of checkpoint '%s'...\n", m_serializationID.c_str());
#endif
	
	p_serializationMgr.clearAll();
	
	serialize(p_serializationMgr);
	
	toki::script::ScriptMgr::serialize(p_serializationMgr);
	
	script::getRegistry().serialize(p_serializationMgr);
	script::TimerMgr::serialize(p_serializationMgr);
	if (m_entityMgr != 0)
	{
		m_entityMgr->serialize(p_serializationMgr);
	}
	if (m_presentationObjectMgr != 0)
	{
		m_presentationObjectMgr->serialize(p_serializationMgr);
	}
	if (m_fluidMgr != 0)
	{
		m_fluidMgr->serialize(p_serializationMgr);
	}
	if (m_lightMgr != 0)
	{
		m_lightMgr->serialize(p_serializationMgr);
	}
	if (m_darknessMgr != 0)
	{
		m_darknessMgr->serialize(p_serializationMgr);
	}
	m_pathMgr.serialize(p_serializationMgr);
	
	if (m_presentationObjectMgr != 0)
	{
		m_presentationObjectMgr->serializeState(p_serializationMgr);
	}
	
	AppGlobal::getDemoMgr().serialize(p_serializationMgr);
	
#if !defined(TT_BUILD_FINAL)
	const u64 timestampEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::serializeAll: Serialization took %u ms.\n", u32(timestampEnd - timestampBegin));
#endif
}


void Game::unserializeAll(const toki::serialization::SerializationMgr& p_serializationMgr,
                          const std::string& p_serializationID)
{
	// Tell the audio player we're loading (if it didn't already know)
	audio::AudioPlayer* audioPlayer = audio::AudioPlayer::getInstance();
	bool shouldResetAudioPlayerLoadingFlag = false;
	if (audioPlayer->isLoadingLevel() == false)
	{
		audioPlayer->setLoadingLevel(true);
		shouldResetAudioPlayerLoadingFlag = true;
	}
	
	bool startInfoLoadOk = false;
	const StartInfo newStartInfo(loadStartInfo(p_serializationMgr, startInfoLoadOk));
	m_progressTypeOverride = ProgressType_Invalid;
	
	// Before destroying anything, detect if we should show the load screen instead
	if (startInfoLoadOk && newStartInfo != m_startInfo)
	{
		if (AppGlobal::getInputRecorder()->isRestoringCheckpoint())
		{
			// Recorder playback should immediately load level and shoebox and not go through
			// load screen logic
			m_startInfo = newStartInfo;
			loadLevel(newStartInfo);
		}
		else
		{
			// Unserializing different level than currently loaded: go to the load screen
			AppGlobal::getShutdownDataMgr().setData(p_serializationMgr.clone(), m_serializationID);
			forceReload();
			if (shouldResetAudioPlayerLoadingFlag)
			{
				audioPlayer->setLoadingLevel(false);
			}
			return;
		}
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 timestampBegin = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::unserializeAll: Starting unserialization of checkpoint '%s'...\n", p_serializationID.c_str());
#endif
	
	// Reset
	if (m_fluidMgr != 0)
	{
		m_fluidMgr->resetLevel();
	}
	
	if (m_entityMgr != 0)
	{
		m_entityMgr->resetAll();
	}
	if (m_presentationObjectMgr != 0)
	{
		m_presentationObjectMgr->reset();
	}
	
	audio::AudioPlayer::getInstance()->stopAllAudio();
	
	m_pathMgr.reset();
	
	// Ensure shoeboxes are gone before resetting particle manager
	const tt::engine::renderer::EngineIDToTextures previousTextures(createEmptyShoeboxesWithPreviousSettings());
	
	resetParticleMgr();
	
	for (s32 layer = 0; layer < ParticleLayer_Count; ++layer)
	{
		TT_ASSERTMSG(m_presentationMgr[layer] == nullptr || m_presentationMgr[layer]->hasRegisteredPresentationObjects() == false,
			"Presentation manager still has registered objects. Do cleanup before assigning new presentation mgr. This can crash!");
		m_presentationMgr[layer] = createPresentationMgr();
	}
	
	input::Controller& controller = AppGlobal::getController(tt::input::ControllerIndex_One);
	controller.stopRumble(true);
	controller.setRumbleEnabled(true);
	
	// Unserialize
	unserialize(p_serializationMgr);
	m_tileRegistrationMgr->reset(); // tileRegistrationMgr needs to be valid while unserializing game so reset if after that function.
	
	script::getRegistry().unserialize(p_serializationMgr);
	script::TimerMgr::unserialize(p_serializationMgr);
	if (m_entityMgr != 0)
	{
		m_entityMgr->unserialize(p_serializationMgr);
	}
	if (m_presentationObjectMgr != 0)
	{
		m_presentationObjectMgr->unserialize(p_serializationMgr);
	}
	if (m_fluidMgr != 0)
	{
		TT_NULL_ASSERT(m_entityMgr);
		m_fluidMgr->unserialize(p_serializationMgr, *m_entityMgr);
	}
	if (m_lightMgr != 0)
	{
		m_lightMgr->unserialize(p_serializationMgr);
	}
	if (m_darknessMgr != 0)
	{
		m_darknessMgr ->unserialize(p_serializationMgr);
	}
	m_pathMgr.unserialize(p_serializationMgr);
	
	// Script needs to be unserialized last so it can access all the resources unserialized above.
	// (The handles need to return valid ptrs for script.)
	toki::script::ScriptMgr::unserialize(p_serializationMgr);
	
	// Presentation objects' state is unserialized after script so callbacks can trigger an go to script.
	if (m_presentationObjectMgr != 0)
	{
		m_presentationObjectMgr->unserializeState(p_serializationMgr);
	}
	
	bool isDemoValid = AppGlobal::getDemoMgr().unserialize(p_serializationMgr);
	
	if (isDemoValid == false && AppGlobal::isInDemoMode())
	{
		AppGlobal::resetDemo();
	}
	
	// Now that the world is in a valid state again, make all movement controllers
	// reacquire their path finding agents (if needed)
	if (m_entityMgr != 0)
	{
		m_entityMgr->getMovementControllerMgr().handlePathMgrReset();
		
		// Notify all entities that progress has been restored
		callOnProgressRestoredOnAllEntities(p_serializationID);
	}
	
	createShoeboxesFromDataInclSkin();
	
	if (shouldResetAudioPlayerLoadingFlag)
	{
		audioPlayer->setLoadingLevel(false);
	}
	
	// Call generic onGameUnserialized to signal to script that the game has been unserialized
	{
		tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
		vmPtr->callSqFun("onGameUnserialized");
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 timestampEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Game::unserializeAll: Unserialization took %u ms.\n", u32(timestampEnd - timestampBegin));
#endif
}


StartInfo Game::loadStartInfo(const toki::serialization::SerializationMgr& p_serializationMgr,
                              bool&                                        p_success_OUT)
{
	StartInfo startInfo;
	
	p_success_OUT = false;
	
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_Game);
	TT_ASSERTMSG(section != 0, "Serialization manager does not contain a section for the Game data.");
	if (section != 0)
	{
		tt::code::BufferReadContext context(section->getReadContext());
		p_success_OUT = startInfo.unserialize(&context);
	}
	
	return startInfo;
}


void Game::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_Game);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Game data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	m_startInfo.serialize(&context);
	bu::putEnum<u8>(m_progressTypeOverride, &context);
	
	bu::put      (m_directionalControlsEnabled, &context);
	const s32 screenspaceCount = static_cast<s32>(m_screenSpaceEntities.size());
	bu::put      (screenspaceCount,             &context);
	for (entity::EntityHandles::const_iterator it = m_screenSpaceEntities.begin();
	     it != m_screenSpaceEntities.end(); ++it)
	{
		bu::putHandle(*it, &context);
	}
	const s32 buttonInputListeningCount = static_cast<s32>(m_buttonInputListeningEntities.size());
	bu::put(buttonInputListeningCount, &context);
	for (PrioritizedInputListeners::const_iterator it = m_buttonInputListeningEntities.begin();
	     it != m_buttonInputListeningEntities.end(); ++it)
	{
		bu::putHandle(it->entity,     &context);
		bu::put      (it->priority,   &context);
		bu::put      (it->isBlocking, &context);
	}
	const s32 mouseInputListeningCount = static_cast<s32>(m_mouseInputListeningEntities.size());
	bu::put(mouseInputListeningCount, &context);
	for (PrioritizedInputListeners::const_iterator it = m_mouseInputListeningEntities.begin();
	     it != m_mouseInputListeningEntities.end(); ++it)
	{
		bu::putHandle(it->entity,     &context);
		bu::put      (it->priority,   &context);
		bu::put      (it->isBlocking, &context);
	}
	const s32 keyboardListeningCount = static_cast<s32>(m_keyboardListeningEntities.size());
	bu::put(keyboardListeningCount, &context);
	for (PrioritizedInputListeners::const_iterator it = m_keyboardListeningEntities.begin();
	     it != m_keyboardListeningEntities.end(); ++it)
	{
		bu::putHandle(it->entity,     &context);
		bu::put      (it->priority,   &context);
		bu::put      (it->isBlocking, &context);
	}
	bu::put      (m_layersVisible.checkFlag(GameLayer_Attributes), &context);
	
	bu::put      (m_gameTimeInSeconds                            , &context);
	
	m_cameraMgr.serialize(&context);
	getMinimap().serialize(&context);
	
	getColorGradingEffectMgr().serialize(&context);
	getFogEffectMgr().serialize(&context);
	
	TT_NULL_ASSERT(m_shoeboxSkinAndEnvironment);
	{
		using tt::engine::scene2d::BlurLayers;
		const BlurLayers backLayers = m_shoeboxSkinAndEnvironment != 0 ? m_shoeboxSkinAndEnvironment->getBackBlurLayers() : BlurLayers();
		const BlurLayers foreLayers = m_shoeboxSkinAndEnvironment != 0 ? m_shoeboxSkinAndEnvironment->getForeBlurLayers() : BlurLayers();
		const s32 splitPriority     = m_shoeboxSkinAndEnvironment != 0 ? m_shoeboxSkinAndEnvironment->getSplitPriority() : 
		                              cfg()->getIntegerDirect("toki.shoebox.split_priority");
		serialization::serializeBlurLayers(backLayers, &context);
		serialization::serializeBlurLayers(foreLayers, &context);
		bu::put(splitPriority, &context);
	}
	
	{
		using tt::engine::scene2d::shoebox::ShoeboxData;
		const s32 shoeboxIncludeCount = static_cast<s32>(m_shoeboxDataIncludesFromScript->includes.size());
		bu::put(shoeboxIncludeCount, &context);
		for (ShoeboxData::Includes::const_iterator it = m_shoeboxDataIncludesFromScript->includes.begin();
		     it != m_shoeboxDataIncludesFromScript->includes.end(); ++it)
		{
			const tt::engine::scene2d::shoebox::IncludeData& include = (*it);
			
			bu::put(include.filename, &context);
			bu::put(include.offset  , &context);
			bu::put(include.priority, &context);
			bu::put(include.scale   , &context);
		}
	}
	
	audio::AudioPlayer::getInstance()->serialize(&context);
	
	const s32 themetilesCount = static_cast<s32>(m_levelOverriddenThemeTiles.size());
	bu::put(themetilesCount, &context);
	for (level::ThemeTiles::const_iterator it = m_levelOverriddenThemeTiles.begin();
	     it != m_levelOverriddenThemeTiles.end(); ++it)
	{
		bu::put        ((*it).first,  &context);
		bu::putEnum<u8>((*it).second, &context);
	}
	
	bu::put(m_stopLightRenderingOnSplit, &context);
	
	bu::put(m_colorGradingAfterHud, &context);
	
	context.flush();
}


void Game::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_Game);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Game data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	StartInfo startInfo;
	startInfo.unserialize(&context);
	
	if (startInfo != m_startInfo)
	{
		// With the new checking in unserializeAll, we should never be unserializing a different
		// level than currently loaded anymore
		TT_PANIC("Unserializing a different level than currently loaded! "
		         "This should have been handled by higher-level code.\n"
		         "Current level: %s\nNew level: %s",
		         m_startInfo.getLevelFilePath().c_str(), startInfo.getLevelFilePath().c_str());
		loadLevel(startInfo);
	}
	
	m_startInfo = startInfo;
	m_progressTypeOverride       = bu::getEnum<u8, ProgressType>(&context);
	
	m_directionalControlsEnabled = bu::get      <bool          >(&context);
	
	m_screenSpaceEntities.clear();
	const s32 screenspaceCount   = bu::get      <s32           >(&context);
	for (s32 i = 0; i < screenspaceCount; ++i)
	{
		m_screenSpaceEntities.push_back(bu::getHandle<entity::Entity>(&context));
	}
	
	m_buttonInputListeningEntities.clear();
	const s32 buttonInputListeningCount = bu::get<s32>(&context);
	for (s32 i = 0; i < buttonInputListeningCount; ++i)
	{
		const entity::EntityHandle handle = bu::getHandle<entity::Entity>(&context);
		const s32 priority                = bu::get<s32>(&context);
		const bool isBlocking             = bu::get<bool>(&context);
		m_buttonInputListeningEntities.push_back(InputListener(handle, priority, isBlocking));
	}
	
	m_mouseInputListeningEntities.clear();
	const s32 mouseInputListeningCount = bu::get<s32>(&context);
	for (s32 i = 0; i < mouseInputListeningCount; ++i)
	{
		const entity::EntityHandle handle = bu::getHandle<entity::Entity>(&context);
		const s32 priority                = bu::get<s32>(&context);
		const bool isBlocking             = bu::get<bool>(&context);
		m_mouseInputListeningEntities.push_back(InputListener(handle, priority, isBlocking));
	}
	m_keyboardListeningEntities.clear();
	const s32 keyboardListeningCount = bu::get<s32>(&context);
	for (s32 i = 0; i < keyboardListeningCount; ++i)
	{
		const entity::EntityHandle handle = bu::getHandle<entity::Entity>(&context);
		const s32 priority                = bu::get<s32>(&context);
		const bool isBlocking             = bu::get<bool>(&context);
		m_keyboardListeningEntities.push_back(InputListener(handle, priority, isBlocking));
	}
	
	m_layersVisible.setFlag(GameLayer_Attributes, bu::get<bool>(&context));
	
	m_gameTimeInSeconds               = bu::get<real64>(&context);
	
	m_cameraMgr.unserialize(&context);
	getMinimap().unserialize(&context);
	
	getColorGradingEffectMgr().unserialize(&context);
	getFogEffectMgr().unserialize(&context);
	
	{
		using tt::engine::scene2d::BlurLayers;
		const BlurLayers backLayers    = serialization::unserializeBlurLayers(&context);
		const BlurLayers foreLayers    = serialization::unserializeBlurLayers(&context);
		const s32        splitPriority = bu::get<s32>(&context);
		
		TT_NULL_ASSERT(m_shoeboxSkinAndEnvironment);
		if (m_shoeboxSkinAndEnvironment != 0)
		{
			m_shoeboxSkinAndEnvironment->setBackBlurLayers(backLayers);
			m_shoeboxSkinAndEnvironment->setForeBlurLayers(foreLayers);
			m_shoeboxSkinAndEnvironment->setSplitPriority(splitPriority);
		}
	}
	
	m_shoeboxDataIncludesFromScript->clear();
	const s32 shoeboxIncludeCount = bu::get<s32>(&context);
	for (s32 i = 0; i < shoeboxIncludeCount; ++i)
	{
		const std::string       filename = bu::get<std::string      >(&context);
		const tt::math::Vector3 offset   = bu::get<tt::math::Vector3>(&context);
		const s32               priority = bu::get<s32              >(&context);
		const real              scale    = bu::get<real             >(&context);
		
		m_shoeboxDataIncludesFromScript->includes.push_back(
			tt::engine::scene2d::shoebox::IncludeData(filename, offset, priority, scale));
	}
	
	audio::AudioPlayer::getInstance()->unserialize(&context);
	
	m_levelOverriddenThemeTiles.clear();
	const s32 themetilesCount = bu::get<s32>(&context);
	for (s32 i = 0; i < themetilesCount; ++i)
	{
		tt::math::Point2 position = bu::get<tt::math::Point2>(&context);
		level::ThemeType theme    = bu::getEnum<u8, level::ThemeType>(&context);
		m_levelOverriddenThemeTiles[position] = theme;
	}
	
	m_stopLightRenderingOnSplit = bu::get<bool>(&context);
	
	m_colorGradingAfterHud = bu::get<bool>(&context);
	
	// Reset other game variables that were not stored in serialization data
	releaseDraggingEntity();
	m_pressingEntities.clear();
	for (ScriptButtons::iterator it = g_scriptButtons.begin(); it != g_scriptButtons.end(); ++it)
	{
		(*it).pressDuration = 0.0f;
	}
	m_spacebarScrollMode.reset();
	m_forceReload      = false;
	m_reloadEntities   = false;
}


void Game::callOnProgressRestoredOnAllEntities(const std::string& p_serializationID) const
{
	TT_NULL_ASSERT(m_entityMgr);
	// Notify all entities that progress has been restored
	
	// Copy all the entities so script can add/remove entities in the callback.
	entity::EntityHandles restoredEntities;
	restoredEntities.reserve(m_entityMgr->getActiveEntitiesCount());
	{
		const entity::Entity* entity = m_entityMgr->getFirstEntity();
		const s32 entityCount = m_entityMgr->getActiveEntitiesCount();
		for (s32 i = 0; i < entityCount; ++i, ++entity)
		{
			TT_ASSERT(entityCount == m_entityMgr->getActiveEntitiesCount());
			restoredEntities.push_back(entity->getHandle());
		}
	}
	
	for (entity::EntityHandles::const_iterator it = restoredEntities.begin();
	     it != restoredEntities.end(); ++it)
	{
		const entity::Entity* entity = (*it).getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			const script::EntityBasePtr& localPtr(entity->getEntityScript());
			TT_NULL_ASSERT(localPtr);
			localPtr->onProgressRestored(p_serializationID);
		}
	}
}


void Game::callOnReloadRequestedOnAllEntities() const
{
// Only works in non-final builds
#if !defined(TT_BUILD_FINAL)
	TT_NULL_ASSERT(m_entityMgr);
	// Notify all entities that reload is coming
	
	// Copy all the entities so script can add/remove entities in the callback.
	entity::EntityHandles entities;
	entities.reserve(m_entityMgr->getActiveEntitiesCount());
	{
		const entity::Entity* entity = m_entityMgr->getFirstEntity();
		const s32 entityCount = m_entityMgr->getActiveEntitiesCount();
		for (s32 i = 0; i < entityCount; ++i, ++entity)
		{
			TT_ASSERT(entityCount == m_entityMgr->getActiveEntitiesCount());
			entities.push_back(entity->getHandle());
		}
	}
	
	for (entity::EntityHandles::const_iterator it = entities.begin();
	     it != entities.end(); ++it)
	{
		const entity::Entity* entity = (*it).getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			const script::EntityBasePtr& localPtr(entity->getEntityScript());
			TT_NULL_ASSERT(localPtr);
			localPtr->callSqFun("onReloadRequested");
		}
	}
#endif
}


CheckPointMgr& Game::getCheckPointMgr()
{
	return isValidProgressType(m_serializationProgressType) ?
	       AppGlobal::getCheckPointMgr(m_serializationProgressType) :
	       AppGlobal::getCheckPointMgr();
}


#if !defined(TT_BUILD_FINAL)
void Game::createColorTable()
{
	using namespace tt::engine::renderer;
	m_colorTable.reset(new QuadBuffer(32, TexturePtr(), BatchFlagQuad_UseVertexColor));
	
	tt::engine::renderer::BatchQuadCollection colorTableBatch;
	
	const real quadSize = 32.0f;
	
	for (s32 i = 0; i < 32; ++i)
	{
		tt::engine::renderer::BatchQuad quad;
		
		// We want to render the edge of the quad so we have the full color range.
		// If we don't do this correct our (red and green) colors go from 0 to 247; instead of to 255.
		const real pixelCorrection = 0.95f;
		
		const real left   = real(i) * quadSize;
		const real right  = (left + quadSize) - pixelCorrection;
		const real top    = 0.0f;
		const real bottom = -(quadSize - pixelCorrection);
		
		quad.topLeft.    setPosition(left,  top   , 0);
		quad.topRight.   setPosition(right, top   , 0);
		quad.bottomLeft. setPosition(left,  bottom, 0);
		quad.bottomRight.setPosition(right, bottom, 0);
		
		real blueness = real(i) / 31.0f;
		tt::math::clamp(blueness, 0.0f, 1.0f);
		s32 blue = static_cast<s32>(blueness * 255.0f);
		tt::math::clamp(blue    , s32(0), s32(255));
		
		quad.topLeft.    setColor(0  ,   0, blue, 255);
		quad.topRight.   setColor(255,   0, blue, 255);
		quad.bottomLeft. setColor(0  , 255, blue, 255);
		quad.bottomRight.setColor(255, 255, blue, 255);
		
		colorTableBatch.push_back(quad);
	}
	
	m_colorTable->setCollection(colorTableBatch);
	m_colorTable->applyChanges();
}
#endif


//////////////////////////////////////////////////////////
// Shoebox update thread


int Game::staticUpdateShoeboxThread(void* p_game)
{
	TT_NULL_ASSERT(p_game);

	if(p_game != 0)
	{
		return static_cast<Game*>(p_game)->updateShoeboxThread();
	}

	return 0;
}


int Game::updateShoeboxThread()
{
	while (m_threadShouldExit == false)
	{
		m_startShoeboxUpdate.wait();
		
		if (m_threadShouldExit) break;
		
		updateShoebox(m_deltaTime);
		
		m_finishedShoeboxUpdate.signal();
	}
	
	return 0;
}


void Game::updateShoebox(real p_deltaTime)
{
	if (m_shoeboxSkinAndEnvironment == 0)
	{
		return;
	}
	
	m_eventsMutex.lock();
	const ShoeboxTagEvents localCopy(m_queuedEvents);
	m_queuedEvents.clear();
	m_eventsMutex.unlock();
	
	bool invalidateShoeboxBatch(false);
	for(ShoeboxTagEvents::const_iterator it = localCopy.begin(); it != localCopy.end(); ++it)
	{
		if (it->event == "hide")
		{
			invalidateShoeboxBatch = true;
		}
		tt::engine::scene2d::shoebox::TagMgr::sendEvent(it->tag, it->event, it->param);
	}
	
	if (invalidateShoeboxBatch)
	{
		m_shoeboxSkinAndEnvironment->invalidateBatches();
	}
	
	if (m_shadowMaskShoebox != 0)
	{
		m_shadowMaskShoebox->update(p_deltaTime);
	}
	m_shoeboxSkinAndEnvironment->update(p_deltaTime);
}


void Game::applyColorGrading()
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
#ifndef TT_BUILD_FINAL
	if (renderer->getDebug()->isMipmapVisualizerActive())
	{
		return;
	}
#endif
	using namespace toki::utils;
	START_RENDER_SECTION(FrameRenderSection_Misc);
	
	if (m_colorGrading != 0 && AppGlobal::shouldTakeLevelScreenshot() == false)
	{
		renderer->setColorMask(tt::engine::renderer::ColorMask_All);
		m_colorGrading->apply(true);
		renderer->setColorMask(tt::engine::renderer::ColorMask_Color);
	}
}


// Namespace end
}
}
