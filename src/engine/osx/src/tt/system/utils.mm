#if defined(TT_PLATFORM_OSX_MAC)
#import <AppKit/AppKit.h>
#endif

#include <tt/platform/tt_error.h>
#include <tt/str/str.h>
#include <tt/system/utils.h>


namespace tt {
namespace system {

bool openWithDefaultApplication(const std::string& p_item)
{
#if defined(TT_PLATFORM_OSX_MAC)
	return [[NSWorkspace sharedWorkspace] openFile:[NSString stringWithUTF8String:p_item.c_str()]];
#else
	TT_PANIC("No implementation for iOS yet. Cannot open item '%s'.", p_item.c_str());
	return false;
#endif
}


bool editWithDefaultApplication(const std::string& p_item)
{
	(void)p_item;
	TT_PANIC("No implementation for Mac OS X or iOS yet. Cannot edit item '%s'.", p_item.c_str());
	
	return false;
}


bool showFileInFileNavigator(const std::string& p_item)
{
#if defined(TT_PLATFORM_OSX_MAC)
	if ([[NSWorkspace sharedWorkspace] respondsToSelector:@selector(activateFileViewerSelectingURLs:)])
	{
		// For Mac OS X 10.6 and up:
		@autoreleasepool {
			NSArray* fileUrls = [NSArray arrayWithObjects:[NSString stringWithUTF8String:p_item.c_str()], nil];
		
			[[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileUrls];
		}

		return true;
	}
	else
	{
		// Backwards compatible implementation:
		
		return [[NSWorkspace sharedWorkspace] selectFile:[NSString stringWithUTF8String:p_item.c_str()] inFileViewerRootedAtPath:@""];
	}
#else
	TT_PANIC("No implementation for iOS yet. Cannot show file '%s' in file navigator.", p_item.c_str());
	return false;
#endif
}


// for SDL2 we are using SDL2's clipboard code
#ifndef TT_PLATFORM_SDL

bool setSystemClipboardText(const str::Strings& p_lines)
{
#if defined(TT_PLATFORM_OSX_MAC)
	
	//@try
	{
		NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
		
		[pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
		
		NSString* clipboardText = [NSString stringWithUTF8String:str::implode(p_lines, "\n").c_str()];
		if ([pasteboard setString:clipboardText forType:NSStringPboardType] == NO)
		{
			TT_PANIC("Could not write new clipboard text to system clipboard.");
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
	
#else
	(void)p_lines;
	TT_PANIC("No implementation for iOS yet. Cannot set clipboard text.");
	return false;
#endif
}


bool getSystemClipboardText(str::Strings* p_lines_OUT)
{
#if defined(TT_PLATFORM_OSX_MAC)
	
	TT_NULL_ASSERT(p_lines_OUT);
	if (p_lines_OUT == 0)
	{
		return false;
	}
	
	//@try
	{
		NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
		
		if ([pasteboard availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]] == nil)
		{
			// Clipboard does not contain text: return 0 lines of text
			p_lines_OUT->clear();
			return true;
		}
		
		NSData* data = [pasteboard dataForType:NSStringPboardType];
		if (data == nil)
		{
			TT_PANIC("Could not retrieve clipboard text from system clipboard.");
			return false;
		}
		
		NSString* text = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
		*p_lines_OUT = str::explode([text UTF8String], "\n", true);
	}
	/*
	@catch (id exception)
	{
		TT_PANIC("Could not retrieve clipboard text from system clipboard.");
		return false;
	}
	// */
	
	return true;
	
#else
	(void)p_lines_OUT;
	TT_PANIC("No implementation for iOS yet. Cannot get clipboard text.");
	return false;
#endif
}

#endif
	

std::string getDesktopPath()
{
#ifdef TT_PLATFORM_OSX_MAC
	// Get the user's Desktop path
	std::string desktopDir;
	@autoreleasepool {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);
		desktopDir = [[paths objectAtIndex:0] UTF8String];
		if (*desktopDir.rbegin() != '/') desktopDir += "/";
	}
	return desktopDir;
#else
	TT_PANIC("No implementation for iOS yet. Cannot get desktop path");
	return "";
#endif
}

std::string getOldMacOSXSavePath()
{
#ifdef TT_PLATFORM_OSX_MAC
	std::string rootDir;

	@autoreleasepool {
		// Get the user's Library path
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
		rootDir = [[paths objectAtIndex:0] UTF8String];
		if (*rootDir.rbegin() != '/') rootDir += "/";

		// Append this application's bundle identifier (requires that this app is actually a bundle)
		rootDir += [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"] UTF8String];
		rootDir += "/";
	}

	return rootDir;
#else
	TT_PANIC("No implementation for iOS yet. Cannot get desktop path");
	return "";
#endif
}

// Namespace end
}
}
