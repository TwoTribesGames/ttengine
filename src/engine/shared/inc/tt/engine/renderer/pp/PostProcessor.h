#if !defined(INC_TT_ENGINE_RENDERER_PP_POSTPROCESSOR_H)
#define INC_TT_ENGINE_RENDERER_PP_POSTPROCESSOR_H


#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/pp/fwd.h>


namespace tt {
namespace engine {
namespace renderer {
namespace pp {


enum UseDepthBuffer
{
	UseDepthBuffer_Enabled,
	UseDepthBuffer_Disabled
};


class PostProcessor
{
public:
	PostProcessor();
	~PostProcessor() {}
	
	void initialize(const FilterPtr& p_default, UseDepthBuffer p_useDepthBuffer);
	void deinitialize();
	
	void handleAntiAliasingChanged(u32 p_sampleCount);
	
	void beginFrame(u32 p_clearFlags);
	void endFrame();
	
	inline void setActive(bool p_active) { m_active = p_active; }
	bool isActive() const;

	RenderTargetPtr getCurrentTarget();
	
	void handleClearColorChanged(const ColorRGBA& p_color);
	
private:
	friend class Effect;
	void prepare(Effect* p_effect, bool p_isLastEffect);
	void setRenderTarget(u32 p_clearFlags);
	
	RenderTargetPtr m_pingTarget;
	RenderTargetPtr m_pongTarget;
	RenderTargetPtr m_aaTarget;
	
	bool m_initialized;
	bool m_active;
	bool m_resetParameters;
	bool m_useDepthBuffer;
	bool m_frameActive;
	s32  m_targetID;
	
	FilterPtr m_default;
};


// Namespace end
}
}
}
}


#endif  // INC_TT_ENGINE_RENDERER_PP_POSTPROCESSOR_H
