#include <sstream>

#include <tt/app/fatal_error.h>
#include <tt/app/Application.h>
#include <tt/app/StartupState.h>
#include <tt/args/CmdLine.h>
#include <tt/cfg/ConfigRegistry.h>
#include <tt/compression/png.h>
#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/renderer/gpu_capabilities.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/engine/renderer/pp/Filter.h>
#include <tt/engine/scene/SceneBlurMgr.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/fs/MemoryArchive.h>
#include <tt/fs/MemoryFileSystem.h>
#include <tt/mem/Heap.h>
#include <tt/mem/mem.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/pres/PresentationCache.h>
#include <tt/stats/stats.h>
#if defined(TT_STEAM_BUILD)
#include <tt/steam/Leaderboards.h>
#endif
#include <tt/system/utils.h>
#include <tt/system/Time.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/event/EventMgr.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/CheckPointMgr.h>
#include <toki/game/Game.h>
#include <toki/input/Recorder.h>
#include <toki/level/LevelData.h>
#include <toki/loc/Loc.h>
#include <toki/main/AppStateMachine.h>
#include <toki/pres/PresentationObjectMgr.h>
#include <toki/savedata/utils.h>
#include <toki/script/ScriptMgr.h>
#include <toki/statelist/statelist.h>

#include <toki/steam/Workshop.h>
#include <toki/utils/GlyphSetMgr.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>
#include <toki/AppMain.h>



