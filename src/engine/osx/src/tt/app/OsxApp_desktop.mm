#if defined(TT_PLATFORM_OSX_MAC)

#import <Cocoa/Cocoa.h>
#include <sstream>
#include <unistd.h>

#include <tt/app/OsxApp_desktop.h>
#include <tt/app/Platform.h>
#include <tt/app/TTdevObjCAppView_desktop.h>
#include <tt/args/CmdLine.h>
#include <tt/cfg/ConfigRegistry.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/OpenGLContextWrapper.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/fs/OsxFileSystem.h>
#include <tt/input/KeyboardController.h>
#include <tt/input/MouseController.h>
#include <tt/input/SDLJoyPadController.h>
#include <tt/input/SDLMouseController.h>
#include <tt/input/SDLKeyboardController.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>
#include <tt/version/Version.h>

#define USE_SDL_INPUT 0 // Change to 1 for SDL input support. (Will need to link to sdl lib.)

namespace tt {
namespace app {

// Helper function to present the user with an error message and exit in final builds
// (simply triggers a panic when called in non-final builds)
inline void reportFatalError(const std::string& p_message)
{
#if defined(TT_BUILD_FINAL)
	// Show fatal error
	NSAlert* alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"Exit"];
	[alert setMessageText:@"Fatal Error"];
	[alert setInformativeText:[NSString stringWithUTF8String:p_message.c_str()]];
	[alert setAlertStyle:NSCriticalAlertStyle];
	
	[alert runModal];
	
