#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iOS) builds only

//#include <tt/app/TTdevObjCAppView_desktop.h>
#include <tt/app/StartupState.h>
#include <tt/app/TTdevObjCOsxApp_desktop.h>
#include <tt/platform/tt_error.h>
#include <tt/settings/settings.h>
#include <tt/str/common.h>
#include <tt/version/Version.h>


static CVReturn displayLinkCallback(CVDisplayLinkRef   /*p_displayLink*/,
                                    const CVTimeStamp* /*p_now*/,
                                    const CVTimeStamp* /*p_outputTime*/,
                                    CVOptionFlags      /*p_flagsIn*/,
                                    CVOptionFlags*     /*p_flagsOut*/,
                                    void*              p_displayLinkContext)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	TTdevObjCOsxApp* objCApp = reinterpret_cast<TTdevObjCOsxApp*>(p_displayLinkContext);
	[objCApp handleDisplayLinkCallback];
	
	[pool release];
	
	return kCVReturnSuccess;
}


@implementation TTdevObjCOsxApp

// FIXME: Create a custom main loop instead of using timers
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
	
	m_displayLink    = 0;
	m_useDisplayLink = false;
	
	m_startupState = 0;
	m_app          = 0;
	m_appSettings  = tt::app::AppSettings();
	// NOTE: Default to using fixed delta time, which is what older projects expect
	m_appSettings.useFixedDeltaTime = true;
	
	m_toggleFullScreenNextUpdate = false;
	
	// Get the information needed to convert mach_absolute_time units to nanoseconds
	mach_timebase_info(&m_timebaseInfo);
	m_prevFrameTimestamp = 0;
}


- (void)applicationWillFinishLaunching:(NSNotification*)p_notification
{
	(void)p_notification;
	
	// Simulate a constructor
	[self construct];
	
	// Have client code provide the app settings
	[self getAppSettings:&m_appSettings];
	
	// FIXME: Storing result of c_str on temporary object will cause memory issues!
	tt::version::setClientVersionInfo(m_appSettings.version, m_appSettings.versionString.c_str());
	tt::settings::setRegion(m_appSettings.region);
	tt::settings::setApplicationName(tt::str::utf8ToUtf16(m_appSettings.name));
	
	
	m_startupState = new tt::app::StartupStateOsx(m_appSettings.version,
	                                              tt::version::getLibRevisionNumber(),
	                                              m_appSettings.name);
	
	// Create a menu bar for the application
	[self createMenuBar:[NSString stringWithUTF8String:m_appSettings.name.c_str()]
	         allowFullScreenToggle:m_appSettings.graphicsSettings.allowHotKeyFullScreenToggle];
	
	// Create the client application (AppInterface-derived)
	tt::app::AppInterface* clientApp = [self createAppInterface:&m_appSettings];
	if (clientApp == 0)
	{
		TT_PANIC("No AppInterface was created.");
		[NSApp terminate:self];
		return;
	}
	
	// Create the C++ application class
	m_app = new tt::app::OsxApp(clientApp, m_appSettings, *m_startupState);
	
	if (m_app->isInitialized() == false)
	{
		return;
	}
	
	// Determine whether to use a display link (and the update interval)
	m_useDisplayLink = (m_appSettings.targetFPS == 0 && m_appSettings.useFixedDeltaTime == false);
	
	if (m_appSettings.targetFPS > 0)
	{
		// Get refresh rate of main monitor
		CGDisplayModeRef currentMode = CGDisplayCopyDisplayMode(kCGDirectMainDisplay);
		double refreshRate = CGDisplayModeGetRefreshRate(currentMode);
		CGDisplayModeRelease(currentMode);
		
		// If refresh rate matches target fps, enable display link
		if ((static_cast<u32>(refreshRate + 0.5) % m_appSettings.targetFPS) <= 1)
		{
			m_useDisplayLink = true;
		}
		else
		{
			// No refresh rate match, disable vsync
			m_app->setVsyncEnabled(false);
		}
	}
	else if (m_appSettings.targetFPS == 0)
	{
		m_appSettings.useFixedDeltaTime = false;
	}
	
	// FIXME: Severe slower framerate when displaylink is enabled
	m_useDisplayLink = false;
    
	if (m_useDisplayLink)
	{
		// Using display link to drive game loop (with real delta time, so value of updateInterval is irrelevant)
		updateInterval = 1.0 / 60.0;
	}
	else
	{
		// Using timer-driven game loop
		//updateInterval = 1.0 / (m_appSettings.targetFPS > 0 ? m_appSettings.targetFPS : 60.0);
		
		// NOTE: Use a very small timer value to generate a frame as soon as possible
		//       Use in combination with vsync to prevent overburdening the system
		updateInterval = 0.001;
	}
	
	if (m_useDisplayLink)
	{
		// Create a display link
		CVDisplayLinkCreateWithActiveCGDisplays(&m_displayLink);
		CVDisplayLinkSetOutputCallback(m_displayLink, &displayLinkCallback, self);
        
		TTdevObjCAppView* view = (TTdevObjCAppView*)m_app->getAppView();
		CGLContextObj     cglContext     = (CGLContextObj)[[view openGLContext] CGLContextObj];
		CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[view pixelFormat] CGLPixelFormatObj];
        
		CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(m_displayLink, cglContext, cglPixelFormat);
	}
	
	// Start the render loop
	[self startTimer];
}