namespace toki {

//--------------------------------------------------------------------------------------------------
// Helper functions


static void drawFPSString(s32 p_fps, s32 p_updateTime, s32 p_renderTime, s32 p_maxUpdateTime, s32 p_maxRenderTime, tt::engine::renderer::TexturePainter& p_painter)
{
	using namespace tt::engine;
	
	p_painter.clear();
	// Create FPS label text and color
	{
		std::stringstream str;
		str << p_fps;
		
		const glyph::GlyphSetPtr& glyphSet(utils::GlyphSetMgr::get(utils::GlyphSetID_Notes));
		if (glyphSet != 0)
		{
			glyphSet->drawMultiLineString(tt::str::widen(str.str()), p_painter, p_fps < 45 ? renderer::ColorRGB::red : renderer::ColorRGB::green,
				glyph::GlyphSet::ALIGN_CENTER, glyph::GlyphSet::ALIGN_TOP, 0, 0, 10);
		}
	}
	// Extended info
	{
		// Calculate theoretical max fps
		const s32 maxAvgFPS = static_cast<s32>(1000000.0f / (p_updateTime + p_renderTime) + 0.5f);
		const s32 maxPeakFPS = static_cast<s32>(1000000.0f / (p_maxUpdateTime + p_maxRenderTime) + 0.5f);
		
		static char buf[256];
		
		sprintf(buf, "  AVG / PEAK\n#FPS: %4d / %4d    \nUPD: %5.2f / %5.2f ms\nRDR: %5.2f / %5.2f ms",
			maxAvgFPS, maxPeakFPS, p_updateTime / 1000.0f, p_maxUpdateTime / 1000.0f, p_renderTime / 1000.0f, p_maxRenderTime / 1000.0f);
		
		const glyph::GlyphSetPtr& glyphSet(utils::GlyphSetMgr::get(utils::GlyphSetID_EditorHelpText));
		if (glyphSet != 0)
		{
			glyphSet->drawMultiLineString(tt::str::widen(buf), p_painter, renderer::ColorRGB::green,
				glyph::GlyphSet::ALIGN_CENTER, glyph::GlyphSet::ALIGN_BOTTOM);
		}
	}
}



#if !defined(TT_BUILD_FINAL)
// Keep track of texture memory usage (but do not update it each frame, because it is relatively expensive)
static s32 g_texMemTotalInBytes             = 0;
static s32 g_texMemShoeboxEnvInBytes        = 0;
static s32 g_texMemShoeboxShadowMaskInBytes = 0;
static s32 g_texMemPrecacheTexInBytes       = 0;
static s32 g_texMemPrecachePresInBytes      = 0;


static const s32 g_texMemBudgetTotalInBytes        = 600 * 1024 * 1024;
static const s32 g_texMemBudgetShoeboxInBytes      = 180 * 1024 * 1024;
static const s32 g_texMemBudgetPrecacheTexInBytes  = 180 * 1024 * 1024; // FIXME: Get correct budget.
static const s32 g_texMemBudgetPrecachePresInBytes = 255 * 1024 * 1024; // FIXME: Get correct budget.

// NOTE: Not using ColorRGB::green here, because of static initialization order issues:
//       This variable could be initialized before ColorRGB::green, making the debug color black
static const tt::engine::renderer::ColorRGBA g_debugColor(0, 255, 0, 255);


inline void debugTextWithShadow(const tt::engine::debug::DebugRendererPtr& p_debug,
                                const std::string&                         p_text,
                                s32                                        p_x,
                                s32                                        p_y,
                                const tt::engine::renderer::ColorRGBA&     p_textColor)
{
	static const tt::engine::renderer::ColorRGBA shadowColor(tt::engine::renderer::ColorRGB::black, 255);
	
	p_debug->renderText(p_text, p_x + 1, p_y + 1, shadowColor);
	p_debug->renderText(p_text, p_x,     p_y,     p_textColor);
}


inline void debugTextWithShadow(const tt::engine::debug::DebugRendererPtr& p_debug,
                                const std::string&                         p_text,
                                s32                                        p_x,
                                s32                                        p_y)
{
	debugTextWithShadow(p_debug, p_text, p_x, p_y, g_debugColor);
}

#endif  // !defined(TT_BUILD_FINAL)


inline void saveScreenshotFallbackImage(const ScreenshotSettings& p_settings)
{
	if (p_settings.type != ScreenshotType_LevelPreview ||
	    p_settings.failureFallbackImage.empty())
	{
		return;
	}
	
	tt::code::BufferPtr sourceContents = tt::fs::getFileContent(p_settings.failureFallbackImage);
	if (sourceContents == 0)
	{
		TT_PANIC("Could not load screenshot failure fallback file '%s'.", p_settings.failureFallbackImage.c_str());
		return;
	}
	
	tt::fs::FilePtr targetFile(tt::fs::open(p_settings.filename, tt::fs::OpenMode_Write, p_settings.fileSystem));
	if (targetFile == 0)
	{
		TT_PANIC("Could not create/overwrite screenshot file '%s' for saving failure fallback image '%s'.",
		         p_settings.filename.c_str(), p_settings.failureFallbackImage.c_str());
		return;
	}
	
	if (targetFile->write(sourceContents->getData(), sourceContents->getSize()) != sourceContents->getSize())
	{
		TT_PANIC("Could not write screenshot failure fallback image '%s' to file '%s'.",
		         p_settings.failureFallbackImage.c_str(), p_settings.filename.c_str());
		return;
	}
}


inline tt::engine::renderer::RenderTargetPtr createRenderTarget(s32                                    p_width,
                                                                s32                                    p_height,
                                                                const tt::engine::renderer::ColorRGBA& p_color)
{
	using namespace tt::engine::renderer;
	static const s32 maxTextureWidth = getMaxTextureWidth();
	
	// Check dimensions
	if (p_width > maxTextureWidth)
	{
		TT_PANIC("Capture width of %d is larger than maximum supported texture width (%d),"
			"if this is a full level screenshot set texels per tile to a lower value.",
			p_width, maxTextureWidth);
		return RenderTargetPtr();
	}
	if (p_height > maxTextureWidth)
	{
		TT_PANIC("Capture height of %d is larger than maximum supported texture height (%d),"
			"if this is a full level screenshot set texels per tile to a lower value.",
			p_height, maxTextureWidth);
		return RenderTargetPtr();
	}
	
	// Create a render target for the result
	RenderTargetPtr target = RenderTarget::create(p_width, p_height);
	if (target == 0)
	{
		return RenderTargetPtr();
	}
	target->setClearColor(p_color);
	
	return target;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

AppMain::AppMain()
:
m_stateMachine(0),
m_archive(),
m_cachedShaders(),
m_badPerfTime(0.0f),
m_badPerfUpdateCount(0)
#if ENABLE_DEBUG_INFO
,
m_maxUpdateTime(0),
m_minUpdateTime(0),
m_averageUpdateTime(0),
m_updateCount(0),
m_fpsQuad(),
m_showFps(false)
#endif
#if !defined(TT_BUILD_FINAL)
,
m_enableMemoryBudgetWarning(false)
#endif
{
	// NOTE: Do not use lib functions here, they are not initialized when this is constructed.
	//       Please use init() for initializing app systems.
}


AppMain::~AppMain()
{
	// Save the current game state when shutting down
	if (hasLoadedGame())
	{
		serialization::savePersistentDataAndShutdownState(true);
	}
	
	// Shut down the state machine gracefully
	if (m_stateMachine != 0)
	{
		static const real fixedUpdate = AppGlobal::getFixedDeltaTime();
		
		// Switch to the exit state
		//m_stateMachine->update(fixedUpdate);  // FIXME: Is this pre-changeState update really necessary?
		m_stateMachine->changeState(statelist::StateID_ExitApp, true);
		
		tt::system::Time* t = tt::system::Time::getInstance();
		const u64 bailOutTime = t->getMilliSeconds() + 3000;
		do
		{
			m_stateMachine->update(fixedUpdate);
		}
		while (m_stateMachine->getCurrentState() != statelist::StateID_ExitApp &&
		       t->getMilliSeconds() < bailOutTime);
		
		// Destroy the application state machine
		delete m_stateMachine;
		m_stateMachine = 0;
	}
	
	AppGlobal::getSharedGraphics().stopBackgroundMusic();
	AppGlobal::destroySharedGraphics();
	AppGlobal::destroySkinConfigs();
	
	AppGlobal::getLoc().destroyAll();
	
	toki::game::light::LightMgr::destroyStaticResources();
	
	tt::stats::destroyInstance();
	
	// Deinit the Script Manager.
	script::ScriptMgr::deinit();
	
	// Clear precache
	AppGlobal::clearPrecache();
	
	AppGlobal::clearShoeboxTextures();
	
	// Clear all script lists used for the editor
	AppGlobal::clearScriptLists();
	
	audio::AudioPlayer::destroyInstance();
	
	// Destroy the particle manager
	tt::engine::particles::ParticleMgr::destroyInstance();
	
	AppGlobal::destroyInputRecorder();
	
	// Make sure that the presentation cache also clears its permanent objects
	tt::pres::PresentationCache::clear();
	
	tt::engine::renderer::Renderer::getInstance()->getPP()->deinitialize();
	tt::engine::scene::SceneBlurMgr::cleanup();
	
#if defined(TT_STEAM_BUILD)
	steam::Workshop::destroyInstance();
	tt::steam::Leaderboards::destroyInstance();
#endif
	
	// Remove memory archive
	if (m_archive != 0)
	{
		tt::fs::MemoryFileSystem::removeMemoryArchive(m_archive.get());
	}
	
	AppOptions::destroyInstance();
}

bool AppMain::init()
{
	if (tt::app::getCmdLine().exists("show_loaded_files"))
	{
		tt::engine::file::FileUtils::getInstance()->setShowLoadedFiles(true);
	}
	
	// Check GPU requirements
	if (AppGlobal::shouldDoGpuCheck())
	{
		static const u32 neededTextureSize = 4 * 1024;
		if (tt::engine::renderer::getMaxTextureWidth() < neededTextureSize)
		{
			std::string errorMsg =
				"Unfortunately your GPU does not meet the minimum hardware requirements for this game. ";
				
			errorMsg += "The maximum supported texture size is " + 
				tt::str::toStr(tt::engine::renderer::getMaxTextureWidth()) + 
				", this should be 4096 or higher to play the game.";
			
			tt::app::reportFatalError(errorMsg);
		}
		
		if (tt::engine::renderer::getNonPowerOfTwoSupport() != tt::engine::renderer::NPOTSupport_Full)
		{
			tt::app::reportFatalError(
				"Unfortunately your GPU does not meet the minimum hardware requirements for this game. "
				"Non-power-of-two textures are not fully supported.");
		}
	}
	
	// Load memory archive
	const std::string archivePath("archive.ma");
	if (tt::fs::fileExists(archivePath))
	{
#if !defined(TT_BUILD_FINAL)
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		// Load and add memory archive to memfs
		m_archive = tt::fs::MemoryArchive::load(archivePath);
		if (m_archive != 0)
		{
			tt::fs::MemoryFileSystem::addMemoryArchive(m_archive.get());
		}
#if !defined(TT_BUILD_FINAL)
		const u64 loadEnd   = tt::system::Time::getInstance()->getMilliSeconds();
		const u32 totalTime = u32(loadEnd - loadStart);
		TT_Printf("AppMain::init: Memory archive '%s' loaded in %u ms\n",
		          archivePath.c_str(), totalTime);
#endif
	}
	else
	{
		TT_Printf("AppMain::init: No memory archive loaded\n");
	}
	
#if defined(TT_STEAM_BUILD)
	steam::Workshop::createInstance();
	tt::steam::Leaderboards::createInstance(std::string(), tt::app::getApplication()->getSaveFsID(), true);
#endif

	AppGlobal::getLoc().createLocStr(loc::SheetID_Game);
	AppGlobal::getLoc().createLocStr(loc::SheetID_Achievements);
	
	// Setup ParticleMgr
	{
		using namespace tt::engine::particles;
		ParticleMgr::createInstance();
		ParticleMgr* pm = ParticleMgr::getInstance();
#if defined(TT_BUILD_FINAL)
		pm->setTriggerCachingEnabled(true);
#else
		pm->setTriggerCachingEnabled(tt::app::getCmdLine().exists("no_particle_caching") == false);
#endif
		pm->setHudRenderGroup(toki::game::ParticleRenderGroup_Hud);
		pm->setFixedTimestepForRenderGroup(toki::game::ParticleRenderGroup_Hud, 1.0f / 60.0f);
	}
	tt::engine::scene2d::shoebox::Shoebox::setShoeboxesPath("");
	
	toki::game::light::LightMgr::createStaticResources(false);
	
#if !defined(TT_BUILD_FINAL) && 0
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_EntityCollisionRect);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_EntityRegisteredRect);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_EntityRegisteredTiles);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_EntityMoveToRect);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_EntityPosition);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_PathMgrAgents);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_DisableDebugRenderForEntityWithParent);
	//AppGlobal::modifyDebugRenderMask().setFlag(toki::DebugRender_EntityTouchRectangle);
	
	//tt::math::Random::getStatic().setContextSeedValue(1u);
