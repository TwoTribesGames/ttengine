#if defined(TT_ASSERT_ON)


#include <stdarg.h>
#include <stdio.h>
#include <sstream>

#if defined(TT_PLATFORM_OSX_MAC)
#import <Cocoa/Cocoa.h>
#include <tt/app/OsxApp_desktop.h>
#include <tt/app/TTdevObjCAppView_desktop.h>
#elif defined(TT_PLATFORM_OSX_IPHONE)
#import <UIKit/UIKit.h>
#include <tt/app/TTdevObjCOsxApp_iphone.h>
#else
#error Unsupported platform.
#endif

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/version/Version.h>



#if defined(TT_PLATFORM_OSX_IPHONE)

NSInteger g_iphonePanicSelectedButton = -1;


@interface IPhonePanicPopupDelegate : NSObject <UIAlertViewDelegate>
{
}

- (void)alertView:(UIAlertView*)p_alertView clickedButtonAtIndex:(NSInteger)p_buttonIndex;
- (void)alertView:(UIAlertView*)p_alertView didDismissWithButtonIndex:(NSInteger)p_buttonIndex;
- (void)alertView:(UIAlertView*)p_alertView willDismissWithButtonIndex:(NSInteger)p_buttonIndex;
- (void)alertViewCancel:(UIAlertView*)p_alertView;

@end


@implementation IPhonePanicPopupDelegate

- (void)alertView:(UIAlertView*)p_alertView clickedButtonAtIndex:(NSInteger)p_buttonIndex
{
	(void)p_alertView;
	g_iphonePanicSelectedButton = p_buttonIndex;
}

- (void)alertView:(UIAlertView*)p_alertView didDismissWithButtonIndex:(NSInteger)p_buttonIndex;
{
	(void)p_alertView;
	g_iphonePanicSelectedButton = p_buttonIndex;
}

- (void)alertView:(UIAlertView*)p_alertView willDismissWithButtonIndex:(NSInteger)p_buttonIndex
{
	(void)p_alertView;
	g_iphonePanicSelectedButton = p_buttonIndex;
}

- (void)alertViewCancel:(UIAlertView*)p_alertView
{
	(void)p_alertView;
	g_iphonePanicSelectedButton = -1;
}

@end

#endif



namespace tt {
namespace platform {
namespace error {

struct AssertInfo
{
	std::string file;
	int         lineNumber;
	
	inline AssertInfo(const std::string& p_file       = std::string(),
	                  int                p_lineNumber = 0)
	:
	file(p_file),
	lineNumber(p_lineNumber)
	{ }
};
typedef std::vector<AssertInfo> AssertList;


static PanicCallback g_panicCallback = 0;
static bool          g_isHeadless    = false;
static AssertList    g_ignoredAsserts;

enum TTConstants
{
	TTConstants_CommonBufferSize = 2048
};


static inline bool isAssertInIgnoreList(const char* p_file, int p_line)
{
	for (AssertList::iterator it = g_ignoredAsserts.begin(); it != g_ignoredAsserts.end(); ++it)
	{
		if ((*it).file == p_file && (*it).lineNumber == p_line)
		{
			return true;
		}
	}
	
	return false;
}



void TTVPrintf(const char* p_fmt, va_list p_vlist)
{
	static char TTCommonBuffer[TTConstants_CommonBufferSize] = { 0 };
	
	(void)vsnprintf(TTCommonBuffer,
	                TTConstants_CommonBufferSize, p_fmt, p_vlist);
	TTCommonBuffer[TTConstants_CommonBufferSize - 1] = 0;
	
	TT_Printf("%s\n", TTCommonBuffer);
}


void registerPanicCallback(PanicCallback p_fun)
{
	g_panicCallback = p_fun;
}


void turnHeadlessModeOn()
{
	g_isHeadless = true;
}


void TTPanic(const char* p_file, int p_line, const char* p_function, 
             const char* p_fmt, ...)
{
	//int returnVal = 0;
	
	va_list vlist;
	va_start(vlist, p_fmt);
	
	static char TTCommonBuffer[TTConstants_CommonBufferSize] = { 0 };
	
	TT_Printf("------------ PANIC -----------\n\n");
	TT_Printf("FILE: %s\n", p_file);
	TT_Printf("LINE: %d\n", p_line);
	TT_Printf("FUNCTION: %s\n", p_function);
	
	TT_Printf("CLIENT VERSION: %s (#%d)\n", tt::version::getClientVersionName(),
		tt::version::getClientRevisionNumber());
	TT_Printf("LIB VERSION: %s (#%d)\n", tt::version::getLibVersionName(),
		tt::version::getLibRevisionNumber());
	
	TT_Printf("\n");
	
	TT_Printf("MESSAGE:\n");
	TTVPrintf(p_fmt, vlist);
	
	TT_Printf("\n");
	
	if (g_isHeadless)
	{
		exit(1);
		return;
	}
	
	if (g_panicCallback != 0)
	{
		g_panicCallback();
	}
	
	if (isAssertInIgnoreList(p_file, p_line))
	{
		return;
	}
	
	(void)vsnprintf(TTCommonBuffer,
	                TTConstants_CommonBufferSize, p_fmt, vlist);
	TTCommonBuffer[TTConstants_CommonBufferSize - 1] = 0;
	
	
	std::ostringstream informativeText;
	informativeText << TTCommonBuffer
	                << "\n\n\nFile: " << p_file
	                << "\nLine: " << p_line
	                << "\n\nFunction:\n" << p_function
	                << "\n\nRevision:\n"
	                << tt::version::getClientRevisionNumber() << "."
	                << tt::version::getLibRevisionNumber();
	
#if defined(TT_PLATFORM_OSX_IPHONE)
	
	// ======== iOS ========
	g_iphonePanicSelectedButton = -1;
	
	// Get the Objective C application class
	TTdevObjCOsxApp* osxApp = nil;
	if (app::hasApplication())
	{
		// NOTE: Assuming getApplication points to an iOS OsxApp instance (since this is iOS)
		app::OsxApp* cppApp = (app::OsxApp*)app::getApplication();
		osxApp = (TTdevObjCOsxApp*)cppApp->getObjCApp();
	}
	
	// Pause the application message pump while the panic dialog is open
	if (osxApp != nil)
	{
		[osxApp pauseTimer];
	}
	
	//TT_Printf("TTPanic: Creating and displaying iPhone alert.\n");
	UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"TT Panic"
		message:[NSString stringWithUTF8String: informativeText.str().c_str()]
		delegate:[IPhonePanicPopupDelegate alloc]
		cancelButtonTitle:@"Debug"
		otherButtonTitles:@"Continue", nil];
	[alert show];
	[alert release];
	
