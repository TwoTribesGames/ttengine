
#include <tt/engine/renderer/directx.h>
#include <tt/engine/debug/DebugFont.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/FullscreenTriangle.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/scene/Fog.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/renderer/pp/PostProcessor.h>


namespace tt {
namespace engine {
namespace renderer {

// Unique instance of renderer
Renderer*             Renderer::ms_instance = 0;
Renderer::ComDeathRow Renderer::ms_deathRow;
thread::Mutex         Renderer::ms_deathRowMutex;


const D3DCULL cullModeLookup[][CullMode_Count] = // row count must be known in a 2d initialization
{
	{             //!< CullFrontOrder_ClockWise,
	D3DCULL_CW,   //!< CullMode_Front,
	D3DCULL_CCW,  //!< CullMode_Back
	},
	{             //!< CullFrontOrder_CounterClockWise,
	D3DCULL_CCW,  //!< CullMode_Front,
	D3DCULL_CW,   //!< CullMode_Back
	}
};
TT_STATIC_ASSERT(sizeof(cullModeLookup) / sizeof(D3DCULL) == CullFrontOrder_Count * CullMode_Count);

/*! \brief Lookup table matching shared enum TextureBlendOperation */
const D3DTEXTUREOP blendOp[] =
{
	D3DTOP_DISABLE,       //<!TextureBlendOperation_Disable,
	
	D3DTOP_SELECTARG1,    //<!TextureBlendOperation_SelectArg1
	D3DTOP_SELECTARG2,    //<!TextureBlendOperation_SelectArg2
	
	D3DTOP_MODULATE,      //<!TextureBlendOperation_Modulate,
	D3DTOP_ADD,           //<!TextureBlendOperation_Add,
	D3DTOP_SUBTRACT,      //<!TextureBlendOperation_Subtract
	
	D3DTOP_MODULATE2X,        //<!TextureBlendOperation_Modulate2X
	D3DTOP_BLENDTEXTUREALPHA, //<!TextureBlendOperation_Decal
};
TT_STATIC_ASSERT(sizeof(blendOp) / sizeof(D3DTEXTUREOP) == TextureBlendOperation_Count);

/*! \brief Lookup table matching shared enum TextureBlendSource */
const u32 blendSrc[] =
{
	D3DTA_TEXTURE,                           //<!TextureBlendSource_Texture,
	D3DTA_DIFFUSE,                           //<!TextureBlendSource_Diffuse,
	D3DTA_CURRENT,                           //<!TextureBlendSource_Previous,
	D3DTA_CONSTANT,                          //<!TextureBlendSource_Constant,
	
	D3DTA_TEXTURE  | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusTexture,
	D3DTA_DIFFUSE  | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusDiffuse,
	D3DTA_CURRENT  | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusPrevious,
	D3DTA_CONSTANT | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusConstant,
};
TT_STATIC_ASSERT(sizeof(blendSrc) / sizeof(u32) == TextureBlendSource_Count);


const D3DCMPFUNC stencilFunc[] = 
{
	D3DCMP_NEVER       , //<! StencilTestFunction_Never
	D3DCMP_LESS        , //<! StencilTestFunction_Less
	D3DCMP_EQUAL       , //<! StencilTestFunction_Equal
	D3DCMP_LESSEQUAL   , //<! StencilTestFunction_LessEqual
	D3DCMP_GREATER     , //<! StencilTestFunction_Greater
	D3DCMP_NOTEQUAL    , //<! StencilTestFunction_NotEqual
	D3DCMP_GREATEREQUAL, //<! StencilTestFunction_GreaterEqual
	D3DCMP_ALWAYS        //<! StencilTestFunction_Always
};
TT_STATIC_ASSERT(sizeof(stencilFunc) / sizeof(u32) == StencilTestFunction_Count);


const D3DSTENCILOP stencilOp[] =
{
	D3DSTENCILOP_KEEP,     //<! StencilOperation_Keep
	D3DSTENCILOP_ZERO,     //<! StencilOperation_Zero
	D3DSTENCILOP_REPLACE,  //<! StencilOperation_Replace
	D3DSTENCILOP_INCRSAT,  //<! StencilOperation_Increment
	D3DSTENCILOP_INCR,     //<! StencilOperation_IncrementWrap
	D3DSTENCILOP_DECRSAT,  //<! StencilOperation_Decrement
	D3DSTENCILOP_DECR,     //<! StencilOperation_DecrementWrap
	D3DSTENCILOP_INVERT    //<! StencilOperation_Invert
};
TT_STATIC_ASSERT(sizeof(stencilOp) / sizeof(u32) == StencilOperation_Count);


//------------------------------------------------------------------------------
// Public member functions

bool Renderer::createInstance(bool p_ios2xMode)
{
	if (ms_instance == 0)
	{
		ms_instance = new Renderer(p_ios2xMode);
	}
	if (ms_instance == 0)
	{
		TT_PANIC("Failed to initialize renderer.");
	}
	return true;
}


void Renderer::destroyInstance()
{
	delete ms_instance;
	ms_instance = 0;
}


void Renderer::reset(bool p_disableZ)
{
	// Re-acquire current device
	m_device = getRenderDevice();
	
	TT_NULL_ASSERT(m_device);
	
	// Reset states
	m_isZBufferEnabled = (p_disableZ == false);
	setDefaultStates(p_disableZ);
}


void Renderer::setDefaultStates(bool)
{
	if(m_device == 0)
	{
		TT_PANIC("No valid D3D Device found");
		return;
	}
	checkFromRenderThread();
	
	// Not using lighting by default
	FixedFunction::setLightingEnabled(false);
	FixedFunction::setAmbientLightColor(ColorRGB::gray);
	
	// A front faced face is defined in clockwise order
	setCullFrontOrder(CullFrontOrder_ClockWise);
	// Cull back faced faces
	setCullMode(CullMode_Back);
	// Enable culling
	setCullingEnabled(true);
	// Force update
	updateCulling();
	
	// Setup alpha blending
	HRESULT hr;
	V( m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) );
	V( m_device->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA) );
	V( m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) );
	setBlendMode(BlendMode_Blend);
	
