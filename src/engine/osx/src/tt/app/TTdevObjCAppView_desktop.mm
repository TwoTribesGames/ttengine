#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iPhone) builds only

#include <tt/app/OsxApp_desktop.h>
#include <tt/app/TTdevObjCAppView_desktop.h>
#include <tt/app/TTOpenGLContext_desktop.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/input/MouseController.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


// Simple tweak of NSWindow so that it can become a key window if it is a borderless window
@interface TTdevObjCFullScreenWindow : NSWindow
@end

@implementation TTdevObjCFullScreenWindow

- (BOOL)canBecomeKeyWindow
{
	return YES;
}

@end



// Helper for keyboard input
inline void mapKeyCodes(NSEvent* p_event, tt::input::Key* p_keyCodes, s32* p_keyCodesLength)
{
	if (*p_keyCodesLength < 1)
	{
		return;
	}
	
	//NSString* utf16 = [p_event characters];
	NSString* utf16 = [p_event charactersIgnoringModifiers];
	
	if ([utf16 length] == 1)
	{
		// A single UTF16 character. Do one of the following:
		// a) Is it in the range [0, 127] ? -> simply return it as a single UTF8 character.
		// b) Is it a special character ? -> simply return it as a special character.
		// c) Else -> convert the UTF16 string to a UTF8 string and return it.
		unichar	ukey        = [utf16 characterAtIndex: 0];
		s32		savedLength = *p_keyCodesLength;
		
		//TT_Printf("mapKeyCodes: Single key: %u ('%c')\n", ukey, static_cast<char>(ukey));
		
		*p_keyCodesLength = 1;
		if (ukey <= 127)
		{
			// Catch ASCII values that may not map properly to TT key codes
			switch (ukey)
			{
			case 25: // treat back-tab as tab
				p_keyCodes[0] = tt::input::Key_Tab;
				break;
				
			case '`':
			case '~':
				p_keyCodes[0] = tt::input::Key_Grave;
				break;
				
			case '\'':
			case '"':
				p_keyCodes[0] = tt::input::Key_Apostrophe;
				break;
				
			case '.':
			case '>':
				p_keyCodes[0] = tt::input::Key_Period;
				break;
				
			case ',':
			case '<':
				p_keyCodes[0] = tt::input::Key_Comma;
				break;
				
			case ';':
			case ':':
				p_keyCodes[0] = tt::input::Key_Semicolon;
				break;
				
			case '[':
			case '{':
				p_keyCodes[0] = tt::input::Key_LeftSquareBracket;
				break;
				
			case ']':
			case '}':
				p_keyCodes[0] = tt::input::Key_RightSquareBracket;
				break;
				
			case '\\':
			case '|':
				p_keyCodes[0] = tt::input::Key_Backslash;
				break;
				
			case '/':
			case '?':
				p_keyCodes[0] = tt::input::Key_Slash;
				break;
				
			case '-':
			case '_':
				p_keyCodes[0] = tt::input::Key_Minus;
				break;
				
			case '=':
			case '+':
				p_keyCodes[0] = tt::input::Key_Plus;
				break;
				
			case 127:
				p_keyCodes[0] = tt::input::Key_Backspace;
				break;
				
			case '!': p_keyCodes[0] = tt::input::Key_1; break;
			case '@': p_keyCodes[0] = tt::input::Key_2; break;
			case '#': p_keyCodes[0] = tt::input::Key_3; break;
			case '$': p_keyCodes[0] = tt::input::Key_4; break;
			case '%': p_keyCodes[0] = tt::input::Key_5; break;
			case '^': p_keyCodes[0] = tt::input::Key_6; break;
			case '&': p_keyCodes[0] = tt::input::Key_7; break;
			case '*': p_keyCodes[0] = tt::input::Key_8; break;
			case '(': p_keyCodes[0] = tt::input::Key_9; break;
			case ')': p_keyCodes[0] = tt::input::Key_0; break;
				
			default:
				p_keyCodes[0] = static_cast<tt::input::Key>(std::toupper(ukey & 0xFF));
				break;
			}
			return;
		}
		
		switch (ukey)
		{
		case NSF1FunctionKey:         p_keyCodes[0] = tt::input::Key_F1;       return;
		case NSF2FunctionKey:         p_keyCodes[0] = tt::input::Key_F2;       return;
		case NSF3FunctionKey:         p_keyCodes[0] = tt::input::Key_F3;       return;
		case NSF4FunctionKey:         p_keyCodes[0] = tt::input::Key_F4;       return;
		case NSF5FunctionKey:         p_keyCodes[0] = tt::input::Key_F5;       return;
		case NSF6FunctionKey:         p_keyCodes[0] = tt::input::Key_F6;       return;
		case NSF7FunctionKey:         p_keyCodes[0] = tt::input::Key_F7;       return;
		case NSF8FunctionKey:         p_keyCodes[0] = tt::input::Key_F8;       return;
		case NSF9FunctionKey:         p_keyCodes[0] = tt::input::Key_F9;       return;
		case NSF10FunctionKey:        p_keyCodes[0] = tt::input::Key_F10;      return;
		case NSF11FunctionKey:        p_keyCodes[0] = tt::input::Key_F11;      return;
		case NSF12FunctionKey:        p_keyCodes[0] = tt::input::Key_F12;      return;
		case NSUpArrowFunctionKey:    p_keyCodes[0] = tt::input::Key_Up;       return;
		case NSDownArrowFunctionKey:  p_keyCodes[0] = tt::input::Key_Down;     return;
		case NSLeftArrowFunctionKey:  p_keyCodes[0] = tt::input::Key_Left;     return;
		case NSRightArrowFunctionKey: p_keyCodes[0] = tt::input::Key_Right;    return;
		case NSPageUpFunctionKey:     p_keyCodes[0] = tt::input::Key_PageUp;   return;
		case NSPageDownFunctionKey:   p_keyCodes[0] = tt::input::Key_PageDown; return;
		case NSHomeFunctionKey:       p_keyCodes[0] = tt::input::Key_Home;     return;
		case NSEndFunctionKey:        p_keyCodes[0] = tt::input::Key_End;      return;
			
		case NSInsertFunctionKey:
		case NSInsertCharFunctionKey:
			p_keyCodes[0] = tt::input::Key_Insert;
			return;
			
		case NSDeleteFunctionKey:
			p_keyCodes[0] = tt::input::Key_Delete;
			return;
			
		case NSBackspaceCharacter:
		case NSDeleteCharFunctionKey:
			// This is actually Backspace
			p_keyCodes[0] = tt::input::Key_Backspace;
			return;
		}
		
		*p_keyCodesLength = savedLength;
	}
	
	//TT_Printf("mapKeyCodes: Multiple keys or unrecognized key code.\n");
	
	/*
	// Special handling for non-US keyboards: Some valid ASCII characters
	// are generated with the ALT modifier. I.e. on German keyboards,
	// the '[' character is generated by entering Alt-5. However,
	// generically, we want to ignore the effect which the ALT key
	// has on the characters.
	utf16 = [p_event charactersIgnoringModifiers];
	
	// Still here? Then either 'utf16' contains more than one UTF16 character
	// and thus is most likely a composite character sequence or 'utf16' 
	// contains a single 'real' Unicode character.
	const char* ptr = [utf16 UTF8String];
	int         i   = 0;
	while (i < *p_keyCodesLength && ptr[i] != '\0')
	{
		p_keyCodes[i] = ptr[i];
		++i;
	}
	*p_keyCodesLength = i;
	//*/ *p_keyCodesLength = 0;
}


