#include <tt/engine/animation/Animation.h>
#include <tt/engine/debug/DebugFont.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/FullscreenTriangle.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/OpenGLContextWrapper.h>
#include "tt/engine/renderer/GLStateCache.h"
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/SubModel.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/scene/Model.h>
#include <tt/engine/scene/Scene.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


#if !defined(TT_BUILD_FINAL)
// Prints OpenGL's texture stage parameters when defined, in non-final builds
//#define DEBUG_TEXTURESTAGES
#endif


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Lookup table to map BlendFactor enum to OpenGL blend factor enum values. */
const GLenum g_blendFactors[] =
{
	GL_ZERO,                // BlendFactor_Zero
	GL_ONE,                 // BlendFactor_One
	GL_SRC_COLOR,           // BlendFactor_SrcColor
	GL_ONE_MINUS_SRC_COLOR, // BlendFactor_InvSrcColor
	GL_SRC_ALPHA,           // BlendFactor_SrcAlpha
	GL_ONE_MINUS_SRC_ALPHA, // BlendFactor_InvSrcAlpha
	GL_DST_ALPHA,           // BlendFactor_DstAlpha
	GL_ONE_MINUS_DST_ALPHA, // BlendFactor_InvDstAlpha
	GL_DST_COLOR,           // BlendFactor_DstColor
	GL_ONE_MINUS_DST_COLOR  // BlendFactor_InvDstColor
};
TT_STATIC_ASSERT(sizeof(g_blendFactors) / sizeof(GLenum) == BlendFactor_Count);

/*! \brief Lookup table to map AlphaTestFunction enum to OpenGL alpha test enum values. */
const GLenum g_alphaTestFunctions[] =
{
	GL_NEVER,    // AlphaTestFunction_Never
	GL_LESS,     // AlphaTestFunction_Less
	GL_EQUAL,    // AlphaTestFunction_Equal
	GL_LEQUAL,   // AlphaTestFunction_LessEqual
	GL_GREATER,  // AlphaTestFunction_Greater
	GL_NOTEQUAL, // AlphaTestFunction_NotEqual
	GL_GEQUAL,   // AlphaTestFunction_GreaterEqual
	GL_ALWAYS,   // AlphaTestFunction_Always
};
TT_STATIC_ASSERT(sizeof(g_alphaTestFunctions) / sizeof(GLenum) == AlphaTestFunction_Count);

const GLenum cullModeLookup[] =
{
	GL_FRONT,     // CullMode_Front
	GL_BACK       // CullMode_Back
};
TT_STATIC_ASSERT(sizeof(cullModeLookup) / sizeof(GLenum) == CullMode_Count);

const GLenum cullFrontOrderLookup[] =
{
	GL_CW,        // CullFrontOrder_ClockWise
	GL_CCW        // CullFrontOrder_CounterClockWise
};
TT_STATIC_ASSERT(sizeof(cullFrontOrderLookup) / sizeof(GLenum) == CullFrontOrder_Count);

/*! \brief Lookup table matching shared enum TextureBlendOperation. */
const GLenum blendOp[] =
{
	GL_NEVER,       // TextureBlendOperation_Disable     - NOT DIRECTLY SUPPORTED -
	
	GL_NEVER,       // TextureBlendOperation_SelectArg1  - NOT DIRECTLY SUPPORTED -
	GL_NEVER,       // TextureBlendOperation_SelectArg2  - NOT DIRECTLY SUPPORTED -
	
	GL_MODULATE,    // TextureBlendOperation_Modulate
	GL_ADD,         // TextureBlendOperation_Add
	GL_SUBTRACT,    // TextureBlendOperation_Substract
	GL_MODULATE,    // TextureBlendOperation_Modulate2X - NOT DIRECTLY SUPPORTED -
	GL_DECAL        // TextureBlendOperation_Decal
};
TT_STATIC_ASSERT(sizeof(blendOp) / sizeof(GLenum) == TextureBlendOperation_Count);

/*! \brief Lookup table matching shared enum TextureBlendSource. */
const GLenum blendSrc[] =
{
	GL_TEXTURE,          // TextureBlendSource_Texture
	GL_PRIMARY_COLOR,    // TextureBlendSource_Diffuse
	GL_PREVIOUS,         // TextureBlendSource_Previous
	GL_CONSTANT,         // TextureBlendSource_Constant
	
	GL_TEXTURE,          // TextureBlendSource_OneMinusTexture
	GL_PRIMARY_COLOR,    // TextureBlendSource_OneMinusDiffuse
	GL_PREVIOUS,         // TextureBlendSource_OneMinusPrevious
	GL_CONSTANT,         // TextureBlendSource_OneMinusConstant
};
TT_STATIC_ASSERT(sizeof(blendSrc) / sizeof(GLenum) == TextureBlendSource_Count);

const GLenum stencilFunc[] =
{
	GL_NEVER   , //<! StencilTestFunction_Never
	GL_LESS    , //<! StencilTestFunction_Less
	GL_EQUAL   , //<! StencilTestFunction_Equal
	GL_LEQUAL  , //<! StencilTestFunction_LessEqual
	GL_GREATER , //<! StencilTestFunction_Greater
	GL_NOTEQUAL, //<! StencilTestFunction_NotEqual
	GL_GEQUAL  , //<! StencilTestFunction_GreaterEqual
	GL_ALWAYS    //<! StencilTestFunction_Always
};
TT_STATIC_ASSERT(sizeof(stencilFunc) / sizeof(u32) == StencilTestFunction_Count);


const GLenum stencilOp[] =
{
	GL_KEEP,      //<! StencilOperation_Keep
	GL_ZERO,      //<! StencilOperation_Zero
	GL_REPLACE,   //<! StencilOperation_Replace
	GL_INCR,      //<! StencilOperation_Increment
	GL_INCR_WRAP, //<! StencilOperation_IncrementWrap
	GL_DECR,      //<! StencilOperation_Decrement
	GL_DECR_WRAP, //<! StencilOperation_DecrementWrap
	GL_INVERT     //<! StencilOperation_Invert
};
TT_STATIC_ASSERT(sizeof(stencilOp) / sizeof(u32) == StencilOperation_Count);

Renderer* Renderer::ms_instance = 0;


//--------------------------------------------------------------------------------------------------
// Public member functions

bool Renderer::createInstance(OpenGLContextWrapper* p_openGLContext, bool p_ios2xMode)
{
	if (ms_instance != 0)
	{
		TT_PANIC("Renderer instance has already been created.");
		return false;
	}
	
	new Renderer(p_openGLContext, p_ios2xMode);
	if (ms_instance == 0)
	{
		TT_PANIC("Creating Renderer failed.");
		return false;
	}
	
	return true;
}


void Renderer::destroyInstance()
{
	TT_ASSERTMSG(ms_instance != 0, "Renderer instance has already been destroyed.");
	delete ms_instance;
	ms_instance = 0;
}


void Renderer::reset(bool p_disableZ)
{
	// Reset states
	m_isZBufferEnabled = (p_disableZ == false);
	setDefaultStates(p_disableZ);
}


