#if defined(TT_PLATFORM_OSX_MAC)
#import <Cocoa/Cocoa.h>
#endif

#include <tt/app/fatal_error.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace app {

void reportFatalError(const std::string& p_message)
{
#if defined(TT_BUILD_FINAL)
#if defined(TT_PLATFORM_OSX_MAC)
	// Show fatal error
	NSAlert* alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"Exit"];
	[alert setMessageText:@"Fatal Error"];
	[alert setInformativeText:[NSString stringWithUTF8String: p_message.c_str()]];
	[alert setAlertStyle:NSCriticalAlertStyle];
	
	[alert runModal];
#endif
#else
	TT_PANIC("%s", p_message.c_str());
#endif
	
	exit(1); // immediately exit, do not continue through regular NSApplication terminate logic
}

// Namespace end
}
}
