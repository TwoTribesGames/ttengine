#if defined(TT_PLATFORM_OSX_MAC)
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#endif

#include <tt/app/TTOpenGLContext.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace renderer {

//--------------------------------------------------------------------------------------------------
// Public member functions

UpScaler::UpScaler(void* p_openGLContext, const math::Point2& p_maxRenderTargetSize)
:
m_openGLContext(p_openGLContext),
m_active(false),
m_maxSize(p_maxRenderTargetSize),
m_scaling(1.0f, 1.0f)
{
}


UpScaler::~UpScaler()
{
}


math::Point2 UpScaler::handleResetDevice()
{
	// Get the current screen size
	const math::Point2 currentSize([(TTOpenGLContext*)m_openGLContext getScreenSize]);
	
	TT_ASSERTMSG(currentSize.x > 0 && currentSize.y > 0,
	             "OpenGL Context Screen Size should be bigger than 0. Found %d x %d",
	             currentSize.x, currentSize.y);
	
	// Start with current size
	math::Point2 screenSize(currentSize);
	m_active = false;
	
	math::Point2 targetSize = m_maxSize;
	
	{
		GLint maxTextureSize = 0;
		// FIXME: Should actually check with RenderTarget for MAX_RENDERBUFFER_SIZE*
		// (*If ARB_framebuffer_object is used otherwise the *_EXT version should be used.)
		// But going off the capabilities MAX texture size is always the same.
		// (See: http://developer.apple.com/graphicsimaging/opengl/capabilities/ )
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		
		if (screenSize.x > maxTextureSize && // If screen is too big, and
		    targetSize.x > maxTextureSize)   // current target settings won't clamp it.
		{
			targetSize.x = maxTextureSize; // Make sure we're not getting too big.
		}
		
		if (screenSize.y > maxTextureSize && // If screen is too big, and
		    targetSize.y > maxTextureSize)   // current target settings won't clamp it.
		{
			targetSize.y = maxTextureSize; // Make sure we're not getting too big.
		}
	}
	
	// Check width
	if (targetSize.x > 0 && currentSize.x > targetSize.x)
	{
		// Resolution is larger than max width: create RT
		real aspect(currentSize.x / static_cast<float>(currentSize.y));
		
		// Use max resolution for width + compute needed height
		screenSize.x = targetSize.x;
		screenSize.y = static_cast<s32>(targetSize.x / aspect);
		
		// Unless this results in the max height to be overwritten
		if (targetSize.y > 0 && screenSize.y > targetSize.y)
		{
			// Use max resolution for height + compute needed width
			screenSize.x = static_cast<s32>(targetSize.y * aspect);
			screenSize.y = targetSize.y;
		}
	}
	
	// Check height
	else if (targetSize.y > 0 && currentSize.y > targetSize.y)
	{
		// Resolution is larger than max height: create RT
		real aspect(currentSize.x / static_cast<float>(currentSize.y));
		
		// Use max resolution for height + compute needed width
		screenSize.x = static_cast<s32>(targetSize.y * aspect);
		screenSize.y = targetSize.y;
		
		// Unless this results in the max width to be overwritten
		if (targetSize.x > 0 && screenSize.x > targetSize.x)
		{
			// Use max resolution for width + compute needed height
			screenSize.x = targetSize.x;
			screenSize.y = static_cast<s32>(targetSize.x / aspect);
		}
	}
	
	// If we modified the screen size, we need a render target
	if (screenSize != currentSize)
	{
		m_active = true;
	}
	
	// Update the back buffer size with the newly calculated size
#if defined(TT_PLATFORM_OSX_MAC)
	
	GLint dim[2] = { static_cast<GLint>(screenSize.x), static_cast<GLint>(screenSize.y) };
	CGLContextObj ctx = (CGLContextObj)[(TTOpenGLContext*)m_openGLContext CGLContextObj];
	CGLSetParameter(ctx, kCGLCPSurfaceBackingSize, dim);
	CGLEnable(ctx, kCGLCESurfaceBackingSize);
	
#elif defined(TT_PLATFORM_OSX_IPHONE)
	
	// Not supported on iPhone (is it?)
	// ...
	
#else
	#error Unsupported platform.
#endif
	
	// Compute scaling factors (<= 1.0, 1.0 = no scaling occurs)
	m_scaling.x = screenSize.x / static_cast<real>(currentSize.x);
	m_scaling.y = screenSize.y / static_cast<real>(currentSize.y);
	
	/*
	TT_Printf("UpScaler::handleResetDevice: Original screen size: %d x %d\n", currentSize.x, currentSize.y);
	TT_Printf("UpScaler::handleResetDevice: Scaled screen size: %d x %d\n", screenSize.x, screenSize.y);
	TT_Printf("UpScaler::handleResetDevice: Scale factors: (%f, %f)\n", m_scaling.x, m_scaling.y);
	//*/
	
	return screenSize;
}


void UpScaler::beginFrame()
{
	// Nothing to do here in OS X implementation
}


void UpScaler::endFrame()
{
	// Nothing to do here in OS X implementation
}

// Namespace end
}
}
}
