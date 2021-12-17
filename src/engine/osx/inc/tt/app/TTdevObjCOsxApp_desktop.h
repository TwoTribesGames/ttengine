#if !defined(INC_TT_APP_TTDEVOBJCOSXAPP_DESKTOP_H)
#define INC_TT_APP_TTDEVOBJCOSXAPP_DESKTOP_H

#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iOS) builds only


#import <Cocoa/Cocoa.h>
#import <CoreVideo/CoreVideo.h>
#include <mach/mach_time.h>

#include <tt/app/AppSettings.h>
#include <tt/app/OsxApp_desktop.h>


namespace tt {
namespace app {
	class StartupStateOsx;
}
}

@class TTdevObjCAppView;

@interface TTdevObjCOsxApp : NSApplication//NSObject// <NSApplicationDelegate>
{
	NSTimer*       updateTimer;
	NSTimeInterval updateInterval;
	
	CVDisplayLinkRef m_displayLink;
	bool             m_useDisplayLink;  //!< Whether to use display link or timer for driving game loop
	
	tt::app::StartupStateOsx* m_startupState;
	tt::app::OsxApp*          m_app;
	tt::app::AppSettings      m_appSettings;
	
	bool m_toggleFullScreenNextUpdate;
	
	mach_timebase_info_data_t m_timebaseInfo; // for converting opaque time units to nanoseconds
	uint64_t                  m_prevFrameTimestamp;
}

// These functions must be overridden by derived classes:
- (void)getAppSettings:(tt::app::AppSettings*)p_settings;
- (tt::app::AppInterface*)createAppInterface:(tt::app::AppSettings*)p_settings;

// Implementation details
- (void)startTimer;
- (void)stopTimer;
- (void)update;
- (void)resetFrameTimestamp;
- (void)handleDisplayLinkCallback;

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)p_application;

/*! \brief Creates a default application menu bar with the items required by the Mac OS X HIG. */
- (void)createMenuBar:(NSString*)p_appName allowFullScreenToggle:(bool)p_allowFullScreenToggle;


@property NSTimeInterval updateInterval;

@end


#endif  // defined(TT_PLATFORM_OSX_MAC)

#endif  // !defined(INC_TT_APP_TTDEVOBJCOSXAPP_DESKTOP_H)
