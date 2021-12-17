#include <cstdio>
#include <algorithm>

#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {
	

RenderTarget::RenderTargetList RenderTarget::sm_RTs;

bool RenderTarget::ms_extensionValuesValid = false;
bool RenderTarget::ms_extensionFBO         = false;
bool RenderTarget::ms_extensionPDS         = false;
bool RenderTarget::ms_extensionARB_FBO     = false;


//--------------------------------------------------------------------------------------------------
// Public member functions


const TexturePtr& RenderTarget::getTexture()
{
	// If we are using an AA render target, copy the contents first
	if (m_fboAA != 0)
	{
#if defined(TT_PLATFORM_OSX_IPHONE)
		TT_PANIC("AA Not supported on iOS");
#else
		TT_ASSERT(hadExtensionARB_FBO());
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboAA);
		checkFramebufferStatus(FramebufferObject_Read);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		checkFramebufferStatus(FramebufferObject_Draw);
		
		TT_ASSERT(m_fboAA != 0);
		TT_ASSERT(m_fbo   != 0);
		TT_ASSERT(m_width > 0 && m_height > 0);
		
		glBlitFramebuffer(0, 0, m_width, m_height,
						  0, 0, m_width, m_height,
						  GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		TT_CHECK_OPENGL_ERROR();
#endif
	}

	return m_texture;
}


void RenderTarget::deviceReset()
{
	// Should be called on window resize
	
	// Get actual backbuffer size -> Render Target size is automatically the same as backbuffer
	if(m_backBufferSize)
	{
		m_width  = static_cast<s32>(m_ratioToBackBuffer * Renderer::getInstance()->getScreenWidth());
		m_height = static_cast<s32>(m_ratioToBackBuffer * Renderer::getInstance()->getScreenHeight());
		
		createFrameBuffers();
	}
}


RenderTargetPtr RenderTarget::createFromBackBuffer(bool p_hasDepthBuffer, s32 p_samples, real p_ratio)
{
	// Create a render target that is the same size as the back buffer
	return RenderTargetPtr(new RenderTarget(
		static_cast<s32>(p_ratio * Renderer::getInstance()->getScreenWidth()),
		static_cast<s32>(p_ratio * Renderer::getInstance()->getScreenHeight()),
		true, p_hasDepthBuffer, p_samples, p_ratio));
}


RenderTargetPtr RenderTarget::create(s32 p_width, s32 p_height, s32 p_samples, bool p_hasDepthBuffer)
{
	// Create a render target with the requested size
	return RenderTargetPtr(new RenderTarget(p_width, p_height, false, p_hasDepthBuffer, p_samples, 1.0f));
}


void RenderTarget::onDeviceReset()
{
	for (RenderTargetList::iterator it = sm_RTs.begin(); it != sm_RTs.end(); ++it)
	{
		(*it)->deviceReset();
	}
}


bool RenderTarget::hardwareSupportsFramebuffers()
{
	/*
	// We're targeting OpenGL 2.1 so no need to check for version 3 because we won't be using it.
	const char* glVersion = reinterpret_cast<const char*> (glGetString(GL_VERSION));
	
	int major(0);
	int minor(0);
	
	if(std::strlen(glVersion) > 0)
	{
		std::sscanf(glVersion, "%d.%d", &major, &minor);
	}
	
	const bool hasFramebufferObjectInCore = major >= 3;
	*/
	if (ms_extensionValuesValid == false)
	{
#ifdef POINTER_C_GENERATED_HEADER_OPENGL_H
		ms_extensionFBO = ogl_ext_EXT_framebuffer_object;
		ms_extensionPDS = ogl_ext_EXT_packed_depth_stencil;
		ms_extensionARB_FBO = ogl_ext_ARB_framebuffer_object;
#else
		const char* extensions = reinterpret_cast<const char*>( glGetString(GL_EXTENSIONS) );
		ms_extensionFBO     = ( strstr( extensions, "GL_EXT_framebuffer_object"      ) != 0 );
		ms_extensionPDS     = ( strstr( extensions, "GL_EXT_packed_depth_stencil"    ) != 0 );
		ms_extensionARB_FBO = ( strstr( extensions, "GL_ARB_framebuffer_object"      ) != 0 );
#endif
		ms_extensionValuesValid = true;
		
		TT_Printf("RenderTarget::hardwareSupportsFramebuffers - "
		          "extensions FBObject: %d, depth stencil: %d, ARB FBO: %d.\n",
		          ms_extensionFBO, ms_extensionPDS, ms_extensionARB_FBO);
	}
	
	return (ms_extensionFBO && ms_extensionPDS) || ms_extensionARB_FBO;
}