- (void)applicationWillTerminate:(NSNotification*)p_application
{
	(void)p_application;
	
	[self stopTimer];
	
	if (m_displayLink != 0)
	{
		CVDisplayLinkRelease(m_displayLink);
		m_displayLink = 0;
	}
	
	delete m_app;
	m_app = 0;
	
	delete m_startupState;
	m_startupState = 0;
}


- (void)startTimer
{
	[self resetFrameTimestamp];
	
	if (m_useDisplayLink)
	{
		TT_NULL_ASSERT(m_displayLink);
		if (m_displayLink != 0 && CVDisplayLinkIsRunning(m_displayLink) == false)
		{
			CVDisplayLinkStart(m_displayLink);
		}
	}
	else
	{
		updateTimer = [NSTimer scheduledTimerWithTimeInterval:updateInterval
						   target:self selector:@selector(update) userInfo:nil repeats:YES];
	}
}
 

- (void)stopTimer
{
	if (m_useDisplayLink)
	{
		TT_NULL_ASSERT(m_displayLink);
		if (m_displayLink != 0 && CVDisplayLinkIsRunning(m_displayLink))
		{
			CVDisplayLinkStop(m_displayLink);
		}
	}
	else
	{
		if (updateTimer != nil)
		{
			[updateTimer invalidate];
			updateTimer = nil;
		}
	}
}


- (void)update
{
	if (m_app == 0)
	{
		return;
	}
	
	if (m_toggleFullScreenNextUpdate)
	{
		m_toggleFullScreenNextUpdate = false;
		m_app->setFullScreen(m_app->isFullScreen() == false);
	}
	
	// Determine how much time passed between updates
	real deltaTime = (m_appSettings.targetFPS == 0) ?
		0 : static_cast<real>(1.0f / m_appSettings.targetFPS);
	
	const uint64_t now              = mach_absolute_time();
	const uint64_t elapsedTimestamp = now - m_prevFrameTimestamp;
	m_prevFrameTimestamp = now;
	// NOTE: m_timebaseInfo provides conversion to nanoseconds. We need seconds.
	const real elapsedTime = static_cast<real>((elapsedTimestamp * m_timebaseInfo.numer) /
	                                           ((real64)m_timebaseInfo.denom * 1000000000.0));
	
	if (m_appSettings.useFixedDeltaTime == false)
	{
		deltaTime = elapsedTime;
	}
	
	// Update and render application
	m_app->update(deltaTime);
	
	m_app->render();
}


- (void)resetFrameTimestamp
{
	// Save the current timestamp as 'previous' frame time
	m_prevFrameTimestamp = mach_absolute_time();
}


- (void)handleDisplayLinkCallback
{
	if (m_app == 0)
	{
		return;
	}
	
	TTdevObjCAppView* view = (TTdevObjCAppView*)m_app->getAppView();
	CGLContextObj contextObj = (CGLContextObj)[[view openGLContext] CGLContextObj];
	CGLLockContext(contextObj);
	
	[[view openGLContext] makeCurrentContext];
	
	[self update];
	
	
	CGLUnlockContext(contextObj);
}


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)p_application
{
	(void)p_application;
	return YES;
}