inline std::wstring getPrintableCharacters(NSEvent* p_event)
{
	std::wstring retval;
	NSString* chars = [p_event characters];
	retval.reserve(static_cast<std::wstring::size_type>([chars length]));
	for (NSUInteger i = 0; i < [chars length]; ++i)
	{
		unichar	ukey        = [chars characterAtIndex: i];
		bool    isPrintable = false;
		
		if (ukey <= 127)
		{
			switch (ukey)
			{
			case 25:
			case 127:
				isPrintable = false;
				break;
				
			default:
				isPrintable = true;
				break;
			}
		}
		else
		{
			switch (ukey)
			{
			case NSF1FunctionKey:
			case NSF2FunctionKey:
			case NSF3FunctionKey:
			case NSF4FunctionKey:
			case NSF5FunctionKey:
			case NSF6FunctionKey:
			case NSF7FunctionKey:
			case NSF8FunctionKey:
			case NSF9FunctionKey:
			case NSF10FunctionKey:
			case NSF11FunctionKey:
			case NSF12FunctionKey:
			case NSUpArrowFunctionKey:
			case NSDownArrowFunctionKey:
			case NSLeftArrowFunctionKey:
			case NSRightArrowFunctionKey:
			case NSPageUpFunctionKey:
			case NSPageDownFunctionKey:
			case NSHomeFunctionKey:
			case NSEndFunctionKey:
			case NSInsertFunctionKey:
			case NSInsertCharFunctionKey:
			case NSDeleteFunctionKey:
			case NSBackspaceCharacter:
			case NSDeleteCharFunctionKey:
			case NSDeleteCharacter:
				isPrintable = false;
				break;
				
			default:
				isPrintable = true;
				break;
			}
		}
		
		if (isPrintable)
		{
			retval += static_cast<std::wstring::value_type>([chars characterAtIndex:i]);
		}
	}

	return retval;
}