void Renderer::setDefaultStates(bool p_disableZ)
{
	// Identity matrix for modelview
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Enable the rendering of a vertex array
	m_stateCache->setClientState(GL_VERTEX_ARRAY, true);

	// Set vertex type to texture by default
	setVertexType(VertexBuffer::Property_Texture0);
	
	// A front faced face is defined in clockwise order
	setCullFrontOrder(CullFrontOrder_ClockWise);
	// Cull back faced faces
	setCullMode(CullMode_Back);
	// Disable culling
	setCullingEnabled(false);
	// Force update
	updateCulling();
	
	if (p_disableZ)
	{
		m_stateCache->setState(GL_DEPTH_TEST, false);
	}
	else
	{
#ifdef TT_PLATFORM_OSX_IPHONE
		TT_WARN("Depth test isn't enabled for the iPhone.");
#else
		m_stateCache->setState(GL_DEPTH_TEST, true);
#endif
	}
	
	setDepthWriteEnabled(true);
	
	// Disable lighting
	setLighting(false);
	
	// Set default states for lighting
	FixedFunction::setAmbientLightColor(ColorRGB::gray);
	
#if TT_OPENGLES_VERSION != 1
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
	glShadeModel(GL_SMOOTH);
	// FIXME: Code copied from HWLight::getMaxLights (which has been deleted). How should this be updated?
	GLint result = 0;
	glGetIntegerv(GL_MAX_LIGHTS, &result);
	TT_CHECK_OPENGL_ERROR();
	for (s32 i = 0; i < static_cast<s32>(result); ++i)
	{
		m_stateCache->setState(GL_LIGHT0 + i, false);
	}
	getLightManager().reset();
	
	// Enable blending
	m_stateCache->setState(GL_BLEND, true);

	// Set the blend mode
	m_srcFactor = BlendFactor_One;
	m_dstFactor = BlendFactor_Zero; // force update of blend mode by setting it to something else first
	setBlendMode(BlendMode_Blend);
	
	setAlphaTestEnabled(false);
	
	// Reset fog
	resetFog();
	
	setColorMask(ColorMask_All);
	
	// Reset potential modified multitexture stages
	resetTextureStages(0, MultiTexture::getChannelCount());
	m_multiTextureDirtyStageRange = 0;
	// Reset texture for stage 0
	setTexture(TexturePtr());
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_MODULATE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	TT_CHECK_OPENGL_ERROR();
}


void Renderer::setColorMask(u32 p_colorMask)
{
	if (m_colorMask != p_colorMask)
	{
		glColorMask(
			(p_colorMask & ColorMask_Red  ) != 0 ? GL_TRUE : GL_FALSE,
			(p_colorMask & ColorMask_Green) != 0 ? GL_TRUE : GL_FALSE,
			(p_colorMask & ColorMask_Blue ) != 0 ? GL_TRUE : GL_FALSE,
			(p_colorMask & ColorMask_Alpha) != 0 ? GL_TRUE : GL_FALSE);
		m_colorMask = p_colorMask;
	}
}


/*! \brief Updates the engine by a given amount of time.
    \param p_elapsedTime The number of seconds to update the engine. */
void Renderer::update(real p_elapsedTime)
{
	// FIXME: remove hardcoded value and prevent accumulated time errors due to loading etc.
	TT_ASSERT(p_elapsedTime >= 0.0f);
	if (p_elapsedTime > 0.5f)
	{
		p_elapsedTime = 0.0f;
	}
	
	m_deltaTime = p_elapsedTime;
	
#if !defined(TT_BUILD_FINAL)
	m_deltaTime *= m_debugRenderer->getSpeed();
	
	/* No debug camera (yet) on OS X
	if (m_debugRenderer->getDebugCameraActive())
	{
		m_debugRenderer->getDebugCamera()->update(p_elapsedTime);
	}
	*/
#endif
	
	// Update the clock
	m_time += p_elapsedTime;
	
	// Update skybox
	if (m_skybox != 0) m_skybox->update();
}


bool Renderer::beginFrame()
{
	m_context->setActive();
	
	// Reset matrix stack etc.
	MatrixStack::getInstance()->resetPositionMatrix();
	TT_CHECK_OPENGL_ERROR();

	if (m_upScaler->isActive())
	{
		glViewport(0, 0, m_backBufferSize.x, m_backBufferSize.y);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		m_upScaler->beginFrame();
		m_upScalerActive = true;
	}
	else if (m_postProcessor->isActive() == false)
	{
		// Clear the entire render target and the Z buffer
		// glClear is timeconsuming on the iPhone
		glClearColor(m_glClearColor.x, m_glClearColor.y, m_glClearColor.z, m_glClearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	// Start rendering
	m_isRendering     = true;
	m_currentViewport = ViewPortID_1;
	
	// Process debug stuff
	m_debugRenderer->beginFrame();
	m_vertexType = 0;
	
	TT_CHECK_OPENGL_ERROR();
	
	return true;
}


bool Renderer::endFrame()
{
	TT_CHECK_OPENGL_ERROR();
	
	// Handle upscaling
	m_upScalerActive = false;
	m_upScaler->endFrame();
	
	// Process debug stuff
	m_debugRenderer->endFrame();
	
	// End rendering
	m_isRendering = false;
	
	TT_CHECK_OPENGL_ERROR();
	
	return true;
}


bool Renderer::present()
{
	// present() should not be called between beginFrame() and endFrame()
	TT_WARNING(m_isRenderingHud == false, "Present called during a begin/endHud pair.");
	if (m_isRendering)
	{
		TT_WARN("Present called during a begin/endFrame pair - ignoring this call.");
		return false;
	}
	
	// Present the renderbuffer
	m_context->swapBuffers();
	
	return true;
}

void Renderer::pushMarker(const char* marker)
{
#if !defined(TT_BUILD_FINAL)
	if (ogl_ext_KHR_debug)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, marker);
	}
#endif
}

void Renderer::popMarker()
{
#if !defined(TT_BUILD_FINAL)
	if (ogl_ext_KHR_debug)
	{
		glPopDebugGroup();
	}
#endif
}

static const char* getOpenGLErrorName(GLenum p_errorCode)
{
	switch (p_errorCode)
	{
		case GL_INVALID_ENUM:                  return "INVALID_ENUM";
		case GL_INVALID_VALUE:                 return "INVALID_VALUE";
		case GL_INVALID_OPERATION:             return "INVALID_OPERATION";
		case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY";
#if TT_OPENGLES_VERSION != 1
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
#endif
	}
	return "Unknown error";
}

void Renderer::checkGLError()
{
	GLenum errorCode = glGetError();
	TT_ASSERTMSG(errorCode == GL_NO_ERROR, "OpenGL has an error! Code %d (0x%X, %s)", errorCode, errorCode, getOpenGLErrorName(errorCode));
}