	exit(1); // immediately exit, do not continue through regular NSApplication terminate logic
	
#else
	TT_PANIC("%s", p_message.c_str());
#endif
}


//--------------------------------------------------------------------------------------------------
// Public member functions

OsxApp::OsxApp(AppInterface* p_app, const AppSettings& p_settings,
               StartupStateOsx& p_startupState)
:
m_startupState(p_startupState),
m_settings(p_settings),
m_app(p_app),
m_platformApi(),
m_initialized(false),
m_frameTime(system::Time::getInstance()->getMicroSeconds()),
m_targetTimeSlice(0),
m_emulateNitro(p_settings.emulate == AppSettings::Emulate_Nitro ||
               p_settings.emulate == AppSettings::Emulate_Twl),
m_steamEnabled(p_settings.emulate  == AppSettings::Emulate_None &&
               p_settings.platform == AppSettings::Platform_Steam),
m_hideCursor(false),
m_useFixedDeltaTime(false),
m_fps30Mode(false),
m_active(true),
m_frameLimiterEnabled(true),
m_vsyncEnabled(true),
#if !defined(TT_BUILD_FINAL)
m_curWaitFrame(0),
m_totalWaitFrames(0),
m_frameStepMode(false),
m_updateTime(0),
m_renderTime(0),
m_textureCacheFileName(),
m_shouldDisplayDebugInfo(false),
#endif
m_osxfs(),
m_cloudfs(),
m_soundSystem(),
m_assetRootDir(),
m_cmdLine(args::CmdLine::getApplicationCmdLine()),
m_appView(0),
m_debugKeys(DebugKeys_All),
m_contextWrapper(0)
{
	//
	// General Initialization
	//
	
	TT_NULL_ASSERT(m_app);
	registerPlatformCallbackInterface(m_app);
	
	m_startupState.setStartupStep(StartupStep_SystemInit);
	
	makeApplicationAvailable(); // FIXME: move this to a point where asset path is also known
	
	bool showBuildLabel = true;
#if defined(TT_BUILD_FINAL)
	showBuildLabel = m_cmdLine.exists("version");
	
	str::Strings keepThese;
	
	keepThese.push_back("width");
	keepThese.push_back("height");
	keepThese.push_back("version");
	
	// Only keepThese command line options in final mode
	m_cmdLine.clear(keepThese);
#endif
	m_emulateNitro = m_emulateNitro || (m_cmdLine.exists("ds") && m_steamEnabled == false); // No DS + Steam
	m_hideCursor   = m_settings.hideCursor || m_cmdLine.exists("nocursor");
	
	const bool ios2xMode = m_settings.emulate == AppSettings::Emulate_IOS_iPhone &&
	                       m_cmdLine.exists("ios2xmode");
	if (m_cmdLine.exists("ios2xmode"))
	{
		TT_ASSERTMSG(m_settings.emulate == AppSettings::Emulate_IOS_iPhone,
		             "2x Mode ('Retina' Mode) is only available in iOS emulation mode.");
	}
	
	// Set up the correct platform emulation
	setPlatformEmulation(m_settings.emulate);
	
	// Initialize filesystem
	m_osxfs = fs::OsxFileSystem::instantiate(0, m_settings.name);
	if (m_osxfs == 0)
	{
		reportFatalError("Could not initialize filesystem.");
	}
	
	// Adjust working directory
	// FIXME: Should we take different emulation directories into account?
	m_assetRootDir = [[[NSBundle mainBundle] resourcePath] UTF8String];
	m_assetRootDir += "/";
	chdir(m_assetRootDir.c_str());
	
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
	
	if (m_settings.targetFPS > 0)
	{
		m_targetTimeSlice = static_cast<s32>(1000000.0f / m_settings.targetFPS);
	}
	//FrameRateManager::setTargetFramerate(targetFPS);
	
	if (m_cmdLine.exists("no-framelimiter"))
	{
		m_frameLimiterEnabled = false;
		m_vsyncEnabled = false;
	}
	
	// Create the application window
	bool startWindowed = true;
	if (createMainWindow(m_settings, showBuildLabel, ios2xMode, m_cmdLine, &startWindowed) == false)
	{
		reportFatalError("Could not create main application window.");
		return; // no use continuing here
	}
	
	// Create the renderer
	TT_NULL_ASSERT(m_appView);
	TTdevObjCAppView* view = (TTdevObjCAppView*)m_appView;
	m_contextWrapper = new engine::renderer::OpenGLContextWrapper([view openGLContext]);
	
	if (tt::engine::renderer::Renderer::createInstance(m_contextWrapper, ios2xMode) == false)
	{
		reportFatalError("Could not initialize render system.");
	}
	
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	renderer->getDebug()->setBaseCaptureFilename(m_settings.name);
	renderer->setClearColor(tt::engine::renderer::ColorRGB::black);
	renderer->getUpScaler()->setMaxSize(m_settings.graphicsSettings.startUpscaleSize);
	renderer->getUpScaler()->setAspectRatioRange(m_settings.graphicsSettings.aspectRatioRange);
	renderer->handleResetDevice();
	
	// Initialize engine's FileUtils
	tt::engine::file::FileUtils::getInstance()->setFileRoot(m_assetRootDir);
	tt::engine::file::FileUtils::getInstance()->generateNamespaceMapping();
	
	
#if !defined(TT_BUILD_FINAL)
	engine::renderer::FixedFunction::initialize(engine::EngineID("FixedFunctionDebug", "shaders"));
#else
	engine::renderer::FixedFunction::initialize(engine::EngineID("FixedFunction", "shaders"));
#endif
	
	// Switch to full screen mode if app settings request this
	if (startWindowed == false)
	{
		setFullScreen(true);
	}
	
	// Create platform API
	if (m_settings.systems != 0)
	{
		m_platformApi = m_settings.systems->instantiatePlatformApi(this);
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
		m_cloudfs = tt::fs::OsxFileSystem::instantiate(1, m_settings.name);
		if (m_cloudfs == 0)
		{
			reportFatalError("Could not initialize cloud filesystem.");
		}
	}
	
	// Initialize audio system
	if (m_settings.systems != 0)
	{
		m_soundSystem = m_settings.systems->instantiateSoundSystem();
	}
	
	// Initialize the input controllers
	tt::input::MouseController::initialize(m_appView);
	tt::input::KeyboardController::initialize(m_appView);
#if USE_SDL_INPUT
	tt::input::SDLJoypadController::initialize();
	tt::input::SDLMouseController::initialize();
	tt::input::SDLKeyboardController::initialize();
#else
	if (m_settings.systems != 0)
	{
		m_settings.systems->sdl_init();
	}
#endif
	
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


OsxApp::~OsxApp()
{
	m_startupState.setStartupStep(StartupStep_Shutdown);
	
	// Application Destruction
	unregisterPlatformCallbackInterface(m_app);
	delete m_app;
	m_app = 0;
	
	//
	// General Cleanup
	//
	engine::renderer::FixedFunction::destroy();
	
	// Deinitialize the controllers
	input::KeyboardController::deinitialize();
	input::MouseController::deinitialize();
#if USE_SDL_INPUT
	tt::input::SDLJoypadController::deinitialize();
	tt::input::SDLMouseController::deinitialize();
	tt::input::SDLKeyboardController::deinitialize();
#else
	if (m_settings.systems != 0)
	{
		m_settings.systems->sdl_shutdown();
	}
#endif
	
	// Shut down renderer
	engine::renderer::Renderer::destroyInstance();
	engine::renderer::Texture::clearDeathRow();
	
	delete m_contextWrapper;
	m_appView = 0; // FIXME: do we need to release this view?
	
	system::Time::destroyInstance();
	
	m_cloudfs.reset();
	
	// Shut down the platform API
	if (m_platformApi != 0)
	{
		m_platformApi->shutdown();
	}
}


void OsxApp::update(real p_elapsedTime)
{
	// DEBUG: Testing app behavior if no updates or renders are performed when app inactive
	if (m_active == false)
	{
		return;
	}
	
	// Update the controllers
	input::KeyboardController::update();
	input::MouseController::update();
#if USE_SDL_INPUT
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			tt::input::SDLJoypadController::processEvent(event);
			tt::input::SDLMouseController::processEvent(event);
			tt::input::SDLKeyboardController::processEvent(event);
		}
	}
	tt::input::SDLJoypadController::update();
	tt::input::SDLMouseController::update();
	tt::input::SDLKeyboardController::update();
#else
	if (m_settings.systems != 0)
	{
		m_settings.systems->sdl_update();
	}
#endif
	
