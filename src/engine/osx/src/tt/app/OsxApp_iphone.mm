#if defined(TT_PLATFORM_OSX_IPHONE)

#import <UIKit/UIKit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <sstream>
#include <unistd.h>

#include <tt/app/objc_helpers/UIApplication.h>
#include <tt/app/OsxApp_iphone.h>
#include <tt/app/Platform.h>
#include <tt/app/TTdevObjCAppView_iphone.h>
#include <tt/app/TTdevObjCOsxApp_iphone.h>
#include <tt/args/CmdLine.h>
#include <tt/fs/BufferedFileSystem.h>
#include <tt/fs/OsxFileSystem.h>
#include <tt/engine/renderer/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/input/IPhoneController.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/snd/snd.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>
#include <tt/version/Version.h>


@interface TTdevObjCAppWindow : UIWindow
{
	UIImageView* m_splashView;
	BOOL m_didStartAnimation;
}
- (id)initWithFrame:(CGRect)p_frame;
- (void)splashAnimationDidStart:(NSString *)animationID context:(void *)context;
- (void)splashAnimationDone:(NSString*)p_animationID finished:(NSNumber*)p_finished context:(void*)p_context;
- (void)showSplash;
- (void)hideSplash;
- (bool)isSplashGone;
- (BOOL)didStartAnimation;
- (void)rotateSplash:(UIInterfaceOrientation) p_orientation;
@end


@implementation TTdevObjCAppWindow

- (id)initWithFrame:(CGRect)p_frame
{
	self = [super initWithFrame:p_frame];
	if (self != nil)
	{
		m_splashView        = nil;
		m_didStartAnimation = NO;
	}
	return self;
}

- (void)splashAnimationDidStart:(NSString *)animationID context:(void *)context
{
	(void)animationID;
	(void)context;
	m_didStartAnimation = YES;
}

- (void)splashAnimationDone:(NSString*)p_animationID finished:(NSNumber*)p_finished context:(void*)p_context;
{
	(void)p_animationID;
	(void)p_finished;
	(void)p_context;
	if (m_splashView != nil)
	{
		[m_splashView removeFromSuperview];
		[m_splashView release];
		m_splashView = nil;
	}
}

- (void)showSplash
{
	if (m_splashView == nil)
	{
		CGRect screenSize = [[UIScreen mainScreen] bounds];
		
		// Create an image view for the Default.png loading image
		m_splashView       = [[UIImageView alloc] initWithFrame:screenSize];
		m_splashView.image = [UIImage imageNamed:@"Default.png"];
		
		[self addSubview:m_splashView];
		[self bringSubviewToFront:m_splashView];
	}
}

- (void)hideSplash
{
	if (m_splashView != nil)
	{
		[UIView beginAnimations:nil context:nil];
		[UIView setAnimationDuration:0.5];
		[UIView setAnimationTransition:UIViewAnimationTransitionNone forView:self cache:YES];
		[UIView setAnimationDelegate:self];
		[UIView setAnimationWillStartSelector:@selector(splashAnimationDidStart:context:)];
		[UIView setAnimationDidStopSelector:@selector(splashAnimationDone:finished:context:)];
		m_splashView.alpha = 0.0f;
		//m_splashView.frame = CGRectMake(-60, -85, 440, 635);
		[UIView commitAnimations];
	}
}


- (bool)isSplashGone
{
	return m_splashView == nil;
}


- (BOOL)didStartAnimation
{
	return m_didStartAnimation;
}


- (void)rotateSplash:(UIInterfaceOrientation) p_orientation
{
	if (m_splashView != nil)
	{
		if (p_orientation == UIInterfaceOrientationPortraitUpsideDown ||
		    p_orientation == UIInterfaceOrientationLandscapeLeft)
		{
			// FLIP
			CGAffineTransform transform = CGAffineTransformIdentity;
			m_splashView.transform      = CGAffineTransformRotate(transform, static_cast<CGFloat>(M_PI));
		}
		m_splashView.center       = self.center;
	}
}

@end