	setAlphaTestEnabled(false);
	
	// Reset fog
	resetFog();
	
	// Enable texture transformations
	for(s32 i = 0; i < 8; ++i)
	{
		V( FP(m_device)->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) );
	}
	
	// Configure Z-buffering
	V( m_device->SetRenderState(D3DRS_ZENABLE, m_isZBufferEnabled ? D3DZB_TRUE : D3DZB_FALSE) );
	setDepthWriteEnabled(true);
	
	setColorMask(ColorMask_All);
	
	// reset potential modified multitexture stages
	resetTextureStages(0, MultiTexture::getChannelCount());
	m_multiTextureDirtyStageRange = 0;
	setTexture(TexturePtr());
}


void Renderer::setColorMask(u32 p_colorMask)
{
	if (p_colorMask != m_colorMask)
	{
		checkFromRenderThread();
		checkD3DSucceeded( m_device->SetRenderState(D3DRS_COLORWRITEENABLE, p_colorMask) );
		m_colorMask = p_colorMask;
	}
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
	if (m_fillMode != p_fillMode)
	{
		m_fillMode = p_fillMode;
		checkFromRenderThread();
		checkD3DSucceeded(
			m_device->SetRenderState(D3DRS_FILLMODE,
				(m_fillMode == FillMode_Wireframe) ? D3DFILL_WIREFRAME : D3DFILL_SOLID) );
	}
}


bool Renderer::isWideScreen() const
{
	return getScreenWidth() / static_cast<real>(getScreenHeight()) >= wideScreenAspectRatio;
}


void Renderer::update(real p_elapsedTime)
{
	// FIXME: remove hardcoded value and prevent accumulated time errors due to loading etc.
	TT_ASSERT(p_elapsedTime >= 0);
	if (p_elapsedTime > 0.5f)
	{
		p_elapsedTime = 0;
	}
	
	m_deltaTime = p_elapsedTime;

#if !defined(TT_BUILD_FINAL)
	m_deltaTime *= m_debugRenderer->getSpeed();

	/*
	if(m_debugRenderer->getDebugCameraActive())
	{
		m_debugRenderer->getDebugCamera()->FrameMove(p_elapsedTime);
	}
	*/
#endif
	
	m_time += m_deltaTime;

	// Update skybox
	if(m_skybox != 0) m_skybox->update();
}


bool Renderer::beginFrame()
{
	if (checkD3DSucceeded(m_device->BeginScene()) == false)
	{
		return false;
	}
	checkD3DSucceeded(m_device->SetViewport(&m_backBufferViewport));
	
	if(m_upScaler->isActive())
	{
		checkD3DSucceeded(m_device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0,0,0,0), 1.0f, 0));
		
		m_upScaler->beginFrame();
		m_upScalerActive = true;
	}
	else if (m_postProcessor->isActive() == false)
	{
		// Clear the entire render target and the zbuffer
		checkD3DSucceeded(m_device->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, m_d3dClearColor, 1.0f, 0));
	}
	
#if !defined(TT_BUILD_FINAL)
	MatrixStack::getInstance()->checkIntegrity();
#endif
	
	// Start rendering
	m_isRendering = true;
	m_currentViewport = ViewPortID_1;
	FixedFunction::setActive();
	MultiTexture::resetAllActiveTextures();
	
	// Process debug stuff
	m_debugRenderer->beginFrame();
	
	return true;
}


bool Renderer::endFrame()
{
	// Handle upscaling
	m_upScalerActive = false;
	m_upScaler->endFrame();
	
	// Process debug stuff
	m_debugRenderer->endFrame();
	
	// End rendering
	m_isRendering = false;
	
	TT_ASSERTMSG(RenderTargetStack::isEmpty(),
		"RenderTargetStack should be empty at end of frame, did you forget to pop() a RenderTarget?");
	
	return checkD3DSucceeded( m_device->EndScene() );
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
	
	// NOTE: The actual call to IDirect3DDevice9::Present() is made by DXUT
	//		 after leaving the OnD3D9FrameRender() Callback
	
	return true;
}


