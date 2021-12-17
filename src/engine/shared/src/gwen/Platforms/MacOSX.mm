/*
	GWEN
	Copyright (c) 2012 Facepunch Studios
	See license in Gwen.h
*/

// Two Tribes implementation of Mac OS X support (using Two Tribes libraries)

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"

#if defined(TT_PLATFORM_OSX_MAC)

#import <AppKit/AppKit.h>

#include <time.h>

#include <tt/input/MouseController.h>
#include <tt/str/str.h>
#include <tt/system/utils.h>


void Gwen::Platform::Sleep( unsigned int /*iMS*/ )
{
	// TODO.
}


void Gwen::Platform::SetCursor(unsigned char p_cursor)
{
	if (p_cursor == 255)  // Two Tribes change: interpret 255 as "don't set a cursor"
	{
		return;
	}
	
	enum GwenCursor
	{
		GwenCursor_Arrow,     // Windows: IDC_ARROW
		GwenCursor_IBeam,     // Windows: IDC_IBEAM
		GwenCursor_SizeNS,    // Windows: IDC_SIZENS
		GwenCursor_SizeWE,    // Windows: IDC_SIZEWE
		GwenCursor_SizeNWSE,  // Windows: IDC_SIZENWSE
		GwenCursor_SizeNESW,  // Windows: IDC_SIZENESW
		GwenCursor_SizeAll,   // Windows: IDC_SIZEALL
		GwenCursor_No,        // Windows: IDC_NO
		GwenCursor_Wait,      // Windows: IDC_WAIT
		GwenCursor_Hand       // Windows: IDC_HAND
	};
	
	using tt::input::MouseCursor;
	MouseCursor::SystemCursor systemCursor;
	switch (p_cursor)
	{
	case GwenCursor_Arrow:     systemCursor = MouseCursor::SystemCursor_Arrow;           break;
	case GwenCursor_IBeam:     systemCursor = MouseCursor::SystemCursor_IBeam;           break;
	case GwenCursor_SizeNS:    systemCursor = MouseCursor::SystemCursor_ResizeUpDown;    break;
	case GwenCursor_SizeWE:    systemCursor = MouseCursor::SystemCursor_ResizeLeftRight; break;
	//case GwenCursor_SizeNWSE:  systemCursor = MouseCursor::SystemCursor_; break;  // no corresponding system cursor
	//case GwenCursor_SizeNESW:  systemCursor = MouseCursor::SystemCursor_; break;  // no corresponding system cursor
	//case GwenCursor_SizeAll:   systemCursor = MouseCursor::SystemCursor_; break;  // no corresponding system cursor
	//case GwenCursor_No:        systemCursor = MouseCursor::SystemCursor_; break;  // no corresponding system cursor
	//case GwenCursor_Wait:      systemCursor = MouseCursor::SystemCursor_; break;  // no corresponding system cursor
	case GwenCursor_Hand:      systemCursor = MouseCursor::SystemCursor_PointingHand;    break;
		
	default:
		// Unsupported cursor identifier
		return;
	}
	
	tt::input::MouseController::setCustomCursor(MouseCursor::create(systemCursor));
}


Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	//@try
	{
		NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
		
		if ([pasteboard availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]] == nil)
		{
			// Clipboard does not contain text
			return Gwen::UnicodeString();
		}
		
		NSData* data = [pasteboard dataForType:NSStringPboardType];
		if (data == nil)
		{
			//TT_PANIC("Could not retrieve clipboard text from system clipboard.");
			return Gwen::UnicodeString();
		}
		
		NSString* text = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
		
		Gwen::UnicodeString retval;
		retval.reserve(static_cast<Gwen::UnicodeString::size_type>([text length]));
		for (NSUInteger i = 0; i < [text length]; ++i)
		{
			retval += static_cast<std::wstring::value_type>([text characterAtIndex:i]);
		}
		
		return retval;
	}
	/*
	@catch (id exception)
	{
		TT_PANIC("Could not retrieve clipboard text from system clipboard.");
		return Gwen::UnicodeString();
	}
	// */
}


bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString& str )
{
	//@try
	{
		NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
		
		[pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
		
		NSString* clipboardText = [NSString stringWithUTF8String:tt::str::utf16ToUtf8(str).c_str()];
		if ([pasteboard setString:clipboardText forType:NSStringPboardType] == NO)
		{
			//TT_PANIC("Could not write new clipboard text to system clipboard.");
			return false;
		}
	}
	/*
	@catch (id exception)
	{
		TT_PANIC("Could not write new clipboard text to system clipboard.");
		return false;
	}
	// */
	
	return true;
}


float Gwen::Platform::GetTimeInSeconds()
{
	float fSeconds = (float) clock() / (float)CLOCKS_PER_SEC;
	return fSeconds;
}


bool Gwen::Platform::FileOpen( const String& /*Name*/, const String& /*StartPath*/, const String& /*Extension*/, Gwen::Event::Handler* /*pHandler*/, Event::Handler::FunctionWithInformation /*fnCallback*/ )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here

	return false;
}


bool Gwen::Platform::FileSave( const String& /*Name*/, const String& /*StartPath*/, const String& /*Extension*/, Gwen::Event::Handler* /*pHandler*/, Gwen::Event::Handler::FunctionWithInformation /*fnCallback*/ )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here

	return false;
}


bool Gwen::Platform::FolderOpen( const String& /*Name*/, const String& /*StartPath*/, Gwen::Event::Handler* /*pHandler*/, Event::Handler::FunctionWithInformation /*fnCallback*/ )
{
	return false;
}


void* Gwen::Platform::CreatePlatformWindow( int /*x*/, int /*y*/, int /*w*/, int /*h*/, const Gwen::String& /*strWindowTitle*/ )
{
	return 0;
}


void Gwen::Platform::DestroyPlatformWindow( void* /*pPtr*/ )
{
	
}


void Gwen::Platform::MessagePump( void* /*pWindow*/, Gwen::Controls::Canvas* /*ptarget*/ )
{
	
}


void Gwen::Platform::SetBoundsPlatformWindow( void* /*pPtr*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/ )
{
	
}


void Gwen::Platform::SetWindowMaximized( void* /*pPtr*/, bool /*bMax*/, Gwen::Point& /*pNewPos*/, Gwen::Point& /*pNewSize*/ )
{

}


void Gwen::Platform::SetWindowMinimized( void* /*pPtr*/, bool /*bMinimized*/ )
{

}


bool Gwen::Platform::HasFocusPlatformWindow( void* /*pPtr*/ )
{
	return true;
}


void Gwen::Platform::GetDesktopSize( int& w, int &h )
{
	w = 1024;
	h = 768;
}


void Gwen::Platform::GetCursorPos( Gwen::Point &/*po*/ )
{
}


#endif  // defined(TT_PLATFORM_OSX_MAC)