namespace tt {
namespace app {

// Helper function to present the user with an error message and exit in final builds
// (simply triggers a panic when called in non-final builds)
inline void reportFatalError(const std::string& p_message)
{
#if defined(TT_BUILD_FINAL)
	// Show fatal error
	/*
	NSAlert* alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"Exit"];
	[alert setMessageText:@"Fatal Error"];
	[alert setInformativeText:[NSString stringWithUTF8String:p_message.c_str()]];
	[alert setAlertStyle:NSCriticalAlertStyle];
	
	[alert runModal];
	*/ (void)p_message;
	
	exit(1); // immediately exit, do not continue through regular NSApplication terminate logic
	
#else
	TT_PANIC("%s", p_message.c_str());
#endif
}



bool g_audioSessionIsInterrupted = false;


inline void interruptionCallback(void* /*p_userData*/, UInt32 p_interruptionState)
{
	const char* interruptionStateName = "unknown";
	switch (p_interruptionState)
	{
	case kAudioSessionBeginInterruption: interruptionStateName = "kAudioSessionBeginInterruption"; break;
	case kAudioSessionEndInterruption:   interruptionStateName = "kAudioSessionEndInterruption";   break;
	}
	
	TT_Printf("tt::app::interruptionCallback: Callback triggered. Interruption state: %u (%s)\n",
	          p_interruptionState, interruptionStateName);
	
	if (p_interruptionState == kAudioSessionBeginInterruption)
	{
		TT_Printf("interruptionCallback: Received kAudioSessionBeginInterruption. Deactivating audio session.\n");
		
		TT_ASSERTMSG(g_audioSessionIsInterrupted == false,
		             "Audio session was already interrupted, but received another interruption.");
		g_audioSessionIsInterrupted = true;
		
		// Deactivate the session
		OSStatus result = AudioSessionSetActive(false);
		TT_ASSERTMSG(result == kAudioSessionNoError,
		             "Deactivating audio session in response to interruption failed with code %d.",
		             result);
		
		// Suspend OpenAL
		tt::snd::suspend(0);
	}
	else if (p_interruptionState == kAudioSessionEndInterruption)
	{
		TT_Printf("interruptionCallback: Received kAudioSessionEndInterruption. Reactivating audio session.\n");
		
		TT_ASSERTMSG(g_audioSessionIsInterrupted,
		             "Audio session was not interrupted, but received an 'end interruption' event.");
		g_audioSessionIsInterrupted = false;
		
		// FIXME: Restore audio session category here!
		// Re-register the audio session category
		//tokitori::main::App::getInstance()->getPlatform()->updateAudioSessionCategory();
		
		// Reactivate the session
		OSStatus result = AudioSessionSetActive(true);
		TT_ASSERTMSG(result == kAudioSessionNoError,
		             "Re-activating audio session after interruption failed with code %d.",
		             result);
		
		// Restore OpenAL
		tt::snd::resume(0);
	}
}


//--------------------------------------------------------------------------------------------------
// Public member functions

OsxApp::OsxApp(AppInterface* p_app, const AppSettings& p_settings, void* p_objCApp,
               StartupStateOsx& p_startupState)
:
m_startupState(p_startupState),
m_app(p_app),
m_initialized(false),
m_frameTime(0),
m_targetTimeSlice(0),
m_emulateNitro(p_settings.emulate == AppSettings::Emulate_Nitro ||
               p_settings.emulate == AppSettings::Emulate_Twl),
m_steamEnabled(p_settings.emulate  == AppSettings::Emulate_None &&
               p_settings.platform == AppSettings::Platform_Steam),
m_hideCursor(false),
m_useFixedDeltaTime(false),
m_fps30Mode(false),
m_active(true),
m_appPaused(false),
m_frameLimiterEnabled(true),
#if !defined(TT_BUILD_FINAL)
m_curWaitFrame(0),
m_totalWaitFrames(0),
m_frameStepMode(false),
m_updateTime(0),
m_renderTime(0),
#endif
m_timerIsActive(false),
m_cmdLine(args::CmdLine::getApplicationCmdLine()),
m_firstUpdate(true),
m_appView(0),
m_appWindow(0),
m_objCApp(p_objCApp)
{
	const u64 appInitStart = system::Time::getInstance()->getMilliSeconds();
	(void)appInitStart;
	
	//
	// General Initialization
	//
	
	TT_NULL_ASSERT(m_app);
	registerPlatformCallbackInterface(m_app);
	
	m_startupState.setStartupStep(StartupStep_SystemInit);
	
	// Initialize audio session
	OSStatus result = AudioSessionInitialize(0, 0, interruptionCallback, this);
	TT_ASSERTMSG(result == kAudioSessionNoError, "Initializing audio session failed with code %d.",
	             result);
	
	// Set the iPhone audio session category to the least invasive one by default (allow other audio to play)
	{
		UInt32 category = kAudioSessionCategory_AmbientSound;
		result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
		TT_ASSERTMSG(result == kAudioSessionNoError,
		             "Setting initial audio session category failed with code %d.", result);
	}
	
	// Prevent the device from going to sleep while the application is active
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	
	makeApplicationAvailable(); // FIXME: move this to a point where asset path is also known
	
	bool showBuildLabel = true;
#if defined(TT_BUILD_FINAL)
	showBuildLabel = m_cmdLine.exists("version");
	
	keepThese.push_back("width");
	keepThese.push_back("height");
	keepThese.push_back("version");
	
	// Only keepThese command line options in final mode
	m_cmdLine.clear(keepThese);
#endif
	m_emulateNitro = m_emulateNitro || (m_cmdLine.exists("ds") && m_steamEnabled == false); // No DS + Steam
	m_hideCursor   = p_settings.hideCursor || m_cmdLine.exists("nocursor");
	
	m_frameLimiterEnabled = m_cmdLine.exists("no-framelimiter") == false;
	
	// Initialize filesystem
	m_osxfs = fs::OsxFileSystem::instantiate(2, p_settings.name);
	if (m_osxfs == 0)
	{
		reportFatalError("Could not initialize OS X filesystem.");
	}
	
	m_buffs = fs::BufferedFileSystem::instantiate(0, 2, 2048);
	if (m_buffs == 0)
	{
		reportFatalError("Could not initialize buffered filesystem.");
	}
	
	// Adjust working directory
	// FIXME: Should we take different emulation directories into account?
	m_assetRootDir = [[[NSBundle mainBundle] resourcePath] UTF8String];
	m_assetRootDir += "/";
	chdir(m_assetRootDir.c_str());
	
	// Frame limiter
	u32 targetFPS(0);
	if (p_settings.targetFPS > 0)
	{
		targetFPS = p_settings.targetFPS;
	}
	else if (m_cmdLine.exists("fps"))
	{
		targetFPS = m_cmdLine.getInteger("fps");
	}
	
	if (targetFPS > 0)
	{
		m_targetTimeSlice = 1000 / targetFPS;
	}
	//FrameRateManager::setTargetFramerate(targetFPS);
	
	
	// Detect 'Retina mode' availability based on system state
	const bool onIPadInIPhoneMode =
		[[[UIDevice currentDevice] model] hasPrefix:@"iPad"] &&
		[[UIDevice currentDevice] respondsToSelector:@selector(userInterfaceIdiom)] &&
		[[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone;
	const bool support2xMode = [[UIScreen mainScreen] respondsToSelector:@selector(scale)] &&
	                           [[UIScreen mainScreen] scale] == 2.0 &&
	                           onIPadInIPhoneMode == false;
	const bool ios2xMode = support2xMode && p_settings.useIOS2xMode;
	
	// Set up the correct platform emulation
	setPlatformEmulation(p_settings.emulate);
	
	// Create the application window
	if (createMainWindow(p_settings, showBuildLabel, ios2xMode, m_cmdLine) == false)
	{
		reportFatalError("Could not create main application window.");
		return; // no use continuing here
	}
	
	// Create the renderer
	TT_NULL_ASSERT(m_appView);
	TTdevObjCAppView* view = (TTdevObjCAppView*)m_appView;
	if (tt::engine::renderer::Renderer::createInstance([view openGLContext],
	                                                   m_emulateNitro, ios2xMode) == false)
	{
		reportFatalError("Could not initialize render system.");
	}
	
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	renderer->getDebug()->setBaseCaptureFilename(p_settings.name);
	renderer->setClearColor(tt::engine::renderer::ColorRGB::black);
	//renderer->getUpScaler()->setMaxSize(p_settings.graphicsSettings.startUpScaleSize);
	
	// Initialize engine's FileUtils
	tt::engine::file::FileUtils::getInstance()->setFileRoot(m_assetRootDir);
	tt::engine::file::FileUtils::getInstance()->generateNamespaceMapping();
	
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
		m_cloudfs = tt::fs::OsxFileSystem::instantiate(1, p_settings.name);
		if (m_cloudfs == 0)
		{
			reportFatalError("Could not initialize cloud filesystem.");
		}
	}
	
	// Initialize audio system
	if (p_settings.systems != 0)
	{
		m_soundSystem = p_settings.systems->instantiateSoundSystem();
	}
	
	// Initialize the input controllers
	tt::input::IPhoneController::initialize();
	
	
	/* NOTE: Client app initialization is done in the first update.
	// Have the client application initialize itself (should be last)
	m_initialized = m_app->init();
	if (m_initialized == false)
	{
		reportFatalError("Could not initialize application.");
	}
	//*/ m_initialized = true;
	
	TT_Printf("OsxApp::OsxApp: Entire OsxApp initialization took %u ms.\n", u32(system::Time::getInstance()->getMilliSeconds() - appInitStart));
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
	
	// Deinitialize the controllers
	input::IPhoneController::deinitialize();
	
	// Shut down renderer
	tt::engine::renderer::Renderer::destroyInstance();
	
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
	if (m_timerIsActive == false)
	{
		// Force ignore update/render message.
		return;
	}
	
	
	// DEBUG: Testing app behavior if no updates or renders are performed when app inactive
	if (m_active == false || m_appPaused)
	{
		return;
	}
	
	if (m_firstUpdate)
	{
		m_firstUpdate = false;
		
		// Have the client application initialize itself (should be last)
		const u64 appInitStart = system::Time::getInstance()->getMilliSeconds();
		(void)appInitStart;
		m_startupState.setStartupStep(StartupStep_ClientInit);
		m_initialized = m_app->init();
		TT_Printf("OsxApp::update: AppInterface init took %u ms.\n",
		          u32(system::Time::getInstance()->getMilliSeconds() - appInitStart));
		if (m_initialized == false)
		{
			reportFatalError("Could not initialize application.");
		}
		
		// Dismiss the splash screen
		if (m_appWindow != 0)
		{
			[(TTdevObjCAppWindow*)m_appWindow hideSplash];
		}
		
		TTdevObjCOsxApp* parent = (TTdevObjCOsxApp*)m_objCApp;
		if (parent != 0)
		{
			[parent resetFrameTimestamp];
		}
		
		m_startupState.setStartupStep(StartupStep_Running);
	}
	
	
	if (m_appWindow != 0 &&
	    [(TTdevObjCAppWindow*)m_appWindow didStartAnimation] == NO)
	{
		TT_Printf("OsxApp::update - Splash animation not yet started. Override elapsedTime: %f, to 0.0f.\n",
		          p_elapsedTime);
		p_elapsedTime = 0.0f;
	}			
	
	// Handle application switching between active/inactive
	// FIXME: Cocoa equivalent?
	/*
	if (m_active != DXUTIsActive())
	{
		if (m_active)
		{
			// Switch to inactive
			m_app->onAppInactive();
			m_active = false;
		}
		else
		{
			// Switch to active
			m_app->onAppActive();
			m_active = true;
		}
	}
	//*/
	
	m_frameTime = system::Time::getInstance()->getMilliSeconds();
	
	// Update the controllers
	input::IPhoneController::update();
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
	if (m_frameStepMode /*&&
	    input::KeyboardController::getState(input::ControllerIndex_One).keys[input::Key_F8].pressed == false*/)
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
	u64 now(system::Time::getInstance()->getMilliSeconds());
	m_updateTime = static_cast<s32>(now - m_frameTime);
#endif
	
	//FrameRateManager::monitorFramerate();
	// NOTE: TTdevObjCOsxApp handles changes in target FPS
	m_fps30Mode = engine::renderer::Renderer::getInstance()->isLowPerformanceMode();
}


void OsxApp::render()
{
	if (m_timerIsActive == false)
	{
		// Force ignore update/render message.
		return;
	}
	
	// DEBUG: Testing app behavior if no updates or renders are performed when app inactive
	if (m_active == false || m_appPaused)
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
				tt::engine::renderer::DebugRendererPtr debugPtr = 
					tt::engine::renderer::Renderer::getInstance()->getDebug();
				
				if (m_frameStepMode)
				{
					debugPtr->printf(5, 5, "FRAME STEP MODE");
				}
				else if (m_totalWaitFrames > 0)
				{
					debugPtr->printf(5, 5, "WAIT: %d", m_totalWaitFrames);
				}
#endif
			}
			
			renderer->endViewPort(*it);
		}
	}
	