void RenderTarget::checkFramebufferStatus(FramebufferObject p_target)
{
#if TT_OPENGLES_VERSION != 1
	if (hadExtensionARB_FBO())
	{
		GLenum target = GL_FRAMEBUFFER;
		switch (p_target)
		{
		case FramebufferObject_Draw:
			target = GL_DRAW_FRAMEBUFFER;
			break;
		case FramebufferObject_Read:
			target = GL_READ_FRAMEBUFFER;
			break;
		case FramebufferObject_Framebuffer:
			target = GL_FRAMEBUFFER;
			break;
		}
		
		GLenum status = glCheckFramebufferStatus(target);
		TT_ASSERTMSG(status == GL_FRAMEBUFFER_COMPLETE,
					 "Framebuffer status not complete: %d (0x%X) '%s'",
					 status, status, getFramebufferErrorAsString(status));
	}
	else
	{
		GLenum target = GL_FRAMEBUFFER_EXT;
		switch (p_target)
		{
		case FramebufferObject_Draw:
			target = GL_DRAW_FRAMEBUFFER_EXT;
			break;
		case FramebufferObject_Read:
			target = GL_READ_FRAMEBUFFER_EXT;
			break;
		case FramebufferObject_Framebuffer:
			target = GL_FRAMEBUFFER_EXT;
			break;
		}
		
		GLenum status = glCheckFramebufferStatusEXT(target);
		TT_ASSERTMSG(status == GL_FRAMEBUFFER_COMPLETE_EXT,
					 "Framebuffer status not complete: %d (0x%X) '%s'",
					 status, status, getFramebufferErrorAsStringEXT(status));
	}
#else
	(void)p_target;
#endif
}


const char* RenderTarget::getFramebufferErrorAsString(GLenum p_error)
{
#if TT_OPENGLES_VERSION != 1
	switch(p_error)
	{
	case GL_FRAMEBUFFER_COMPLETE:                      return "GL_FRAMEBUFFER_COMPLETE";                      // if the framebuffer bound to target is complete.
	// Error status:
	case GL_FRAMEBUFFER_UNDEFINED:                     return "GL_FRAMEBUFFER_UNDEFINED";                     // is returned if target is the default framebuffer, but the default framebuffer does not exist.
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";         // is returned if any of the framebuffer attachment points are framebuffer incomplete.
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; // is returned if the framebuffer does not have at least one image attached to it.
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";        // is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAWBUFFERi.
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";        // is returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.
	case GL_FRAMEBUFFER_UNSUPPORTED:                   return "GL_FRAMEBUFFER_UNSUPPORTED";                   // is returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";        // is returned if the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES.
	                                                                                                          // is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.
	//case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";      // is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.
	}
#endif
	TT_PANIC("Unknown Framebuffer error: %d, 0x%x.", p_error, p_error);
	return "<unknown>";
}


const char* RenderTarget::getFramebufferErrorAsStringEXT(GLenum p_error)
{
#if TT_OPENGLES_VERSION != 1
	switch(p_error)
	{
	case GL_FRAMEBUFFER_COMPLETE_EXT:                      return "GL_FRAMEBUFFER_COMPLETE_EXT";
	// Error status:
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:         return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:         return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:            return "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:        return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:        return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT";
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:                   return "GL_FRAMEBUFFER_UNSUPPORTED_EXT";
	}
#endif
	TT_PANIC("Unknown Framebuffer error: %d, 0x%x.", p_error, p_error);
	return "<unknown>";
}


//--------------------------------------------------------------------------------------------------
// Private member functions

RenderTarget::RenderTarget(s32 p_width, s32 p_height, bool p_fromBackBuffer,
                           bool p_hasDepthBuffer, s32 p_samples, real p_ratio)
