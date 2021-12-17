#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iPhone builds only

#include <tt/app/TTOpenGLContext_iphone.h>
#include <tt/engine/opengl_headers.h>


@implementation TTOpenGLContext

- (void)setFrameBufferID:(GLuint)p_frameBuffer
{
	m_viewFramebuffer = p_frameBuffer;
}


- (void)setRenderBufferID:(GLuint)p_renderBuffer
{
	m_viewRenderbuffer = p_renderBuffer;
}


- (tt::math::Point2)getScreenSize
{
	//glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_viewFramebuffer);
	//glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_viewRenderbuffer);
	
	tt::math::Point2 screenSize(0, 0);
	GLint val = 0;
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &val);
	screenSize.x = static_cast<s32>(val);
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &val);
	screenSize.y = static_cast<s32>(val);
	
	return screenSize;
}


- (void)flushBuffer
{
	//glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_viewRenderbuffer);
	[self presentRenderbuffer:GL_RENDERBUFFER_OES];
}


- (void)makeCurrentContext
{
	[EAGLContext setCurrentContext:self];
}


- (void)dealloc
{
	if ([EAGLContext currentContext] == self)
	{
		[EAGLContext setCurrentContext:0];
	}
	
	[super dealloc];
}

@end


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