	// Done rendering
	renderer->endFrame();
	
	u64 now(system::Time::getInstance()->getMilliSeconds());
	
#if !defined(TT_BUILD_FINAL)
	// Compute render time
	m_renderTime = static_cast<s32>(now - m_frameTime);
#endif
	
	// Framerate limiter
	/* FIXME: Find better way
	if (m_frameLimiterEnabled)
	{
		s32 timePassed = static_cast<s32>(now - m_frameTime);
		s32 frameTime  = m_fps30Mode ? 33 : m_targetTimeSlice;
		
		while (timePassed < frameTime)
		{
			[NSThread sleepForTimeInterval:0.001];
			now = system::Time::getInstance()->getMilliSeconds();
			timePassed = static_cast<s32>(now - m_frameTime);
		}
	}
	//*/ (void)now;
	
	// Show frame
	renderer->present();
}


void OsxApp::onPlatformMenuEnter()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuEnter();
	}
}


void OsxApp::onPlatformMenuExit()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuExit();
	}
}


void OsxApp::onAppActive()
{
	if (g_audioSessionIsInterrupted)
	{
		// Audio session was interrupted, but we still became active? Reclaim audio session...
		interruptionCallback(0, kAudioSessionEndInterruption);
	}
	
	m_active = true;
	if (m_firstUpdate == false && m_initialized && m_app != 0)
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
	if (m_firstUpdate == false && m_initialized && m_app != 0)
	{
		for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
		{
			(*it)->onAppInactive();
		}
	}
}


