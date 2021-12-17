#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/gpu_capabilities.h>


namespace tt {
namespace engine {
namespace renderer {


bool getDeviceCapabilities(D3DCAPS9& p_capabilities)
{
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == 0 ||
	    checkD3DSucceeded( device->GetDeviceCaps(&p_capabilities) ) == false)
	{
		return false;
	}
	return true;
}


bool hasShaderSupport()
{
	D3DCAPS9 capabilities;
	if (getDeviceCapabilities(capabilities) == false)
	{
		return false;
	}
	
	// We need SM 2.0 or higher
	return capabilities.PixelShaderVersion >= D3DPS_VERSION(2, 0);
}


u32 getMaxTextureWidth()
{
	D3DCAPS9 capabilities;
	if (getDeviceCapabilities(capabilities) == false)
	{
		return false;
	}
	
	return capabilities.MaxTextureWidth;
}


NPOTSupport getNonPowerOfTwoSupport()
{
	D3DCAPS9 capabilities;
	if (getDeviceCapabilities(capabilities) == false)
	{
		return NPOTSupport_None;
	}
	
	if ((capabilities.TextureCaps & D3DPTEXTURECAPS_POW2) != 0)
	{
		if ((capabilities.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0)
		{
			// * Only CLAMP address mode is supported
			// * Mipmapping is not supported
			// * DXT compression is not supported
			return NPOTSupport_Limited;
		}

		return NPOTSupport_None;
	}

	return NPOTSupport_Full;
}


bool hasStencilBufferSupport()
{
	D3DCAPS9 capabilities;
	if (getDeviceCapabilities(capabilities) == false)
	{
		return false;
	}
	
	const DWORD StencilCapsNeeded = D3DSTENCILCAPS_KEEP     | D3DSTENCILCAPS_REPLACE |
	                                D3DSTENCILCAPS_INCR     | D3DSTENCILCAPS_DECR    |
	                                D3DSTENCILCAPS_TWOSIDED | D3DSTENCILCAPS_REPLACE;
	
	if ((capabilities.StencilCaps & StencilCapsNeeded ) != StencilCapsNeeded)
	{
		// Not all stencil capabilities avaliable.
		return false;
	}
	
	return true;
}


// Namespace end
}
}
}
