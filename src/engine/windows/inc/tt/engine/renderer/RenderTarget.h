#if !defined(INC_TT_ENGINE_RENDERER_PP_RENDERTARGET_H)
#define INC_TT_ENGINE_RENDERER_PP_RENDERTARGET_H

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/pp/fwd.h>
#include <tt/engine/renderer/fwd.h>

struct IDirect3DSurface9;

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


class RenderTarget : public D3DResource
{
public:
	virtual ~RenderTarget();
	
	void setClearColor(const ColorRGBA& p_color) {m_clearColor = p_color;}
	
	TexturePtr getTexture();
	
	void selectTexture(s32 p_samplerUnit);
	
	virtual void deviceLost();
	virtual void deviceReset();
	
	static RenderTargetPtr create(s32  p_width,
	                              s32  p_height,
	                              s32  p_samples = 0,
	                              bool p_hasDepthBuffer = false);
	
	static RenderTargetPtr createFromBackBuffer(bool p_hasDepthBuffer = false,
	                                            s32  p_samples = 0,
	                                            real p_ratio = 1.0f);
	
private:
	RenderTarget(s32  p_width,
	             s32  p_height,
	             bool p_fromBackBuffer,
	             bool p_hasDepthBuffer,
	             s32  p_samples,
	             real p_ratio);
	
	void setActive(u32 p_clearFlags = ClearFlag_All);
	inline void setInactive() {}
	
	ColorRGBA m_clearColor;
	s32       m_width;
	s32       m_height;
	s32       m_samples;
	s32       m_samplerUnit;
	bool      m_hasDepthBuffer;
	bool      m_backBufferSize;
	real      m_ratioToBackbuffer;
	
	IDirect3DSurface9*  m_colorBuffer;
	IDirect3DSurface9*  m_depthBuffer;
	
	TexturePtr m_texture;
	
	// Only RenderTargetStack should be able to activate RenderTargets
	friend class RenderTargetStack;
};



}
}
}

#endif //INC_TT_ENGINE_RENDERER_PP_RENDERTARGET_H