	handleCommonInput();
	
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
	    input::KeyboardController::getState(input::ControllerIndex_One).keys[input::Key_P].pressed == false)
	{
		return;
	}
	
	m_curWaitFrame = 0;
#endif
	
	// Update the renderer
	engine::renderer::Renderer::getInstance()->update(p_elapsedTime);
	
	// Update the application
	m_app->update(p_elapsedTime);
	
#if !defined(TT_BUILD_FINAL)
	// Compute update time
	u64 now(system::Time::getInstance()->getMicroSeconds());
	m_updateTime = static_cast<s32>(now - m_frameTime);
#endif
	
	//FrameRateManager::monitorFramerate();
	// NOTE: TTdevObjCOsxApp handles changes in target FPS
	m_fps30Mode = engine::renderer::Renderer::getInstance()->isLowPerformanceMode();
}


void OsxApp::render()
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
					tt::engine::debug::DebugRendererPtr debugPtr =
						tt::engine::renderer::Renderer::getInstance()->getDebug();
					
					if (m_frameStepMode)
					{
						debugPtr->printf(5, 5, "FRAME STEP MODE");
					}
					else if (m_totalWaitFrames > 0)
					{
						debugPtr->printf(5, 5, "WAIT: %d", m_totalWaitFrames);
					}
				}
#endif
			}
			
			renderer->endViewPort(*it);
		}
	}
	
	// Done rendering
	renderer->endFrame();
	engine::renderer::Texture::clearDeathRow();
	
#ifndef TT_BUILD_FINAL
	// Compute update time
	u64 now(system::Time::getInstance()->getMicroSeconds());
	m_renderTime = static_cast<s32>(now - m_frameTime);
#endif
	
	// Framerate limiter
	if (m_frameLimiterEnabled)
	{
		u64 currentTime(system::Time::getInstance()->getMicroSeconds());
		u32 timePassed = static_cast<u32>(currentTime - m_frameTime);
		u32 targetTime = m_targetTimeSlice;
		
		while(timePassed < (targetTime - 100))
		{
			
			if(m_targetTimeSlice - timePassed > 1100)
			{
				// Sleep for 1ms
				[NSThread sleepForTimeInterval:0.001];
			}
			
			currentTime = system::Time::getInstance()->getMicroSeconds();
			
			timePassed = static_cast<u32>(currentTime - m_frameTime);
		}
	}
	
	m_frameTime = system::Time::getInstance()->getMicroSeconds();
	
	// Show frame
	renderer->present();
}


void OsxApp::onPlatformMenuEnter()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuEnter();
	}
	
	// Disable custom mouse cursor, so that the Steam overlay works correctly
	[(TTdevObjCAppView*)m_appView disableCustomCursor];
}


void OsxApp::onPlatformMenuExit()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuExit();
	}
	
	// Restore custom mouse cursor
	[(TTdevObjCAppView*)m_appView restoreCustomCursor];
}


void OsxApp::onAppActive()
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


void OsxApp::onAppInactive()
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
	
	
void OsxApp::onResetDevice()
{
	if (m_initialized && m_app != 0)
	{
		for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
		{
			(*it)->onResetDevice();
		}
	}
}