void Renderer::beginHud()
{
	TT_ASSERTMSG(m_isRenderingHud == false, "Already rendering HUD (beginHud already called).");
	TT_CHECK_OPENGL_ERROR();
	
	// Render all transparent objects
	renderTransparents();
	
	setLights(false, true);
	setCullingEnabled(false);
	
	// Setup HUD camera
	m_activeCamera = m_hudCamera;
	m_activeCamera->select();
	
	// Disable Z test
	if (m_isZBufferEnabled)
	{
		m_stateCache->setState(GL_DEPTH_TEST, false);
	}
	
	// Disable AA
	if(m_useAA)
	{
		m_stateCache->setState(GL_MULTISAMPLE, false);
	}
	
	m_isRenderingHud = true;
}


void Renderer::endHud()
{
	TT_ASSERTMSG(m_isRenderingHud, "endHud called without matching beginHud.");
	
	m_debugRenderer->showSafeFrame();
	
	// Restore viewport camera
	m_activeCamera = ViewPort::getViewPorts().at(m_currentViewport).getCamera();
	m_activeCamera->select();
	
	// Re-enable Z-buffering
	if (m_isZBufferEnabled)
	{
		m_stateCache->setState(GL_DEPTH_TEST, true);
		// Because of rendering issues on the iPhone depth testing is not turned on.
	}
	
	// Restore AA
	if(m_useAA)
	{
		m_stateCache->setState(GL_MULTISAMPLE, true);
	}	
	
	m_isRenderingHud = false;
}


void Renderer::setClearColor(const ColorRGBA& p_color)
{
#ifndef TT_BUILD_FINAL
	if(m_debugRenderer != 0 && m_debugRenderer->isOverdrawDebugActive())
	{
		// Keep clearcolor
		return;
	}
#endif
	if (p_color != m_clearColor)
	{
		// Set new clear color
		m_glClearColor = p_color.normalized();
		m_clearColor   = p_color;
		m_postProcessor->handleClearColorChanged(p_color);
	}
}


void Renderer::setColor(const ColorRGB& p_color)
{
	m_stateCache->setColor(
		(GLfloat)p_color.r / 255.0f,
		(GLfloat)p_color.g / 255.0f,
		(GLfloat)p_color.b / 255.0f,
		1.0f);
}


void Renderer::setVertexType(u32 p_vertexType, bool p_useFixedFunction)
{
	TT_ASSERT(p_vertexType != 0);
	
	if (p_useFixedFunction)
	{
		if (m_vertexType == p_vertexType)
		{
			return;
		}
		
		// Caching of vertex
		m_vertexType = p_vertexType;
	}
	else
	{
		m_vertexType = 0;
	}
	
	bool hasNormals((p_vertexType & VertexBuffer::Property_Normal) != 0);
	bool hasVtxColors((p_vertexType & VertexBuffer::Property_Diffuse) != 0);

	m_stateCache->setClientState(GL_NORMAL_ARRAY, hasNormals && m_lightingEnabled);

	if (hasVtxColors)
	{
		m_stateCache->setClientState(GL_COLOR_ARRAY, true);
	}
	else
	{
		m_stateCache->setClientState(GL_COLOR_ARRAY, false);
		m_stateCache->setColor(1, 1, 1, 1);
	}
	
	// NOTE: Assume that in multitexturing channel 0 is always used
	//       The client state for texture coordinates of channels > 0 needs
	//       to be set by FixedFunction::TextureStage() because we do not know here which channels are used
	
	if (p_vertexType & VertexBuffer::PropertyTextureMask)
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		setTexture(TexturePtr());
	}
	
	if (p_useFixedFunction)
	{
		if (m_lightingEnabled)
		{
			setLights(hasNormals, hasVtxColors);
		}
		FixedFunction::setVertexColorEnabled(hasVtxColors);
	}
	//*/
	
	TT_CHECK_OPENGL_ERROR();
}


void Renderer::setBlendMode(BlendMode p_mode)
{
	switch (p_mode)
	{
	case BlendMode_Blend:         setCustomBlendMode(BlendFactor_SrcAlpha, BlendFactor_InvSrcAlpha); break;
	case BlendMode_Add:           setCustomBlendMode(BlendFactor_SrcAlpha, BlendFactor_One);         break;
	case BlendMode_Modulate:      setCustomBlendMode(BlendFactor_Zero,     BlendFactor_SrcColor);    break;
	case BlendMode_Premultiplied: setCustomBlendMode(BlendFactor_One,      BlendFactor_InvSrcAlpha); break;
		
	default:
		TT_PANIC("Unsupported blend mode: %d", p_mode);
		return;
	}
}


void Renderer::setCustomBlendMode(BlendFactor p_src, BlendFactor p_dst)
{
#ifndef TT_BUILD_FINAL
	if(m_debugRenderer->isOverdrawDebugActive())
	{
		// Keep additive
		return;
	}
#endif
	
	if(p_src != m_srcFactor || p_dst != m_dstFactor)
	{
		m_srcFactor = p_src;
		m_dstFactor = p_dst;
		updateBlendModes();
	}
}


void Renderer::setBlendModeAlpha(BlendModeAlpha p_mode)
{
	switch (p_mode)
	{
	case BlendModeAlpha_NoOverride: break; // Do nothing, just break.
	case BlendModeAlpha_Disabled:  resetCustomBlendModeAlpha();                               break;
	case BlendModeAlpha_Lightmask: setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_One); break;
	default:
		TT_PANIC("Unsupported blend mode %d.", p_mode);
		return;
	}
}


void Renderer::setCustomBlendModeAlpha(BlendFactor p_srcAlpha, BlendFactor p_dstAlpha)
{
#ifndef TT_BUILD_FINAL
	if(m_debugRenderer->isOverdrawDebugActive())
	{
		// Keep additive
		return;
	}
#endif
	
	if (m_separateAlphaBlendEnabled == false ||
	    p_srcAlpha != m_srcFactorAlpha       ||
	    p_dstAlpha != m_dstFactorAlpha)
	{
		m_separateAlphaBlendEnabled = true;
		m_srcFactorAlpha            = p_srcAlpha;
		m_dstFactorAlpha            = p_dstAlpha;
		updateBlendModes();
	}
}


void Renderer::resetCustomBlendModeAlpha()
{
	if (m_separateAlphaBlendEnabled)
	{
		m_separateAlphaBlendEnabled = false;
		m_srcFactorAlpha            = BlendFactor_Zero;
		m_dstFactorAlpha            = BlendFactor_Zero;
		
		updateBlendModes();
	}
}

	
void Renderer::setPremultipliedAlphaEnabled(bool p_enabled)
{
#ifndef TT_BUILD_FINAL
	if(m_debugRenderer->isOverdrawDebugActive())
	{
		// Keep non-premultiplied
		return;
	}
#endif
	
	if(m_premultipliedAlphaEnabled != p_enabled)
	{
		m_premultipliedAlphaEnabled = p_enabled;
		FixedFunction::setPremultipliedAlpha(m_premultipliedAlphaEnabled);
		updateBlendModes();
	}
}
	

void Renderer::setAlphaTestEnabled(bool p_enableAlphaTest)
{
	m_alphaTestEnabled = p_enableAlphaTest;

	m_stateCache->setState(GL_ALPHA_TEST, m_alphaTestEnabled);
}