- (void)createMenuBar:(NSString*)p_appName allowFullScreenToggle:(bool)p_allowFullScreenToggle
{
	NSMenu*     mainMenu = [[NSMenu alloc] initWithTitle:p_appName];
	NSMenuItem* mainItem = [mainMenu addItemWithTitle:p_appName action:nil keyEquivalent:@""];
	TT_NULL_ASSERT(mainItem);
	
	// Application menu
	NSMenu*     subMenu = [[NSMenu alloc] initWithTitle:p_appName];
	NSMenuItem* subItem = nil;
	
	subItem = [subMenu addItemWithTitle:[@"About " stringByAppendingString:p_appName]
								 action:@selector(actionAboutApp:) keyEquivalent:@""];
	TT_NULL_ASSERT(subItem);
	
	[subMenu addItem:[NSMenuItem separatorItem]];
	
	subItem = [subMenu addItemWithTitle:@"Services" action:NULL keyEquivalent:@""];
	TT_NULL_ASSERT(subItem);
	
	NSMenu* servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
	[subMenu setSubmenu:servicesMenu forItem:subItem];
	[NSApp setServicesMenu:servicesMenu];
	
	subItem = [subMenu addItemWithTitle:[@"Hide " stringByAppendingString:p_appName]
								 action:@selector(actionHideApp:) keyEquivalent:@"h"];
	TT_NULL_ASSERT(subItem);
	
	subItem = [subMenu addItemWithTitle:@"Hide Others" action:@selector(actionHideOthers:)
						  keyEquivalent:@"h"];
	TT_NULL_ASSERT(subItem);
	[subItem setKeyEquivalentModifierMask:NSAlternateKeyMask | NSCommandKeyMask];
	
	subItem = [subMenu addItemWithTitle:@"Show All" action:@selector(actionShowAll:)
						  keyEquivalent:@""];
	TT_NULL_ASSERT(subItem);
	
	[subMenu addItem:[NSMenuItem separatorItem]];
	
	subItem = [subMenu addItemWithTitle:[@"Quit " stringByAppendingString:p_appName]
								 action:@selector(actionQuitApp:) keyEquivalent:@"q"];
	TT_NULL_ASSERT(subItem);
	
	[mainMenu setSubmenu:subMenu forItem:mainItem];
	
	
	// View menu
	if (p_allowFullScreenToggle)
	{
		NSMenuItem* viewItem = [mainMenu addItemWithTitle:@"View" action:nil keyEquivalent:@""];
		TT_NULL_ASSERT(viewItem);
		subMenu = [[NSMenu alloc] initWithTitle:@"View"];
		
		subItem = [subMenu addItemWithTitle:@"Toggle Full Screen"
									 action:@selector(actionToggleFullScreen:) keyEquivalent:@"f"];
		TT_NULL_ASSERT(subItem);
		
		[mainMenu setSubmenu:subMenu forItem:viewItem];
	}
	
	
	// Window menu
	NSMenuItem* windowItem = [mainMenu addItemWithTitle:@"Window" action:nil keyEquivalent:@""];
	TT_NULL_ASSERT(windowItem);
	subMenu = [[NSMenu alloc] initWithTitle:@"Window"];
	[mainMenu setSubmenu:subMenu forItem:windowItem];
	[NSApp setWindowsMenu:subMenu];
	
	
	// Set the menu bar for the application
	[NSApp setMainMenu:mainMenu];
}


//--------------------------------------------------------------------------------------------------
// Menu item action handlers

- (void)actionAboutApp:(id)p_sender
{
	(void)p_sender;
	
	// Pass a custom application name and version (uses the Info.plist version for the
	// primary version information, but the client and lib revision numbers for the rest)
	NSArray* optKeys   = [NSArray arrayWithObjects:@"ApplicationName", @"Version", nil];
	NSArray* optValues = [NSArray arrayWithObjects:
			[NSString stringWithUTF8String:m_appSettings.name.c_str()],
			[NSString stringWithFormat:@"%d.%d", (int)m_appSettings.version, (int)tt::version::getLibRevisionNumber()],
			nil];
	
	// Display a standard About panel with the custom values
	NSDictionary* options = [NSDictionary dictionaryWithObjects:optValues forKeys:optKeys];
	[NSApp orderFrontStandardAboutPanelWithOptions:options];
}


- (void)actionHideApp:(id)p_sender
{
	[NSApp hide:p_sender];
}


- (void)actionHideOthers:(id)p_sender
{
	[NSApp hideOtherApplications:p_sender];
}


- (void)actionShowAll:(id)p_sender
{
	[NSApp unhideAllApplications:p_sender];
}


- (void)actionQuitApp:(id)p_sender
{
	[NSApp terminate:p_sender];
}


- (void)actionToggleFullScreen:(id)p_sender
{
	(void)p_sender;
	if (m_app != 0 && m_app->isInitialized())
	{
		m_toggleFullScreenNextUpdate = true;
	}
}

@end


#endif  // defined(TT_PLATFORM_OSX_MAC)