	// Start a local message pump to make the alert 'modal'
	while (!alert.hidden && alert.superview != nil)
	{
		[[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
	}
	
	// Check which button was selected on the panic popup
	if (g_iphonePanicSelectedButton == 0) // 'debug' button selected
	{
		// Trigger a breakpoint
#if defined(TT_PLATFORM_IPH_HARDWARE)
		asm("trap");  // this assembly instruction triggers a breakpoint on the iPhone hardware
#elif defined(TT_PLATFORM_IPH_SIMULATOR)
		asm("int3");  // this assembly instruction triggers a breakpoint on the iPhone simulator
#else
		int* nullPointer = 0;
		*nullPointer = 42;
#endif
	}
	
	// Resume the application message pump
	if (osxApp != nil)
	{
		[osxApp resumeTimer];
	}
	
#else
	
	// ======== Mac OS X ========
	// NOTE: This appears to only work when a GUI application with window is already set up
	NSAlert* alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"Exit"];
	[alert addButtonWithTitle:@"Debug Break"];
	[alert addButtonWithTitle:@"Ignore"];
	[alert addButtonWithTitle:@"Ignore All"];
	[alert setMessageText:@"Two Tribes Panic"];
	[alert setInformativeText:[NSString stringWithUTF8String: informativeText.str().c_str()]];
	[alert setAlertStyle:NSCriticalAlertStyle];
	
	// Temporarily make the application window be at a normal window level
	NSWindow* appWindow       = nil;
	NSInteger origWindowLevel = 0;
	if (app::hasApplication())
	{
		// NOTE: Assuming getApplication points to a desktop OsxApp instance (since this is desktop OS X)
		app::OsxApp*      osxApp = (app::OsxApp*)app::getApplication();
		TTdevObjCAppView* view   = (TTdevObjCAppView*)osxApp->getAppView();
		appWindow       = [view window];
		origWindowLevel = [appWindow level];
		[appWindow setLevel:NSNormalWindowLevel];
	}
	
	NSInteger response = [alert runModal];
	
	switch (response)
	{
	case NSAlertFirstButtonReturn: // Exit
		// Exit the application right away, so that picking "Exit" truly does exit
		// (the [NSApp terminate] can result in many more panics before finally quitting)
		abort();
		//[NSApp terminate:nil];
		break;
		
	case NSAlertSecondButtonReturn: // Debug Break
		// Trigger a breakpoint
		// FIXME: Find out how to trigger a breakpoint programmatically in Xcode / GCC / GDB
#if TARGET_CPU_X86
		// Trigger a breakpoint using an interrupt
#if defined(__clang__)
		asm("int3");
#else
		__asm {int 3};
#endif
#elif TARGET_CPU_PPC || TARGET_CPU_PPC64
		// Trigger a breakpoint using an interrupt
		asm {trap};
#else
		// Trigger a breakpoint in a less elegant manner: dereference a null pointer
		{
			int* nullPointer = 0;
			*nullPointer = 42;
		}
#endif
		break;
		
	case NSAlertThirdButtonReturn: // Ignore
		// Nothing to do here; panic is being ignored
		break;
			
	case NSAlertThirdButtonReturn + 1: // Ignore All
		// Add this assert to the ignore list (and do nothing else)
		g_ignoredAsserts.push_back(AssertInfo(p_file, p_line));
		break;
	}
	
	// Restore the application window's window level
	if (appWindow != nil)
	{
		[appWindow setLevel:origWindowLevel];
	}
	
#endif
}


void TTWarning(const char* p_file, int p_line, const char* p_function, 
               const char* p_fmt, ...)
{
	va_list vlist;
	va_start(vlist, p_fmt);
	
	TT_Printf("WARNING file: %s:%d\n%s\n", p_file, p_line, p_function);
	// FIXME: Change to assert dialog box
	TTVPrintf(p_fmt, vlist);
	TT_Printf("\n");
}

// Namespace end
}
}
}


#endif  // defined(TT_ASSERT_ON)
