#if !defined(INC_TT_ENGINE_RENDERER_PP_RENDERTARGET_H)
#define INC_TT_ENGINE_RENDERER_PP_RENDERTARGET_H

#include <list>

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/pp/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/opengl_headers.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {


enum ClearFlag
{
	ClearFlag_DontClear   = 0x0,
	ClearFlag_ColorBuffer = 0x1,
	ClearFlag_DepthBuffer = 0x2,
	
	ClearFlag_All = ClearFlag_ColorBuffer|ClearFlag_DepthBuffer
};



class RenderTarget
{
public:
	enum FramebufferObject
	{
		FramebufferObject_Draw,
		FramebufferObject_Read,
		FramebufferObject_Framebuffer
	};
	~RenderTarget();
	
	inline void setClearColor(const ColorRGBA& p_color) { m_clearColor = p_color.normalized(); }
	
	void deviceReset();
	
	const TexturePtr& getTexture();
	
	static RenderTargetPtr create(s32  p_width,
	                              s32  p_height,
	                              s32  p_samples = 0,
	                              bool p_hasDepthBuffer = false);
	
	static RenderTargetPtr createFromBackBuffer(bool p_hasDepthBuffer = false,
	                                            s32  p_samples = 0,
	                                            real p_ratio = 1.0f);
	
	static void onDeviceReset();
	
	static bool hardwareSupportsFramebuffers();
	static inline bool hasExtensionFBO()     { TT_ASSERT(ms_extensionValuesValid); return ms_extensionFBO;     }
	static inline bool hasExtensionPDS()     { TT_ASSERT(ms_extensionValuesValid); return ms_extensionPDS;     }
	static inline bool hadExtensionARB_FBO() { TT_ASSERT(ms_extensionValuesValid); return ms_extensionARB_FBO; }
	
	static inline void bindFramebuffer(GLint p_framebufferObject)
	{
#if TT_OPENGLES_VERSION != 1
		if (hadExtensionARB_FBO())
		{
			glBindFramebuffer(   GL_FRAMEBUFFER,     p_framebufferObject);
		}
		else
		{
			TT_ASSERT(hasExtensionFBO());
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, p_framebufferObject);
		}
#else
		(void)p_framebufferObject;
#endif
	}
	
	static void checkFramebufferStatus(FramebufferObject p_target = FramebufferObject_Framebuffer);
	static const char* getFramebufferErrorAsString(   GLenum p_error);
	static const char* getFramebufferErrorAsStringEXT(GLenum p_error);
	
private:
	RenderTarget(s32 p_width, s32 p_height, bool p_fromBackBuffer, bool p_hasDepthBuffer, s32 p_samples, real p_ratio);
	
	friend class RenderTargetStack;
	void setActive(u32 p_clearFlags = ClearFlag_All);
	inline void setInactive() {}
	
	void createFrameBuffers();
	void createBuffersForAA();
	void attachDepthBuffer();
	
	static void registerRenderTarget(RenderTarget* p_rt);
	static void unregisterRenderTarget(RenderTarget* p_rt);
	
	// No copying
	RenderTarget(const RenderTarget&);
	RenderTarget& operator=(const RenderTarget&);
	
	
	typedef std::list<RenderTarget*> RenderTargetList; // List to keep order of registration.
	static RenderTargetList sm_RTs;
	
	math::Vector4 m_clearColor;
	s32           m_width;
	s32           m_height;
	s32           m_samples;
	bool          m_hasDepthBuffer;
	bool          m_backBufferSize;
	real          m_ratioToBackBuffer;
	
	GLuint m_fbo;
	GLuint m_fboAA;
	GLuint m_colorBuffer;
	GLuint m_depthBuffer;
	GLuint m_colorBufferAA;
	GLuint m_depthBufferAA;
	
	TexturePtr m_texture;
	
	static bool ms_extensionValuesValid;
	static bool ms_extensionFBO;  // GL_EXT_framebuffer_object
	static bool ms_extensionPDS;  // GL_EXT_packed_depth_stencil
	
	static bool ms_extensionARB_FBO; // GL_ARB_framebuffer_object. (Reflects OpenGL 3.0 specs, 
	                                 // Intergrates: EXT_framebuffer_object, EXT_framebuffer_blit,
	                                 //              EXT_framebuffer_multisample, EXT_packed_depth_stencil.
	                                 // Needed to support AA framebuffers.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_PP_RENDERTARGET_H)