void OsxApp::onAppEnteredBackground()
{
	// FIXME: Do we need to perform any special processing? "app inactive" notification was called before this...
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onAppEnteredBackground();
	}
	
	// Store that we are now in the background (which is a valid point where the application can be terminated)
	m_startupState.setStartupStep(StartupStep_InBackground);
}


void OsxApp::onAppLeftBackground()
{
	// FIXME: Do we need to perform any special processing? "app active" notification will be called after this...
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onAppLeftBackground();
	}
	
	// Store that we are no longer in the background
	m_startupState.setStartupStep(StartupStep_Running);
}


bool OsxApp::onShouldAutoRotateToOrientation(AppOrientation p_orientation) const
{
	bool should = true;
	for (PlatformCallbackInterfaces::const_iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		should = should && (*it)->onShouldAutoRotateToOrientation(p_orientation);
	}
	return should;
}


void OsxApp::onWillRotateToOrientation(AppOrientation p_orientation, real p_duration)
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onWillRotateToOrientation(p_orientation, p_duration);
	}
	
	if (p_duration <= 0.001f)
	{
		// Update rotation of splash screen.
		[(TTdevObjCAppWindow*)m_appWindow rotateSplash:tt::app::getUIInterfaceOrientation(p_orientation)];
	}
}


void OsxApp::onDidRotateFromOrientation(AppOrientation p_fromOrientation)
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onDidRotateFromOrientation(p_fromOrientation);
	}
}