void Renderer::setAlphaTestFunction(AlphaTestFunction p_alphaTestFunction)
{
	if (isValidAlphaTestFunction(p_alphaTestFunction) == false)
	{
		TT_PANIC("invalid AlphaTestFunction parameter");
		p_alphaTestFunction = AlphaTestFunction_Never;
	}

	m_alphaTestFunction = p_alphaTestFunction;
	
	// normalize the alpha test value
	u8 u8Max = std::numeric_limits<u8>::max();
	real val = static_cast<real>(u8Max) / getAlphaTestValue();
	
	glAlphaFunc(g_alphaTestFunctions[m_alphaTestFunction], val);
}


void Renderer::setAlphaTestValue(u8 p_alphaTestValue)
{
	m_alphaTestValue = p_alphaTestValue;
	
	// update the alpha test routine
	setAlphaTestFunction(m_alphaTestFunction);
}


void Renderer::setAlphaTest(AlphaTestFunction p_alphaTestFunction, u8 p_alphaTestValue)
{
	m_alphaTestValue = p_alphaTestValue;
	
	setAlphaTestFunction(p_alphaTestFunction);
}


void Renderer::clearStencil(s32 p_value)
{
	glClearStencil(p_value);
	glClear(GL_STENCIL_BUFFER_BIT);
	
	TT_CHECK_OPENGL_ERROR();
}


void Renderer::setStencilEnabled(bool p_enabled)
{
	m_stateCache->setState(GL_STENCIL_TEST, p_enabled);
}


void Renderer::setStencilFunction(StencilSide p_side, StencilTestFunction p_function, s32 p_reference, s32 p_mask)
{
	if (p_side == StencilSide_Front)
	{
		glStencilFuncSeparate(GL_FRONT, stencilFunc[p_function], p_reference, p_mask);
	}
	else
	{
		glStencilFuncSeparate(GL_BACK, stencilFunc[p_function], p_reference, p_mask);
	}
}


void Renderer::setStencilOperation(StencilSide      p_side, StencilOperation  p_stencilFail,
                                   StencilOperation p_zFail, StencilOperation p_stencilPass)
{
	if (p_side == StencilSide_Front)
	{
		glStencilOpSeparate(GL_FRONT, stencilOp[p_stencilFail], stencilOp[p_zFail], stencilOp[p_stencilPass]);
	}
	else
	{
		glStencilOpSeparate(GL_BACK, stencilOp[p_stencilFail], stencilOp[p_zFail], stencilOp[p_stencilPass]);
	}
}


void Renderer::setTexture(const TexturePtr& p_texture)
{
	// a single texture always operates on stage 0
	Texture::setActiveChannel(0);

	TT_ASSERTMSG(m_multiTextureDirtyStageRange == 0, 
	             "A previous setMultiTexture was not restored yet, call resetMultiTexture() first.");
	
	// Change texture
	if (p_texture != 0)
	{
		// FIXME: Should go through FixedFunction
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB  , GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		
		// Activate texture in stage 0
		p_texture->select(0);
	}
	else
	{
		// Don't disable texture_2D here. That should be done through setVertexType. (If needed.)
		FixedFunction::disableTexture(0);
		MultiTexture::setActiveTexture(0,0);
		setPremultipliedAlphaEnabled(false);
	}
}


void Renderer::resetMultiTexture()
{
	FixedFunction::resetTextureStages();
	Texture::setActiveChannel(0);
	Texture::setActiveClientChannel(0);
}


/*! \brief Changes the currently active rendering camera.
    \param camera The camera to use for the rendering of the scene
    \return The previous rendering camera. */
scene::CameraPtr Renderer::setMainCamera(const scene::CameraPtr& p_camera,
                                         bool p_update,
                                         ViewPortID p_viewport)
{
	scene::CameraPtr oldCamera = ViewPort::getViewPort(p_viewport).getCamera();
	
	// Set the current camera
	TT_ASSERT(ViewPort::hasViewPort(p_viewport));
	ViewPort::getViewPorts()[p_viewport].setCamera(p_camera);
	
	if (p_update && p_camera != 0)
	{
		p_camera->update();
	}
	
	if (m_isRendering)
	{
		// NOTE: Changing cameras during rendering is not advisable
		//       this is basically a patch to fix Toki Tori
		m_activeCamera = p_camera;
		m_activeCamera->select();
	}
	
	return oldCamera;
}


/*! \brief Retrieve the main camera used to render the current scene.
    \return The main rendering camera. */
scene::CameraPtr Renderer::getMainCamera(ViewPortID p_viewport) const
{
	TT_ASSERT(static_cast<std::size_t>(p_viewport) < ViewPort::getViewPorts().size());
	return ViewPort::getViewPorts()[p_viewport].getCamera();
}


CullFrontOrder Renderer::setCullFrontOrder(CullFrontOrder p_cullFrontOrder)
{
	TT_ASSERTMSG(isValidCullFrontOrder(p_cullFrontOrder), "invalid cull front order passed");

	// check if call is redundant
	if (p_cullFrontOrder == m_cullFrontOrder) return m_cullFrontOrder;

	CullFrontOrder prevCullFrontOrder = m_cullFrontOrder;
	m_cullFrontOrder = p_cullFrontOrder;

	updateCulling();

	return prevCullFrontOrder;
}


CullMode Renderer::setCullMode(CullMode p_cullMode)
{
	TT_ASSERTMSG(isValidCullMode(p_cullMode), "invalid cull mode passed");	

	// check if call is redundant
	if (p_cullMode == m_cullMode) return m_cullMode;

	CullMode prevCullMode = m_cullMode;
	m_cullMode = p_cullMode;

	updateCulling();

	return prevCullMode;
}


void Renderer::setCullingEnabled(bool p_cullingEnabled)
{
	// check if call is redundant
	if (p_cullingEnabled == m_cullingEnabled) return;

	m_cullingEnabled = p_cullingEnabled;
	
	updateCulling();
}
	

void Renderer::setFillMode(FillMode p_fillMode)
{
	if (p_fillMode != m_fillMode)
	{
		m_fillMode = p_fillMode;
		glPolygonMode(GL_FRONT_AND_BACK, (m_fillMode == FillMode_Wireframe) ? GL_LINE : GL_FILL);
	}
}


void Renderer::setLighting(bool p_enable)
{
	setLights(p_enable, false);
	m_lightingEnabled = p_enable;
}


void Renderer::setFog(const scene::FogPtr&)
{
	// Not implemented
}


void Renderer::setFogColor(const ColorRGBA& p_color)
{
	if (m_fogColor != p_color)
	{
		FixedFunction::setFogColor(p_color);
		m_fogColor = p_color;
	}
}


void Renderer::setFogSetting(FogSetting p_setting, real p_value)
{
	TT_ASSERT(p_setting >= 0 && p_setting < FogSetting_Count);
	
	if (m_fogSettings[p_setting] != p_value)
	{
		FixedFunction::setFogSetting(p_setting, p_value);
		m_fogSettings[p_setting] = p_value;
	}
}


