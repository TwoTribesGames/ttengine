#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iOS builds only

#import <QuartzCore/QuartzCore.h>

//#include <tt/app/TTdevObjCAppView_iphone.h>
#include <tt/app/objc_helpers/UIApplication.h>
#include <tt/app/StartupState.h>
#include <tt/app/TTdevObjCOsxApp_iphone.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/version/Version.h>


// Toggle whether to use CADisplayLink to drive the game loop or NSTimer (CADisplayLink requires at least iOS 3.1)
#define TT_IOS_APP_USE_DISPLAY_LINK 1


@implementation TTdevObjCOsxApp

@synthesize updateInterval;


//--------------------------------------------------------------------------------------------------
// Functions that must be overridden by derived classes

- (void)getAppSettings:(tt::app::AppSettings*)p_settings
{
	(void)p_settings;
	TT_PANIC("TTdevObjCOsxApp::getAppSettings must be overridden by client code.");
}


- (tt::app::AppInterface*)createAppInterface:(tt::app::AppSettings*)p_settings
{
	(void)p_settings;
	TT_PANIC("TTdevObjCOsxApp::createAppInterface must be overridden by client code.");
	return 0;
}


//--------------------------------------------------------------------------------------------------
// 'Private' implementation details

- (void)construct
{
	// Initialize member variables (doing this here by lack of constructor)
	updateTimer    = nil;
	updateInterval = 0.0;
	displayLink    = nil;
	
	m_startupState = 0;
	m_app          = 0;
	m_appSettings  = tt::app::AppSettings();
	// NOTE: Default to using fixed delta time, which is what older projects expect
	m_appSettings.useFixedDeltaTime = true;
	
	m_initialAppActivation = true;
	
	// Get the information needed to convert mach_absolute_time units to nanoseconds
	mach_timebase_info(&m_timebaseInfo);
	m_prevFrameTimestamp = 0;
}


/* FIXME: Apple recommends responding to this instead:
- (BOOL)application:(UIApplication*)p_application didFinishLaunchingWithOptions:(NSDictionary*)p_launchOptions
{
	// ...
	return YES;
}
//*/


- (void)applicationDidFinishLaunching:(UIApplication*)p_application
{
	(void)p_application;
	
	// Simulate a constructor
	[self construct];
	
	// Have client code provide the app settings
	[self getAppSettings:&m_appSettings];
	
	m_startupState = new tt::app::StartupStateOsx(m_appSettings.version,
	                                              tt::version::getLibRevisionNumber(),
	                                              m_appSettings.name);
	
	// Hide the status bar
	UIApplication* sharedApp = [UIApplication sharedApplication];
	if ([sharedApp respondsToSelector:@selector(setStatusBarHidden:withAnimation:)])
	{
		[sharedApp setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone];
	}
	else
	{
		// Casting to generic Objective C pointer type 'id' here to circumvent deprecation warning
		[(id)sharedApp setStatusBarHidden:YES animated:NO];
	}
	
	
	// Set basic application information
	// FIXME: Storing result of c_str on temporary object will cause memory issues!
	tt::version::setClientVersionInfo(m_appSettings.version, m_appSettings.versionString.c_str());
	tt::settings::setRegion(m_appSettings.region);
	tt::settings::setApplicationName(tt::str::widen(m_appSettings.name));
	
	// Create the client application (AppInterface-derived)
	tt::app::AppInterface* clientApp = [self createAppInterface:&m_appSettings];
	if (clientApp == 0)
	{
		TT_PANIC("No AppInterface was created.");
		//[NSApp terminate:self];
		return;
	}
	
	// Create the C++ application class
	m_app = new tt::app::OsxApp(clientApp, m_appSettings, self, *m_startupState);
	if (m_app->isInitialized() == false)
	{
		TT_PANIC("OsxApp failed to initialize. Not starting render loop.");
		return;
	}
	
	// Start the render loop
	updateInterval = 1.0 / m_appSettings.targetFPS;
	
	[self startTimer];
}


- (void)applicationWillTerminate:(UIApplication*)p_application
{
	(void)p_application;
	
	[self stopTimer];
	
	delete m_app;
	m_app = 0;
	
	delete m_startupState;
	m_startupState = 0;
}


- (void)applicationDidBecomeActive:(UIApplication*)p_application
{
	(void)p_application;
	
	// Ignore the first app activation notification (sent during app startup)
	if (m_initialAppActivation)
	{
		m_initialAppActivation = false;
		return;
	}
	
	if (m_app != 0)
	{
		m_app->onAppActive();
	}
}


- (void)applicationWillResignActive:(UIApplication*)p_application
{
	(void)p_application;
	if (m_app != 0)
	{
		m_app->onAppInactive();
	}
}


- (void)applicationDidEnterBackground:(UIApplication*)p_application
{
	(void)p_application;
	if (m_app != 0)
	{
		m_app->onAppEnteredBackground();
	}
}


- (void)applicationWillEnterForeground:(UIApplication*)p_application
{
	(void)p_application;
	if (m_app != 0)
	{
		m_app->onAppLeftBackground();
	}
}


- (void)didReceiveMemoryWarning
{
	TT_WARN("didReceiveMemoryWarning");
}


- (void)startTimer
{
	m_app->handleTimerIsActive(true);
	
	[self resetFrameTimestamp];
	
#if TT_IOS_APP_USE_DISPLAY_LINK
	// Start with timer and switch to display link when splash screen is gone.
	// This is done so we don't have a bunch of delayed touch input.
	// (Happens when creating lots of touch events during initial load on iPhone 3G.)
	
	updateTimer = [NSTimer scheduledTimerWithTimeInterval:updateInterval
	               target:self selector:@selector(update) userInfo:nil repeats:YES];
	
#else
	updateTimer = [NSTimer scheduledTimerWithTimeInterval:updateInterval
	               target:self selector:@selector(update) userInfo:nil repeats:YES];
#endif
}