std::string OsxApp::getAssetRootDir() const
{
	return m_assetRootDir;
}


fs::identifier OsxApp::getSaveFsID() const
{
	return 1;
}


const args::CmdLine& OsxApp::getCmdLine() const
{
	return m_cmdLine;
}


const StartupState& OsxApp::getStartupState() const
{
	return m_startupState;
}


void OsxApp::setFullScreen(bool p_fullScreen)
{
	TT_ASSERTMSG(m_appView != 0, "Cannot switch full screen mode when application is not initialized.");
	if (m_appView != 0)
	{
		TTdevObjCAppView* view = (TTdevObjCAppView*)m_appView;
		if ([view isFullScreen] != p_fullScreen)
		{
			[view setFullScreen:p_fullScreen];
		}
	}
}


bool OsxApp::isFullScreen() const
{
	if (m_appView == 0)
	{
		return false;
	}
	return [(TTdevObjCAppView*)m_appView isFullScreen];
}
	
	
void OsxApp::setFullScreenResolution(const math::Point2 &p_resolution)
{
	m_settings.graphicsSettings.fullscreenSize =
		m_settings.graphicsSettings.getCorrectedScreenSize(p_resolution);
}
	
	
tt::math::Point2 OsxApp::getFullScreenResolution() const
{
	return m_settings.graphicsSettings.fullscreenSize;
}
	

tt::math::Point2 OsxApp::getDesktopSize() const
{
	NSRect screenDimensions = [[NSScreen mainScreen] frame];
	
	return tt::math::Point2(screenDimensions.size.width, screenDimensions.size.height);
}


bool OsxApp::shouldDisplayDebugInfo() const
{
#if !defined(TT_BUILD_FINAL)
	return m_shouldDisplayDebugInfo;
#else
	return false;
#endif
}


void OsxApp::terminate(bool p_graceful)
{
	if (p_graceful)
	{
		[NSApp terminate:nil];
	}
	else
	{
		exit(0);
	}
}