void Renderer::setFogMode(FogMode p_mode)
{
	if (m_fogMode != p_mode)
	{
		FixedFunction::setFogMode(p_mode);
		m_fogMode = p_mode;
	}
}


void Renderer::setFogEnabled(bool p_enable)
{
#ifndef TT_BUILD_FINAL
	if(m_debugRenderer->isOverdrawDebugActive())
	{
		// Keep disabled
		return;
	}
#endif
	
	if (m_fogEnabled != p_enable)
	{
		FixedFunction::setFogEnabled(p_enable);
		m_fogEnabled = p_enable;
	}
}


void Renderer::setZBufferEnabled(bool p_enabled)
{
	m_isZBufferEnabled = p_enabled;
	m_stateCache->setState(GL_DEPTH_TEST, m_isZBufferEnabled);
}


void Renderer::setDepthWriteEnabled(bool p_enabled)
{
	m_isDepthWriteEnabled = p_enabled;
	glDepthMask(m_isDepthWriteEnabled ? GL_TRUE : GL_FALSE);
}


void Renderer::beginViewPort(ViewPort& p_viewport, bool p_last)
{
	m_lastViewPort = p_last;
	m_activeCamera = p_viewport.getCamera();
	
	// Check the camera
	if (m_activeCamera == 0)
	{
		TT_PANIC("No active camera, cannot render anything.");
	}
	
	p_viewport.begin();
	
	// Start capturing
	if (m_postProcessor->isActive())
	{
		// No need to clear color if we have a skybox.
		m_postProcessor->beginFrame(m_skybox != 0 ? ClearFlag_DepthBuffer : ClearFlag_All);
		m_postProcessingActive = true;
	}
	
	// Render skybox
	renderSkybox();
	
	m_debugRenderer->showAxis();
}


void Renderer::endViewPort(ViewPort& p_viewport)
{
	renderTransparents();
	
	// Apply post-processing
	if (m_postProcessor->isActive())
	{
		m_postProcessingActive = false;
		m_postProcessor->endFrame();
	}
	
	if (m_debugRenderer->getDebugCameraActive() && m_activeCamera != 0)
	{
		m_activeCamera->visualize();
	}
	
	m_debugRenderer->flush();

	p_viewport.end();
	
	if ((m_currentViewport + 1) < ViewPort::getViewPorts().size())
	{
		m_currentViewport = static_cast<ViewPortID>(m_currentViewport + 1);
	}
}


math::Vector2 Renderer::getUpScaling() const
{
	return m_upScaler->getScaleFactor();
}


void Renderer::setScissorRect(const tt::math::PointRect& p_rect)
{
	m_stateCache->setState(GL_SCISSOR_TEST, true);
	// NOTE: X/Y position in glScissor specifies *bottom left* position of the rectangle
	glScissor(static_cast<GLint>(p_rect.getLeft()),
	          static_cast<GLint>(getScreenHeight() - p_rect.getTop() - p_rect.getHeight()),
	          static_cast<GLsizei>(p_rect.getWidth()), static_cast<GLsizei>(p_rect.getHeight()));
}


void Renderer::resetScissorRect()
{
	m_stateCache->setState(GL_SCISSOR_TEST, false);
}


void Renderer::setCustomViewport(s32 p_x, s32 p_y, s32 p_width, s32 p_height, real, real)
{
	// If running in iOS 2x (Retina) mode, upscale the coordinates and dimensions that were passed
	if (isIOS2xMode())
	{
		p_x      *= 2;
		p_y      *= 2;
		p_width  *= 2;
		p_height *= 2;
	}
	
	// Set the viewport, with an inverted Y (OpenGL Y starts at the bottom instead of the top)
	glViewport(p_x, (m_screenSize.y - p_y - p_height), p_width, p_height);
}
	
	
void Renderer::saveBackbuffer()
{
	// Nothing to do here for OpenGL
}
	
	
void Renderer::restoreBackbuffer()
{
	RenderTarget::bindFramebuffer(0);
	glViewport(0, 0, m_backBufferSize.x, m_backBufferSize.y);
}
	
	
s32 Renderer::setAntiAliasing(bool p_enable, s32 p_samples)
{
#if defined(TT_PLATFORM_OSX_IPHONE)
	TT_ASSERTMSG(p_enable == false, "AA Not supported on iOS");
	return 0;
#else
	m_useAA = p_enable;
	m_samplesAA = p_samples;
	
	if(m_useAA)
	{
		GLint maxSamples(0);
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
		
		if(m_samplesAA > maxSamples)
		{
			m_samplesAA = maxSamples;
		}
	}
	if (m_samplesAA <= 0)
	{
		m_useAA = false;
	}
	
	m_stateCache->setState(GL_MULTISAMPLE, m_useAA);

	TT_Printf("Renderer::setAntiAliasing - m_useAA: %d, m_samplesAA: %d\n", m_useAA, m_samplesAA);
	
	if (m_postProcessor != 0)
	{
		m_postProcessor->handleAntiAliasingChanged(m_samplesAA);
	}	
	
	return m_samplesAA;
#endif
}


void Renderer::addTransparentObject(real                  p_distance,
                                    scene::SceneObject*   p_object,
                                    const math::Matrix44& p_matrix)
{
	m_transparentObjects.insert(
		RenderObjects::value_type(p_distance, RenderObject(p_object, p_matrix)));
}


void Renderer::renderTransparents()
{
	if (m_transparentObjects.empty()) return;
	
	m_isRenderingTransparents = true;
	
	MatrixStack::getInstance()->setMode(MatrixStack::Mode_Position);
	MatrixStack::getInstance()->push();
	
	// const_reverse_iterator != with .rend() didn't work if the container object wasn't const.
	const RenderObjects& constTransObjs = m_transparentObjects;
	for (RenderObjects::const_reverse_iterator it = constTransObjs.rbegin(); it != constTransObjs.rend(); ++it)
	{
		TT_NULL_ASSERT(it->second.first);
		
		// FIXME: This should be stored to still support scene info
		RenderContext rc;
		rc.pass = RenderPass_Transparents;
		
		// Render back faces (not always wanted, should be settable by data)
		/*
		setCullMode(CullMode_Front);
		
		MatrixStack::getInstance()->load44(it->second.second);
		it->second.first->render(rc);
		*/
		
		// Render front faces
		setCullMode(CullMode_Back);
		
		MatrixStack::getInstance()->load44(it->second.second);
		it->second.first->render(rc);
	}
	
	MatrixStack::getInstance()->pop();
	
	// Clear the list
	m_transparentObjects.clear();
	
	m_isRenderingTransparents = false;
	
	//overwriteAlphaChannel = false;
	
	TT_CHECK_OPENGL_ERROR();
}


bool Renderer::handleResetDevice()
{
	// Get target size from up-scaler
	m_backBufferSize = m_context->getBackBufferSize();
	m_screenSize = m_upScaler->handleResolutionChanged(m_backBufferSize.x, m_backBufferSize.y);
	
	// Reset states
	reset();
	
	// Reset lighting
	m_lightManager.reset();
	
	// Reset fog
	resetFog();
	
	// Reset textures
	MultiTexture::resetAllActiveTextures();
	
	// Create cameras and viewports
	createDefaultCameras();
	ViewPort::createLayout(Layout_Standard);
	
	// RenderTargets might need to resize because of screensize change.
	RenderTarget::onDeviceReset();
	
	return true;
}
	