void Renderer::beginHud()
{
	TT_ASSERTMSG(m_isRenderingHud == false, "Already rendering HUD (beginHud already called).");
	
	// Render all transparent objects
	renderTransparents();
	
	if (m_lightingEnabled)
	{
		FixedFunction::setLightingEnabled(false);
	}
	
	// Setup HUD camera
	m_activeCamera = m_hudCamera;
	m_activeCamera->select();
	
	// Disable Z test
	if(m_isZBufferEnabled)
	{
		checkD3DSucceeded( m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE) );
	}
	
	// Disable AA
	if (isAAEnabled())
	{
		checkD3DSucceeded( m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE) );
	}
	
	m_isRenderingHud = true;
}


void Renderer::endHud()
{
	TT_ASSERTMSG(m_isRenderingHud, "endHud called without matching beginHud.");
	
	m_debugRenderer->showSafeFrame();
	
	// Restore main camera for current viewport
	m_activeCamera = ViewPort::getViewPort(m_currentViewport).getCamera();
	m_activeCamera->select();
	
	HRESULT hr;
	
	// Restore Z test
	if(m_isZBufferEnabled)
	{
		V( m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE) );
	}
	
	// Restore AA
	if (isAAEnabled())
	{
		V( m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE) );
	}
	
	
	m_isRenderingHud = false;
}


void Renderer::setClearColor(const ColorRGBA& p_color)
{
#ifndef TT_BUILD_FINAL
	if(m_debugRenderer->isOverdrawDebugActive())
	{
		// Keep clearcolor
		return;
	}
#endif
	m_clearColor    = p_color;
	m_d3dClearColor = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
	
	m_postProcessor->handleClearColorChanged(p_color);
}


void Renderer::setVertexType(u32 p_vertexType)
{
	TT_ASSERT(p_vertexType != 0);
	
	const bool hasNormals((p_vertexType & VertexBuffer::Property_Normal) != 0);
	const bool hasVtxColors((p_vertexType & VertexBuffer::Property_Diffuse) != 0);
	
	if(m_lightingEnabled)
	{
		FixedFunction::setLightingEnabled(hasNormals);
	}
	
	// Activate the fvf
	checkD3DSucceeded( m_device->SetFVF(getFVFFromVertexType(p_vertexType)) );
	FixedFunction::setVertexColorEnabled(hasVtxColors);
}


void Renderer::setBlendMode(BlendMode p_mode)
{
	switch (p_mode)
	{
	case BlendMode_Blend        : setCustomBlendMode(BlendFactor_SrcAlpha, BlendFactor_InvSrcAlpha); break;
	case BlendMode_Add          : setCustomBlendMode(BlendFactor_SrcAlpha, BlendFactor_One);         break;
	case BlendMode_Modulate     : setCustomBlendMode(BlendFactor_Zero    , BlendFactor_SrcColor);    break;
	case BlendMode_Premultiplied: setCustomBlendMode(BlendFactor_One     , BlendFactor_InvSrcAlpha); break;
	default:
		TT_PANIC("Unsupported blend mode %d.", p_mode);
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

	if(m_premultipliedAlpha != p_enabled)
	{
		m_premultipliedAlpha = p_enabled;
		FixedFunction::setPremultipliedAlpha(m_premultipliedAlpha);
		updateBlendModes();
	}
}


void Renderer::setAlphaTestEnabled(bool p_enableAlphaTest)
{
	m_alphaTestEnabled = p_enableAlphaTest;
	
	HRESULT hr;
	V( m_device->SetRenderState(D3DRS_ALPHATESTENABLE, m_alphaTestEnabled) );
}


void Renderer::setAlphaTestFunction(AlphaTestFunction p_alphaTestFunction)
{
	if (isValidAlphaTestFunction(p_alphaTestFunction) == false)
	{
		TT_PANIC("invalid AlphaTestFunction parameter");
		p_alphaTestFunction = AlphaTestFunction_Never;
	}
	
	m_alphaTestFunction = p_alphaTestFunction;
	
	HRESULT hr;
	V( m_device->SetRenderState(D3DRS_ALPHAFUNC, m_alphaTestFunction + 1) );
}


void Renderer::setAlphaTestValue(u8 p_alphaTestValue)
{
	DWORD val = p_alphaTestValue;
	
	HRESULT hr;
	V( m_device->SetRenderState(D3DRS_ALPHAREF, val) );
}


void Renderer::setAlphaTest(AlphaTestFunction p_alphaTestFunction, u8 p_alphaTestValue)
{
	setAlphaTestFunction(p_alphaTestFunction);
	setAlphaTestValue(p_alphaTestValue);
}


void Renderer::clearStencil(s32 p_value)
{
	checkD3DSucceeded( getRenderDevice(true)->Clear(0, 0, D3DCLEAR_STENCIL, D3DCOLOR(), 1.0f, p_value) );
}


void Renderer::setStencilEnabled(bool p_enabled)
{
	checkD3DSucceeded( getRenderDevice(true)->SetRenderState(D3DRS_STENCILENABLE,       p_enabled) );
	checkD3DSucceeded( getRenderDevice(true)->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, p_enabled) );
}