@implementation TTdevObjCAppView

- (void)resetKeyboardGetStringVariables
{
	m_kbdGetStringCallback         = 0;
	m_kbdGetStringCallbackUserData = 0;
	m_kbdGetStringAcceptKey        = tt::input::Key_Count;
	m_kbdGetStringCancelKey        = tt::input::Key_Count;
	m_kbdGetStringCurrentString.clear();
}


- (id)initWithFrame:(NSRect)p_frameRect pixelFormat:(NSOpenGLPixelFormat*)p_format app:(tt::app::OsxApp*)p_app
{
	self = [super initWithFrame:p_frameRect pixelFormat:p_format];
	if (self != nil)
	{
		// Initialize member variables (Objective C needs constructors...)
		m_windowedModeWindow = 0;
		m_fullScreenWindow   = 0;
		m_isFullScreen       = false;
		m_trackingArea       = 0;
		m_app                = p_app;
		
		m_customCursorEnabled = true;
		
		[self resetKeyboardGetStringVariables];
		
		// Perform initial OpenGL setup
		// - Create a custom OpenGLContext subclass
		TTOpenGLContext* context = [[TTOpenGLContext alloc] initWithFormat:p_format shareContext:0];
		[context setView:self];
		[self setOpenGLContext:context];
		
		TT_ASSERT(context == [self openGLContext]);
		context = (TTOpenGLContext*)[self openGLContext];
		
		[context makeCurrentContext];
		
		// Setup v-sync
		GLint swapInt = p_app->isVsyncEnabled() ? 1 : 0;
		[context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	}
	
	return self;
}


//--------------------------------------------------------------------------------------------------
// OpenGL and full screen support

- (void)setWindowedModeWindow:(NSWindow*)p_window
{
	m_windowedModeWindow = p_window;
}


- (void)setFullScreen:(bool)p_fullScreen
{
	//TT_Printf("TTdevObjCAppView::setFullScreen: Should switch to %s mode...\n",
	//          p_fullScreen ? "full-screen" : "windowed");
	
	if (m_isFullScreen == p_fullScreen)
	{
		// No change in full-screen mode
		return;
	}
	
	if (p_fullScreen)
	{
		// ---- Switch to full-screen mode
		
		// Instantiate new borderless window
		NSRect frame = [[NSScreen mainScreen] frame];
		
		m_fullScreenWindow = [[TTdevObjCFullScreenWindow alloc]
							  initWithContentRect:frame
							  styleMask:NSBorderlessWindowMask
							  backing:NSBackingStoreBuffered
							  defer:NO
							  screen:[m_windowedModeWindow screen]];
		if (m_fullScreenWindow == nil)
		{
			TT_PANIC("Creating full-screen window failed.");
			return;
		}
		
		// For Mac OS X Lion: do not attempt to restore this window!
		if ([m_fullScreenWindow respondsToSelector:@selector(setRestorable:)])
		{
            // Don't call [m_fullScreenWindow setRestoreable: No] directly because we also build against 10.5.
            IMP setRestorableMethod = [m_fullScreenWindow methodForSelector:@selector(setRestorable:)];
            setRestorableMethod(m_fullScreenWindow, @selector(setRestorable:), NO);
		}
		
		m_isFullScreen = p_fullScreen;
		
		// Set the options for our new fullscreen window
		[m_fullScreenWindow setTitle:[m_windowedModeWindow title]];
		[m_fullScreenWindow setReleasedWhenClosed:YES];
		[m_fullScreenWindow setOpaque:YES];
		[m_fullScreenWindow setAcceptsMouseMovedEvents:YES];
		[m_fullScreenWindow setIgnoresMouseEvents:NO];
		[m_fullScreenWindow setContentView:self];
		
		[m_fullScreenWindow setHidesOnDeactivate:YES];
		
		[m_fullScreenWindow setLevel:NSMainMenuWindowLevel + 1];
		[m_fullScreenWindow makeFirstResponder:self];
		
		//[self becomeFirstResponder];
		[m_fullScreenWindow makeKeyAndOrderFront:self];
		
		// Hide the windowed mode window
		[m_windowedModeWindow setAcceptsMouseMovedEvents:NO];
		[m_windowedModeWindow orderOut:self];
		
		TT_ASSERT([self window] == m_fullScreenWindow);
	}
	else
	{
		// ---- Switch to windowed mode
		
		TT_NULL_ASSERT(m_fullScreenWindow);
		TT_NULL_ASSERT(m_windowedModeWindow);
		
		[m_windowedModeWindow setOpaque:YES];
		[m_windowedModeWindow setAcceptsMouseMovedEvents:YES];
		[m_windowedModeWindow setIgnoresMouseEvents:NO];
		[m_windowedModeWindow setContentView:self];
		[m_windowedModeWindow makeFirstResponder:self];
		//[self becomeFirstResponder];
		[m_windowedModeWindow makeKeyAndOrderFront:self];
		
		m_isFullScreen = p_fullScreen;
		[m_fullScreenWindow close];
		
		TT_ASSERT([self window] == m_windowedModeWindow);
	}
	
	// Need to lock the OpenGL context here because rendering can be done from a different thread (DisplayLink)
	CGLContextObj contextObj = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLLockContext(contextObj);
	
	[[self openGLContext] makeCurrentContext];
	
	m_app->handleResolutionChanged();
	
	CGLUnlockContext(contextObj);
	
	tt::input::KeyboardController::handleViewChanged();
	tt::input::MouseController::handleViewChanged();
}


- (bool)isFullScreen
{
	return m_isFullScreen;
}


- (void)disableCustomCursor
{
	m_customCursorEnabled = false;
	[[self window] invalidateCursorRectsForView:self];
}


- (void)restoreCustomCursor;
{
	m_customCursorEnabled = false;
	[[self window] invalidateCursorRectsForView:self];
}


- (bool)startWideStringRetrieval:(tt::input::KeyboardController::GetStringCallback)p_callback
                        userData:(void*)p_callbackUserData
                   initialString:(const std::wstring&)p_initialString
                       acceptKey:(tt::input::Key)p_acceptKey
                       cancelKey:(tt::input::Key)p_cancelKey
{
	// NOTE: Not validating parameters here, because KeyboardController has already done so
	if (m_kbdGetStringCallback != 0)
	{
		TT_PANIC("KeyboardController is already busy retrieving a string.");
		return false;
	}
	
	// Store string retrieval information
	m_kbdGetStringCallback         = p_callback;
	m_kbdGetStringCallbackUserData = p_callbackUserData;
	m_kbdGetStringAcceptKey        = p_acceptKey;
	m_kbdGetStringCancelKey        = p_cancelKey;
	m_kbdGetStringCurrentString    = p_initialString;
	
	// Send the callback a "starting" update
	m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, m_kbdGetStringCurrentString,
	                       tt::input::KeyboardController::GetStringStatus_Starting);
	
	return true;
}