void Renderer::checkFromRenderThread() const
{
#if defined(TT_PLATFORM_SDL)
	TT_ASSERTMSG(SDL_ThreadID() == m_threadID,
		"This function can only be called from the render thread.");
#else
	// NOTE: This doesn't work if display link is used, because then frames
	//       are rendered from the display link thread

	//TT_ASSERTMSG(pthread_self() == m_threadID,
	//	"This function can only be called from the render thread.");
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Renderer::Renderer(OpenGLContextWrapper* p_openGLContext, bool p_ios2xMode)
:
m_context(p_openGLContext),
m_isRendering(false),
m_isRenderingHud(false),
m_useAA(false),
m_lightingEnabled(false),
m_fogEnabled(false),
m_fogMode(FogMode_Linear),
m_fogColor(ColorRGB::white),
m_isZBufferEnabled(false),
m_isDepthWriteEnabled(true),
m_deltaTime(0.0f),
m_time(0.0f),
m_frame(0),
m_samplesAA(0),
m_shouldClear(true),
m_srcFactor(BlendFactor_SrcAlpha),
m_dstFactor(BlendFactor_InvSrcAlpha),
m_separateAlphaBlendEnabled(false),
m_srcFactorAlpha(BlendFactor_Zero),
m_dstFactorAlpha(BlendFactor_Zero),
m_premultipliedAlphaEnabled(false),
m_alphaTestEnabled(false),
m_alphaTestFunction(AlphaTestFunction_Never),
m_alphaTestValue(0),
m_cullFrontOrder(CullFrontOrder_ClockWise),
m_cullMode(CullMode_Back),
m_cullingEnabled(false),
m_fillMode(FillMode_Solid),
m_colorMask(ColorMask_None),
m_activeCamera(),
m_hudCamera(),
m_activeTexture(),
m_defaultMaterial(),
m_skybox(),
m_lastViewPort(false),
m_currentViewport(ViewPortID_1),
m_lightManager(),
m_vertexType(0),
m_clearColor(ColorRGB::black),
m_glClearColor(0,0,0,0),
m_transparentObjects(),
m_isRenderingTransparents(false),
m_debugRenderer(new debug::DebugRenderer),
m_postProcessor(new pp::PostProcessor),
m_postProcessingActive(false),
m_screenSize(math::Point2::zero),
m_backBufferSize(math::Point2::zero),
m_upScaler(new UpScaler(math::Point2::zero, true)),
m_upScalerActive(false),
m_lowPerformanceMode(false),
m_multiTextureDirtyStageRange(0),
m_ios2xMode(p_ios2xMode),
#if TT_PLATFORM_SDL
m_threadID(SDL_ThreadID()),
#else
m_threadID(pthread_self()),
#endif
m_stateCache(new GLStateCache)
{
	// this is needed so it is assigned early as code below indirectly calls into FixedFunctionHardware which wants an instance of the renderer to access the state cache.
	ms_instance = this;

	// Set up OpenGL. This needs te be called before other code that uses OpenGL, like the MatrixStack.
	// It only needs to be called once.
	TT_NULL_ASSERT(m_context);
	m_context->init();
	setupGraphics();
	
	// Check for available texture stages
	GLint textureUnitsAvailable = 0;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &textureUnitsAvailable);
	
	MultiTexture::setChannelCount(textureUnitsAvailable);
	
	// Initialize matrix stack
	MatrixStack::createInstance();
	
	// Create FileUtils
	file::FileUtils::createInstance();
	
	// Fog defaults
	m_fogSettings[FogSetting_Start]   = 0.0f;
	m_fogSettings[FogSetting_End]     = 1.0f;
	m_fogSettings[FogSetting_Density] = 1.0f;
	
	// Set states
	setDefaultStates(false);
	FullscreenTriangle::create();
}


Renderer::~Renderer()
{
	MultiTexture::resetChannelCount();
	
	// Destroy FileUtils Singleton
	file::FileUtils::destroyInstance();
	
	// Destroy Matrix Stack Singleton
	MatrixStack::destroyInstance();
	
	FullscreenTriangle::destroy();

	// Destroy GLStateCache
	delete m_stateCache;
}


void Renderer::createDefaultCameras()
{
	real width  = real(getScreenWidth());
	real height = real(getScreenHeight());
	
	// Create HUD camera
	real hudX =  width  / 2.0f;
	real hudY = -height / 2.0f;
	math::Vector3 pos(hudX, hudY, 500.0f);
	math::Vector3 lookAt(hudX, hudY, 0.0f);
	m_hudCamera.reset(new scene::Camera(pos, lookAt, width, height, 1.0f, 4096.0f));
	m_hudCamera->update();
}


void Renderer::setupGraphics()
{
	// Save renderer window dimensions
	m_screenSize = m_context->getBackBufferSize();
	
	// Allow RenderTarget to initialize itself
	RenderTarget::hardwareSupportsFramebuffers();
	
	// Set the standard clear color to black
	setClearColor(ColorRGB::black);
}


void Renderer::setLights(bool p_enable, bool p_useVtxColor)
{
	FixedFunction::setLightingEnabled(p_enable);
	
	if(p_useVtxColor)
	{
#if !defined(TT_PLATFORM_OSX_IPHONE) // FIXME: Need OpenGL ES define
		m_stateCache->setState(GL_COLOR_MATERIAL, true);
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); // Use vertex color as diffuse property
#endif
	}
	else
	{
		m_stateCache->setState(GL_COLOR_MATERIAL, false);
	}

	
	TT_CHECK_OPENGL_ERROR();
}


void Renderer::renderSkybox()
{
	if (m_skybox == 0) return;
	
	// Disable Z-writes / checks
	m_stateCache->setState(GL_DEPTH_TEST, false);
	glDepthMask(GL_FALSE);
	
	TT_NULL_ASSERT(m_activeCamera);
	
	// Render at camera position
	MatrixStack* stack = MatrixStack::getInstance();
	stack->push();
	stack->translate(m_activeCamera->getActualPosition());
	m_skybox->render();
	stack->pop();
	
	// If skybox contains any transparent objects, render those now as well
	renderTransparents();
	
	// Restore Z-writes / checks
	if (m_isZBufferEnabled)
	{
		m_stateCache->setState(GL_DEPTH_TEST, true);
	}
	if (m_isDepthWriteEnabled)
	{
		glDepthMask(GL_TRUE);
	}
}


void Renderer::updateCulling() const
{
	if (m_cullingEnabled == false)
	{
		m_stateCache->setState(GL_CULL_FACE, false);
	}
	else
	{
		m_stateCache->setState(GL_CULL_FACE, true);
		glFrontFace(cullFrontOrderLookup[m_cullFrontOrder]);
		glCullFace(cullModeLookup[m_cullMode]);
	}
}


