#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iPhone) builds only

#import <Cocoa/Cocoa.h>

#include <tt/app/TTdevObjCAppView_desktop.h>
#include <tt/fileformat/cursor/CursorData.h>
#include <tt/fileformat/cursor/CursorDirectory.h>
#include <tt/input/MouseController.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace input {

bool            MouseController::ms_initialized = false;
MouseController MouseController::ms_controller;
MouseController MouseController::ms_temporary;
bool            MouseController::ms_customCursorWasSet = false;
MouseCursorPtr  MouseController::ms_customCursor;
void*           MouseController::ms_appView = 0;
bool            MouseController::ms_cursorVisible = true;

static MouseCursorPtr g_systemCursors[MouseCursor::SystemCursor_Count];


//--------------------------------------------------------------------------------------------------
// Custom mouse cursor support class

MouseCursorPtr MouseCursor::create(const std::string& p_windowsCursorFilename, s32 p_cursorIndex)
{
	// Load the Windows cursor data
	using namespace fileformat::cursor;
	CursorDirectoryPtr dir(CursorDirectory::load(p_windowsCursorFilename));
	if (dir == 0)
	{
		TT_PANIC("Loading Windows cursor file '%s' failed.", p_windowsCursorFilename.c_str());
		return MouseCursorPtr();
	}
	
	if (p_cursorIndex < 0 || p_cursorIndex >= dir->getCursorCount())
	{
		TT_PANIC("Windows cursor file '%s' does not have cursor index %d. It has %d cursor(s).",
		         p_windowsCursorFilename.c_str(), p_cursorIndex, dir->getCursorCount());
		return MouseCursorPtr();
	}
	
	const CursorData* cursorData = dir->getCursor(p_cursorIndex);
	TT_NULL_ASSERT(cursorData);
	if (cursorData == 0)
	{
		return MouseCursorPtr();
	}
	
	const s32 cursorWidth  = cursorData->getWidth();
	const s32 cursorHeight = cursorData->getHeight();
	
	// Create a bitmap image representation for the cursor
	NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc]
			initWithBitmapDataPlanes: 0
			              pixelsWide: cursorWidth
			              pixelsHigh: cursorHeight
			           bitsPerSample: 8
			         samplesPerPixel: 4
			                hasAlpha: YES
			                isPlanar: NO
			          colorSpaceName: NSDeviceRGBColorSpace
			            bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
			             bytesPerRow: cursorData->getWidth() * 4
			            bitsPerPixel: 32 ];
	if (bitmapRep == nil)
	{
		TT_PANIC("Could not create a Mac OS X bitmap image representation for Windows cursor file '%s'.",
		         p_windowsCursorFilename.c_str());
		return MouseCursorPtr();
	}
	
	// Copy the actual image data (so that the image representation performs its own memory management)
	const mem::size_type dataSize = cursorWidth * cursorHeight * 4;
	mem::copy8([bitmapRep bitmapData], cursorData->getImageData(), dataSize);
	
	// Create an image using the image representation
	NSImage* img = [[NSImage alloc] initWithSize:NSMakeSize(cursorWidth, cursorHeight)];
	[img addRepresentation:bitmapRep];
	
	// Finally, we can create a cursor based on the image
	NSCursor* cursor = [[NSCursor alloc]
			initWithImage: img
			      hotSpot: NSMakePoint((CGFloat)cursorData->getHotSpot().x,
				                       (CGFloat)cursorData->getHotSpot().y)];
	if (cursor == nil)
	{
		TT_PANIC("Creating cursor for Windows cursor file '%s' failed.", p_windowsCursorFilename.c_str());
		return MouseCursorPtr();
	}
	
	return MouseCursorPtr(new MouseCursor(cursor, true));
}


MouseCursorPtr MouseCursor::create(const std::string& p_imageFilename, const math::Point2& p_hotSpot)
{
	NSImage* img = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:p_imageFilename.c_str()]];
	if (img == nil)
	{
		TT_PANIC("Loading cursor image '%s' failed.", p_imageFilename.c_str());
		return MouseCursorPtr();
	}
	
	NSCursor* cursor = [[NSCursor alloc] initWithImage:img hotSpot:NSMakePoint((CGFloat)p_hotSpot.x, (CGFloat)p_hotSpot.y)];
	if (cursor == nil)
	{
		TT_PANIC("Creating cursor with image '%s' failed.", p_imageFilename.c_str());
		return MouseCursorPtr();
	}
	
	//[cursor retain];
	return MouseCursorPtr(new MouseCursor(cursor, true));
}


MouseCursorPtr MouseCursor::create(SystemCursor p_systemCursor)
{
	if (p_systemCursor < 0 || p_systemCursor >= SystemCursor_Count)
	{
		TT_PANIC("Invalid system cursor: %d", p_systemCursor);
		return MouseCursorPtr();
	}
	
	if (g_systemCursors[p_systemCursor] == 0)
	{
		NSCursor* cursor = nil;
		switch (p_systemCursor)
		{
		case SystemCursor_Arrow:           cursor = [NSCursor arrowCursor];           break;
		case SystemCursor_IBeam:           cursor = [NSCursor IBeamCursor];           break;
		case SystemCursor_Crosshair:       cursor = [NSCursor crosshairCursor];       break;
		case SystemCursor_ClosedHand:      cursor = [NSCursor closedHandCursor];      break;
		case SystemCursor_OpenHand:        cursor = [NSCursor openHandCursor];        break;
		case SystemCursor_PointingHand:    cursor = [NSCursor pointingHandCursor];    break;
		case SystemCursor_ResizeLeft:      cursor = [NSCursor resizeLeftCursor];      break;
		case SystemCursor_ResizeRight:     cursor = [NSCursor resizeRightCursor];     break;
		case SystemCursor_ResizeLeftRight: cursor = [NSCursor resizeLeftRightCursor]; break;
		case SystemCursor_ResizeUp:        cursor = [NSCursor resizeUpCursor];        break;
		case SystemCursor_ResizeDown:      cursor = [NSCursor resizeDownCursor];      break;
		case SystemCursor_ResizeUpDown:    cursor = [NSCursor resizeUpDownCursor];    break;
			
		default:
			TT_PANIC("Unsupported system cursor: %d", p_systemCursor);
			return MouseCursorPtr();
		}
		
		g_systemCursors[p_systemCursor].reset(new MouseCursor(cursor, false));
	}
	
	return g_systemCursors[p_systemCursor];
}