- (void)acceptGetString
{
	if (m_kbdGetStringCallback == 0)
	{
		return;
	}
	
	if (m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, m_kbdGetStringCurrentString,
	                           tt::input::KeyboardController::GetStringStatus_Complete))
	{
		[self resetKeyboardGetStringVariables];
	}
}


- (void)cancelGetString
{
	if (m_kbdGetStringCallback == 0)
	{
		return;
	}
	
	if (m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, m_kbdGetStringCurrentString,
	                           tt::input::KeyboardController::GetStringStatus_Cancelled))
	{
		[self resetKeyboardGetStringVariables];
	}
}


- (bool)isRetrievingWideString
{
	return m_kbdGetStringCallback != 0;
}


- (void)windowDidBecomeKey:(NSNotification*)p_notification
{
	if (m_app != 0 && [p_notification object] == [self window])
	{
		m_app->onAppActive();
	}
}


- (void)windowDidResignKey:(NSNotification*)p_notification
{
	if (m_app != 0 && [p_notification object] == [self window])
	{
		m_app->onAppInactive();
	}
}


//--------------------------------------------------------------------------------------------------
// OpenGL maintenance

- (void)prepareOpenGL
{
	[super prepareOpenGL];
	
	// FIXME: Do OpenGL setup here...
	/* From documentation:
	 This method is called only once after the OpenGL context is made the current context.
	 Subclasses that implement this method can use it to configure the Open GL state in preparation for drawing.
	 */
}