void Renderer::updateBlendModes() const
{
	// Override for premultiplied alpha
	BlendFactor srcColorFactor(m_srcFactor);
	if(m_premultipliedAlphaEnabled && srcColorFactor == BlendFactor_SrcAlpha)
	{
		srcColorFactor = BlendFactor_One;
	}
	
	TT_CHECK_OPENGL_ERROR();
	if (m_separateAlphaBlendEnabled)
	{
		m_stateCache->setBlendFunc(g_blendFactors[srcColorFactor],   g_blendFactors[m_dstFactor], 
		                           g_blendFactors[m_srcFactorAlpha], g_blendFactors[m_dstFactorAlpha]);
	}
	else
	{
		m_stateCache->setBlendFunc(g_blendFactors[srcColorFactor], g_blendFactors[m_dstFactor]);
	}
	TT_CHECK_OPENGL_ERROR();
}


void Renderer::resetFog()
{
	FixedFunction::setFogEnabled(m_fogEnabled);
	FixedFunction::setFogMode   (m_fogMode);
	FixedFunction::setFogColor  (m_fogColor);
	
	for (s32 i = 0; i < FogSetting_Count; ++i)
	{
		FixedFunction::setFogSetting(static_cast<FogSetting>(i), m_fogSettings[i]);
	}
}


bool Renderer::setTextureStage(u8 p_index, const TextureStageData& p_textureStageData, bool p_resetting)
{
	if (p_index >= MultiTexture::getChannelCount())
	{
		TT_PANIC("Texture stage %u out of bounds (max = %u).", p_index, MultiTexture::getChannelCount() - 1);
		return false;
	}
	
	// Activate texture stage
	Texture::setActiveChannel(static_cast<s32>(p_index));
	
	// If resetting, a texture is not required
	if (p_resetting == false)
	{
		// Check texture existence
		if (p_textureStageData.getTexture() == 0)
		{
			TT_PANIC("Texture not set for multitexture stage %u.", p_index);
			return false;
		}
		else
		{
			// Bind texture to OpenGL's corresponding fixed function pipeline stage
			p_textureStageData.getTexture()->select(p_index);
			glEnable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		}
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		Texture::unbindTexture();
		
		MultiTexture::resetActiveTexture(p_index);
	}
	
	
	//--- APPLY TEXTURE STAGE PARAMETERS ---
	// Texture coordinates set index
	// OpenGL texture coordinate sets do not exist, texcoord sets are actually bound to the corresponding stage:
	// (set 0 = stage 0, set 1 = stage 1) and therefore handled by the data layer
	
	// Color/alpha blend operation
	bool useColorArg2 = false;
	bool useAlphaArg2 = false;
	
	switch (p_textureStageData.getColorBlendOperation())
	{
		// Check if there is a non-directly supported option
	case TextureBlendOperation_Disable:
		// In OpenGL this is done on a higher level, so this is already done
		break;
		
	case TextureBlendOperation_SelectArg1:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		break;
		
	case TextureBlendOperation_SelectArg2:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		useColorArg2 = true;
		break;
		
		// Directly supported options
	default:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,
		          blendOp[p_textureStageData.getColorBlendOperation()]);
		break;
	}
	
	switch (p_textureStageData.getAlphaBlendOperation())
	{
		// Check if there is a non-directly supported option
	case TextureBlendOperation_Disable:
		// In OpenGL this is done on a higher level, so this is already done
		break;
		
	case TextureBlendOperation_SelectArg1:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		break;
		
	case TextureBlendOperation_SelectArg2:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		useAlphaArg2 = true;
		break;
		
		// Directly supported options
	default:
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,
		          blendOp[p_textureStageData.getAlphaBlendOperation()]);
		break;
	}
	
	// Color sources
	switch (p_textureStageData.getColorSource1())
	{
		// Check if there is a non-directly supported option
	case TextureBlendSource_OneMinusTexture:
	case TextureBlendSource_OneMinusDiffuse:
	case TextureBlendSource_OneMinusPrevious:
	case TextureBlendSource_OneMinusConstant:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
		if (useColorArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, blendSrc[p_textureStageData.getColorSource1()]);
		}
		else
		{
			// Make color source 2 -> 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, blendSrc[p_textureStageData.getColorSource2()]);
		}
		break;
		
		// Directly supported options
	default:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		
		if (useColorArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, blendSrc[p_textureStageData.getColorSource1()]);
		}
		else
		{
			// Make color source 2 -> 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, blendSrc[p_textureStageData.getColorSource2()]);
		}
		break;
	}
	
	switch (p_textureStageData.getColorSource2())
	{
		// Check if there is a non-directly supported option
	case TextureBlendSource_OneMinusTexture:
	case TextureBlendSource_OneMinusDiffuse:
	case TextureBlendSource_OneMinusPrevious:
	case TextureBlendSource_OneMinusConstant:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_ONE_MINUS_SRC_COLOR);
		if (useColorArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, blendSrc[p_textureStageData.getColorSource2()]);
		}
		break;
		
		// Directly supported options
	default:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		
		if (useColorArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, blendSrc[p_textureStageData.getColorSource2()]);
		}
		break;
	}
	
	// Alpha sources
	switch (p_textureStageData.getAlphaSource1())
	{
		// Check if there is a non-directly supported option
	case TextureBlendSource_OneMinusTexture:
	case TextureBlendSource_OneMinusDiffuse:
	case TextureBlendSource_OneMinusPrevious:
	case TextureBlendSource_OneMinusConstant:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (useAlphaArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, blendSrc[p_textureStageData.getAlphaSource1()]);
		}
		else
		{
			// Make alpha source 2 -> 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, blendSrc[p_textureStageData.getAlphaSource2()]);
		}
		break;
		
		// Directly supported options
	default:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		
		if (useAlphaArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, blendSrc[p_textureStageData.getAlphaSource1()]);
		}
		else
		{
			// Make alpha source 2 -> 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, blendSrc[p_textureStageData.getAlphaSource2()]);
		}
		break;
	}
	
	switch (p_textureStageData.getAlphaSource2())
	{
		// Check if there is a non-directly supported option
	case TextureBlendSource_OneMinusTexture:
	case TextureBlendSource_OneMinusDiffuse:
	case TextureBlendSource_OneMinusPrevious:
	case TextureBlendSource_OneMinusConstant:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (useAlphaArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, blendSrc[p_textureStageData.getAlphaSource2()]);
		}
		break;
		
		// Directly supported options
	default:
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		
		if (useAlphaArg2 == false)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, blendSrc[p_textureStageData.getAlphaSource2()]);
		}
		break;
	}
	
	// Constant color
	if (p_textureStageData.getColorSource1() == TextureBlendSource_Constant ||
	    p_textureStageData.getColorSource1() == TextureBlendSource_OneMinusConstant ||
	    p_textureStageData.getColorSource2() == TextureBlendSource_Constant ||
	    p_textureStageData.getColorSource2() == TextureBlendSource_OneMinusConstant ||
	    p_textureStageData.getAlphaSource1() == TextureBlendSource_Constant ||
	    p_textureStageData.getAlphaSource1() == TextureBlendSource_OneMinusConstant ||
	    p_textureStageData.getAlphaSource2() == TextureBlendSource_Constant ||
	    p_textureStageData.getAlphaSource2() == TextureBlendSource_OneMinusConstant)
	{
		real u8Max = static_cast<real>(std::numeric_limits<u8>::max());
		const ColorRGBA& RGBA8(p_textureStageData.getConstant());
		GLfloat constantNormalizedRGBA[4] = { RGBA8.r / u8Max, RGBA8.g / u8Max, RGBA8.b / u8Max, RGBA8.a / u8Max };
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constantNormalizedRGBA);
	}
	
	// Transformation matrix preparation
	MatrixStack* stack = MatrixStack::getInstance();
	
	// If only resetting, the MatrixStack does not need a reset
	if (p_resetting == false && p_index == 0)
	{
		// Stage 0 is also controlled by MatrixStack, reset MatrixStack
		stack->resetTextureMatrix();
	}
	
	// Transformation matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glLoadMatrixf(reinterpret_cast<const GLfloat*>(&p_textureStageData.getMatrix()));
	
	TT_CHECK_OPENGL_ERROR();
	
	//--- APPLY TEXTURE STAGE PARAMETERS end ---
	