void Renderer::setStencilFunction(StencilSide p_side, StencilTestFunction p_function, s32 p_reference, s32 p_mask)
{
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device != 0)
	{
		checkD3DSucceeded( device->SetRenderState((p_side == StencilSide_Front) ?
			D3DRS_STENCILFUNC : D3DRS_CCW_STENCILFUNC , stencilFunc[p_function]) );
		checkD3DSucceeded( device->SetRenderState(D3DRS_STENCILREF,  p_reference) );
		checkD3DSucceeded( device->SetRenderState(D3DRS_STENCILMASK, p_mask) );
	}
}


void Renderer::setStencilOperation(StencilSide      p_side, StencilOperation  p_stencilFail,
                                   StencilOperation p_zFail, StencilOperation p_stencilPass)
{
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device != 0)
	{
		if (p_side == StencilSide_Front)
		{
			checkD3DSucceeded( device->SetRenderState(D3DRS_STENCILFAIL , stencilOp[p_stencilFail]) );
			checkD3DSucceeded( device->SetRenderState(D3DRS_STENCILZFAIL, stencilOp[p_zFail      ]) );
			checkD3DSucceeded( device->SetRenderState(D3DRS_STENCILPASS , stencilOp[p_stencilPass]) );
		}
		else
		{
			checkD3DSucceeded( device->SetRenderState(D3DRS_CCW_STENCILFAIL , stencilOp[p_stencilFail]) );
			checkD3DSucceeded( device->SetRenderState(D3DRS_CCW_STENCILZFAIL, stencilOp[p_zFail      ]) );
			checkD3DSucceeded( device->SetRenderState(D3DRS_CCW_STENCILPASS , stencilOp[p_stencilPass]) );
		}
	}
}


void Renderer::setTexture(const TexturePtr& p_texture)
{
	checkFromRenderThread();
	HRESULT hr;
	V( FP(m_device)->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0) );
	
	TT_ASSERTMSG(m_multiTextureDirtyStageRange == 0,
	             "a previous setMultiTexture() call was not restored yet, call resetMultiTexture() first.");
	
	// Change texture
	if (p_texture != 0)
	{
		// set texture stage color/alpha blend mode
		V( FP(m_device)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE) );
		V( FP(m_device)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE) );
		
		// activate texture in stage 0
		p_texture->select(0);
	}
	else
	{
		FixedFunction::disableTexture(0);
		MultiTexture::setActiveTexture(0,0);
		setPremultipliedAlphaEnabled(false);
	}
}


void Renderer::resetMultiTexture()
{
	FixedFunction::resetTextureStages();
}


scene::CameraPtr Renderer::setMainCamera(const scene::CameraPtr& p_camera,
                                         bool p_update,
                                         ViewPortID p_viewport)
{
	scene::CameraPtr oldCamera = ViewPort::getViewPort(p_viewport).getCamera();
	
	// Set the current camera
	TT_ASSERT(ViewPort::hasViewPort(p_viewport));
	ViewPort::getViewPorts()[p_viewport].setCamera(p_camera);
	
	if (p_camera != 0 && p_update)
	{
		p_camera->update();
	}
	
	if(m_isRendering)
	{
		// NOTE: changing camera's during rendering is not advisable
		//       this is basically a patch to fix Toki Tori
		m_activeCamera = p_camera;
		m_activeCamera->select();
	}
	
	return oldCamera;
}


scene::CameraPtr Renderer::getMainCamera(ViewPortID p_viewport) const
{
	TT_ASSERT(static_cast<std::size_t>(p_viewport) < ViewPort::getViewPorts().size());

	return ViewPort::getViewPorts()[p_viewport].getCamera();
}


void Renderer::setLighting(bool p_enable)
{
	// Store setting
	m_lightingEnabled = p_enable;

	if(p_enable == false)
	{
		FixedFunction::setLightingEnabled(false);
	}
}