//--------------------------------------------------------------------------------------------------
// Mouse input event handling (for MouseController)

- (BOOL)acceptsFirstResponder
{
	return YES;
}


- (void)viewDidMoveToWindow
{
	//TT_Printf("TTdevObjCAppView::viewDidMoveToWindow: View moved to window 0x%08X. New size: %d x %d\n",
	//          [self window], (int)[self bounds].size.width, (int)[self bounds].size.height);
	[super viewDidMoveToWindow];
	
	// Register ourselves as window delegate (we want to know when window becomes active/inactive)
	if ([self window] != nil)
	{
		[[self window] setDelegate:self];
	}
	
	
	// Set up a tracking area for the entire content rect (receive mouse enter/exit/move events)
	if (m_trackingArea != 0)
	{
		[self removeTrackingArea:m_trackingArea];
		[m_trackingArea release];
		m_trackingArea = 0;
	}
	
	NSTrackingAreaOptions opts =
		NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingCursorUpdate |
		NSTrackingActiveAlways;
	m_trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
		options:opts
		owner:self
		userInfo:nil];
	
	[self addTrackingArea:m_trackingArea];
	
	// Reset cursor rects (for optional custom cursor)
	[[self window] invalidateCursorRectsForView:self];
}


- (void)setCursorPositionFromEvent:(NSEvent*)p_event cursorValid:(bool)p_valid
{
	// Translate cursor position from bottom-left (0, 0) to top-left (0, 0)
	NSRect r = [self bounds];
	const int maxX = static_cast<int>(r.origin.x + r.size.width);
	const int maxY = static_cast<int>(r.origin.y + r.size.height);
	
	// Update MouseController with the new cursor information
	tt::input::MouseController& mouse(tt::input::MouseController::getTemporaryState());
	mouse.cursor.x     = static_cast<int>([p_event locationInWindow].x);
	mouse.cursor.y     = static_cast<int>(maxY - [p_event locationInWindow].y);
	mouse.cursor.valid = p_valid;
	
	// If we are emulating the DS, offset the mouse
	using namespace tt::engine::renderer;
	if(Renderer::hasInstance())
	{
		// Handle up-scaling by Renderer (must down-scale cursor position)
		const tt::math::Vector2 scaling(Renderer::getInstance()->getUpScaler()->getScaleFactor());
		const tt::math::Vector2 offset (Renderer::getInstance()->getUpScaler()->getOffset());
		mouse.cursor.x -= static_cast<s32>(offset.x);
		mouse.cursor.y -= static_cast<s32>(offset.y);
		mouse.cursor.x  = static_cast<s32>(mouse.cursor.x * scaling.x);
		mouse.cursor.y  = static_cast<s32>(mouse.cursor.y * scaling.y);
	}
	
	if (mouse.cursor.x < 0 || mouse.cursor.x >= maxX ||
	    mouse.cursor.y < 0 || mouse.cursor.y >= maxY)
	{
		// Cursor outside of the window: automatically invalid
		mouse.cursor.valid = false;
	}
}


