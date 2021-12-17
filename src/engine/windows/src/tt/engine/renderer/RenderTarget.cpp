
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {


RenderTarget::~RenderTarget()
{
	D3DResourceRegistry::unregisterResource(this);
	Renderer::getInstance()->addToDeathRow(m_colorBuffer);
	Renderer::getInstance()->addToDeathRow(m_depthBuffer);
}


TexturePtr RenderTarget::getTexture()
{
	// If we are using an AA render target, copy the contents first
	if(m_samples > 1 && m_texture != 0)
	{
		// Acquire the texture surface
		IDirect3DTexture9* d3dTexture = static_cast<IDirect3DTexture9*>(m_texture->getD3DTexture());
		
		if (d3dTexture != 0 && m_colorBuffer != 0)
		{
			IDirect3DSurface9* textureSurface(0);
			
			if (checkD3DSucceeded( d3dTexture->GetSurfaceLevel(0, &textureSurface) ))
			{
				// Copy the contents of the render target into the texture surface
				checkD3DSucceeded(
					getRenderDevice(true)->StretchRect(m_colorBuffer, 0, textureSurface, 0, D3DTEXF_LINEAR) );
				
				// Release reference to texture surface
				textureSurface->Release();
			}
		}
	}

	return m_texture;
}


void RenderTarget::selectTexture(s32 p_samplerUnit)
{
	getTexture()->select(p_samplerUnit);
	m_samplerUnit = p_samplerUnit;
}


void RenderTarget::deviceLost()
{
	// Release all D3D resources
	safeRelease(m_colorBuffer);
	safeRelease(m_depthBuffer);
}


void RenderTarget::deviceReset()
{
	// NOTE: This function can be called twice 
	// (from PostProcess->reset() [constructor] & D3DResourceRegistry)
	// Therefore only create if necessary

	if(m_colorBuffer == 0)
	{
		// Get actual backbuffer size -> Render Target size is automatically the same as backbuffer
		if(m_backBufferSize)
		{
			m_width  = static_cast<s32>(m_ratioToBackbuffer * Renderer::getInstance()->getScreenWidth());
			m_height = static_cast<s32>(m_ratioToBackbuffer * Renderer::getInstance()->getScreenHeight());
		}
		
		// Resize render target texture
		m_texture->resize(m_width, m_height);
		
		if (m_texture == 0) return;
		
		// Handle AA: for non-AA RTs we can render directly into the texture surface, but
		//            multisampled (AA) render targets must be created specifically
		if(m_samples == 0)
		{
			IDirect3DTexture9* d3dTexture = static_cast<IDirect3DTexture9*>(m_texture->getD3DTexture());
			
			if (d3dTexture != 0)
			{
				checkD3DSucceeded( d3dTexture->GetSurfaceLevel(0, &m_colorBuffer) );
			}
		}
		else
		{
			checkD3DSucceeded(
				getRenderDevice(true)->CreateRenderTarget(
					m_width, m_height, D3DFMT_A8R8G8B8,
					static_cast<D3DMULTISAMPLE_TYPE>(m_samples), 0, false, &m_colorBuffer, 0) );
		}
		TT_NULL_ASSERT(m_colorBuffer);
	}
	
	// A depth buffer is only needed if we render a real 3D scene into the target
	// The multisample format must match the format of the color buffer
	if(m_hasDepthBuffer && m_depthBuffer == 0)
	{
		checkD3DSucceeded(
			getRenderDevice(true)->CreateDepthStencilSurface(
				m_width, m_height, D3DFMT_D24S8,
				static_cast<D3DMULTISAMPLE_TYPE>(m_samples), 0, true, &m_depthBuffer, 0) );
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


///////////////////////////////////////////
// Private member functions

RenderTarget::RenderTarget(s32 p_width, s32 p_height, bool p_fromBackBuffer,
                           bool p_hasDepthBuffer, s32 p_samples, real p_ratio)
:
m_width(p_width),
m_height(p_height),
m_samples(p_samples),
m_samplerUnit(-1),
m_hasDepthBuffer(p_hasDepthBuffer),
m_backBufferSize(p_fromBackBuffer),
m_ratioToBackbuffer(p_ratio),
m_colorBuffer(0),
m_depthBuffer(0)
{
	// 1 is not a valid sample count
	if(m_samples == 1)
	{
		m_samples = 0;
	}
	
	// Register after texture resource so order should be correct
	D3DResourceRegistry::registerResource(this);
	
	m_texture = Texture::createForRenderTarget(static_cast<s16>(m_width), static_cast<s16>(m_height));
	
	if(getRenderDevice() != 0)
	{
		// Create device resources
		deviceReset();
	}
}


void RenderTarget::setActive(u32 p_clearFlags)
{
	Renderer::getInstance()->checkFromRenderThread();
	
	// Should not set a render target without a valid color buffer
	// (unless we want to do a depth pre-pass in the future...)
	if (m_colorBuffer == 0) return;
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == 0) return;
	
	// Make sure the texture is not bound anymore, before using it to render into
	if (m_samplerUnit >= 0)
	{
		if (m_texture.get() == MultiTexture::getActiveTexture(m_samplerUnit))
		{
			MultiTexture::resetActiveTexture(m_samplerUnit);
			device->SetTexture(m_samplerUnit, 0);
			m_samplerUnit = -1;
		}
	}

	// Set color buffer
	checkD3DSucceeded( device->SetRenderTarget(0, m_colorBuffer) );
	
	// Set depth buffer
	checkD3DSucceeded( device->SetDepthStencilSurface(m_depthBuffer) );
	
	if(p_clearFlags != ClearFlag_DontClear)
	{
		DWORD clearFlags(0);

		if ((p_clearFlags & ClearFlag_ColorBuffer) == ClearFlag_ColorBuffer)
		{
			clearFlags |= D3DCLEAR_TARGET;
		}

		if(m_hasDepthBuffer && (p_clearFlags & ClearFlag_DepthBuffer) == ClearFlag_DepthBuffer)
		{
			clearFlags |= D3DCLEAR_ZBUFFER;
		}

		checkD3DSucceeded( device->Clear(0, 0, clearFlags,
			D3DCOLOR_ARGB(m_clearColor.a, m_clearColor.r, m_clearColor.g, m_clearColor.b), 1.0f, 0) );
	}
}


}
}
}