// FIXME: setFog doesn't work anymore
void Renderer::setFog(const scene::FogPtr& p_fog)
{
	if(getRenderDevice() == 0) return;
	HRESULT hr;

	if(p_fog == 0)
	{
		// Disable fog blending.
		V( FP(m_device)->SetRenderState(FPRS(D3DRS_FOGENABLE), FALSE) );
	}
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


s32 Renderer::setAntiAliasing(bool p_enable, s32 p_samples)
{
	m_useAA     = p_enable;
	m_samplesAA = p_samples;
	
	if(m_useAA)
	{
		m_samplesAA = p_samples;
		
		// Try to find the nearest supported setting
		while(m_samplesAA > 1)
		{
			// Test to see if this is supported
			if(SUCCEEDED(DXUTGetD3D9Object()->CheckDeviceMultiSampleType(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				DXUTGetDeviceSettings().d3d9.AdapterFormat,
				DXUTIsWindowed(),
				static_cast<D3DMULTISAMPLE_TYPE>(m_samplesAA),
				0)))
			{
				// Found a workable setting
				break;
			}
			// Not supported, decrease nr of samples
			--m_samplesAA;
		}

		// 1 sample is not valid
		if(m_samplesAA <= 1)
		{
			m_samplesAA = 0;
		}
	}
	else
	{
		m_samplesAA = 0;
	}

	// NOTE: The changes only take effect after the next device change

	return m_samplesAA;
}


void Renderer::setZBufferEnabled(bool p_enabled)
{
	m_isZBufferEnabled = p_enabled;
	checkD3DSucceeded( m_device->SetRenderState(D3DRS_ZENABLE, m_isZBufferEnabled ? D3DZB_TRUE : D3DZB_FALSE) );
}


void Renderer::setDepthWriteEnabled(bool p_enabled)
{
	m_isZWriteEnabled = p_enabled;
	checkD3DSucceeded( m_device->SetRenderState(D3DRS_ZWRITEENABLE, m_isZWriteEnabled ? TRUE : FALSE) );
}


s32 Renderer::getScreenWidth() const
{
	return m_ios2xMode ? (m_screenSize.x / 2) : m_screenSize.x;
}


s32 Renderer::getScreenHeight() const
{
	return m_ios2xMode ? (m_screenSize.y / 2) : m_screenSize.y;
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
	
#if !defined(TT_BUILD_FINAL)
	/*
	if(m_debugRenderer->getDebugCameraActive())
	{
		getRenderDevice()->SetTransform(
			D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(
			m_debugRenderer->getDebugCamera()->GetViewMatrix()));
	}
	*/
#endif

	// Render skybox
	renderSkybox();

	m_debugRenderer->showAxis();
}


void Renderer::endViewPort(ViewPort& p_viewport)
{
	renderTransparents();

	// Apply post-processing
	if (m_postProcessingActive)
	{
		m_postProcessingActive = false;
		m_postProcessor->endFrame();
	}

	m_debugRenderer->flush();

	if(m_debugRenderer->getDebugCameraActive())
	{
		m_activeCamera->visualize();
	}

	p_viewport.end();
	
	// Go to next viewport if there is one
	const s32 totalViewports = static_cast<s32>(ViewPort::getViewPorts().size());
	if (m_currentViewport < (totalViewports - 1))
	{
		m_currentViewport = static_cast<ViewPortID>(m_currentViewport + 1);
	}
}


void Renderer::addTransparentObject(real p_distance,
									scene::SceneObject* p_object,
									const math::Matrix44& p_matrix)
{
	m_transparentObjects.insert(RenderObjects::value_type(p_distance, RenderObject(p_object, p_matrix)));
}


void Renderer::renderTransparents()
{
	if (m_transparentObjects.empty()) return;
	
	m_isRenderingTransparents = true;

	MatrixStack::getInstance()->push();
	
	for (RenderObjects::const_reverse_iterator it = m_transparentObjects.rbegin();
	     it != m_transparentObjects.rend(); ++it)
	{
		TT_NULL_ASSERT(it->second.first);

		// FIXME: This should be stored to still support scene info
		RenderContext rc;
		rc.pass = RenderPass_Transparents;
		HRESULT hr;
		
		// Render back faces... (Not always wanted, should be settable by data)
		/*
		V( m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW) );
		
		MatrixStack::getInstance()->load44(it->second.second);
		it->second.first->render(rc);*/
		
		// Render front faces...
		V( m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW) );
		
		MatrixStack::getInstance()->load44(it->second.second);
		it->second.first->render(rc);
	}
	
	MatrixStack::getInstance()->pop();
	
	// Clear the list
	m_transparentObjects.clear();
	
	m_isRenderingTransparents = false;
}


const math::Vector2& Renderer::getUpScaling() const
{
	return m_upScaler->getScaleFactor();
}


void Renderer::setScissorRect(const tt::math::PointRect& p_rect)
{
	// NOTE: Might want a faster way of conversion here
	RECT scissorRect;
	scissorRect.left   = p_rect.getLeft();
	scissorRect.top    = p_rect.getTop();
	scissorRect.right  = p_rect.getRight()  + 1;
	scissorRect.bottom = p_rect.getBottom() + 1;
	
	HRESULT hr;
	
	V( m_device->SetScissorRect(&scissorRect) );
	V( m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, true) );
}


void Renderer::resetScissorRect()
{
	m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
}


void Renderer::setCustomViewport(s32 p_x, s32 p_y, s32 p_width, s32 p_height, real p_near, real p_far)
{
	TT_NULL_ASSERT(getRenderDevice());
	
	// If running in iOS 2x (Retina) mode, upscale the coordinates and dimensions that were passed
	if (isIOS2xMode())
	{
		p_x      *= 2;
		p_y      *= 2;
		p_width  *= 2;
		p_height *= 2;
	}
	
	D3DVIEWPORT9 vp;
	vp.X      = p_x;
	vp.Y      = p_y;
	vp.Width  = p_width;
	vp.Height = p_height;
	vp.MinZ   = p_near;
	vp.MaxZ   = p_far;
	
	// Setup a custom viewport
	checkD3DSucceeded(m_device->SetViewport(&vp));
}