#endif
	
	AppGlobal::createSharedGraphics();
	AppGlobal::setCheckPointMgr(game::CheckPointMgr::create(ProgressType_Main     ), ProgressType_Main); 
	AppGlobal::setCheckPointMgr(game::CheckPointMgr::create(ProgressType_UserLevel), ProgressType_UserLevel); 
	AppGlobal::createTileRegistrationMgr();
	
	AppGlobal::getController(tt::input::ControllerIndex_One).init();
	
	AppGlobal::getDemoMgr().init();
	
	tt::engine::renderer::TexturePtr tex = tt::engine::renderer::Texture::createForText(224, 160, true);
	m_fpsQuad = tt::engine::renderer::QuadSprite::createQuad(tex);
	
	AppGlobal::createInputRecorder();
	
	initializePostProcessing();
	
	AppOptions::getInstance().setBlurQualityInGame();
	AppOptions::getInstance().setShadowQualityInGame();
	
	// NOTE: This must be the last call of AppMain::init()
	//       The state machine might start threads that cannot run parallel with initialization
	
	m_stateMachine = new main::AppStateMachine(statelist::StateID_LoadApp);
	
	return true;
}


void AppMain::update(real p_elapsedTime)
{
	tt::engine::debug::DebugStats::beginUpdate();
	AppGlobal::updateAppTime(p_elapsedTime);
	
	// See: http://gafferongames.com/game-physics/fix-your-timestep/
	
	const real fixedDeltaTime = AppGlobal::getFixedDeltaTime();
	
	if(AppGlobal::isBusyLoading())
	{
		m_showFps = false;
	}
	
	if (AppGlobal::isBusyLoading())
	{
		p_elapsedTime = 0.0f;
		AppGlobal::setBusyLoading(false);
	}
	else if (tt::app::getApplication()->isActive() && AppGlobal::isBadPerformanceDetected() == false) // App has focus
	{
		// Slow/bad performance detection code.
		
		// Make sure extremely long frames (serialization) are not skewing the results
		if (p_elapsedTime > 0.25f)
		{
			m_badPerfTime += fixedDeltaTime;
		}
		else
		{
			m_badPerfTime += p_elapsedTime;
		}
		++m_badPerfUpdateCount;
		
		// Did we count long enough?
		if (m_badPerfTime > 30.0f)
		{
			const real averageFrameTime = 30.0f / m_badPerfUpdateCount;
			const real fixedFrameTime = AppGlobal::hasDoubleUpdateMode() ? (fixedDeltaTime * 2.0f) : fixedDeltaTime;
			
			// Were enough updates too slow?
			if (averageFrameTime >= fixedFrameTime * 1.25f)
			{
				AppGlobal::setBadPerformanceDetected();
			}
			
			// Reset counters
			m_badPerfTime            = 0.0f;
			m_badPerfUpdateCount     = 0;
		}
	}
	
	// Run with fixed framerate
	p_elapsedTime = fixedDeltaTime;
	
	if (AppGlobal::hasDoubleUpdateMode())
	{
		p_elapsedTime *= 2.0f;
	}
	
	s32 recorderUpdates = 1;
	if(AppGlobal::getInputRecorder()->getState() == input::Recorder::State_Play)
	{
		recorderUpdates = AppGlobal::getInputRecorder()->getPlaybackSpeed();
	}
	
	real totalElapsedTime = 0.0f; // Need to know total elapsed time for when we're doing more updates for recording playback
	
#if ENABLE_DEBUG_INFO
	s32 updateCount = 0;
#endif // #if ENABLE_DEBUG_INFO
	
	do
	{
		--recorderUpdates;
		
		if (hasLoadedGame())
		{
			// If we're running a recording we should use it's timing, not our own.
			AppGlobal::getInputRecorder()->updateElapsedTime(&p_elapsedTime);
		}
		
		totalElapsedTime += p_elapsedTime;
		
		u32 updateCountCheck(0);
		
		// Do a GameTick.
		{
		
#if ENABLE_DEBUG_INFO
			++updateCount;
			u64 updateStart = tt::system::Time::getInstance()->getMicroSeconds();
#endif // #if ENABLE_DEBUG_INFO
			
			AppGlobal::incrementUpdateFrameCount();
			
			AppGlobal::getController(tt::input::ControllerIndex_One).updatePlatformState();
			AppGlobal::getController(tt::input::ControllerIndex_One).update(fixedDeltaTime);
			
			if (hasLoadedGame())
			{
				AppGlobal::getInputRecorder()->update(p_elapsedTime);
			}
			
			TT_NULL_ASSERT(m_stateMachine);
			m_stateMachine->update(p_elapsedTime);
			
			AppGlobal::getController(tt::input::ControllerIndex_One).clearPlatformState();
			
#if ENABLE_DEBUG_INFO
			u64 updateTime = tt::system::Time::getInstance()->getMicroSeconds() - updateStart;
			
			if (m_averageUpdateTime == 0) m_averageUpdateTime = updateTime;
			m_averageUpdateTime += updateTime;
			tt::math::makeHalf(m_averageUpdateTime);
			
			if (updateTime > m_maxUpdateTime) m_maxUpdateTime = updateTime;
			if (updateTime < m_minUpdateTime || m_minUpdateTime == 0) m_minUpdateTime = updateTime;
#endif // #if ENABLE_DEBUG_INFO
			
			updateCountCheck++;
		}
	}
	while (recorderUpdates > 0);
	
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer::getInstance()->update(totalElapsedTime);
	}
	
