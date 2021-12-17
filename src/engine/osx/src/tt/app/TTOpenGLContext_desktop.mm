#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iPhone) builds only

#include <tt/app/TTOpenGLContext_desktop.h>


@implementation TTOpenGLContext

- (tt::math::Point2)getScreenSize
{
	NSRect windowRect = [[self view] bounds];
	
	return tt::math::Point2(
		static_cast<s32>(windowRect.size.width),
		static_cast<s32>(windowRect.size.height));
}

@end

#endif  // defined(TT_PLATFORM_OSX_MAC)
