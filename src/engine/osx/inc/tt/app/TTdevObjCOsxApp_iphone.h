#if !defined(INC_TT_APP_TTDEVOBJCOSXAPP_IPHONE_H)
#define INC_TT_APP_TTDEVOBJCOSXAPP_IPHONE_H

#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iOS builds only


#include <mach/mach_time.h>
#import <UIKit/UIKit.h>

#include <tt/app/AppSettings.h>
#include <tt/app/OsxApp_iphone.h>


namespace tt {
namespace app {
	class StartupStateOsx;
}
}

@class TTdevObjCAppView;

@interface TTdevObjCOsxApp : UIApplication <UIApplicationDelegate>
{
	NSTimer*       updateTimer;
	NSTimeInterval updateInterval;
	id             displayLink;
	
	tt::app::StartupStateOsx* m_startupState;
	tt::app::OsxApp*          m_app;
	tt::app::AppSettings      m_appSettings;
	
	// When receiving app activation notification: is this the first time we get this notification?
	// (the initial app startup also produces an app activation notification)
	bool m_initialAppActivation;
	
	
	mach_timebase_info_data_t m_timebaseInfo; // for converting opaque time units to nanoseconds
	uint64_t m_prevFrameTimestamp;
}

// These functions must be overridden by derived classes:
- (void)getAppSettings:(tt::app::AppSettings*)p_settings;
- (tt::app::AppInterface*)createAppInterface:(tt::app::AppSettings*)p_settings;

// Implementation details
- (void)startTimer;
- (void)startDisplayLink;
- (void)stopTimer;
- (void)pauseTimer;
- (void)resumeTimer;
- (void)update;
- (void)resetFrameTimestamp;


@property NSTimeInterval updateInterval;

@end


#endif  // defined(TT_PLATFORM_OSX_IPHONE)

#endif  // !defined(INC_TT_APP_TTDEVOBJCOSXAPP_IPHONE_H)