#if ENABLE_DEBUG_INFO
	m_updateCount = updateCount;
#endif // #if ENABLE_DEBUG_INFO
	
	m_stateMachine->updateForRender(totalElapsedTime);
	
	
#if !defined(TT_BUILD_FINAL)
	// Support Alt+T as shortcut to disable the texture memory budget warning
	const input::Controller::State::EditorState& debugInput(AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor);
	if (debugInput.keys[tt::input::Key_T      ].pressed       &&
	    debugInput.keys[tt::input::Key_Alt    ].down          &&
	    debugInput.keys[tt::input::Key_Control].down == false &&
	    debugInput.keys[tt::input::Key_Shift  ].down == false)
	{
		m_enableMemoryBudgetWarning = (m_enableMemoryBudgetWarning == false);
	}
	
	// Update texture memory usage once in a while (because it is relatively expensive)
	// Do update the counter every frame if the frame counter is visible (for instant feedback)
	static s32 counterUpdateTexMemUsage = 0;
	--counterUpdateTexMemUsage;
	if (counterUpdateTexMemUsage <= 0 || tt::app::getApplication()->shouldDisplayDebugInfo())
	{
		// MARTIJN: Trying to 'fix' the ResourceCache thread crash bug
		g_texMemTotalInBytes      = 0;//tt::engine::renderer::TextureCache::getTotalMemSize();
		g_texMemShoeboxEnvInBytes = AppGlobal::hasGame() ?
				AppGlobal::getGame()->getEnvironmentShoeboxTexMemSize() : 0;
		g_texMemShoeboxShadowMaskInBytes = AppGlobal::hasGame() ?
				AppGlobal::getGame()->getShadowMaskShoeboxTexMemSize() : 0;
		g_texMemPrecacheTexInBytes  = AppGlobal::getPrecacheTextureMemUsage();
		g_texMemPrecachePresInBytes = AppGlobal::getPrecachePresentationMemUsage();
		
		counterUpdateTexMemUsage = 30;
	}
#endif
	
#if TT_DEMO_BUILD == 0
#if defined(TT_STEAM_BUILD)
	// Update leaderboards
	if (tt::steam::Leaderboards::hasInstance())
	{
		tt::steam::Leaderboards::getInstance()->update();
	}
#endif
#endif
	tt::engine::debug::DebugStats::endUpdate();
}


void AppMain::render()
{
	tt::engine::debug::DebugStats::beginRender();
	tt::engine::renderer::Renderer* renderer(tt::engine::renderer::Renderer::getInstance());
	if (AppGlobal::shouldTakeLevelScreenshot())
	{
		takeLevelScreenshot();
		
		// Restore main camera
		renderer->getMainCamera()->select();
	}

	u64 rendertime = tt::system::Time::getInstance()->getMicroSeconds();
	
	TT_NULL_ASSERT(m_stateMachine);
	m_stateMachine->render();
	
	static s32 fps   = 0;
	
	rendertime = tt::system::Time::getInstance()->getMicroSeconds() - rendertime;
	
	static u64 startTime = 0;
	u64 endTime = tt::system::Time::getInstance()->getMilliSeconds();
	
	if ((endTime - startTime) >= 1000) // A second passed, update fps string.
	{
		startTime = endTime;
		
		m_showFps = tt::app::getApplication()->shouldDisplayDebugInfo() || // Did we (developers) turn on fps?
		            AppGlobal::shouldShowFps();                            // Was the show fps cmdline arg passed?
		
		if (m_showFps)
		{
			fps = static_cast<s32>(tt::engine::debug::DebugStats::getAverageFPS(60) + 0.5f);
			const s32 updateTime = static_cast<s32>(tt::engine::debug::DebugStats::getAverageUpdateTime(60));
			const s32 renderTime = static_cast<s32>(tt::engine::debug::DebugStats::getAverageRenderTime(60));
			const s32 maxUpdateTime = static_cast<s32>(tt::engine::debug::DebugStats::getMaxUpdateTime(60));
			const s32 maxRenderTime = static_cast<s32>(tt::engine::debug::DebugStats::getMaxRenderTime(60));
			tt::engine::renderer::TexturePainter painter(m_fpsQuad->getTexture()->lock());
			drawFPSString(fps, updateTime, renderTime, maxUpdateTime, maxRenderTime, painter);
		}
	}
	
	if(m_showFps && AppGlobal::isBusyLoading() == false)
	{
#if defined(TT_PLATFORM_WIN)
	static const real pixelPerfectOffset = 0.5f;
#else
	static const real pixelPerfectOffset = 0.0f;
#endif
		const s32 fpsQuadX = static_cast<s32>(tt::engine::renderer::Renderer::getInstance()->getScreenWidth() - m_fpsQuad->getWidth() / 2);
		const s32 fpsQuadY = static_cast<s32>(m_fpsQuad->getHeight() / 2);
		m_fpsQuad->setPosition(fpsQuadX + pixelPerfectOffset, fpsQuadY + pixelPerfectOffset, 0.0f);
		m_fpsQuad->update();
		
		renderer->beginHud();
		m_fpsQuad->render();
		renderer->endHud();
	}
	
	tt::engine::debug::DebugStats::endRender();
	renderDebugInfo(fps, rendertime);
}


void AppMain::overrideGraphicsSettings(tt::app::GraphicsSettings* p_current_OUT, const tt::math::Point2& p_desktopSize)
{
	AppOptions::createInstance();
	AppOptions& savedOptions = AppOptions::getInstance();
	
#if defined(TT_BUILD_FINAL) // Make sure we don't detect the app kills done during normal development.
	const tt::app::StartupState& startupState = tt::app::getApplication()->getStartupState();
	if (startupState.didLastRunExitCleanly() == false)
	{
		// Had problems with the previous run.
		// Lower FXSettings and turn off fullscreen.
		//savedOptions.setVisualFxEnabled(false);
		savedOptions.windowed = true;
		
	}
#endif
	
	if (savedOptions.fullscreenSize.x <= 0 || savedOptions.fullscreenSize.y <= 0)
	{
		// No (valid) fullscreen resolution yet. Use desktop size.
		savedOptions.setFullscreenSize(p_desktopSize);
	}
	p_current_OUT->fullscreenSize = savedOptions.fullscreenSize;
	
	if (savedOptions.windowedSize.x > 0 && savedOptions.windowedSize.y > 0)
	{
		p_current_OUT->windowedSize = savedOptions.windowedSize;
	}
	
	if (savedOptions.upscaleSize.x <= 0 || savedOptions.upscaleSize.y <= 0)
	{
		savedOptions.setUpscaleSize(p_desktopSize, savedOptions.fullscreenSize);
	}
	p_current_OUT->startUpscaleSize = savedOptions.upscaleSize;
	
	// Commandline can override saved settings.
	p_current_OUT->startWindowed = savedOptions.windowed || tt::args::CmdLine::getApplicationCmdLine().exists("windowed");
	
	savedOptions.saveIfDirty();
}