void Renderer::saveBackbuffer()
{
	if (m_backBuffer != 0)
	{
		// Already have a reference to the backbuffer
		return;
	}
	
	TT_NULL_ASSERT(m_device);
	if (checkD3DSucceeded(m_device->GetRenderTarget(0, &m_backBuffer)) == false)
	{
		return;
	}
	
	HRESULT hr = m_device->GetDepthStencilSurface(&m_backBufferDepth);
	if (hr != D3D_OK && hr != D3DERR_NOTFOUND)
	{
		TT_PANIC("Failed to acquire Depth Buffer (HRESULT = %d)", hr);
	}
}


void Renderer::restoreBackbuffer()
{
	if(m_backBuffer != 0)
	{
		checkD3DSucceeded( m_device->SetRenderTarget(0, m_backBuffer) );
		safeRelease(m_backBuffer);
	}
	
	if(m_backBufferDepth != 0)
	{
		checkD3DSucceeded( m_device->SetDepthStencilSurface(m_backBufferDepth) );
		safeRelease(m_backBufferDepth);
	}
}


void Renderer::handleCreateDevice()
{
	// Acquire current device
	m_device = getRenderDevice();
	TT_NULL_ASSERT(m_device);
	
	m_screenSize.x = DXUTGetDeviceSettings().d3d9.pp.BackBufferWidth;
	m_screenSize.y = DXUTGetDeviceSettings().d3d9.pp.BackBufferHeight;
	m_backBufferSize = m_screenSize;
	
	createDefaultCameras();
	ViewPort::createLayout(Layout_Standard);
	
	FullscreenTriangle::deviceCreated();
}


void Renderer::handleLostDevice()
{
	Shader::resetActiveShader();
}


bool Renderer::handleResetDevice()
{
	const ColorRGBA oldClearColor(m_clearColor);
	
	// Get target size from up-scaler
	const D3DPRESENT_PARAMETERS& params = DXUTGetDeviceSettings().d3d9.pp;
	m_backBufferSize = math::Point2(params.BackBufferWidth, params.BackBufferHeight);
	
	m_screenSize = m_upScaler->handleResolutionChanged(m_backBufferSize.x, m_backBufferSize.y);
	
	m_backBufferViewport.X = m_backBufferViewport.Y = 0;
	m_backBufferViewport.Width  = params.BackBufferWidth;
	m_backBufferViewport.Height = params.BackBufferHeight;
	m_backBufferViewport.MinZ = 0;
	m_backBufferViewport.MaxZ = 1;
	
	// Reset States
	reset();
	
	// Reset Lighting
	m_lightManager.reset();
	
	// Reset textures
	MultiTexture::resetAllActiveTextures();
	Texture::resetAllSamplerStates();
	
	// Create cameras & viewports
	createDefaultCameras();
	ViewPort::resetLayout();
	
	// Reconfigure post-processing
	m_postProcessor->handleAntiAliasingChanged(m_samplesAA);
	
	// Restore the clear color
	setClearColor(oldClearColor);
	
	// Reset fog
	resetFog();

	MatrixStack::getInstance()->resetTextureMatrix();
	FixedFunction::setActive();
	
	return true;
}


void Renderer::handleDestroyDevice()
{
	FullscreenTriangle::deviceDestroyed();
}


void Renderer::checkFromRenderThread() const
{
	TT_ASSERTMSG(m_renderThreadID == GetCurrentThreadId(),
		"This function should only be called from the render thread.");
}


u32 Renderer::getFVFFromVertexType(u32 p_type)
{
	if (m_vertexAttributeMap.find(p_type) == m_vertexAttributeMap.end())
	{
		// Add new vertex type to the map
		u32 fvf = D3DFVF_XYZ;
		
		if ((VertexBuffer::Property_Normal  & p_type) != 0) fvf |= D3DFVF_NORMAL;
		if ((VertexBuffer::Property_Diffuse & p_type) != 0) fvf |= D3DFVF_DIFFUSE;
		
		s32 count = 0;
		static const s32 maxSupportedTextureCoords = 4;
		
		for(s32 i = 0; i < maxSupportedTextureCoords; ++i)
		{
			if ((p_type & (VertexBuffer::Property_Texture0 << i)) != 0)
			{
				fvf |= D3DFVF_TEXCOORDSIZE2(i);
				++count;
			}
		}
		
		fvf |= (count << D3DFVF_TEXCOUNT_SHIFT);
		
		m_vertexAttributeMap[p_type] = fvf;
	}
	
	return m_vertexAttributeMap[p_type];
}


void Renderer::addToDeathRow(IUnknown* p_comObject)
{
	if (p_comObject == 0) return;
	
	thread::CriticalSection criticalSection(&ms_deathRowMutex);
	ms_deathRow.push_back(p_comObject);
}


