#include <tt/engine/renderer/gpu_capabilities.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/math/Vector4.h>
#include <toki/effects/ColorGrading.h>


namespace toki {
namespace effects {


const char* const ColorGrading::getLookupName(Lookup p_index)
{
	switch (p_index)
	{
	case Lookup_One: return "One";
	case Lookup_Two: return "Two";
	default:
		TT_PANIC("Unknown Lookup: %d\n");
		return "";
	}
}


ColorGrading::ColorGrading()
:
m_lerpValues(0,0,0,0),
m_shader()
{
	using namespace tt::engine::renderer;
	
	for (s32 i = 0; i < Lookup_Count; ++i)
	{
		m_lookupTexture     [i].reset();
		m_lookupSamplerIndex[i] = -1;
	}
	
	m_shader = ShaderCache::get("ColorGrading", "shaders");
	TT_NULL_ASSERT(m_shader);
	
	if(m_shader != 0)
	{
		m_filters.push_back(pp::Filter::create(m_shader));
	}
	
	// This is needed for OSX (shader is selected when created)
	Shader::resetActiveShader();
}


void ColorGrading::setLookupTexture(const tt::engine::renderer::TexturePtr& p_texture, Lookup p_index)
{
	TT_ASSERT(isValidLookup(p_index));
	TT_ASSERT(p_index != Lookup_One || p_texture != 0); // lookup one may NOT be null.
	m_lookupTexture [p_index] = p_texture;
}


void ColorGrading::setLerpParameters(real p_lerpLookup, real p_lerpWithOriginal)
{
	m_lerpValues.x = p_lerpLookup;       // Lerp between 3D textures 0 = second texture, 1 = first texture
	m_lerpValues.y = p_lerpWithOriginal; // Lerp between previous result and original 0 = prev, 1 = original
}


void ColorGrading::acquireHandles()
{
	if (m_shader != 0)
	{
		const std::string baseName("ColorLookup3D");
		for (s32 i = 0; i < Lookup_Count; ++i)
		{
			Lookup lookup = static_cast<Lookup>(i);
			const std::string name(baseName + getLookupName(lookup));
			m_lookupSamplerIndex[i] = m_shader->getSamplerIndex(name);
			TT_ASSERTMSG(m_lookupSamplerIndex[i] >= 0, "Could not find sampler '%s'", name.c_str());
		}
	}
}


void ColorGrading::setFrameParameters(u32 /*p_filterIndex*/)
{
	TT_NULL_ASSERT(m_shader);
	
	std::string name("lookupStrength");
	bool result = m_shader->setParameter(name, m_lerpValues);
	TT_ASSERTMSG(result, "Failed to set ColorGrading effect shader parameter: '%s'!", name.c_str());
	
	for (s32 i = 0; i < Lookup_Count; ++i)
	{
		if (m_lookupTexture[i] != 0)
		{
			m_lookupTexture[i]->select(static_cast<u32>(m_lookupSamplerIndex[i]));
		}
		else
		{
			// If no second texture is used, bind first texture to this sampler as well
			// The first texture should always be set
			TT_NULL_ASSERT(m_lookupTexture[0]);
			m_lookupTexture[0]->select(static_cast<u32>(m_lookupSamplerIndex[i]));
		}
	}
}


// Namespace end
}
}