std::string OsxApp::getAssetRootDir() const
{
	return m_assetRootDir;
}


fs::identifier OsxApp::getSaveFsID() const
{
	return 2;
}


const args::CmdLine& OsxApp::getCmdLine() const
{
	return m_cmdLine;
}


const StartupState& OsxApp::getStartupState() const
{
	return m_startupState;
}


void OsxApp::setFullScreen(bool /*p_fullScreen*/)
{
	TT_WARN("Full-screen switching is not supported on iPhone.");
}


bool OsxApp::isFullScreen() const
{
	// Always full-screen on iPhone
	return true;
}


bool OsxApp::shouldDisplayDebugInfo() const
{
#if !defined(TT_BUILD_FINAL)
	return true;
#else
	return false;
#endif
}


void OsxApp::terminate(bool p_graceful)
{
	if (p_graceful)
	{
		// iPhone applications are not allowed to terminate themselves
		// (against Apple guidelines; must be initiated by user using the Home button)
		TT_PANIC("iOS applications are not allowed to terminate themselves.");
	}
	else
	{
		exit(0);
	}
}


void OsxApp::setPaused(bool p_paused)
{
	m_appPaused = p_paused;
}


bool OsxApp::isSplashGone() const
{
	return [(TTdevObjCAppWindow*)m_appWindow isSplashGone];
}



