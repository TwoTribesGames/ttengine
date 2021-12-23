#include <sstream>

#include <tt/app/SDL2App.h>
#include <tt/args/CmdLine.h>
#include <tt/cfg/ConfigRegistry.h>
#if defined(TT_PLATFORM_WIN)
#include <tt/app/ComHelper.h>
#include <tt/fs/WindowsFileSystem.h>
typedef tt::fs::WindowsFileSystem  SDL2FileSystem;
#else
#include <tt/fs/PosixFileSystem.h>
typedef tt::fs::PosixFileSystem  SDL2FileSystem;
#endif
#include <tt/fs/MemoryFileSystem.h>
#include <tt/engine/debug/DebugRenderer.h>
//#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/OpenGLContextWrapper.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/http/HttpConnectMgr.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/input/ControllerType.h>
#include <tt/input/SDLMouseController.h>
#include <tt/input/SDLKeyboardController.h>
#include <tt/input/SDLJoypadController.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_error_sdl2.h>
#include <tt/platform/tt_printf.h>
#include <tt/app/fatal_error.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>
#include <tt/version/Version.h>

#include <SDL2/SDL.h>

#include <lodepng/SDL_LodePNG.h>

namespace tt {
namespace app {

#if !defined(TT_BUILD_FINAL)
// This function is registered with tt error to be called when a panic is triggered.
static void onPanic()
{
	// If we are running in fullscreen, first switch back to windowed
//	tt::app::getApplication()->setFullScreen(false);
}
#endif //#if !defined(TT_BUILD_FINAL)

u32   FrameRateManager::ms_targetFPS    = 0;
float FrameRateManager::ms_averageFPS   = 0;
bool  FrameRateManager::ms_vsyncEnabled = false;
FrameRateManager::FrameRates         FrameRateManager::ms_framerateHistory;

//--------------------------------------------------------------------------------------------------
// Public member functions

SDL2App::SDL2App(AppInterface* p_app, const AppSettings& p_settings, const std::string& p_appIcon)
:
m_startupState(p_settings.version, version::getLibRevisionNumber()),
m_settings(p_settings),
m_app(p_app),
m_initialized(false),
m_frameTime(system::Time::getInstance()->getMicroSeconds()),
m_targetTimeSlice(16667),
m_fps30Mode(false),
m_active(true),
m_done(false),
m_frameLimiterEnabled(true),
m_frameLimiterForced(false),
#if !defined(TT_BUILD_FINAL)
m_curWaitFrame(0),
m_totalWaitFrames(0),
m_frameStepMode(false),
m_updateTime(0),
m_renderTime(0),
m_shouldDisplayDebugInfo(false),
#endif
m_cmdLine(args::CmdLine::getApplicationCmdLine()),
m_screen(0),
m_last(0, 0),
m_debugKeys(DebugKeys_All)
{
	// Initialize filesystem NOW!!
	if (m_settings.useMemoryFS)
	{
		m_memfs = fs::MemoryFileSystem::instantiate(0, 2);
		if (m_memfs == 0)
		{
			reportFatalError("Could not initialize memory filesystem.");
		}
	}
	m_hostfs = SDL2FileSystem::instantiate(m_settings.useMemoryFS ? 2 : 0, p_settings.name);
	if (m_hostfs == 0)
	{
		reportFatalError("Could not initialize filesystem.");
	}
	// Lazy Initialization of startupState
	m_startupState.initialize(m_hostfs->getSaveRootDir());

#if defined(TT_PLATFORM_WIN)
	ComHelper::initCom();
#endif

	TT_NULL_ASSERT(m_app);
	registerPlatformCallbackInterface(m_app);
	
	m_startupState.setStartupStep(StartupStep_SystemInit);
	
	//
	// General Initialization
	//
	version::setClientVersionInfo(m_settings.version, m_settings.versionString.c_str());
	settings::setRegion(m_settings.region);
	settings::setApplicationName(str::widen(m_settings.name));

	// Initialize SDL & register SDL cleanup
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	if (SDL_GetNumVideoDisplays() > 1) {
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	}

	SDL_DisableScreenSaver();

	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(0, &mode);
	m_desktopSize = tt::math::Point2(mode.w, mode.h);

	makeApplicationAvailable(); // FIXME: move this to a point where asset path is also known
	
	bool showBuildLabel = true;
#if defined(TT_BUILD_FINAL)
	showBuildLabel = m_cmdLine.exists("version");
	
	str::Strings keepThese;
	
	keepThese.push_back("width");
	keepThese.push_back("height");
	keepThese.push_back("version");
	keepThese.push_back("no_aspect_ratio_restriction");
	keepThese.push_back("force_framelimiter");

	// Only keepThese command line options in final mode
	m_cmdLine.clear(keepThese);
#endif

	m_settings.hideCursor   = m_settings.hideCursor || m_cmdLine.exists("nocursor");

	// Frame limiter
	if (m_cmdLine.exists("fps"))
	{
		s32 fps(m_cmdLine.getInteger("fps"));
		if (fps > 0)
		{
			m_settings.targetFPS = fps;
		}
	}
	
	m_frameLimiterEnabled = (m_settings.targetFPS > 0);
	FrameRateManager::setTargetFramerate(m_settings.targetFPS);

	if (m_cmdLine.exists("no-framelimiter"))
	{
		m_frameLimiterEnabled = false;
		m_frameLimiterForced = false;
	}
	else
	{
		m_frameLimiterForced = true;
	}

	m_settings.graphicsSettings.useIOS2xMode = (m_settings.graphicsSettings.useIOS2xMode || m_cmdLine.exists("ios2xmode"));

#if TT_SUPPORTS_PLATFORM_EMULATION
	setPlatformEmulation(m_settings.emulate);
#endif

	// Adjust working directory
	{

		std::string assetRootDirectory;
		str::Strings checkPaths;

		// 0) Commandline override
		if (m_cmdLine.exists("dataDir"))
		{
			checkPaths.push_back(m_cmdLine.getString("dataDir"));
		}

		// 1) Current working directory
		checkPaths.push_back(fs::getWorkingDir());

		// 2) executable directory
		char *base = SDL_GetBasePath();
		std::string path = base;
		SDL_free(base);

		checkPaths.push_back(path);

		// 3) From packs folder	
		str::Strings subdirComponents(str::explode(path, "\\/"));
		while (subdirComponents.empty() == false && subdirComponents.back() != "packs")
		{
			subdirComponents.pop_back();
		}

		// Packs dir found
		if (subdirComponents.empty() == false)
		{
			subdirComponents.pop_back();
			subdirComponents.push_back("assets");
			subdirComponents.push_back("output");
			checkPaths.push_back(str::implode(subdirComponents, std::string(1, m_hostfs->getDirSeparator())));
		}

		// search checkPaths
		std::string assetRootDir;
		for (str::Strings::const_iterator it = checkPaths.begin(); it != checkPaths.end(); ++it)
		{
			std::string composedDir = composeAssetRootDir(*it);
			if (fs::dirExists(composedDir) && fs::fileExists(composedDir + "/namespace.txt"))
			{
				assetRootDir = composedDir;
				break;
			}
			else if (fs::fileExists(*it + "/namespace.txt"))
			{
				assetRootDir = *it;
				break;
			}

		}

		if (assetRootDir.empty())
		{
			std::string errorMsg = "Could not find data directory.\nThe following data directories were checked:\n";
			for (str::Strings::const_iterator it = checkPaths.begin(); it != checkPaths.end(); ++it)
			{
				errorMsg += (*it) + "\n";
			}

#ifdef TT_BUILD_FINAL
			errorMsg += "If you moved or removed this directory, please try to reinstall the game.";
#endif

			reportFatalError(errorMsg);
		}

		setAssetRootDir(assetRootDir);
	}

	// Create the application window
	if (createMainWindow(showBuildLabel, m_cmdLine, p_appIcon) == false)
	{
		reportFatalError("Could not create main application window.");
		return; // no use continuing here
	}
	platform::error::setAppWindowHandle(m_screen);

#if !defined(TT_BUILD_FINAL)
	tt::platform::error::registerPanicCallback(onPanic);
#endif

	// create the context
	SDL_GLContext context = SDL_GL_CreateContext(m_screen);
	if (context == 0)
	{
		TT_PANIC("Unable to create GL Context: %s", SDL_GetError());
	}
#if !defined(TT_BUILD_FINAL)
	int val;
	#define CHECK(x, y) if (SDL_GL_GetAttribute(x, &val)== 0) { \
			TT_Printf("GL Check: %s == %d\n", #y, val); \
		} else {\
			TT_Printf("GL Check: %s -- failed to fetch: %s\n", #y, SDL_GetError()); \
		}
	CHECK(SDL_GL_ACCELERATED_VISUAL, "Accelerated");
	CHECK(SDL_GL_MULTISAMPLEBUFFERS, "MSAA Buffers");
	CHECK(SDL_GL_MULTISAMPLESAMPLES, "MSAA Samples");
#endif

//	if (SDL_GL_SetSwapInterval(-1) == -1) {
	SDL_GL_SetSwapInterval(1);
//	}
	
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		TT_PANIC("gl load failed");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Initialization Error", "Could not initialize OpenGL", 0);
		return;
	}

	// Create the renderer
	using tt::engine::renderer::Renderer;
	m_contextWrapper = new engine::renderer::OpenGLContextWrapper(m_screen, context);
	if (Renderer::hasInstance() == false &&
	    Renderer::createInstance(m_contextWrapper, m_settings.graphicsSettings.useIOS2xMode) == false)
	{
		reportFatalError("Could not initialize render system.");
	}
	
	Renderer* renderer(Renderer::getInstance());

	// Set target timeslice if target FPS is set.
	if (m_settings.targetFPS > 0)
	{
		m_targetTimeSlice = static_cast<u32>(1000000.0f / m_settings.targetFPS);
	}

	renderer->getDebug()->setBaseCaptureFilename(p_settings.name);
	renderer->setClearColor(tt::engine::renderer::ColorRGB::black);

	renderer->getUpScaler()->setMaxSize(p_settings.graphicsSettings.startUpscaleSize);
	if (m_cmdLine.exists("no_aspect_ratio_restriction") == false)
	{
		renderer->getUpScaler()->setAspectRatioRange(m_settings.graphicsSettings.aspectRatioRange);
	}

	renderer->handleResetDevice();

	bool framebufferSupport = engine::renderer::RenderTarget::hardwareSupportsFramebuffers();
	TT_Printf("Framebuffer support: %s\n", framebufferSupport ? "YES" : "NO");

	// Initialize engine's FileUtils
	tt::engine::file::FileUtils::getInstance()->generateNamespaceMapping();
	
#if !defined(TT_BUILD_FINAL)
	engine::renderer::FixedFunction::initialize(engine::EngineID("FixedFunctionDebug", "shaders"));
#else
	engine::renderer::FixedFunction::initialize(engine::EngineID("FixedFunction", "shaders"));
#endif

	// Create platform API
	if (p_settings.systems != 0)
	{
		m_platformApi = p_settings.systems->instantiatePlatformApi(this);
	}
	
	// Handle platform API initialization
	// FIXME: Properly handle these:
	//if (m_steamEnabled && cmdLine.exists("nosteam") == false)
	if (m_platformApi != 0)
	{
		if (m_platformApi->init() == false)
		{
			reportFatalError("Could not initialize platform API.");
		}
	}
	else
	{
		// Setup cloud FS to use regular filesystem
		m_cloudfs = SDL2FileSystem::instantiate(1, p_settings.name);
		if (m_cloudfs == 0)
		{
			reportFatalError("Could not initialize cloud filesystem.");
		}
	}

	// FIXME: how is this controlled?
	// m_settings.useFixedDeltaTime
	if (m_settings.useFixedDeltaTime)
	{
		reportFatalError("Not implemented!");
	}

	// Initialize audio system
	if (p_settings.systems != 0)
	{
		m_soundSystem = p_settings.systems->instantiateSoundSystem();
	}
	
	// Initialize the input controllers
	tt::input::SDLMouseController::initialize();
	tt::input::SDLKeyboardController::initialize();
	tt::input::SDLJoypadController::initialize();

	http::HttpConnectMgr::createInstance();

	// Have the client application initialize itself (should be last)
	m_startupState.setStartupStep(StartupStep_ClientInit);
	m_initialized = m_app->init();
	
	if (m_initialized == false)
	{
		reportFatalError("Could not initialize application.");
	}
	else
	{
		m_startupState.setStartupStep(StartupStep_Running);
	}
}


SDL2App::~SDL2App()
{
	m_startupState.setStartupStep(StartupStep_Shutdown);
	
	// Application Destruction
	unregisterPlatformCallbackInterface(m_app);
	delete m_app;
	m_app = 0;
	
	//
	// General Cleanup
	//
	
	// Deinitialize the controllers
	input::SDLKeyboardController::deinitialize();
	input::SDLMouseController::deinitialize();
	input::SDLJoypadController::deinitialize();
	
	http::HttpConnectMgr::destroyInstance();
	
	// Shut down renderer
	tt::engine::renderer::Renderer::destroyInstance();
	delete m_contextWrapper;
	
	system::Time::destroyInstance();
	
	m_cloudfs.reset();
	
	// Shut down the platform API
	if (m_platformApi != 0)
	{
		m_platformApi->shutdown();
	}
}


void SDL2App::update(real p_elapsedTime)
{
	if (m_active == false)
	{
		SDL_Delay(1);
		return;
	}

	m_frameTime = system::Time::getInstance()->getMicroSeconds();
	
	updateInputControllers();

	if (http::HttpConnectMgr::hasInstance())
	{
		http::HttpConnectMgr::getInstance()->processResponses();
	}

	// Update platform API
	if (m_platformApi != 0)
	{
		m_platformApi->update();
	}
	
	// Framestep handling
#if !defined(TT_BUILD_FINAL)
	if (m_curWaitFrame < m_totalWaitFrames)
	{
		++m_curWaitFrame;
		return;
	}
	
	// FIXME: Move keyhandling to handleCommonInput
	if (m_frameStepMode &&
	    input::SDLKeyboardController::getState(input::ControllerIndex_One).keys[input::Key_P].pressed == false)
	{
		return;
	}
	
	m_curWaitFrame = 0;
#endif
	
	// Update the renderer
	engine::renderer::Renderer::getInstance()->update(p_elapsedTime);
	
	// Update the application
	m_app->update(p_elapsedTime);
	
	if (m_fps30Mode)
	{
		// Extra update
		engine::renderer::Renderer::getInstance()->update(0.0f);
		m_app->update(0.0f);
	}
	
#if !defined(TT_BUILD_FINAL)
	// Compute update time
	u64 now(system::Time::getInstance()->getMicroSeconds());
	m_updateTime = static_cast<s32>(now - m_frameTime);
#endif
	
	//FrameRateManager::monitorFramerate();
	m_fps30Mode = engine::renderer::Renderer::getInstance()->isLowPerformanceMode();
}


void SDL2App::render()
{
	// DEBUG: Testing app behavior if no updates or renders are performed when app inactive
	if (m_active == false)
	{
		return;
	}
	
	using engine::renderer::Renderer;
	Renderer* renderer = Renderer::getInstance();
	
	// Render the frame
	renderer->beginFrame();
	{
		using engine::renderer::ViewPort;
		using engine::renderer::ViewPortContainer;
		
		for(ViewPortContainer::iterator it = ViewPort::getViewPorts().begin();
		    it != ViewPort::getViewPorts().end(); ++it)
		{
			renderer->beginViewPort(*it, it == (ViewPort::getViewPorts().end() - 1));
			
			// Make sure we're not rendering app with an openGL error.
			TT_CHECK_OPENGL_ERROR();
			
			m_app->render();
			
			// Make sure the app didn't have an openGL error.
			TT_CHECK_OPENGL_ERROR();
			
			// Last viewport
			if (it == (ViewPort::getViewPorts().end() - 1))
			{
				// Force an empty hud pass
				renderer->beginHud();
				renderer->endHud();
				
#if !defined(TT_BUILD_FINAL)
				if (m_shouldDisplayDebugInfo)
				{
					engine::debug::DebugRendererPtr debugPtr(renderer->getDebug());
					const s32 xpos = renderer->getScreenWidth() / 2;
					if (m_frameStepMode)
					{
						debugPtr->printf(xpos, 5, "FRAME STEP MODE");
					}
					else if (m_totalWaitFrames > 0)
					{
						debugPtr->printf(xpos, 5, "WAIT: %d", m_totalWaitFrames);
					}
					else if (m_totalWaitFrames < 0)
					{
						debugPtr->printf(xpos, 5, "FASTMODE");
					}
				}
#endif
			}
			
			renderer->endViewPort(*it);
		}
	}
	
	// Done rendering
	renderer->endFrame();

#if !defined(TT_BUILD_FINAL)
	// Compute render time
	u64 now(system::Time::getInstance()->getMicroSeconds());
	m_renderTime = static_cast<s32>(now - m_frameTime);
#endif
	
	// Framerate limiter
	if ((m_frameLimiterEnabled && FrameRateManager::isVsyncEnabled() == false) || m_frameLimiterForced)
	{
		u64 currentTime(system::Time::getInstance()->getMicroSeconds());
		u32 timePassed = static_cast<u32>(currentTime - m_frameTime);
		u32 targetTime = m_targetTimeSlice;

		static const u32 leaveTimeForVsync           =  300; // 0.3 ms
		static const u32 oneMilliSecondInNanoSeconds = 1000; // 1.0 ms

		while(timePassed < (targetTime - leaveTimeForVsync))
		{
			if((m_targetTimeSlice - timePassed) > (oneMilliSecondInNanoSeconds + leaveTimeForVsync))
			{
				// Sleep for 1ms
				SDL_Delay(1);
			}
			
			currentTime = system::Time::getInstance()->getMicroSeconds();
			
			timePassed = static_cast<u32>(currentTime - m_frameTime);
		}
	}
	
	m_frameTime = system::Time::getInstance()->getMicroSeconds();
	
	// Show frame
	renderer->present();
}


void SDL2App::onPlatformMenuEnter()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuEnter();
	}	
}


