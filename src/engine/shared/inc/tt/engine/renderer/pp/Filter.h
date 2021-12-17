#if !defined(INC_TT_ENGINE_RENDERER_PP_FILTER_H)
#define INC_TT_ENGINE_RENDERER_PP_FILTER_H


#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/pp/fwd.h>


namespace tt {
namespace engine {
namespace renderer {
namespace pp {


enum TargetBlend
{
	TargetBlend_Blend,
	TargetBlend_Replace
};


class Filter
{
public:
	~Filter() {}

	void setInput (const TexturePtr& p_source);
	inline void setOutput(const RenderTargetPtr& p_target) { m_output = p_target; }

	void apply(TargetBlend p_targetBlend = TargetBlend_Blend);

	inline ShaderPtr getShader() const { return m_shader; }

	static FilterPtr create(const ShaderPtr& p_shader);
	
	inline bool hasOriginalSampler() { return m_originalSampler >= 0; }
	inline u32  getOriginalSampler() { return static_cast<u32>(m_originalSampler); }
private:
	Filter(const ShaderPtr& p_shader);

	ShaderPtr       m_shader;
	TexturePtr      m_input;
	RenderTargetPtr m_output;

	s32    m_screenBufferSampler;
	s32    m_originalSampler;
};


}
}
}
}

#endif //INC_TT_ENGINE_RENDERER_PP_FILTER_H