void AppMain::onRequestReloadAssets()
{
	// Do not reload when still loading the application
	if (m_stateMachine != 0 && m_stateMachine->getCurrentState() == statelist::StateID_LoadApp)
	{
		return;
	}

	// Do not reload while recording
	if(AppGlobal::getInputRecorder()->getState() == input::Recorder::State_Record)
	{
		return;
	}
	
	// Clear precache
	AppGlobal::clearPrecache();
	
	AppGlobal::clearShoeboxTextures();
	
	// Clear and reload script lists used for the editor
	AppGlobal::clearScriptLists();
	AppGlobal::loadScriptLists();
	
	// Reload the config hive
	tt::cfg::ConfigRegistry::reloadAllHives();
	
	// Forward the reload request to the currently active state
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onRequestReloadAssets();
	}
	
	// Reload precache
	AppGlobal::loadPrecache();
}


void AppMain::onPlatformMenuEnter()
{
	AppGlobal::getController(tt::input::ControllerIndex_One).onPlatformMenuEnter();
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onPlatformMenuEnter();
	}
}


void AppMain::onPlatformMenuExit()
{
	AppGlobal::getController(tt::input::ControllerIndex_One).onPlatformMenuExit();
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onPlatformMenuExit();
	}
}


void AppMain::onAppInactive()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onAppInactive();
	}
	
	if(AppGlobal::isFrameCounterAllowed())
	{
		AppGlobal::toggleFrameCounter();
	}
}


void AppMain::onAppActive()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onAppActive();
	}
	
	if(AppGlobal::isFrameCounterAllowed() == false)
	{
		AppGlobal::toggleFrameCounter();
	}
}


void AppMain::onAppPaused()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onAppPaused();
	}
}


void AppMain::onAppResumed()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onAppResumed();
	}
}


void AppMain::onAppEnteredBackground()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onAppEnteredBackground();
	}
}


void AppMain::onAppLeftBackground()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onAppLeftBackground();
	}
}


void AppMain::onLostDevice()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onLostDevice();
	}
}


void AppMain::onResetDevice()
{
	if (m_stateMachine != 0 && m_stateMachine->getCurrentStatePtr() != 0)
	{
		m_stateMachine->getCurrentStatePtr()->onResetDevice();
	}
	
	// When in windowed mode, store the size of the window. (We support user resizing.)
	if (tt::engine::renderer::Renderer::hasInstance() &&
	    tt::app::getApplication()->isFullScreen() == false)
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		
		AppOptions::getInstance().setWindowedSize(renderer->getBackbufferSize());
	}
	
	// The user can switch to full screen using application short cuts (alt + enter)
	// We can detect such a change here and store in as the new options setting here.
	AppOptions::getInstance().setWindowed(tt::app::getApplication()->isFullScreen() == false);
	
	AppOptions::getInstance().saveIfDirty();
}