- (void)startDisplayLink
{
#if TT_IOS_APP_USE_DISPLAY_LINK
	if (displayLink != nil)
	{
		// Already using display link, nothing to start.
		return;
	}
	
	// Calculate the frame interval based on the desired frame rate
	// NOTE: There is currently (using iOS SDK 4.1) no way of retrieving the device's
	//       hardware refresh rate. This is assumed to always be 60 Hz.
	const s32 hardwareRefreshRate = 60;
	const s32 desiredFPS          = m_app->isFps30Mode() ? s32(30) : m_appSettings.targetFPS;
	NSInteger frameInterval = 1;
	if ((hardwareRefreshRate % m_appSettings.targetFPS) != 0)
	{
		// Requested framerate cannot be used; round up to the nearest usable value
		TT_PANIC("Cannot use a target frame rate of %d FPS: must be a factor of hardware refresh rate (%d FPS).",
		         desiredFPS, hardwareRefreshRate);
		frameInterval = static_cast<NSInteger>(tt::math::ceil(hardwareRefreshRate / (float)desiredFPS));
	}
	else
	{
		// Requested framerate is acceptable for the hardware refresh rate
		frameInterval = hardwareRefreshRate / desiredFPS;
	}
	
	displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
	[displayLink setFrameInterval:static_cast<NSInteger>(frameInterval)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
#endif
}


- (void)stopTimer
{
	m_app->handleTimerIsActive(false);
	
#if TT_IOS_APP_USE_DISPLAY_LINK
	if (displayLink != nil)
	{
		[displayLink invalidate];
		displayLink = nil;
	}
#else
	if (updateTimer != nil)
	{
		[updateTimer invalidate];
		updateTimer = nil;
	}
#endif
}


- (void)pauseTimer
{
	m_app->handleTimerIsActive(false);
	
#if TT_IOS_APP_USE_DISPLAY_LINK
	if (displayLink != nil)
	{
		((CADisplayLink*)displayLink).paused = YES;
		TT_ASSERT(updateTimer == nil);
	}
	else if (updateTimer != nil)
	{
		[updateTimer invalidate];
		updateTimer = nil;
	}
#else
	[self stopTimer];
#endif
}


- (void)resumeTimer
{
	m_app->handleTimerIsActive(true);
	
#if TT_IOS_APP_USE_DISPLAY_LINK
	if (displayLink != nil)
	{
		((CADisplayLink*)displayLink).paused = NO;
	}
	else
	{
		updateTimer = [NSTimer scheduledTimerWithTimeInterval:updateInterval
		               target:self selector:@selector(update) userInfo:nil repeats:YES];
	}

#else
	[self startTimer];
#endif
}


- (void)update
{
	if (m_app == 0)
	{
		return;
	}
	
	// Determine how much time passed between updates
	real deltaTime = static_cast<real>(updateInterval);
	
	const uint64_t now = mach_absolute_time();
	const uint64_t elapsedTimestamp = now - m_prevFrameTimestamp;
	m_prevFrameTimestamp = now;
	// NOTE: m_timebaseInfo provides conversion to nanoseconds. We need seconds.
	const real elapsedTime = static_cast<real>((elapsedTimestamp * m_timebaseInfo.numer) /
	                                           ((real64)m_timebaseInfo.denom * 1000000000.0));
	
	if (m_appSettings.useFixedDeltaTime == false)
	{
		deltaTime = elapsedTime;
	}
	
	const bool prevFps30Mode = m_app->isFps30Mode();
	
	// Update and render application
	m_app->update(deltaTime);
	
	m_app->render();
	
	
	// If FPS 30 mode was switched, update timer
	const bool fps30Mode = m_app->isFps30Mode();
	if (fps30Mode != prevFps30Mode)
	{
		updateInterval = 1.0 / (fps30Mode ? s32(30) : m_appSettings.targetFPS);
		
#if TT_IOS_APP_USE_DISPLAY_LINK
		// Check if display link was already started.
		if (displayLink != nil)
		{
			// Display link was already stated, so restart with new updateInterval.

			[self stopTimer];
			[self startDisplayLink];
		}
		// If not, the new update interval will be used when display link is started.
		
#else
		[self stopTimer];
		[self startTimer];
#endif
	}
#if TT_IOS_APP_USE_DISPLAY_LINK
	else if (elapsedTime >= 2.0f)
	{
		// Too much time has passed. (Might be loading.)
		if (displayLink != nil)
		{
			TT_Printf("TTdevObjCOsxApp_iphone - update - too much elapsedTime: %f. Resetting Display Link!\n",
					  elapsedTime);
			// If the display link was being used, input might be been queueing up.
			// Restart displaying to remove input queue.
			[self stopTimer];
			[self startTimer]; // Use Timer for one frame
		}
	}
	else if (displayLink == nil && m_app->isSplashGone())
	{
		// App has finished loading. No longer update with timer, switch to display link updates.
		// NOTE: This was done to stop input done during loading from queueing and blocking input once
		//       the load is done.
		
		// Stop timer.
		if (updateTimer != nil)
		{
			[updateTimer invalidate];
			updateTimer = nil;
		}
		
		// Start displaylink.
		[self startDisplayLink];
	}
#endif
}


- (void)resetFrameTimestamp
{
	// Save the current timestamp as 'previous' frame time
	m_prevFrameTimestamp = mach_absolute_time();
}


@end


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