MouseCursor::MouseCursor(void* p_cursor, bool p_haveOwnership)
:
m_cursor(p_cursor),
m_haveOwnership(p_haveOwnership)
{
}


MouseCursor::~MouseCursor()
{
	if (m_cursor != 0 && m_haveOwnership)
	{
		[(NSCursor*)m_cursor release];
		m_cursor = 0;
	}
}


//--------------------------------------------------------------------------------------------------
// Public member functions

bool MouseController::isConnected(ControllerIndex p_index)
{
	return p_index == ControllerIndex_One && isInitialized();
}


const MouseController& MouseController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "MouseController has not been initialized yet.");
	TT_ASSERTMSG(p_index == ControllerIndex_One, "Invalid controller index: %d", p_index);
	return ms_controller;
}


void MouseController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("MouseController has not been initialized yet.");
		return;
	}
	
	// Update controller state
	ms_controller.left.update  (ms_temporary.left.down);
	ms_controller.middle.update(ms_temporary.middle.down);
	ms_controller.right.update (ms_temporary.right.down);
	ms_controller.cursor = ms_temporary.cursor;
	
	ms_controller.trackpadSwipeLeft.update (ms_temporary.trackpadSwipeLeft.down);
	ms_controller.trackpadSwipeRight.update(ms_temporary.trackpadSwipeRight.down);
	ms_controller.trackpadSwipeUp.update   (ms_temporary.trackpadSwipeUp.down);
	ms_controller.trackpadSwipeDown.update (ms_temporary.trackpadSwipeDown.down);
	
	ms_controller.wheelNotches = ms_temporary.wheelNotches;
	
	// Reset swipe stats, since no "swipe released" event is sent
	ms_temporary.wheelNotches = 0;
	ms_temporary.trackpadSwipeLeft.update(false);
	ms_temporary.trackpadSwipeRight.update(false);
	ms_temporary.trackpadSwipeUp.update(false);
	ms_temporary.trackpadSwipeDown.update(false);
}


bool MouseController::isInitialized()
{
	return ms_initialized;
}


bool MouseController::initialize(void* p_appView)
{
	if (isInitialized())
	{
		TT_PANIC("MouseController is already initialized.");
		return false;
	}
	
	if (p_appView == 0)
	{
		TT_PANIC("Invalid AppView pointer passed.");
		return false;
	}
	
	ms_appView     = p_appView;
	ms_initialized = true;
	
	return true;
}


void MouseController::deinitialize()
{
	TT_ASSERTMSG(isInitialized(), "MouseController is not initialized.");
	
	ms_customCursor.reset();
	
	ms_appView     = 0;
	ms_initialized = false;
}


void MouseController::setCustomCursor(const MouseCursorPtr& p_cursor)
{
	if (ms_initialized == false)
	{
		TT_PANIC("Cannot set custom cursor when controller is not initialized.");
		return;
	}
	
	ms_customCursorWasSet = true;
	ms_customCursor       = p_cursor;
	
	// Update the cursor
	TTdevObjCAppView* view = (TTdevObjCAppView*)ms_appView;
	[[view window] invalidateCursorRectsForView:view];
	
	/*
	NSCursor* newCursor = 0;
	if (p_cursor != 0)
	{
		newCursor = (NSCursor*)p_cursor->m_cursor;
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
		[view addCursorRect:[view bounds] cursor:newCursor];
		if (ms_controller.cursor.valid)
		{
			// Cursor in window; force cursor update
			[newCursor set];
		}
	}
	//*/
}


void MouseController::setCursorVisible(bool p_visible)
{
	if (ms_cursorVisible != p_visible)
	{
		ms_cursorVisible = p_visible;
		
		// Ensure the view is up to date
		TTdevObjCAppView* view = (TTdevObjCAppView*)ms_appView;
		[[view window] invalidateCursorRectsForView:view];
	}
}


void MouseController::handleViewChanged()
{
	ms_temporary.left.update(false);
	ms_temporary.middle.update(false);
	ms_temporary.right.update(false);
	ms_temporary.trackpadSwipeLeft.update(false);
	ms_temporary.trackpadSwipeRight.update(false);
	ms_temporary.trackpadSwipeUp.update(false);
	ms_temporary.trackpadSwipeDown.update(false);
	
	if (ms_initialized && ms_customCursorWasSet)
	{
		setCustomCursor(ms_customCursor);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

MouseController::MouseController()
:
left(),
middle(),
right(),
cursor(),
wheelNotches(0),
trackpadSwipeLeft(),
trackpadSwipeRight(),
trackpadSwipeUp(),
trackpadSwipeDown()
{
}

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_MAC)
