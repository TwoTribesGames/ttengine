#if !defined(TT_STEAM_BUILD) && defined(TT_BUILD_FINAL)
	#define TT_VALIDATE_APPSTORE_RECEIPT 1
#else
	#define TT_VALIDATE_APPSTORE_RECEIPT 0
#endif

#import <Cocoa/Cocoa.h>

#include <tt/thread/Semaphore.h>

#if TT_VALIDATE_APPSTORE_RECEIPT
#include "appstore/receipt_validation.h"
#endif


int main(int p_argc, char* p_argv[])
{
#if !defined(TT_STEAM_BUILD)
	// For sandboxed Mac OS X apps, the semaphore name must start with the "application group identifier"
	// (this value is copied from the entitlements file)
	tt::thread::Semaphore::setNamePrefix("YEYVX78875.toki2");
#endif
	
#if TT_VALIDATE_APPSTORE_RECEIPT
	// Only perform receipt validation for Mac App Store builds (FIXME: use a separate macro to indicate this?)
	// NOTE: This validation must be performed before any UI is shown (Apple requirement)
	return CheckReceiptAndRun(p_argc, p_argv);
#else
	return NSApplicationMain(p_argc, (const char**)p_argv);
#endif
}
