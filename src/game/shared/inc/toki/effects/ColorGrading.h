#if !defined(INC_TOKI_EFFECTS_COLORGRADING_H)
#define INC_TOKI_EFFECTS_COLORGRADING_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/pp/Effect.h>
#include <tt/math/Vector4.h>


namespace toki {
namespace effects {


class ColorGrading : public tt::engine::renderer::pp::Effect
{
public:
	enum Lookup
	{
		Lookup_One,
		Lookup_Two,
		
		Lookup_Count
	};
	static inline bool isValidLookup(Lookup p_index) { return p_index >= 0 && p_index < Lookup_Count; }
	static const char* const getLookupName(Lookup p_index);
	
	ColorGrading();
	~ColorGrading() {}
	
	void setLookupTexture( const tt::engine::renderer::TexturePtr& p_texture, Lookup p_index);
	void setLerpParameters(real p_lerpLookup, real p_lerpWithOriginal);
	
	virtual void setFrameParameters(u32 p_filterIndex);
	virtual void acquireHandles();
	
private:
	tt::engine::renderer::TexturePtr m_lookupTexture     [Lookup_Count];
	s32                              m_lookupSamplerIndex[Lookup_Count];
	tt::math::Vector4                m_lerpValues;
	
	tt::engine::renderer::ShaderPtr  m_shader;
};


}
}

#endif // INC_TOKI_EFFECTS_COLORGRADING_H