void SDL2App::onPlatformMenuExit()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuExit();
	}
}


void SDL2App::onAppActive()
{
	m_active = true;
	if (/*m_firstUpdate == false &&*/ m_initialized && m_app != 0)
	{
		for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
		{
			(*it)->onAppActive();
		}
	}
}


void SDL2App::onAppInactive()
{
	m_active = false;
	if (/*m_firstUpdate == false &&*/ m_initialized && m_app != 0)
	{
		for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
		{
			(*it)->onAppInactive();
		}
	}
}

s32 SDL2App::run()
{
	if (m_initialized == false)
	{
		return 1;
	}

	m_prevFrameTimestamp = system::Time::getInstance()->getMicroSeconds();
	while (!m_done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Handle events
			input::SDLMouseController::processEvent(event);
			input::SDLKeyboardController::processEvent(event);
			input::SDLJoypadController::processEvent(event);
			
			if (event.type == SDL_WINDOWEVENT) {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						if (!m_active) {
							onAppActive();
							if (isFullScreen()) {
								// Warp mouse
								SDL_WarpMouseInWindow(m_screen, m_last.x, m_last.y);
							}
						}
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST: {
						const char* hint = SDL_GetHint("IGNORE_FOCUS_LOST");
						if (m_active && (!hint || hint[0] != '1')) onAppInactive();
						{
							int x, y;
							SDL_GetMouseState(&x, &y);
							m_last.x = x;
							m_last.y = y;
						}
					}
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						tt::engine::renderer::Renderer::getInstance()->handleResetDevice();
						m_app->onResetDevice();
						break;
					case SDL_WINDOWEVENT_CLOSE:
						terminate(true);
						break;
				}
			}
			if (event.type == SDL_QUIT) {
				terminate(true);
			}
		}
		
		u64 deltaTime = m_targetTimeSlice;

		const u64 now = system::Time::getInstance()->getMicroSeconds();
		const u64 elapsedTimestamp = now - m_prevFrameTimestamp;
		
		m_prevFrameTimestamp = now;
		
		if (m_settings.useFixedDeltaTime == false) 
		{
			deltaTime = elapsedTimestamp;
		}
		
		update(static_cast<real>(deltaTime) / 1000000.0f);
		
		render();
	}
	
	return 0;
}