void* OsxApp::getAppViewController() const
{
	return ((TTdevObjCAppView*)m_appView).viewController;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void OsxApp::handleCommonInput()
{
#if 0//!defined(TT_BUILD_FINAL)
	
	const input::KeyboardController& kbd(input::KeyboardController::getState(input::ControllerIndex_One));
	
	/*
	if (DXUTWasKeyPressed(VK_F8))
	{
		static bool wireframe = false;
		wireframe = (wireframe == false);
		
		DXUTGetD3D9Device()->SetRenderState(D3DRS_FILLMODE, wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
	}
	*/
	
	// Safe Frame rendering support
	if (kbd.keys[input::Key_F9].pressed)
	{
		engine::renderer::Renderer::getInstance()->getDebug()->toggleSafeFrame();
	}
	
	// Axis display toggle
	if (kbd.keys[input::Key_F10].pressed)
	{
		engine::renderer::Renderer::getInstance()->getDebug()->toggleAxis(1.0f);
	}
	
	/*
	if (DXUTWasKeyPressed(VK_F11))
	{
		static bool lighting = false;
		lighting = (lighting == false);
		
		engine::renderer::Renderer::getInstance()->setLighting(lighting);
	}
	*/
	
	// Framestep keys
	if (kbd.keys[input::Key_P].pressed)
	{
		m_frameStepMode = !m_frameStepMode;
		TT_Printf("OsxApp::handleCommonInput: Toggling frame-step mode (new state: %s).\n",
		          m_frameStepMode ? "on" : "off");
	}
	
	if (kbd.keys[input::Key_Plus].pressed)
	{
		m_totalWaitFrames /= 2;
		TT_Printf("OsxApp::handleCommonInput: Reducing frame skip to %d.\n",
		          m_totalWaitFrames);
	}
	
	if (kbd.keys[input::Key_Minus].pressed)
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
	
	// Screenshot shortcut
	if (kbd.keys[input::Key_Control].down && kbd.keys[input::Key_S].pressed)
	{
		TT_Printf("OsxApp::handleCommonInput: Taking screenshot...\n");
		engine::renderer::Renderer::getInstance()->getDebug()->saveScreenToFile();
	}
	
#endif  // !defined(TT_BUILD_FINAL)
}


bool OsxApp::createMainWindow(const AppSettings& p_settings, bool p_showBuildLabel, 
							  bool p_ios2xMode, const args::CmdLine& p_cmdLine)
{
	CGRect screenSize = [[UIScreen mainScreen] bounds];
	
	TTdevObjCAppWindow* window = [[TTdevObjCAppWindow alloc] initWithFrame:screenSize];
	m_appWindow = window;
	
//	bool              applyTransform = false;
//	CGAffineTransform transform      = CGAffineTransformIdentity;
	
	//*
	// Store device orientation at start.
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	UIDeviceOrientation deviceOrientation = [[UIDevice currentDevice] orientation];
	[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
	
	// Overwrite orientation based on current device orientation. (Start at 'active' orientation instead of rotating to it.)
	// This was something Chillingo wants because of user feedback.
	tt::app::AppOrientation appOrientation = tt::app::getAppOrientation(deviceOrientation);
	// End override based on device orientation.
	//*/
	
	TT_Printf("OsxApp::createMainWindow - orientations - device: '%s', statusBar: '%s'\n",
			  tt::app::getAppOrientationName(tt::app::getAppOrientation(deviceOrientation), true),
			  tt::app::getAppOrientationName(tt::app::getAppOrientation([[UIApplication sharedApplication] statusBarOrientation]), true));
	
	UIInterfaceOrientation orientation = (tt::app::isAppOrientationValid(appOrientation)) ? 
										  tt::app::getUIInterfaceOrientation(appOrientation) :
										  [[UIApplication sharedApplication] statusBarOrientation];
	{
		//UIInterfaceOrientation orientation = (UIInterfaceOrientation)orientationObject;
		
		if (m_app->onShouldAutoRotateToOrientation(tt::app::getAppOrientation(orientation)) == false)
		{
			switch (orientation)
			{
				case UIInterfaceOrientationPortrait:
				case UIInterfaceOrientationPortraitUpsideDown:
					orientation = UIInterfaceOrientationLandscapeLeft;
					break;
					
				case UIInterfaceOrientationLandscapeLeft:					
				case UIInterfaceOrientationLandscapeRight:
					orientation = UIInterfaceOrientationPortrait;
					break;
					
				default:
					TT_PANIC("Unrecognized interface orientation: %d", orientation);
					if (m_app->onShouldAutoRotateToOrientation(
							tt::app::getAppOrientation(UIInterfaceOrientationLandscapeLeft)))
					{
						orientation = UIInterfaceOrientationLandscapeLeft;
					}
					else
					{
						orientation = UIInterfaceOrientationPortrait;
					}
					break;
			}
			TT_ASSERT(m_app->onShouldAutoRotateToOrientation(tt::app::getAppOrientation(orientation)));
		}
		
		switch (orientation)
		{
		case UIInterfaceOrientationPortrait:
			// Nothing to do here; orientation is portrait by default
			break;
			
		case UIInterfaceOrientationPortraitUpsideDown:
			// Rotate view 180 degrees
//			applyTransform = true;
//			transform = CGAffineTransformRotate(transform, static_cast<CGFloat>(M_PI));
			break;
			
		case UIInterfaceOrientationLandscapeLeft:
			// Rotate view 90 degrees counter-clockwise
//			applyTransform = true;
//			transform = CGAffineTransformRotate(transform, static_cast<CGFloat>(-M_PI * 0.5f));
			std::swap(screenSize.size.width, screenSize.size.height);
			break;
			
		case UIInterfaceOrientationLandscapeRight:
			// Rotate view 90 degrees clockwise
//			applyTransform = true;
//			transform = CGAffineTransformRotate(transform, static_cast<CGFloat>(M_PI * 0.5f));
			std::swap(screenSize.size.width, screenSize.size.height);
			break;
			
		default:
			TT_WARN("Unrecognized interface orientation: %d", orientation);
			break;
		}
	}
	TTdevObjCAppView* appView = [[TTdevObjCAppView alloc] initWithFrame:screenSize app:this ios2xMode:p_ios2xMode];
	m_appView = appView;
	
//	if (applyTransform)
//	{
//		appView.transform = transform;
//		appView.center    = window.center;
//	}
	
	/*
	if (p_viewController != 0)
	{
		m_viewController = (UIViewController*)p_viewController;
		m_viewController.view = appView;
	}
	*/
	
	[window addSubview:appView];
	
	[appView becomeFirstResponder];
	[window makeKeyAndVisible];
	
	[window showSplash];
	
	// FIXME: Implement iPhone equivalent of the code below:
	(void)p_settings;
	(void)p_showBuildLabel;
	
	return true;
	
#if 0
	AppSettings settings(p_settings);
	determineScreenSize(&settings, m_app, getDesktopSize(), p_cmdLine);	
	
	const bool windowed = settings.graphicsSettings.startWindowed;
	math::Point2 size(settings.graphicsSettings.getScreenSize(windowed));
	
	// Create a rectangle for the (main) window such that the window is centered in the screen
	NSRect rect;
	NSRect screenDimensions = [[NSScreen mainScreen] visibleFrame];
	
	// FIXME: Take "start full-screen" setting into account
	rect.size.width  = static_cast<CGFloat>(size.x);
	rect.size.height = static_cast<CGFloat>(size.y);
	rect.origin.x    = screenDimensions.origin.x + ((screenDimensions.size.width  - rect.size.width)  / 2);
	rect.origin.y    = screenDimensions.origin.y + ((screenDimensions.size.height - rect.size.height) / 2);
	
	// Create a non-resizable (but minimizable) window with a title bar
	NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
	NSWindow* window = [[NSWindow alloc] initWithContentRect:rect
										       styleMask:windowStyle
											     backing:NSBackingStoreBuffered
											       defer:NO];
	
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
	
	// Create an OpenGL pixel format for the content view
	// FIXME: Handle pixel formats in a more flexible and robust manner
	NSOpenGLPixelFormatAttribute attributes[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		(NSOpenGLPixelFormatAttribute)0
	};
	
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	if (pixelFormat == nil)
	{
		TT_PANIC("Creating OpenGL pixel format failed.");
		return false;
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
#endif  // 0
}

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