void OsxApp::setPaused(bool /*p_paused*/)
{
	// Nothing to do here on Mac OS X?
}
	

	
void OsxApp::setTargetFPS(u32 p_fps)
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
}
	
	
void OsxApp::handleResolutionChanged()
{
	if (engine::renderer::Renderer::hasInstance())
	{
		engine::renderer::Renderer::getInstance()->handleResetDevice();
		onResetDevice();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void OsxApp::handleCommonInput()
{
#if !defined(TT_BUILD_FINAL)
	
	if (m_debugKeys == DebugKeys_None)
	{
		return;
	}
	
	using engine::renderer::Renderer;
	Renderer* renderer = Renderer::getInstance();
	
	const input::KeyboardController& kbd(input::KeyboardController::getState(input::ControllerIndex_One));
	
	const bool ctrlDown        = kbd.keys[input::Key_Control].down;
	const bool altDown         = kbd.keys[input::Key_Alt    ].down;
	const bool shiftDown       = kbd.keys[input::Key_Shift  ].down;
	const bool anyModifierDown = ctrlDown || altDown || shiftDown;
	
	// Function keys:
	if ((m_debugKeys & DebugKeys_Function) != 0)
	{
		/* Console window
		if (kbd.keys[input::Key_F2].pressed && anyModifierDown == false)
		{
			toggleConsole();
		}
		//*/
		
		// Screenshot shortcut
		if (kbd.keys[input::Key_F3].pressed && ctrlDown == false && altDown == false)
		{
			TT_Printf("OsxApp::handleCommonInput: Taking screenshot...\n");
			using engine::debug::DebugRenderer;
			renderer->getDebug()->captureScreen(
												shiftDown ?
												DebugRenderer::ScreenCaptureMode_ToClipboard :
												DebugRenderer::ScreenCaptureMode_ToFile);
		}
		
		if (kbd.keys[input::Key_F5].pressed && anyModifierDown == false)
		{
			// Send request to reload currently loaded assets
			for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
			{
				(*it)->onRequestReloadAssets();
			}
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
		
		/*
		if (kbd.keys[input::Key_F11].pressed && anyModifierDown == false)
		{
			static bool lighting = false;
			lighting = (lighting == false);
			
			renderer->setLighting(lighting);
		}
		//*/
	}
	
	// Normal (non-function) keys:
	if ((m_debugKeys & DebugKeys_Normal) == 0)
	{
		return;
	}
	
	if (kbd.keys[input::Key_O].pressed && anyModifierDown == false)
	{
		// Force clear color to black, otherwise this won't work (additive blending)
		renderer->setClearColor(engine::renderer::ColorRGB::black);
		
		engine::debug::DebugRendererPtr debug(renderer->getDebug());
		// Toggle debug overdraw
		debug->setOverdrawDebugActive(debug->isOverdrawDebugActive() == false);
		TT_Printf("OsxApp::handleCommonInput: 'O' was pressed, toggling overdraw debug: %s\n",
		          debug->isOverdrawDebugActive() ? "On" : "Off");
	}
	
	/* Console window
	if (kbd.keys[input::Key_F2].pressed && anyModifierDown == false)
	{
		toggleConsole();
	}
	//*/
	
	// Framestep keys
	if (m_frameStepMode)
	{
		if (kbd.keys[input::Key_P].pressed &&
		    shiftDown                      &&
		    ctrlDown == false              &&
		    altDown  == false)
		{
			m_frameStepMode = false;
			TT_Printf("OsxApp::handleCommonInput: Toggling frame-step mode (new state: %s).\n",
					  m_frameStepMode ? "on" : "off");
		}
	}
	else if (kbd.keys[input::Key_P].pressed && anyModifierDown == false)
	{
		m_frameStepMode = true;
		TT_Printf("OsxApp::handleCommonInput: Toggling frame-step mode (new state: %s).\n",
	              m_frameStepMode ? "on" : "off");
	}
	
	if (kbd.keys[input::Key_Plus].pressed && anyModifierDown == false)
	{
		m_totalWaitFrames /= 2;
		TT_Printf("OsxApp::handleCommonInput: Reducing frame skip to %d.\n",
		          m_totalWaitFrames);
	}
	
	if (kbd.keys[input::Key_Minus].pressed && anyModifierDown == false)
	{
		if (m_totalWaitFrames == 0)
		{
			m_totalWaitFrames = 1;
		}
		else
		{
			m_totalWaitFrames *= 2;
		}
		
		if (m_totalWaitFrames > MAX_WAIT_FRAMES)
		{
			m_totalWaitFrames = MAX_WAIT_FRAMES;
		}
		
		TT_Printf("OsxApp::handleCommonInput: Increasing frame skip to %d.\n",
		          m_totalWaitFrames);
	}
	
	/*
	if (ctrlDown && kbd.keys[input::Key_T].pressed &&
	    altDown == false && shiftDown == false)
	{
		engine::cache::FileTextureCache::dumpToFile(m_textureCacheFileName);
		engine::renderer::TextureCache::dumpToFile(m_textureCacheFileName);
		
		// FIXME: Also open this document
		//ShellExecuteA(NULL, "open", m_textureCacheFilename.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	else */ if (kbd.keys[input::Key_T].pressed && shiftDown && ctrlDown == false && altDown == false)
	{
		engine::cache::FileTextureCache::dump();
		engine::renderer::TextureCache::dump();
	}
	
	if (kbd.keys[input::Key_F].pressed && anyModifierDown == false)
	{
		m_shouldDisplayDebugInfo = (m_shouldDisplayDebugInfo == false);
	}
	
#endif  // !defined(TT_BUILD_FINAL)
}


bool OsxApp::createMainWindow(AppSettings& p_settings, bool p_showBuildLabel,
							  bool p_ios2xMode, const args::CmdLine& p_cmdLine, bool* p_startWindowed_OUT)
{
	(void)p_ios2xMode; // FIXME: Why is this unused?
	
	determineScreenSize(&p_settings, m_app, getDesktopSize(), p_cmdLine);
	
	*p_startWindowed_OUT = p_settings.graphicsSettings.startWindowed;
	// NOTE: Always create the initial window with the windowed mode settings. Full-screen will be handled below.
	math::Point2 size(p_settings.graphicsSettings.getScreenSize(/*windowed*/true));
	
	// Create a rectangle for the (main) window such that the window is centered in the screen
	NSRect rect;
	NSRect screenDimensions = [[NSScreen mainScreen] visibleFrame];
	
	rect.size.width  = static_cast<CGFloat>(size.x);
	rect.size.height = static_cast<CGFloat>(size.y);
	rect.origin.x    = screenDimensions.origin.x + ((screenDimensions.size.width  - rect.size.width)  / 2);
	rect.origin.y    = screenDimensions.origin.y + ((screenDimensions.size.height - rect.size.height) / 2);
	
	// Create a window with a title bar
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
	if (p_settings.graphicsSettings.allowResize || m_cmdLine.exists("resizable"))
	{
		windowStyle |= NSResizableWindowMask;
	}
	
	NSWindow* window = [[NSWindow alloc] initWithContentRect:rect
										       styleMask:windowStyle
											     backing:NSBackingStoreBuffered
											       defer:NO];
	
	// For Mac OS X Lion: do not attempt to restore this window!
	if ([window respondsToSelector:@selector(setRestorable:)])
	{
        // Don't call [window setRestoreable: No] directly because we also build against 10.5.
        IMP setRestorableMethod = [window methodForSelector:@selector(setRestorable:)];
        setRestorableMethod(window, @selector(setRestorable:), NO);
	}
	
	// Set the window title
	std::ostringstream wndTitle;
	wndTitle << p_settings.name;
	
#if !defined(TT_BUILD_FINAL)
	wndTitle << " (REVISION: " << tt::version::getClientRevisionNumber()
	         << "." << tt::version::getLibRevisionNumber() << ")";
	
	if (m_emulateNitro)
	{
		wndTitle << " [DS MODE]";
	}
	(void)p_showBuildLabel;
	
#else
	
	if (p_showBuildLabel)
	{
		wndTitle << " (build " << tt::version::getClientRevisionNumber()
		         << "." << tt::version::getLibRevisionNumber() << ")";
	}
	
#endif
	
	[window setTitle:[NSString stringWithUTF8String:wndTitle.str().c_str()]];
	[window setOpaque:YES];
	[window setAcceptsMouseMovedEvents:YES];
	[window setIgnoresMouseEvents:NO];
	
	// TODO: Get these from AppSettings?
	const GraphicsSettings& settings = p_settings.graphicsSettings;
	bool needDepthBuffer  (settings.depthBufferBits     > 0);
	bool needStencilBuffer(settings.stencilBufferBits   > 0);
	bool needAntiAliasing (settings.antiAliasingSamples > 0);
	std::vector<NSOpenGLPixelFormatAttribute>::size_type multisampleIndex = 0;
	
	std::vector<NSOpenGLPixelFormatAttribute> attributes;
	attributes.push_back(NSOpenGLPFATripleBuffer);
	attributes.push_back(NSOpenGLPFAAccelerated);
	
	if (needDepthBuffer)
	{
		attributes.push_back(NSOpenGLPFADepthSize);
		attributes.push_back(settings.depthBufferBits);
	}
	
	if (needStencilBuffer)
	{
		attributes.push_back(NSOpenGLPFAStencilSize);
		attributes.push_back(settings.stencilBufferBits);
	}
	
	if (needAntiAliasing)
	{
		multisampleIndex = attributes.size();
		attributes.push_back(NSOpenGLPFAMultisample);
		attributes.push_back(NSOpenGLPFASampleBuffers);
		attributes.push_back(1);
		attributes.push_back(NSOpenGLPFASamples);
		attributes.push_back(settings.antiAliasingSamples);
	}
	
	// Attribute array should always end with 0
	attributes.push_back(0);
	
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:&attributes[0]];
	
	if (pixelFormat == nil)
	{
		// Try double buffering (OSX 10.6 or older)
		attributes[0] = NSOpenGLPFADoubleBuffer;
		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:&attributes[0]];
	}
	
	if (pixelFormat == nil && needAntiAliasing)
	{
		TT_PANIC("Creating OpenGL pixel format failed. Trying again without MultiSample.");
		attributes[multisampleIndex] = 0;
		
		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:&attributes[0]];
		if (pixelFormat == nil)
		{
			TT_PANIC("Creating OpenGL pixel format failed.");
			return false;
		}
	}
	
	// Create a custom content view for app framework handling
	TTdevObjCAppView* view = [[TTdevObjCAppView alloc]
							  initWithFrame:NSMakeRect(0.0f, 0.0f, rect.size.width, rect.size.height)
							  pixelFormat:pixelFormat app:this];
	if (view == nil)
	{
		TT_PANIC("Creating custom application view (OpenGL) failed.");
		return false;
	}
	
	m_appView = view;
	[window setContentView:view];
	[view becomeFirstResponder];
	
	[view setWindowedModeWindow:window];
	
	[window makeKeyAndOrderFront:NSApp];
	
	return true;
}

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_MAC)