std::string SDL2App::getAssetRootDir() const
{
	return m_assetRootDir;
}


fs::identifier SDL2App::getSaveFsID() const
{
	return 1;
}


const args::CmdLine& SDL2App::getCmdLine() const
{
	return m_cmdLine;
}


const StartupState& SDL2App::getStartupState() const
{
	return m_startupState;
}


void SDL2App::setFullScreen(bool p_fullScreen)
{
    if (isFullScreen() != p_fullScreen) {
        bool good = true;
		int ret = SDL_SetWindowFullscreen(m_screen, p_fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		good = (ret == 0);
        if (!good) {
            reportFatalError("Failed to toggle video resolution!");
        }
		// device reset is handled in SDL_WINDOWEVENT_RESIZED
	}
}


bool SDL2App::isFullScreen() const
{
    if (!m_screen)
    {
        return false;
    }
	Uint32 flags = SDL_GetWindowFlags(m_screen);
	return (flags & SDL_WINDOW_FULLSCREEN) > 0;
}
	

void SDL2App::setFullScreenResolution(const math::Point2 &p_resolution)
{
	m_settings.graphicsSettings.fullscreenSize =
		m_settings.graphicsSettings.getCorrectedScreenSize(p_resolution);
}
	
	
tt::math::Point2 SDL2App::getFullScreenResolution() const
{
	return m_settings.graphicsSettings.fullscreenSize;
}


tt::math::Point2 SDL2App::getDesktopSize() const
{
    return m_desktopSize;
}


bool SDL2App::shouldDisplayDebugInfo() const
{
#if !defined(TT_BUILD_FINAL)
	return m_shouldDisplayDebugInfo;
#else
	return false;
#endif
}


void SDL2App::terminate(bool p_graceful)
{
    if (p_graceful) {
        m_done = true;
    } else {
        exit(1);
    }
}


void SDL2App::setPaused(bool /*p_paused*/)
{
	// Nothing to do here on SDL2
}

void SDL2App::setPlatformMenuEnabled(bool /*p_enabled*/)
{
	// Nothing to do here on SDL2
}


void SDL2App::setTargetFPS(u32 p_fps)
{
	m_settings.targetFPS = p_fps;
	m_fps30Mode = (p_fps == 30);
	
	if (m_cmdLine.exists("no-framelimiter") == false)
	{
		m_frameLimiterEnabled = (m_settings.targetFPS > 0);
	}
	
	if (m_settings.targetFPS > 0)
	{
		m_targetTimeSlice = static_cast<s32>(1000000.0f / m_settings.targetFPS);
	}
	FrameRateManager::setTargetFramerate(m_settings.targetFPS);
}
	
	
void SDL2App::handleResolutionChanged()
{
	if (engine::renderer::Renderer::hasInstance())
	{
		engine::renderer::Renderer::getInstance()->handleResetDevice();
		onResetDevice();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SDL2App::updateInputControllers()
{
	// Update the controllers
	input::SDLKeyboardController::update();
	input::SDLMouseController::update();
	input::SDLJoypadController::update();

	// Update the controller type
	tt::input::updateCurrentControllerType();

	handleCommonInput();
}

void SDL2App::handleCommonInput()
{
	const input::SDLKeyboardController& kbd(input::SDLKeyboardController::getState(input::ControllerIndex_One));

	if (kbd.keys[input::Key_Enter].pressed && kbd.keys[input::Key_Control].down)
	{
		TT_Printf("SDL2App::handleCommonInput: Toggle Fullscreen.\n");
		setFullScreen(isFullScreen() == false);
	}

#if !defined(TT_BUILD_FINAL)
	if(m_debugKeys == DebugKeys_None) return;

	using engine::renderer::Renderer;
	Renderer* renderer = Renderer::getInstance();

	const bool ctrlDown        = kbd.keys[input::Key_Control].down;
	const bool altDown         = kbd.keys[input::Key_Alt    ].down;
	const bool shiftDown       = kbd.keys[input::Key_Shift  ].down;
	const bool anyModifierDown = ctrlDown || altDown || shiftDown;

	if((m_debugKeys & DebugKeys_Function) != 0)
	{
		// console window
		if (kbd.keys[input::Key_F2].pressed && anyModifierDown == false)
		{
//			toggleConsole();
		}

		// Screenshot shortcut
		if (kbd.keys[input::Key_F3].down && ctrlDown == false && altDown == false)
		{
			using tt::engine::debug::DebugRenderer;
			if (kbd.keys[input::Key_Shift].down)
			{
				renderer->getDebug()->captureScreen(DebugRenderer::ScreenCaptureMode_ToClipboard);
			}
			else
			{
				renderer->getDebug()->captureScreen(DebugRenderer::ScreenCaptureMode_ToFile);
			}
		}

		if (kbd.keys[input::Key_F5].pressed && anyModifierDown == false)
		{
			// Send request to reload currently loaded assets
			for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
			{
				(*it)->onRequestReloadAssets();
			}
		}

		if (kbd.keys[input::Key_F6].pressed && anyModifierDown == false)
		{
			// Reload all graphical assets
			engine::renderer::TextureCache::reload();
			engine::renderer::ShaderCache::reload();
			engine::renderer::FixedFunction::setOverdrawModeEnabled(false);
		}

		if (kbd.keys[input::Key_F8].pressed && anyModifierDown == false)
		{
			renderer->getDebug()->toggleWireFrame();
		}

		// Safe Frame rendering support
		if (kbd.keys[input::Key_F9].pressed && anyModifierDown == false)
		{
			renderer->getDebug()->toggleSafeFrame();
		}

		// Axis display toggle
		if (kbd.keys[input::Key_F10].pressed && anyModifierDown == false)
		{
			renderer->getDebug()->toggleAxis(1.0f);
		}

		if (kbd.keys[input::Key_F11].pressed && anyModifierDown == false)
		{
			static bool lighting = true;
			lighting = (lighting == false);
			renderer->setLighting(lighting);
		}
	}

	// Normal keys (P,O,T,F)
	if((m_debugKeys & DebugKeys_Normal) == 0) return;

	// Overdraw mode
	if (kbd.keys[input::Key_O].pressed && anyModifierDown == false)
	{
		engine::debug::DebugRendererPtr debug(renderer->getDebug());
		// Toggle debug overdraw.
		debug->setOverdrawDebugActive(debug->isOverdrawDebugActive() == false);
		TT_Printf("WinApp::handleCommonInput - 'O' was pressed, toggling overdraw debug: %s\n",
				  debug->isOverdrawDebugActive() ? "On" : "Off");
	}

	// Mipmap Visualization Mode
	if (kbd.keys[input::Key_M].pressed && altDown)
	{
		engine::debug::DebugRendererPtr debug(renderer->getDebug());
		// Toggle mipmap visualization
		debug->setMipmapVisualizerActive(debug->isMipmapVisualizerActive() == false);
		TT_Printf("WinApp::handleCommonInput - 'M' was pressed, toggling mipmap visualization: %s\n",
				  debug->isMipmapVisualizerActive() ? "On" : "Off");
	}

	// Framestep keys
	if (m_frameStepMode)
	{
		if (kbd.keys[input::Key_P].pressed &&
			shiftDown                      &&
			ctrlDown == false              &&
			altDown  == false)
		{
			m_frameStepMode = false;
		}
	}
	else if (kbd.keys[input::Key_P].pressed && anyModifierDown == false)
	{
		m_frameStepMode = true;
	}

 	if (kbd.keys[input::Key_Plus].pressed && anyModifierDown == false && m_totalWaitFrames >= 0)
	{
		if (m_totalWaitFrames == 0)
		{
			m_totalWaitFrames = -1; // fast mode
		}
		else
		{
			m_totalWaitFrames /= 2;
		}
	}
	
	if (kbd.keys[input::Key_Minus].pressed && anyModifierDown == false)
	{
		if (m_totalWaitFrames <= 0)
		{
			++m_totalWaitFrames;
		}
		else
		{
			m_totalWaitFrames *= 2;
		}
		
		if (m_totalWaitFrames > MAX_WAIT_FRAMES)
		{
			m_totalWaitFrames = MAX_WAIT_FRAMES;
		}
	}

//	if (ctrlDown && kbd.keys[input::Key_T].pressed &&
//		altDown == false && shiftDown == false)
//	{
//		engine::cache::FileTextureCache::dumpToFile(m_textureCacheFileName);
//		engine::renderer::TextureCache::dumpToFile(m_textureCacheFileName);
//
//		//ShellExecuteA(NULL, "open", m_textureCacheFileName.c_str(), NULL, NULL, SW_SHOWNORMAL);
//	}
//
//	else if (kbd.keys[input::Key_T].pressed && shiftDown && ctrlDown == false && altDown == false)
//	{
//		engine::cache::FileTextureCache::dump();
//		engine::renderer::TextureCache::dump();
//	}

	if (kbd.keys[input::Key_F].pressed && shiftDown && ctrlDown == false && altDown == false)
	{
		m_shouldDisplayDebugInfo = (m_shouldDisplayDebugInfo == false);
	}
#endif  // !defined(TT_BUILD_FINAL)
}


bool SDL2App::setVideoMode(const bool p_windowed, const math::Point2& p_size, const std::string& title)
{
	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (!p_windowed) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	TT_Printf("Setting %s Video Mode %d,%d\n", p_windowed ? "Windowed" : "Fullscreen", p_size.x, p_size.y);
	m_screen = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, p_size.x, p_size.y, flags);
	if (!m_screen) {
		TT_Printf("SDL Error: %s\n", SDL_GetError());
	}
	m_last = math::Point2(p_size.x / 2, p_size.y / 2);
	return m_screen != 0;
}

bool SDL2App::createMainWindow(bool p_showBuildLabel, const args::CmdLine& p_cmdLine, const std::string& p_appIcon)
{
	AppSettings& settings(m_settings);
	determineScreenSize(&settings, m_app, getDesktopSize(), p_cmdLine);	
	
	const bool windowed = settings.graphicsSettings.startWindowed;
	math::Point2 size(settings.graphicsSettings.getScreenSize(true));

	// Build up the title
	std::string windowTitle;
	{
		std::ostringstream wndTitle;
		wndTitle << m_settings.name;

#if !defined(TT_BUILD_FINAL)
		wndTitle << " (REVISION: " << tt::version::getClientRevisionNumber()
				 << "." << tt::version::getLibRevisionNumber() << ")";

		if (isDualScreen())
		{
			wndTitle << " [DualSreen MODE]";
		}

		if (m_cmdLine.exists("window-title-suffix"))
		{
			wndTitle << " " << m_cmdLine.getString("window-title-suffix");
		}

		(void)p_showBuildLabel;
#else
		if (p_showBuildLabel)
		{
			wndTitle << " (build " << tt::version::getClientRevisionNumber()
					 << "." << tt::version::getLibRevisionNumber() << ")";
		}
#endif
		windowTitle = wndTitle.str();
	}
	
	// Set OpenGL Attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	if (!setVideoMode(windowed, size, windowTitle)) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		size = settings.graphicsSettings.minimumSize;
		if (!setVideoMode(windowed, size, windowTitle)) {
			reportFatalError("Could not create main window.");
		}
	}

	if (fs::fileExists(p_appIcon)) {
		SDL_Surface *icon = SDL_LodePNG(p_appIcon.c_str());
		if (icon) {
			SDL_SetWindowIcon(m_screen, icon);
			SDL_FreeSurface(icon);
		}
	}
	return true;
}



std::string SDL2App::composeAssetRootDir(const std::string& p_basePath) const
{
	std::string currentDirStr(p_basePath);
	if (m_settings.emulationOverrideDir.empty() == false)
	{
		// Use client-specified directory
		currentDirStr += m_settings.emulationOverrideDir;
	}
	else
	{
		// Set directory based on platform
		switch (m_settings.emulate)
		{
		case AppSettings::Emulate_None:       currentDirStr += "/osx"; break;
		default: TT_PANIC("Unsupported emulation mode: %d", m_settings.emulate); break;
		}
	}

	return currentDirStr;
}

void SDL2App::setAssetRootDir(const std::string& p_path)
{
	bool result = m_hostfs->setWorkingDir(p_path);
	if (result == false)
	{
		std::string errorMsg = "Could not find data directory.\n";
		errorMsg += "Trying to set data directory '" + p_path + "' failed.\n\n";
#ifdef TT_BUILD_FINAL
		errorMsg += "If you moved or removed this directory, please try to reinstall the game.";
#endif

		reportFatalError(errorMsg);
	}

	std::string currentDir = m_hostfs->getWorkingDir();
	m_assetRootDir = currentDir;
	if (*m_assetRootDir.rbegin() != m_hostfs->getDirSeparator())
	{
		m_assetRootDir += m_hostfs->getDirSeparator();
	}
}

//////////////////////
// FrameRateManager

void FrameRateManager::setTargetFramerate(s32 p_fps)
{
	if(static_cast<u32>(p_fps) != ms_targetFPS)
	{
		// Trigger device reset
		ms_targetFPS = p_fps;
	}
}

void FrameRateManager::modifyDeviceSettings(SDL_Window* window)
{
	int display = SDL_GetWindowDisplayIndex(window);
	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(display, &mode);

	if (ms_targetFPS == 0)
	{
		ms_vsyncEnabled = true;
		SDL_GL_SetSwapInterval(1);
		return;
	}

	// For windowed modes
	if ((SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) == 0)
	{
		u32 refreshRate = mode.refresh_rate;
		if (refreshRate == 59) ++refreshRate;

		// Turn off vsync unless the desktop refresh rate matches
		if (refreshRate % ms_targetFPS == 0)
		{
			// Turn on vsync
			SDL_GL_SetSwapInterval(1);
			
			// Only report vsync as on if it is an exact match, because we still need frame limiting
			// for 30 fps mode
			ms_vsyncEnabled = (refreshRate == ms_targetFPS);
		}
		else
		{
			// Turn off vsync
			SDL_GL_SetSwapInterval(0);
			ms_vsyncEnabled = false;
		}
	}
	// For fullscreen modes
	else
	{
		// We want settings that match our target fps + vsync
		u32 refreshRate = ms_targetFPS;
		if (ms_targetFPS < 60 && (60 % ms_targetFPS) == 0)
		{
			refreshRate = 60;
		}
		
		if (mode.refresh_rate == refreshRate)
		{
			SDL_GL_SetSwapInterval(1);
			
			// Only report vsync as on if it is an exact match, because we still need frame limiting
			// for 30 fps mode
			ms_vsyncEnabled = (refreshRate == ms_targetFPS);
		}
		else
		{
			// Using closest match -> Turn off vsync
			SDL_GL_SetSwapInterval(0);
			ms_vsyncEnabled = false;
		}
	}
}

void FrameRateManager::monitorFramerate()
{
	// Remove last element
	if(ms_framerateHistory.size() > 100) ms_framerateHistory.pop_back();
	
	// Add new element
	ms_framerateHistory.push_front(0);
	
	// Compute average
	ms_averageFPS = 0;
	for(FrameRates::iterator it = ms_framerateHistory.begin(); it != ms_framerateHistory.end(); ++it)
	{
		ms_averageFPS += *it;
	}
	ms_averageFPS = ms_averageFPS / ms_framerateHistory.size();

	TT_Printf("Vsync: %s FPS: %g (%g) Target: %u\n", ms_vsyncEnabled ? "ON" : "OFF", 0, ms_averageFPS, ms_targetFPS);
}

// Namespace end
}
}