void AppMain::onSetPlayerCount(u32 p_newCount)
{
	if (script::ScriptMgr::getVM() != nullptr)
	{
		script::ScriptMgr::getVM()->callSqFun("onSetPlayerCount", p_newCount);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void AppMain::renderDebugInfo(s32, u64 p_renderTime)
{
#if !defined(TT_BUILD_FINAL)
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	
	// Do not render debug text twice (Rendering debug text to DRC not working yet)
	if (renderer->getActiveViewPort() == tt::engine::renderer::ViewPortID_DRC)
	{
		// NOTE: This assumes that the rest of this function only renders debug text, nothing else
		return;
	}
	
	static const real oneMB = 1024.0f * 1024.0f;  // 1 MB in bytes
	
	const bool shouldDisplayDebugInfo = tt::app::getApplication()->shouldDisplayDebugInfo();
	
	tt::engine::debug::DebugRendererPtr debug = renderer->getDebug();
	static const s32 lineHeight = 15;
	static const s32 x = 10;
	s32 y = 10;
	using tt::str::toStr;
	
	if (shouldDisplayDebugInfo)
	{
		game::Game* game = AppGlobal::hasGame() ? AppGlobal::getGame() : 0;

		static s32 updateTimer = 0;
		static real fps  = 60.f;
		static real mspf = 16.7f;
		static real maxmspf = 16.7f;
		static const s32 frameWindow = 60;

		updateTimer += 1;
		if (updateTimer > frameWindow)
		{
			updateTimer = 0;

			using tt::engine::debug::DebugStats;
			fps     = DebugStats::getAverageFPS(frameWindow);
			mspf    = DebugStats::getAverageFrameTime(frameWindow);
			maxmspf = DebugStats::getMaxFrameTime(frameWindow);
		}
		
		{
			char msg[256] = { 0 };
			sprintf(msg, "FPS: %.1f (%.1f ms) [%.1f ms]", fps, mspf, maxmspf);
			debugTextWithShadow(debug, msg, x, y);
		}
		
		if (game != 0)
		{
			char msg[256] = { 0 };
			sprintf(msg, "TV FOV: %.2f", game->getCamera().getCurrentFOV());
			debugTextWithShadow(debug, msg, x + 160, y);
			
			sprintf(msg, "DRC FOV: %.2f", game->getDrcCamera().getCurrentFOV());
			debugTextWithShadow(debug, msg, x + 270, y);
			
			sprintf(msg, "Debug FOV: %.2f", game->getCamera().getDebugFOV());
			debugTextWithShadow(debug, msg, x + 380, y);
			
			sprintf(msg, "Load Times [App: %.2f | Level: %.2f | Total: %.2f]",
				(AppGlobal::getLoadTimeApp() / 1000.0f),
				(AppGlobal::getLoadTimeLevel() / 1000.0f),
				((AppGlobal::getLoadTimeApp() + AppGlobal::getLoadTimeLevel()) / 1000.0f));
			debugTextWithShadow(debug, msg, x + 490, y);
		}
		
		y += lineHeight;
		
		/*tt::engine::renderer::Renderer* renderer(tt::engine::renderer::Renderer::getInstance());
		std::stringstream s;
		s << "Resolution: " << renderer->getScreenWidth() << "x" << renderer->getScreenHeight();
		debugTextWithShadow(debug, s.str(), x, y);
		y += lineHeight;*/
		
		debugTextWithShadow(debug, "Update Count: " + toStr(m_updateCount),       x,       y);
		debugTextWithShadow(debug, "Time: Avg "     + toStr(m_averageUpdateTime), x + 110, y);
		debugTextWithShadow(debug, "Min "           + toStr(m_minUpdateTime),     x + 220, y);
		debugTextWithShadow(debug, "Max "           + toStr(m_maxUpdateTime),     x + 290, y);
		
		m_averageUpdateTime = m_minUpdateTime = m_maxUpdateTime = 0;
		y += lineHeight;
		
		debugTextWithShadow(debug, "Update Frame: " + toStr(AppGlobal::getUpdateFrameCount()), x, y);
		y += lineHeight;
		
		debugTextWithShadow(debug, "Render Time: " + toStr(p_renderTime), x, y);
		y += lineHeight;
		
		if (hasLoadedGame())
		{
			game::entity::EntityMgr& entityMgr = game->getEntityMgr();
			
			debugTextWithShadow(debug, "Entities: " + toStr(entityMgr.getActiveEntitiesCount()), x, y);
			debugTextWithShadow(debug, "DMCs: "     + toStr(entityMgr.getMovementControllerMgr().getActiveControllerCount()),
			                    x + 95, y);
			debugTextWithShadow(debug, "Sensors: "  + toStr(entityMgr.getSensorMgr().getActiveSensorCount()),
			                    x + 180, y);
			debugTextWithShadow(debug, "Pres: "     + toStr(game->getPresentationObjectMgr().getActiveCount()),
			                    x + 280, y);
			debugTextWithShadow(debug, "Level: "    + game->getStartInfo().getLevelName(),
			                    x + 360, y);
			
			y += lineHeight;
		}
	}
	
	const bool texMemTotalOverBudget        = g_texMemTotalInBytes        > g_texMemBudgetTotalInBytes;
	const bool texMemShoeboxOverBudget      = g_texMemShoeboxEnvInBytes   > g_texMemBudgetShoeboxInBytes;
	const bool texMemPrecacheTexOverBudget  = g_texMemPrecacheTexInBytes  > g_texMemBudgetPrecacheTexInBytes;
	const bool texMemPrecachePresOverBudget = g_texMemPrecachePresInBytes > g_texMemBudgetPrecachePresInBytes;
	const bool texMemOverBudget             = texMemTotalOverBudget       || texMemShoeboxOverBudget      ||
	                                          texMemPrecacheTexOverBudget || texMemPrecachePresOverBudget;
	
	if (shouldDisplayDebugInfo || (m_enableMemoryBudgetWarning && texMemOverBudget))
	{
		const real texMemTotalInMB             = g_texMemTotalInBytes             / oneMB;
		const real texMemShoeboxEnvInMB        = g_texMemShoeboxEnvInBytes        / oneMB;
		const real texMemShoeboxShadowMaskInMB = g_texMemShoeboxShadowMaskInBytes / oneMB;
		const real texMemPrecacheTexInMB       = g_texMemPrecacheTexInBytes       / oneMB;
		const real texMemPrecachePresInMB      = g_texMemPrecachePresInBytes      / oneMB;
		
		std::string overBudgetType;
		if (texMemOverBudget)
		{
			overBudgetType = " ==> Over Budget with: ";
			bool first = true;
			if (texMemTotalOverBudget)
			{
				if (first == false) { overBudgetType += ", "; } else { first = false; }
				overBudgetType += "total";
			}
			if (texMemShoeboxOverBudget)
			{
				if (first == false) { overBudgetType += ", "; } else { first = false; }
				overBudgetType += "shoebox";
			}
			if (texMemPrecacheTexOverBudget)
			{
				if (first == false) { overBudgetType += ", "; } else { first = false; }
				overBudgetType += "precache textures";
			}
			if (texMemPrecachePresOverBudget)
			{
				if (first == false) { overBudgetType += ", "; } else { first = false; }
				overBudgetType += "precache presentation";
			}
		}
		
		char msg[256] = { 0 };
		sprintf(msg, "Textures: %.2f MB total (environment shoebox: %.2f MB -- shadow mask shoebox: %.2f MB -- precache textures: %.2f MB -- precache presentation: %.2f MB)%s",
		        texMemTotalInMB, texMemShoeboxEnvInMB, texMemShoeboxShadowMaskInMB, texMemPrecacheTexInMB, texMemPrecachePresInMB, overBudgetType.c_str());
		debugTextWithShadow(debug, msg, x, y, 
				(m_enableMemoryBudgetWarning && texMemOverBudget) ?
					tt::engine::renderer::ColorRGB::red : g_debugColor);
		y += lineHeight;
	}
	
	if (shouldDisplayDebugInfo && tt::engine::particles::ParticleMgr::hasInstance())
	{
		using namespace tt::engine::particles;
		ParticleMgr* pm = ParticleMgr::getInstance();
		char msg[256] = { 0 };
		sprintf(msg, "Particles Triggers: %3d (%3d) Emitters: %3d (%3d) Particles: %3d (%3d)",
		        pm->getNonCulledTriggerCount(), pm->getTriggerCount(),
		        pm->getNonCulledEmitterCount(), pm->getEmitterCount(),
		        pm->getNonCulledParticleCount(), pm->getParticleCount());
		
		debugTextWithShadow(debug, msg, x, y);
		y += lineHeight;
	}
	
	if (shouldDisplayDebugInfo)
	{
		int backBufferWidth  = renderer->getScreenWidth();
		int backBufferHeight = renderer->getScreenHeight();
		bool vsync = true;
		int  refreshRate = 0;

#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
		IDirect3DDevice9* device = tt::engine::renderer::getRenderDevice();
		if (device != 0)
		{ 
			IDirect3DSwapChain9* swapChain(0);
			device->GetSwapChain(0, &swapChain);

			D3DPRESENT_PARAMETERS presentParameters = { 0 };
			swapChain->GetPresentParameters(&presentParameters);
			swapChain->Release();

			backBufferWidth  = presentParameters.BackBufferWidth;
			backBufferHeight = presentParameters.BackBufferHeight;
			refreshRate      = presentParameters.FullScreen_RefreshRateInHz;
			vsync            = presentParameters.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE;
		}
#endif
		
		
		char msg[256] = { 0 };
		sprintf(msg, "%dx%d @ %dHz Vsync %s", backBufferWidth, backBufferHeight, refreshRate, vsync ? "ON" : "OFF");
		debugTextWithShadow(debug, msg, x, y);
		
		if (hasLoadedGame())
		{
			y += lineHeight * 2;
			game::entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
			const tt::str::Strings audioCues(audio::AudioPlayer::hasInstance() ?
						tt::str::explode(audio::AudioPlayer::getInstance()->getDebugPositionalSoundNames(), "\n") :
						tt::str::Strings());
			const tt::str::Strings culledInfo(tt::str::explode(entityMgr.getDebugCulledInfo(), "\n"));
			const tt::str::Strings unculledInfo(tt::str::explode(entityMgr.getDebugUnculledInfo(), "\n"));
			const tt::str::Strings particleEffects(tt::engine::particles::ParticleMgr::hasInstance() ?
						tt::str::explode(tt::engine::particles::ParticleMgr::getInstance()->getDebugParticleNames(), "\n") :
						tt::str::Strings());
			const tt::str::Strings scriptTimings(tt::str::explode(entityMgr.getDebugTimings(), "\n"));
			
			const s32 startY = y;
			for (tt::str::Strings::const_iterator it = unculledInfo.begin(); it != unculledInfo.end(); ++it)
			{
				debugTextWithShadow(debug, *it, x, y);
				y += lineHeight;
			}
			y = startY;
			for (tt::str::Strings::const_iterator it = culledInfo.begin(); it != culledInfo.end(); ++it)
			{
				debugTextWithShadow(debug, *it, x + 200, y);
				y += lineHeight;
			}
			y = startY;
			for (tt::str::Strings::const_iterator it = audioCues.begin(); it != audioCues.end(); ++it)
			{
				debugTextWithShadow(debug, *it, x + 400, y);
				y += lineHeight;
			}
			y = startY;
			for (tt::str::Strings::const_iterator it = particleEffects.begin(); it != particleEffects.end(); ++it)
			{
				debugTextWithShadow(debug, *it, x + 600, y);
				y += lineHeight;
			}
			y = startY;
			for (tt::str::Strings::const_iterator it = scriptTimings.begin(); it != scriptTimings.end(); ++it)
			{
				debugTextWithShadow(debug, *it, x + 800, y);
				y += lineHeight;
			}
		}
	}
	
#else
	(void)p_renderTime;
#endif  // !defined(TT_BUILD_FINAL)
}


void AppMain::initializePostProcessing()
{
	using namespace tt::engine::renderer;
	Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	pp::PostProcessorPtr postProcessor(renderer->getPP());
	
	if (tt::engine::renderer::hasShaderSupport() == false)
	{
		return;
	}
	
	// Load shaders
	pp::FilterPtr identity       = pp::Filter::create(ShaderCache::get("Identity",  "shaders"));
	pp::FilterPtr upscale        = pp::Filter::create(ShaderCache::get("Identity",  "shaders"));
	pp::FilterPtr blurHorizontal = pp::Filter::create(ShaderCache::get("GaussBlurH","shaders"));
	pp::FilterPtr blurVertical   = pp::Filter::create(ShaderCache::get("GaussBlurV","shaders"));
	pp::FilterPtr boxBlur        = pp::Filter::create(ShaderCache::get("BoxBlur",   "shaders"));
	pp::FilterPtr triBlur        = pp::Filter::create(ShaderCache::get("TriBlur",   "shaders"));
	pp::FilterPtr lightGlow      = pp::Filter::create(ShaderCache::get("LightGlow", "shaders"));
	pp::FilterPtr colorGrading   = pp::Filter::create(ShaderCache::get("ColorGrading", "shaders"));
	
	// Cache all these shaders so they won't be reloaded anymore during level switches.
	m_cachedShaders.clear();
	m_cachedShaders.push_back(identity);
	m_cachedShaders.push_back(upscale);
	m_cachedShaders.push_back(blurHorizontal);
	m_cachedShaders.push_back(blurVertical);
	m_cachedShaders.push_back(boxBlur);
	m_cachedShaders.push_back(triBlur);
	m_cachedShaders.push_back(lightGlow);
	m_cachedShaders.push_back(colorGrading);
	
	if (identity != 0)
	{
		postProcessor->initialize(identity, pp::UseDepthBuffer_Disabled);
		postProcessor->setActive(AppOptions::getInstance().postProcessing);
	}
	
	AppGlobal::setDoubleUpdateMode(AppOptions::getInstance().in30FpsMode);
	
	// Initialize blur system
	real renderTargetRatio(0.4f);
	switch (AppOptions::getInstance().blurQuality)
	{
	case tt::engine::scene2d::BlurQuality_NoBlur             : renderTargetRatio = 0.10f; break;
	case tt::engine::scene2d::BlurQuality_OnePassThreeSamples: renderTargetRatio = 0.20f; break;
	case tt::engine::scene2d::BlurQuality_OnePassFourSamples : renderTargetRatio = 0.25f; break;
	case tt::engine::scene2d::BlurQuality_TwoPassConvolution : renderTargetRatio = 0.40f; break;
	}
	
	tt::engine::scene::SceneBlurMgr::initialize(
		blurHorizontal, blurVertical, upscale, boxBlur, triBlur, renderTargetRatio);
	
	game::light::LightMgr::setLightGlowsFilter(lightGlow);
	game::light::LightMgr::setShadowFilter(blurHorizontal, blurVertical);
	
	FixedFunction::setActive();
}


bool AppMain::hasLoadedGame() const
{
	return AppGlobal::hasGame() &&
	       m_stateMachine->getCurrentState() != statelist::StateID_LoadGame;
}


void AppMain::takeLevelScreenshot()
{
	if (hasLoadedGame() == false)
	{
		TT_PANIC("Cannot take level screenshot when no level is loaded.");
		return;
	}
	
	const ScreenshotSettings& settings(AppGlobal::getLevelScreenshotSettings());
	if (settings.type == ScreenshotType_None)
	{
		TT_PANIC("takeLevelScreenshot was called when no screenshot should be taken.");
		return;
	}
	
	// The screenshot request should be disabled when this function returns and the camera settings
	// must be restored, so make sure that always happens
	class DisableScreenshotUponReturn
	{
	public:
		DisableScreenshotUponReturn()
		:
		m_originalDistance(AppGlobal::getGame()->getCamera().getCameraDistance()),
		m_originalPosition(AppGlobal::getGame()->getCamera().getTargetPosition()),
		m_originalFov     (AppGlobal::getGame()->getCamera().getCurrentFOV())
		{ }
		inline ~DisableScreenshotUponReturn()
		{
			AppGlobal::stopLevelScreenshot();
			
			// Restore Camera Settings
			game::Camera& gameCamera = AppGlobal::getGame()->getCamera();
			gameCamera.restoreAspectRatio();
			gameCamera.setCameraDistance(m_originalDistance);
			gameCamera.setPosition      (m_originalPosition, false);
			gameCamera.setFOV           (m_originalFov,      true);
			gameCamera.update(0.0f);
		}
	private:
		real              m_originalDistance;
		tt::math::Vector2 m_originalPosition;
		real              m_originalFov;
	};
	DisableScreenshotUponReturn noMoreScreenshot;
	
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	game::Game*   game       = AppGlobal::getGame();
	game::Camera& gameCamera = game->getCamera();
	
	if (settings.type == ScreenshotType_FullLevel)
	{
		TT_Printf("Taking screenshot of entire level...\n");
		
		const real texelsPerTile = cfg()->getRealDirect("toki.level_screenshot_texels_per_tile");
		
		// Get current level dimensions
		const s32 levelWidthInTiles  = game->getAttributeLayer()->getWidth();
		const s32 levelHeightInTiles = game->getAttributeLayer()->getHeight();
		
		const s32 captureWidth  = static_cast<s32>(texelsPerTile * levelWidthInTiles);
		const s32 captureHeight = static_cast<s32>(texelsPerTile * levelHeightInTiles);
		
		using namespace tt::engine::renderer;
		RenderTargetPtr target = createRenderTarget(captureWidth, captureHeight, ColorRGBA(255,255,255,0));
		if (target == 0) return;
		
		const real cameraDistance = 2.0f * levelWidthInTiles;
		
		// Set the fov to match the texels / tile setting
		const real desiredFov = 2 * tt::math::atan2(0.5f * levelHeightInTiles, cameraDistance);
		
		const tt::math::Vector2 position(levelWidthInTiles / 2.0f, levelHeightInTiles / 2.0f);
		
		gameCamera.setCameraDistance(cameraDistance);
		gameCamera.setLevelAspectRatio(texelsPerTile);
		gameCamera.setFOV(tt::math::radToDeg(desiredFov), true);
		gameCamera.setPosition(position, true);
		gameCamera.updateWithCurrentState();
		
		// Turn off background and foreground shoebox layers
		game->setGameLayerVisible(game::GameLayer_ShoeboxBackground, false);
		game->setGameLayerVisible(game::GameLayer_ShoeboxForeground, false);
		
		m_stateMachine->updateForRender(0.0f);
		
		// RENDER START
		
		// Apply new camera setting to render
		renderer->getMainCamera()->select();
		
		// Render level to render target
		RenderTargetStack::push(target, ClearFlag_All);
		m_stateMachine->render();
		game->getLightMgr().restoreAlphaChannel();
		RenderTargetStack::pop();
		
		// RENDER END
		
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
		// Save level screenshot
		std::string filename = tt::system::getDesktopPath() +
			game->getLevelData()->getLevelFilename() + ".png";
		if (SUCCEEDED(D3DXSaveTextureToFileA(filename.c_str(), D3DXIFF_PNG, target->getTexture()->getD3DTexture(), 0)))
		{
			TT_Printf("Screenshot saved successully!\n");
		}
#endif
		
		game->setGameLayerVisible(game::GameLayer_ShoeboxBackground, true);
		game->setGameLayerVisible(game::GameLayer_ShoeboxForeground, true);
	}
	else if (settings.type == ScreenshotType_LevelPreview)
	{
		TT_Printf("Taking screenshot with current camera settings, size %d x %d, saving to file '%s' on FS ID %d...\n",
		          settings.width, settings.height, settings.filename.c_str(), settings.fileSystem);
		
		const s32 captureWidth  = settings.width;
		const s32 captureHeight = settings.height;
		
		using namespace tt::engine::renderer;
		RenderTargetPtr target = createRenderTarget(captureWidth, captureHeight, renderer->getClearColor());
		if (target == 0)
		{
			saveScreenshotFallbackImage(settings);
			return;
		}
		
		renderer->getMainCamera()->setViewPort(
			0, 0, static_cast<real>(captureWidth), static_cast<real>(captureHeight));
		renderer->getMainCamera()->update();
		
		m_stateMachine->updateForRender(0.0f);
		
		// RENDER START
		
		// Apply new camera setting to render
		renderer->getMainCamera()->select();
		
		// Render level to render target
		RenderTargetStack::push(target, ClearFlag_All);
		m_stateMachine->render();
		game->getLightMgr().restoreAlphaChannel();
		
#if defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_SDL)
		const s32 pitch(4 * captureWidth);
		u8* imageData = new u8[pitch * captureHeight];
		glReadPixels(0, 0, captureWidth, captureHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		TT_CHECK_OPENGL_ERROR();
		
		if (imageData != 0)
		{
			// Flip resulting image
			u8* flippedImageData = new u8[pitch * captureHeight];
			u8* dstRow = flippedImageData;
			u8* srcRow = imageData + pitch * (captureHeight -1);
			
			for (s32 i = 0; i < captureHeight; ++i)
			{
				tt::mem::copy8(dstRow, srcRow, pitch);
				dstRow += pitch;
				srcRow -= pitch;
			}
			delete[] imageData;
			
			tt::fs::FilePtr file(tt::fs::open(settings.filename, tt::fs::OpenMode_Write, settings.fileSystem));
			if (file != 0)
			{
				tt::compression::PixelData pixelData;
				pixelData.width    = captureWidth;
				pixelData.height   = captureHeight;
				pixelData.depth    = 1;
				pixelData.bitDepth = 8;
				pixelData.format   = tt::ImageFormat_RGBA8;
				pixelData.pixels   = flippedImageData;
				
				if (tt::compression::compressPNG(file, pixelData))
				{
					TT_Printf("Screenshot saved successully!\n");
				}
				else
				{
					file.reset();
					saveScreenshotFallbackImage(settings);
				}
			}
			else
			{
				saveScreenshotFallbackImage(settings);
			}
			
			delete[] flippedImageData;
		}
#endif
		RenderTargetStack::pop();
		
		// RENDER END
		
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
		// Save level screenshot
		ID3DXBuffer* imageData = 0;
		D3DXSaveTextureToFileInMemory(&imageData, D3DXIFF_PNG, target->getTexture()->getD3DTexture(), 0);
		TT_ASSERTMSG(imageData != 0, "Could not create PNG image data from render target for screenshot.");
		if (imageData != 0)
		{
			tt::fs::FilePtr file(tt::fs::open(settings.filename, tt::fs::OpenMode_Write, settings.fileSystem));
			TT_ASSERTMSG(file != 0, "Could not open screenshot file '%s' for writing.", settings.filename.c_str());
			if (file != 0)
			{
				file->write(imageData->GetBufferPointer(), static_cast<tt::fs::size_type>(imageData->GetBufferSize()));
				TT_Printf("Screenshot saved successully!\n");
			}
			else
			{
				saveScreenshotFallbackImage(settings);
			}
			tt::safeRelease(imageData);
		}
		else
		{
			saveScreenshotFallbackImage(settings);
		}
#endif
	}
}


// Namespace end
}