:
m_clearColor(0,0,0,0),
m_width(p_width),
m_height(p_height),
m_samples(p_samples),
m_hasDepthBuffer(p_hasDepthBuffer),
m_backBufferSize(p_fromBackBuffer),
m_ratioToBackBuffer(p_ratio),
m_fbo(0),
m_fboAA(0),
m_colorBuffer(0),
m_depthBuffer(0),
m_colorBufferAA(0),
m_depthBufferAA(0),
m_texture()
{
	createFrameBuffers();
	
	registerRenderTarget(this);
}


RenderTarget::~RenderTarget()
{
#if TT_OPENGLES_VERSION != 1
	// Make sure these are created before deleting them,
	// because if they are not created the platform might not support them.
	// (and the delete will fail even on a null!)
	if (m_fbo != 0)
	{
		if (hadExtensionARB_FBO())
		{
			glDeleteFramebuffers(   1, &m_fbo);
		}
		else
		{
			TT_ASSERT(hasExtensionFBO());
			glDeleteFramebuffersEXT(1, &m_fbo);
		}
	}
	if(m_depthBuffer != 0)
	{
		if (hadExtensionARB_FBO())
		{
			glDeleteRenderbuffers(   1, &m_depthBuffer);
		}
		else
		{
			TT_ASSERT(hasExtensionFBO());
			glDeleteRenderbuffersEXT(1, &m_depthBuffer);
		}
	}
	if(m_fboAA != 0)
	{
		TT_ASSERT(hadExtensionARB_FBO());
		glDeleteFramebuffers( 1, &m_fboAA);
		glDeleteRenderbuffers(1, &m_colorBufferAA);
		glDeleteRenderbuffers(1, &m_depthBufferAA);
	}	
#endif
	
	unregisterRenderTarget(this);
}
	
	void RenderTarget::setActive(u32 p_clearFlags)
	{
#if TT_OPENGLES_VERSION != 1
		GLuint fbo = (m_fboAA == 0) ? m_fbo : m_fboAA;
		TT_NULL_ASSERT(fbo);
		
		bindFramebuffer(fbo);
		glViewport(0,0, m_width, m_height);
		
		if (p_clearFlags != ClearFlag_DontClear)
		{
			GLbitfield clearMask(0);
			
			if ((p_clearFlags & ClearFlag_ColorBuffer) == ClearFlag_ColorBuffer)
			{
				clearMask |= GL_COLOR_BUFFER_BIT;
			}
			
			if (m_hasDepthBuffer && (p_clearFlags & ClearFlag_DepthBuffer) == ClearFlag_DepthBuffer)
			{
				clearMask |= GL_DEPTH_BUFFER_BIT;
			}
			auto renderer = Renderer::getInstance();
            const u32 colorMask = renderer->getColorMask();
			renderer->setColorMask(ColorMask_All);
			glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
			glClear(clearMask);
			renderer->setColorMask(colorMask);
		}
		
		checkFramebufferStatus();
		
		TT_CHECK_OPENGL_ERROR();
#endif
	}


void RenderTarget::createFrameBuffers()
{
#if TT_OPENGLES_VERSION != 1
	if (m_samples == 1)
	{
		m_samples = 0;
	}
	
	if(m_samples > 0)
	{
		createBuffersForAA();
	}
	
	if(m_fbo == 0)
	{
		if (hadExtensionARB_FBO())
		{
			glGenFramebuffers(   1, &m_fbo);
		}
		else
		{
			TT_ASSERT(hasExtensionFBO());
			glGenFramebuffersEXT(1, &m_fbo);
		}
	}
	m_texture = Texture::createForRenderTarget(m_width, m_height);
	m_texture->select();
	m_texture->setAddressMode(AddressMode_Clamp, AddressMode_Clamp);
	m_texture->setFilterMode(FilterMode_Linear, FilterMode_Linear);
	
	bindFramebuffer(m_fbo);
	if (hadExtensionARB_FBO())
	{
		glFramebufferTexture2D(   GL_FRAMEBUFFER,     GL_COLOR_ATTACHMENT0,    GL_TEXTURE_2D, m_texture->getGLName(), 0);
	}
	else
	{
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->getGLName(), 0);
	}
	
	if(m_hasDepthBuffer)
	{
		attachDepthBuffer();
	}
	
	checkFramebufferStatus();
	TT_CHECK_OPENGL_ERROR();
	
	bindFramebuffer(0);
	
	checkFramebufferStatus();