void Renderer::clearDeathRow()
{
	thread::CriticalSection criticalSection(&ms_deathRowMutex);
	for (ComDeathRow::iterator it = ms_deathRow.begin(); it != ms_deathRow.end(); ++it)
	{
		safeRelease(*it);
	}
	ms_deathRow.clear();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Renderer::Renderer(bool p_ios2xMode)
:
m_isRendering(false),
m_isRenderingHud(false),
m_useAA(false),
m_lightingEnabled(false),
m_fogEnabled(false),
m_fogMode(FogMode_Linear),
m_fogColor(ColorRGB::white),
m_isZBufferEnabled(true),
m_isZWriteEnabled(true),
m_deltaTime(0),
m_time(0),
m_frame(0),
m_samplesAA(0),
m_srcFactor(BlendFactor_SrcAlpha),
m_dstFactor(BlendFactor_InvSrcAlpha),
m_separateAlphaBlendEnabled(false),
m_srcFactorAlpha(BlendFactor_Zero),
m_dstFactorAlpha(BlendFactor_Zero),
m_premultipliedAlpha(false),
m_alphaTestEnabled(false),
m_alphaTestFunction(AlphaTestFunction_Never),
m_alphaTestValue(0),
m_cullFrontOrder(CullFrontOrder_ClockWise),
m_cullMode(CullMode_Back),
m_cullingEnabled(true),
m_fillMode(FillMode_Solid),
m_colorMask(ColorMask_None),
m_lastViewPort(false),
m_currentViewport(ViewPortID_1),
m_vertexType(0),
m_clearColor(ColorRGB::black),
m_isRenderingTransparents(false),
m_debugRenderer(new debug::DebugRenderer),
m_postProcessor(new pp::PostProcessor),
m_postProcessingActive(false),
m_upScaler(new UpScaler(math::Point2::zero)),
m_upScalerActive(false),
m_lowPerformanceMode(false),
m_device(0),
m_backBuffer(0),
m_backBufferDepth(0),
m_renderThreadID(GetCurrentThreadId()),
m_multiTextureDirtyStageRange(0),
m_ios2xMode(p_ios2xMode)
{
	TT_NULL_ASSERT(m_upScaler);
	
	// Initialize matrix stack
	MatrixStack::createInstance();
	
	// Initialize file utilities
	file::FileUtils::createInstance();
	
	MultiTexture::setChannelCount(8); // FIXME: Should check the hardware to find out how many channels are supported.
	
	// Fog defaults
	m_fogSettings[FogSetting_Start]   = 0.0f;
	m_fogSettings[FogSetting_End]     = 1.0f;
	m_fogSettings[FogSetting_Density] = 1.0f;
}


Renderer::~Renderer()
{
	MultiTexture::resetChannelCount();
	
	// Destroy FileUtils Singleton
	file::FileUtils::destroyInstance();
	
	// Destroy Matrix Stack Singleton
	MatrixStack::destroyInstance();
}


void Renderer::createDefaultCameras()
{
	const real width  = static_cast<real>(getScreenWidth ());
	const real height = static_cast<real>(getScreenHeight());
	
	// Create HUD camera
	const real hudX =  width  / 2.0f;
	const real hudY = -height / 2.0f;
	const math::Vector3 pos   (hudX, hudY, 500);
	const math::Vector3 lookAt(hudX, hudY,   0);
	m_hudCamera = scene::CameraPtr(new scene::Camera(pos, lookAt, width, height, 1.0f, 4096.0f));
	m_hudCamera->update();
}


void Renderer::renderSkybox()
{
	if(m_skybox == 0) return;
	
	// Disable Z-writes / checks
	HRESULT hr;
	V( m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE) );
	V( m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) );
	
	TT_NULL_ASSERT(m_activeCamera);
	
	// Render @ camera position
	MatrixStack* stack = MatrixStack::getInstance();
	stack->push();
	stack->translate(m_activeCamera->getActualPosition());
	m_skybox->render();
	stack->pop();
	
	// If skybox contains any transparent objects, render those now as well
	renderTransparents();
	
	// Restore Z-writes / checks
	if(m_isZBufferEnabled)
	{
		V( m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE) );
	}
	if (m_isZWriteEnabled)
	{
		V( m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE) );
	}
}


void Renderer::updateCulling() const
{
	D3DCULL cullMode;
	
	if (m_cullingEnabled == false)
	{
		cullMode = D3DCULL_NONE;
	}
	else
	{
		/* 
		DirectX 9 has a constant frontface order of clockwise
		and a default culling of backface (CCW culling assuming the above)
		*/
		cullMode = cullModeLookup[m_cullFrontOrder][m_cullMode];
	}
	
	TT_NULL_ASSERT(m_device);
	
	HRESULT hr;
	V( m_device->SetRenderState(D3DRS_CULLMODE, cullMode) );
}


void Renderer::updateBlendModes()
{
	// Override for premultiplied alpha
	BlendFactor srcColorFactor(m_srcFactor);
	if(m_premultipliedAlpha && srcColorFactor == BlendFactor_SrcAlpha)
	{
		srcColorFactor = BlendFactor_One;
	}

	// NOTE: This code depends on the order of the BlendFactor enum (and the DirectX D3DBLEND enum)
	HRESULT hr;

	V( m_device->SetRenderState(D3DRS_SRCBLEND,  static_cast<D3DBLEND>(srcColorFactor + 1)) );
	V( m_device->SetRenderState(D3DRS_DESTBLEND, static_cast<D3DBLEND>(m_dstFactor + 1)) );

	if(m_separateAlphaBlendEnabled)
	{
		V( m_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE) );
		V( m_device->SetRenderState(D3DRS_SRCBLENDALPHA,  static_cast<D3DBLEND>(m_srcFactorAlpha + 1)) );
		V( m_device->SetRenderState(D3DRS_DESTBLENDALPHA, static_cast<D3DBLEND>(m_dstFactorAlpha + 1)) );
	}
	else
	{
		V( m_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE) );
	}
}