- (void)mouseMoved:(NSEvent*)p_event
{
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)mouseDown:(NSEvent*)p_event
{
	tt::input::MouseController::getTemporaryState().left.update(true);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)mouseDragged:(NSEvent*)p_event
{
	tt::input::MouseController::getTemporaryState().left.update(true);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)mouseUp:(NSEvent*)p_event
{
	tt::input::MouseController::getTemporaryState().left.update(false);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)rightMouseDown:(NSEvent*)p_event
{
	tt::input::MouseController::getTemporaryState().right.update(true);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)rightMouseDragged:(NSEvent*)p_event
{
	tt::input::MouseController::getTemporaryState().right.update(true);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)rightMouseUp:(NSEvent*)p_event
{
	tt::input::MouseController::getTemporaryState().right.update(false);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)mouseEntered:(NSEvent*)p_event
{
	//TT_Printf("TTdevObjCAppView::mouseEntered: Mouse entered view at position (%d, %d).\n",
	//          (int)[p_event locationInWindow].x, (int)[p_event locationInWindow].y);
	[self setCursorPositionFromEvent:p_event cursorValid:true];
}


- (void)mouseExited:(NSEvent*)p_event
{
	//TT_Printf("TTdevObjCAppView::mouseExited: Mouse exited view at position (%d, %d).\n",
	//          (int)[p_event locationInWindow].x, (int)[p_event locationInWindow].y);
	[self setCursorPositionFromEvent:p_event cursorValid:false];
}


/*
- (void)magnifyWithEvent:(NSEvent*)p_event
{
	TT_Printf("ObjectiveCMouseController::magnifyWithEvent: Detected trackpad magnification of %.2f\n",
	//          0.0f);
	          [p_event deltaZ]);
	//          [p_event magnification]); // 'magnification' property is only available in >= 10.6
	(void)p_event;
}


- (void)rotateWithEvent:(NSEvent*)p_event
{
	TT_Printf("ObjectiveCMouseController::rotateWithEvent: Detected trackpad rotation of %.2f\n",
	          [p_event rotation]);
}
//*/


- (void)swipeWithEvent:(NSEvent*)p_event
{
	const CGFloat x = [p_event deltaX];
	const CGFloat y = [p_event deltaY];
	tt::input::MouseController& mouse(tt::input::MouseController::getTemporaryState());
	
	mouse.trackpadSwipeLeft.update (x > 0.0f);
	mouse.trackpadSwipeRight.update(x < 0.0f);
	mouse.trackpadSwipeUp.update   (y > 0.0f);
	mouse.trackpadSwipeDown.update (y < 0.0f);
}