#endif
}

	
void RenderTarget::createBuffersForAA()
{
#if defined(TT_PLATFORM_OSX_IPHONE)
	TT_PANIC("AA Not supported on iOS");
	return;
#else
	GLint maxSamples(0);
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	
	if(maxSamples < 2)
	{
		TT_WARN("Anti-Aliasing not supported by OpenGL / hardware.");
		m_samples = 0;
		return;
	}
	
	if (hadExtensionARB_FBO() == false)
	{
		TT_WARN("OpenGL ARB_framebuffer_object extension not supported by hardware. (Needed for AA RenderTarget.)");
		m_samples = 0;
		return;
	}
	
	if(m_samples > maxSamples)
	{
		TT_WARN("Requested %d AA samples, but only %d supported!", m_samples, maxSamples);
		m_samples = maxSamples;
	}
	
	TT_ASSERT(hadExtensionARB_FBO());
	
	if(m_fboAA == 0)
	{
		glGenFramebuffers(1, &m_fboAA);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboAA);
	
	if(m_colorBufferAA == 0)
	{
		glGenRenderbuffers(1, &m_colorBufferAA);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorBufferAA);
	
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_RGBA8, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorBufferAA);
	
	if(m_depthBufferAA == 0)
	{
		glGenRenderbuffers(1, &m_depthBufferAA);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH_STENCIL, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_RENDERBUFFER, m_depthBufferAA);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferAA);
	
	glBindFramebuffer( GL_FRAMEBUFFER,  0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
#endif
}
	

void RenderTarget::attachDepthBuffer()
{
#if TT_OPENGLES_VERSION != 1
	// NOTE: Frame Buffer must be bound
	
	if(m_depthBuffer == 0)
	{
		if (hadExtensionARB_FBO())
		{
			glGenRenderbuffers(   1, &m_depthBuffer);
		}
		else
		{
			glGenRenderbuffersEXT(1, &m_depthBuffer);
		}
	}
	
	if (hadExtensionARB_FBO())
	{
		glBindRenderbuffer(   GL_RENDERBUFFER    , m_depthBuffer);
	}
	else
	{
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
	}
	
	if (hadExtensionARB_FBO())
	{
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_RENDERBUFFER, m_depthBuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
		
		checkFramebufferStatus();
		
		TT_CHECK_OPENGL_ERROR();
		
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	else
	{
		const GLenum internalformatDepthBuffer = 
#if defined(TT_PLATFORM_OSX_IPHONE)
		// FIXME: This can be replaced by one of these extensions.
		// extension GL_OES_packed_depth_stencil adds DEPTH_STENCIL_OES
		// and extension GL_OES_depth24 adds OES_depth24;
		// See: http://developer.apple.com/library/ios/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/OpenGLESPlatforms/OpenGLESPlatforms.html#//apple_ref/doc/uid/TP40008793-CH106-SW7
		GL_DEPTH_COMPONENT16; // OpenGL ES 1 doesn't support depth component 24 so use 16 instead.
							  // (or use one of the extensions above.)
#else
		GL_DEPTH_STENCIL_EXT; //
		TT_ASSERT(hasExtensionPDS());
#endif
		glRenderbufferStorageEXT(GL_RENDERBUFFER, internalformatDepthBuffer, m_width, m_height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,   GL_RENDERBUFFER_EXT, m_depthBuffer);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);
		
		checkFramebufferStatus();
		
		TT_CHECK_OPENGL_ERROR();
		
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	}
#endif
}


void RenderTarget::registerRenderTarget(RenderTarget* p_rt)
{
	TT_NULL_ASSERT(p_rt);
	sm_RTs.push_back(p_rt);
}


void RenderTarget::unregisterRenderTarget(RenderTarget* p_rt)
{
	TT_NULL_ASSERT(p_rt);
	RenderTargetList::iterator it = std::find(sm_RTs.begin(), sm_RTs.end(), p_rt);
	
	if (it != sm_RTs.end())
	{
		sm_RTs.erase(it);
	}
}


// Namespace end
}
}
}