void Renderer::resetFog()
{
	FixedFunction::setFogEnabled(m_fogEnabled);
	FixedFunction::setFogMode   (m_fogMode);
	FixedFunction::setFogColor  (m_fogColor);
	
	// Set settings
	for (s32 i = 0; i < FogSetting_Count; ++i)
	{
		FixedFunction::setFogSetting(static_cast<FogSetting>(i), m_fogSettings[i]);
	}
}


bool Renderer::setTextureStage(u32 p_index, const TextureStageData& p_textureStageData, bool p_resetting)
{
	checkFromRenderThread();
	if (p_index > MultiTexture::getChannelCount() - 1)
	{
		TT_PANIC("texture stage out of bounds (%u, max = %u)", p_index, MultiTexture::getChannelCount() - 1);
		return false;
	}
	
	// if resetting, an empty texture is allowed
	if (p_resetting == false)
	{
		// check if texture exists
		// a texture stage does not need a texture necessarily, if needed, this requirement can be dropped
		if (p_textureStageData.getTexture() == 0)
		{
			TT_PANIC("Texture not set for multitexture stage %u", p_index);
			return false;
		}
		else
		{
			// apply texture to current stage
			p_textureStageData.getTexture()->select(p_index);
		}
	}
	else
	{
		// Remember which Texture is active in which channel to minimize texture switches.
		// There is no need to switch while keeping the same texture.
		MultiTexture::resetActiveTexture(p_index);
	}
	
	HRESULT hr;
	
	// texture coordinates set index
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_TEXCOORDINDEX, p_textureStageData.getTexCoordIndex()) );
	
	// color/alpha blend mode
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_COLOROP,
	                                  blendOp[p_textureStageData.getColorBlendOperation()]) );
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_ALPHAOP,
	                                  blendOp[p_textureStageData.getAlphaBlendOperation()]) );
	
	// color sources
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_COLORARG1,
	                                  blendSrc[p_textureStageData.getColorSource1()]) );
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_COLORARG2,
	                                  blendSrc[p_textureStageData.getColorSource2()]) );
	// alpha sources
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_ALPHAARG1,
	                                  blendSrc[p_textureStageData.getAlphaSource1()]) );
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_ALPHAARG2,
	                                  blendSrc[p_textureStageData.getAlphaSource2()]) );
	
	// constant color
	const ColorRGBA& constantRGBA(p_textureStageData.getConstant());
	DWORD constantARGB = D3DCOLOR_ARGB(constantRGBA.a, constantRGBA.r, constantRGBA.g, constantRGBA.b);
	V( FP(m_device)->SetTextureStageState(p_index, D3DTSS_CONSTANT, constantARGB) );
	
	// texture transformation matrix preperation
	// if only resetting, the MatrixStack does not need a reset
	if (p_resetting == false && p_index == 0)
	{
		// the texture transformation matrix of stage 0 is also controlled by MatrixStack, reset MatrixStack
		MatrixStack::getInstance()->resetTextureMatrix();
	}
	
	// texture transformation matrix
	// if only resetting, do not copy the matrix (this assumes the default value is an identity matrix,
	// which has zero m_31,m_32,m_41,m_42, otherwise the 'convert to windows style' doés need to take place)
	if (p_resetting == false)
	{
		math::Matrix44 texMatrix(p_textureStageData.getMatrix());
		// convert to windows style
		texMatrix.m_31 = texMatrix.m_41;
		texMatrix.m_32 = texMatrix.m_42;
		
		V( FP(m_device)->SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + p_index),
		                          reinterpret_cast<const D3DXMATRIX*>(&texMatrix)) );
	}
	else
	{
		V( FP(m_device)->SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + p_index),
		                          reinterpret_cast<const D3DXMATRIX*>(&p_textureStageData.getMatrix())) );
	}
	
	return true;
}


void Renderer::resetTextureStages(u32 p_indexStart, u32 p_indexEnd)
{
	if (p_indexStart > MultiTexture::getChannelCount() || p_indexEnd > MultiTexture::getChannelCount())
	{
		TT_PANIC("index/indices out of bounds: p_indexStart = %u, p_indexEnd = %u, max textures = %u",
		         p_indexStart, p_indexEnd, MultiTexture::getChannelCount());
	}
	TT_ASSERTMSG(p_indexStart <= p_indexEnd, "start index (%u) is higher than end index (%u)",
	             p_indexStart, p_indexEnd);
	
	TextureStageData defaultStage;
	defaultStage.setAlphaBlendOperation(TextureBlendOperation_Disable);
	defaultStage.setColorBlendOperation(TextureBlendOperation_Disable);
	
	for (u32 i = p_indexStart; i < p_indexEnd; ++i)
	{
		defaultStage.setTexCoordIndex(i);
		if (setTextureStage(i, defaultStage, true) == false)
		{
			TT_PANIC("resetting texture stage %u failed", i);
		}
	}
}


// Namespace end
}
}
}