/* NOTE: These are OS X >= 10.6 only
- (void)touchesBeganWithEvent:(NSEvent*)p_event
{
	TT_Printf("ObjectiveCMouseController::touchesBeganWithEvent: Currently %d touches active.\n",
	          [p_event touchesMatchingPhase:NSTouchPhaseTouching inView:self]);
}


- (void)touchesMovedWithEvent:(NSEvent*)p_event
{
	TT_Printf("ObjectiveCMouseController::touchesMovedWithEvent: Currently %d touches active.\n",
	          [p_event touchesMatchingPhase:NSTouchPhaseTouching inView:self]);
}


- (void)touchesEndedWithEvent:(NSEvent*)p_event
{
	TT_Printf("ObjectiveCMouseController::touchesEndedWithEvent: Currently %d touches active.\n",
	          [p_event touchesMatchingPhase:NSTouchPhaseTouching inView:self]);
}


- (void)touchesCancelledWithEvent:(NSEvent*)p_event
{
	TT_Printf("ObjectiveCMouseController::touchesCancelledWithEvent: Touches cancelled.\n");
	trackpadSwipeLeft  = false;
	trackpadSwipeRight = false;
	trackpadSwipeUp    = false;
	trackpadSwipeDown  = false;
}
//*/


//* NOTE: Two-finger scrolling (on notepad trackpad) also triggers this event
- (void)scrollWheel:(NSEvent*)p_event
{
	//const CGFloat x = [p_event deltaX];
	const CGFloat y = [p_event deltaY];
	tt::input::MouseController& mouse(tt::input::MouseController::getTemporaryState());
	mouse.wheelNotches = static_cast<s32>(tt::math::round(y));
	//TT_Printf("TTdevObjCAppView::scrollWheel: Scroll wheel moved by (%f, %f)\n", [p_event deltaX], [p_event deltaY]);
}
//*/


- (void)resetCursorRects
{
	[super resetCursorRects];
	
	
	if ((m_customCursorEnabled == false ||
	    tt::input::MouseController::wasCustomCursorSet() == false) &&
	    tt::input::MouseController::isCursorVisible())
	{
		return;
	}
	
	
	NSCursor* newCursor = 0;
	if (tt::input::MouseController::getCustomCursor() != 0 &&
	    tt::input::MouseController::isCursorVisible())
	{
		newCursor = (NSCursor*)tt::input::MouseController::getCustomCursor()->getPlatformCursor();
	}
	
	if (newCursor == 0)
	{
		// Create an empty cursor as an alternative to [NSCursor hide]
		// FIXME: Does this leak? Objective C memory management is a mystery to me...
		NSImage* img = [[NSImage alloc] initWithSize:NSMakeSize(16.0f, 16.0f)];
		[img lockFocus];
		[img unlockFocus];
		newCursor = [[NSCursor alloc] initWithImage:img hotSpot:NSMakePoint(0.0f, 0.0f)];
	}
	
	if (newCursor != 0)
	{
		[self addCursorRect:[self bounds] cursor:newCursor];
		//if (m_isFullScreen)
		{
			[newCursor set];
		}
	}
}


- (void)cursorUpdate:(NSEvent*)p_event
{
	//TT_Printf("Cursor update...\n");
	[super cursorUpdate:p_event];
}


//--------------------------------------------------------------------------------------------------
// Keyboard input event handling (for KeyboardController)

