#if !defined(INC_TT_ENGINE_RENDERER_PP_EFFECT_H)
#define INC_TT_ENGINE_RENDERER_PP_EFFECT_H

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/pp/Filter.h>

namespace tt {
namespace engine {
namespace renderer {
namespace pp {


class Effect
{
public:
	Effect()
	:
	m_enabled(false)
	{}
	virtual ~Effect() {}

	virtual void setConstantParameters() {}
	virtual void setFrameParameters(u32 /*p_filterIndex*/) {}
	virtual void acquireHandles() {}
	virtual void update() {}

	void setInput (const TexturePtr& p_input)       { m_original = p_input;  }
	void setOutput(const RenderTargetPtr& p_output) { m_result   = p_output; }

	void apply(bool p_isLastEffect = false);

	void setEnabled(bool p_enable);
	bool isEnabled() const { return m_enabled; }

protected:
	typedef std::vector<FilterPtr> Filters;
	Filters m_filters;

	typedef std::vector<RenderTargetPtr> RenderTargets;
	RenderTargets m_renderTargets;

private:
	bool m_enabled;

	TextureWeakPtr      m_original;
	RenderTargetWeakPtr m_result;
};


}
}
}
}

#endif // INC_TT_ENGINE_RENDERER_PP_EFFECT_H
