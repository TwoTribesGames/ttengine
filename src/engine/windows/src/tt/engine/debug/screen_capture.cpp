#include <tt/engine/debug/screen_capture.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/Renderer.h>


namespace tt {
namespace engine {
namespace debug {


static IDirect3DSurface9* captureSurface()
{
	IDirect3DSurface9* result(nullptr);
	
	IDirect3DDevice9* device = renderer::getRenderDevice();
	
	if (device != 0)
	{
		renderer::checkD3DSucceeded( device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &result) );
	}
	return result;
}


bool saveScreenCaptureToFile(const std::string& p_filename, bool p_withTransparency)
{
	renderer::Renderer::getInstance()->checkFromRenderThread();
	IDirect3DSurface9* screenCapture = captureSurface();
	
	if (screenCapture == nullptr)
	{
		return false;
	}
	
	IDirect3DSurface9* surfaceToSave = screenCapture;
	
	if (p_withTransparency == false)
	{
		// Copy the back buffer to a surface that has no alpha channel, to strip it of transparency
		using renderer::checkD3DSucceeded;
		
		D3DSURFACE_DESC desc;
		if (checkD3DSucceeded(screenCapture->GetDesc(&desc)) == false)
		{
			safeRelease(screenCapture);
			return false;
		}
		
		IDirect3DDevice9* device = renderer::getRenderDevice(true);
		
		if (checkD3DSucceeded( device->CreateOffscreenPlainSurface(
			desc.Width, desc.Height, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &surfaceToSave, 0) ) == false)
		{
			safeRelease(screenCapture);
			return false;
		}
		
		if (checkD3DSucceeded( D3DXLoadSurfaceFromSurface(surfaceToSave, 0, 0, screenCapture, 0, 0, D3DX_DEFAULT, 0) ) == false)
		{
			safeRelease(screenCapture);
			safeRelease(surfaceToSave);
			return false;
		}
		
		safeRelease(screenCapture);
	}
	
	
	bool succes = renderer::checkD3DSucceeded(
		D3DXSaveSurfaceToFileA(p_filename.c_str(), D3DXIFF_PNG, surfaceToSave, 0, 0) );
	
	safeRelease(surfaceToSave);
	
	return succes;
}


bool saveScreenCaptureToClipboard()
{
	renderer::Renderer::getInstance()->checkFromRenderThread();
	IDirect3DSurface9* screenCapture = captureSurface();
	
	if (screenCapture == nullptr)
	{
		return false;
	}
	
	using renderer::checkD3DSucceeded;

	D3DSURFACE_DESC desc;
	if (checkD3DSucceeded(screenCapture->GetDesc(&desc)) == false)
	{
		safeRelease(screenCapture);
		return false;
	}
	
	IDirect3DSurface9* dstSurface(nullptr);
	IDirect3DDevice9* device = renderer::getRenderDevice(true);

	if (checkD3DSucceeded( device->CreateOffscreenPlainSurface(
		desc.Width, desc.Height, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &dstSurface, 0) ) == false)
	{
		safeRelease(screenCapture);
		return false;
	}
	
	if (checkD3DSucceeded( D3DXLoadSurfaceFromSurface(dstSurface, 0, 0, screenCapture, 0, 0, D3DX_DEFAULT, 0) ) == false)
	{
		safeRelease(screenCapture);
		safeRelease(dstSurface);
		return false;
	}
	
	safeRelease(screenCapture);
	
	HDC captureDC;
	if (checkD3DSucceeded(dstSurface->GetDC(&captureDC)) == false)
	{
		safeRelease(dstSurface);
		return false;
	}
	
	HDC memoryDC = CreateCompatibleDC(captureDC);
	
	HBITMAP bitmap = CreateCompatibleBitmap(captureDC, desc.Width, desc.Height);
	
	HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memoryDC, bitmap));
	
	BitBlt(memoryDC, 0, 0, desc.Width, desc.Height, captureDC, 0, 0, SRCCOPY);
	
	if (OpenClipboard(DXUTGetHWND()))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, bitmap);
		CloseClipboard();
	}
	
	// Restore previous value
	bitmap = static_cast<HBITMAP>(::SelectObject(memoryDC, oldBitmap));

	checkD3DSucceeded( dstSurface->ReleaseDC(captureDC) );
	safeRelease(dstSurface);
	DeleteDC(memoryDC);
	
	return true;
}


// Namespace end
}
}
}