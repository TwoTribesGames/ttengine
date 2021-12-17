#if !defined(INC_TT_APP_TTOPENGLCONTEXT_DESKTOP_H)
#define INC_TT_APP_TTOPENGLCONTEXT_DESKTOP_H

#if defined(TT_PLATFORM_OSX_MAC) // this file is for desktop (non-iPhone) builds only


#import <Cocoa/Cocoa.h>

#include <tt/math/Point2.h>


@interface TTOpenGLContext : NSOpenGLContext
{
}

- (tt::math::Point2)getScreenSize;

@end


#endif  // defined(TT_PLATFORM_OSX_MAC)

#endif  // !defined(INC_TT_APP_TTOPENGLCONTEXT_DESKTOP_H)
