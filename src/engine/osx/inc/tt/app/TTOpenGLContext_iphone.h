#if !defined(INC_TT_APP_TTOPENGLCONTEXT_IPHONE_H)
#define INC_TT_APP_TTOPENGLCONTEXT_IPHONE_H

#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iPhone builds only


#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

#include <tt/math/Point2.h>


@interface TTOpenGLContext : EAGLContext
{
@private
	GLuint m_viewFramebuffer;
	GLuint m_viewRenderbuffer;
}

- (void)setFrameBufferID:(GLuint)p_frameBuffer;
- (void)setRenderBufferID:(GLuint)p_renderBuffer;

- (tt::math::Point2)getScreenSize;
- (void)flushBuffer;
- (void)makeCurrentContext;

@end


#endif  // defined(TT_PLATFORM_OSX_IPHONE)

#endif  // !defined(INC_TT_APP_TTOPENGLCONTEXT_IPHONE_H)