- (void)keyDown:(NSEvent*)p_event
{
	if (m_kbdGetStringCallback != 0)
	{
		// String retrieval active: do not handle normal input
		/*
		{
			NSString* chars = [p_event characters];
			NSLog(@"Key characters down: '%@'", chars);
			for (NSUInteger i = 0; i < [chars length]; ++i)
			{
				NSLog(@"- 0x%04X (%u)", [chars characterAtIndex:i], [chars characterAtIndex:i]);
			}
		}
		//*/
		
		// Check if accept or cancel key was pressed
		tt::input::Key codes[16];
		s32 codeCount = 16;
		
		mapKeyCodes(p_event, codes, &codeCount);
		
		for (s32 i = 0; i < codeCount; ++i)
		{
			if (codes[i] == m_kbdGetStringAcceptKey)
			{
				m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, m_kbdGetStringCurrentString,
				                       tt::input::KeyboardController::GetStringStatus_Complete);
				[self resetKeyboardGetStringVariables];
				return;
			}
			else if (codes[i] == m_kbdGetStringCancelKey)
			{
				m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, m_kbdGetStringCurrentString,
				                       tt::input::KeyboardController::GetStringStatus_Cancelled);
				[self resetKeyboardGetStringVariables];
				return;
			}
			else if (codes[i] == tt::input::Key_Tab)
			{
				// Ignore tab characters (not valid)
				return;
			}
			else if (codes[i] == tt::input::Key_Backspace)
			{
				if (m_kbdGetStringCurrentString.empty() == false)
				{
					m_kbdGetStringCurrentString.erase(m_kbdGetStringCurrentString.length() - 1);
					m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, m_kbdGetStringCurrentString,
					                       tt::input::KeyboardController::GetStringStatus_CharRemoved);
				}
				return;
			}
		}
		
		const std::wstring addedChars(getPrintableCharacters(p_event));
		if (addedChars.empty() == false)
		{
			const std::wstring newString(m_kbdGetStringCurrentString + addedChars);
			if (m_kbdGetStringCallback(m_kbdGetStringCallbackUserData, newString,
			                           tt::input::KeyboardController::GetStringStatus_CharAdded))
			{
				m_kbdGetStringCurrentString = newString;
			}
		}
		
		return;
	}
	
	// Append the typed Unicode characters to the KeyboardController::chars string
	std::wstring typedChars;
	NSString* chars = [p_event characters];
	for (NSUInteger i = 0; i < [chars length]; ++i)
	{
		typedChars += static_cast<std::wstring::value_type>([chars characterAtIndex:i]);
	}
	tt::input::KeyboardController::addTypedChars(typedChars);
	
	// Now handle raw key code presses/releases
	if ([p_event isARepeat])
	{
		// Not interested in key repeats
		return;
	}
	
	//NSLog(@"keyDown: '%@' ('%@')", [p_event characters], [p_event charactersIgnoringModifiers]);
	
	tt::input::Key codes[16];
	s32 codeCount = 16;
	
	mapKeyCodes(p_event, codes, &codeCount);
	
	for (s32 i = 0; i < codeCount; ++i)
	{
		//TT_Printf("keyDown: Key code %d ('%s') down.\n", codes[i], tt::input::getKeyName(codes[i]));
		tt::input::KeyboardController::setKeyDown(codes[i], true);
	}
}


- (void)keyUp:(NSEvent*)p_event
{
	if (m_kbdGetStringCallback != 0)
	{
		// String retrieval active: do not handle normal input
		return;
	}
	
	
	if ([p_event isARepeat])
	{
		// Not interested in key repeats
		return;
	}
	
	tt::input::Key codes[16];
	s32 codeCount = 16;
	
	mapKeyCodes(p_event, codes, &codeCount);
	
	for (s32 i = 0; i < codeCount; ++i)
	{
		tt::input::KeyboardController::setKeyDown(codes[i], false);
	}
}


- (void)flagsChanged:(NSEvent*)p_event
{
	if (m_kbdGetStringCallback != 0)
	{
		// String retrieval active: do not handle normal input
		return;
	}
	
	NSUInteger flags = [p_event modifierFlags];
	tt::input::KeyboardController::setKeyDown(tt::input::Key_Alt,     (flags & NSAlternateKeyMask) == NSAlternateKeyMask);
	tt::input::KeyboardController::setKeyDown(tt::input::Key_Shift,   (flags & NSShiftKeyMask    ) == NSShiftKeyMask);
	tt::input::KeyboardController::setKeyDown(tt::input::Key_Control, (flags & NSControlKeyMask  ) == NSControlKeyMask);
}


- (void)windowDidResize:(NSNotification*)notification
{
	(void)notification;
	CGLContextObj contextObj = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLLockContext(contextObj);
	
	[[self openGLContext] makeCurrentContext];
	
	m_app->handleResolutionChanged();
	
	CGLUnlockContext(contextObj);
}

@end

#endif  // defined(TT_PLATFORM_OSX_MAC)
