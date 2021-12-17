#if !defined(INC_TT_APP_TTDEVOBJCAPPVIEW_DESKTOP_H)
#define INC_TT_APP_TTDEVOBJCAPPVIEW_DESKTOP_H

#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iPhone) builds only


#import <Cocoa/Cocoa.h>
#include <string>

#include <tt/input/KeyboardController.h>


namespace tt {
namespace app {
	class OsxApp;
}
}


// This class is responsible for:
// - OpenGL setup and maintenance
// - Receiving mouse events and pushing them to MouseController
// - Receiving keyboard events and pushing them to KeyboardController

@interface TTdevObjCAppView : NSOpenGLView<NSWindowDelegate>
{
@private
	NSWindow* m_windowedModeWindow; // window this view should be in when in windowed mode
	NSWindow* m_fullScreenWindow;   // window this view should be in when in full-screen mode
	bool      m_isFullScreen; // currently full-screen?
	
	NSTrackingArea* m_trackingArea;
	tt::app::OsxApp* m_app;
	
	bool m_customCursorEnabled;
	
	// Keyboard "get string" support
	tt::input::KeyboardController::GetStringCallback m_kbdGetStringCallback;
	void*                                            m_kbdGetStringCallbackUserData;
	tt::input::Key                                   m_kbdGetStringAcceptKey;
	tt::input::Key                                   m_kbdGetStringCancelKey;
	std::wstring                                     m_kbdGetStringCurrentString;
}


- (id)initWithFrame:(NSRect)p_frameRect pixelFormat:(NSOpenGLPixelFormat*)p_format app:(tt::app::OsxApp*)p_app;
- (void)setWindowedModeWindow:(NSWindow*)p_window;

- (void)setFullScreen:(bool)p_fullScreen;
- (bool)isFullScreen;

- (void)disableCustomCursor;
- (void)restoreCustomCursor;

- (bool)startWideStringRetrieval:(tt::input::KeyboardController::GetStringCallback)p_callback
                        userData:(void*)p_callbackUserData
                   initialString:(const std::wstring&)p_initialString
                       acceptKey:(tt::input::Key)p_acceptKey
                       cancelKey:(tt::input::Key)p_cancelKey;
- (void)acceptGetString;
- (void)cancelGetString;
- (bool)isRetrievingWideString;

- (void)windowDidResize:(NSNotification *) notification;

@end


#endif  // defined(TT_PLATFORM_OSX_MAC)

#endif  // !defined(INC_TT_APP_TTDEVOBJCAPPVIEW_DESKTOP_H)