#if defined(DEBUG_TEXTURESTAGES)
	printStageInfo(p_index);
#endif
	return true;
}


#if !defined(TT_BUILD_FINAL)
void Renderer::printStageInfo(u8 p_index) const
{
	static int printed = 0;
	static const int maxPrints = 20;
	++printed;
	if (printed < maxPrints)
	{
		GLint values[10] = { 0 };
		GLfloat constant[4] = { 0 };
		
		glGetTexEnviv(GL_TEXTURE_ENV, GL_COMBINE_RGB,       &values[0]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND0_RGB,      &values[1]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_SRC0_RGB,          &values[2]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND1_RGB,      &values[3]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_SRC1_RGB,          &values[4]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,     &values[5]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA,    &values[6]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_SRC0_ALPHA,        &values[7]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA,    &values[8]);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_SRC1_ALPHA,        &values[9]);
		glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constant);
		
		TT_Printf("Stage          :%d\n"
		          "Combine  RGB   :%s\n"
		          "Operand0 RGB   :%s\n"
		          "Source0  RGB   :%s\n" 
		          "Operand1 RGB   :%s\n" 
		          "Source1  RGB   :%s\n" 
		          "Combine  ALPHA :%s\n" 
		          "Operand0 ALPHA :%s\n" 
		          "Source0  ALPHA :%s\n" 
		          "Operand1 ALPHA :%s\n" 
		          "Source1  ALPHA :%s\n" 
		          "Constant RGBA  :%f,%f,%f,%f\n",
		          p_index,
		          getStringFromTexEnvValue(GL_COMBINE_RGB,  values[0]),
		          getStringFromTexEnvValue(GL_OPERAND0_RGB, values[1]),
		          getStringFromTexEnvValue(GL_SRC0_RGB,     values[2]),
		          getStringFromTexEnvValue(GL_OPERAND0_RGB, values[3]),
		          getStringFromTexEnvValue(GL_SRC0_RGB,     values[4]),
		          getStringFromTexEnvValue(GL_COMBINE_RGB,  values[5]),
		          getStringFromTexEnvValue(GL_OPERAND0_RGB, values[6]),
		          getStringFromTexEnvValue(GL_SRC0_RGB,     values[7]),
		          getStringFromTexEnvValue(GL_OPERAND0_RGB, values[8]),
		          getStringFromTexEnvValue(GL_SRC0_RGB,     values[9]),
		          constant[0], constant[1], constant[2], constant[3]);
	}
}


const char* Renderer::getStringFromTexEnvValue(s32 p_mode, GLint p_value) const
{
	switch (p_mode)
	{
	case GL_COMBINE_RGB:
		switch (p_value)
		{
		case GL_REPLACE:   return "GL_REPLACE";
		case GL_MODULATE:  return "GL_MODULATE";
		case GL_ADD:       return "GL_ADD";
		case GL_SUBTRACT:  return "GL_SUBTRACT";
		case GL_NEVER:     return "GL_NEVER (shared/multitexture::Stage[index] disabled or selectArg1,2)";
		default:
			TT_PANIC("Unrecognized OpenGL TexEnv p_value (%d) for debug output (p_mode = GL_COMBINE_RGB)", p_value);
			return "UNRECOGNIZED";
		}
		break;
		
	case GL_OPERAND0_RGB:
		switch (p_value)
		{
		case GL_SRC_COLOR:            return "GL_SRC_COLOR";
		case GL_ONE_MINUS_SRC_COLOR:  return "GL_ONE_MINUS_SRC_COLOR";
		case GL_SRC_ALPHA:            return "GL_SRC_ALPHA";
		case GL_ONE_MINUS_SRC_ALPHA:  return "GL_ONE_MINUS_SRC_ALPHA";
		default:
			TT_PANIC("Unrecognized OpenGL TexEnv p_value (%d) for debug output (p_mode = GL_OPERAND0_RGB)", p_value);
			return "";
		}
		break;
		
	case GL_SRC0_RGB:
		switch (p_value)
		{
		case GL_TEXTURE:              return "GL_TEXTURE";
		case GL_CONSTANT:             return "GL_CONSTANT";
		case GL_PRIMARY_COLOR:        return "GL_PRIMARY_COLOR"; 
		case GL_PREVIOUS:             return "GL_PREVIOUS"; 
		default:
			TT_PANIC("Unrecognized OpenGL TexEnv p_value (%d) for debug output (p_mode = GL_SRC0_RGB)", p_value);
			return ""; 
		}
		break;
	}
	
	TT_PANIC("Unrecognized OpenGL TexEnv p_mode (%d) for debug output", p_mode);
	return "";
}
#endif // !defined(TT_BUILD_FINAL)


void Renderer::resetTextureStages(u8 p_indexStart, u8 p_indexEnd)
{
	if (p_indexStart > MultiTexture::getChannelCount() ||
	    p_indexEnd   > MultiTexture::getChannelCount())
	{
		TT_PANIC("Index/indices out of bounds: p_indexStart = %u, p_indexEnd = %u, max textures = %u",
		         p_indexStart, p_indexEnd, MultiTexture::getChannelCount());
	}
	TT_ASSERTMSG(p_indexStart <= p_indexEnd, "Start index (%u) is higher than end index (%u)",
	             p_indexStart, p_indexEnd);
	
	TextureStageData defaultStage;
	defaultStage.setAlphaBlendOperation(TextureBlendOperation_Disable);
	defaultStage.setColorBlendOperation(TextureBlendOperation_Disable);
	
	for (u8 i = p_indexStart; i < p_indexEnd; ++i)
	{
		if (setTextureStage(i, defaultStage, true) == false)
		{
			TT_PANIC("Resetting texture stage %u failed", i);
		}
	}
}

// Namespace end
}
}
}
