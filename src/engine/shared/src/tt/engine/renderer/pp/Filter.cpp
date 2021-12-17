
#include <tt/engine/renderer/FullscreenTriangle.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/pp/Filter.h>


namespace tt {
namespace engine {
namespace renderer {
namespace pp {


void Filter::setInput(const TexturePtr& p_texture)
{
	TT_NULL_ASSERT(p_texture);
	m_input = p_texture;
	m_input->setFilterMode(FilterMode_Linear, FilterMode_Linear);
	m_input->setAddressMode(AddressMode_Clamp, AddressMode_Clamp);

	math::Vector4 inverseScreenSize(1.0f / m_input->getWidth(), 1.0f / m_input->getHeight(), 0.0f, 0.0f);
#ifdef TT_PLATFORM_WIN
	// DirectX9 Texel sample offset
	// To align the texture exactly with framebuffer pixels, we must shift the texture coordinates by half a texel
	inverseScreenSize.z = 0.5f;
	inverseScreenSize.w = 0.5f;
#endif

	m_shader->setParameter("inverseScreenSize"  , inverseScreenSize);
	m_shader->setParameter("inverseScreenSizePS", inverseScreenSize);
}


void Filter::apply(TargetBlend p_targetBlend)
{
	if(m_output != 0)
	{
		// Activate render target (if 0 the current RT is kept)
		const u32 clearFlags = (p_targetBlend == TargetBlend_Replace) ? ClearFlag_DontClear : ClearFlag_All;
		RenderTargetStack::push(m_output, clearFlags);
	}
	
	if (m_screenBufferSampler != -1)
	{
		TT_NULL_ASSERT(m_input);
		m_input->select(static_cast<u32>(m_screenBufferSampler));
	}
	
	m_shader->select();
	
	Renderer* renderer = Renderer::getInstance();
	
	if (p_targetBlend == TargetBlend_Replace)
	{
		// Overwrite all pixels
		renderer->resetCustomBlendModeAlpha(); // No separate alpha mode; use custom settings for both color and alpha.
		renderer->setCustomBlendMode     (BlendFactor_One, BlendFactor_Zero);
	}
	
	const bool depthEnabled = renderer->isZBufferEnabled();
	renderer->setZBufferEnabled(false);
	
	FullscreenTriangle::draw();
	
	renderer->setZBufferEnabled(depthEnabled);
	
	if (m_output != 0)
	{
		RenderTargetStack::pop();
	}
}


FilterPtr Filter::create(const ShaderPtr& p_shader)
{
	if (p_shader == 0)
	{
		TT_NULL_ASSERT(p_shader);
		return FilterPtr();
	}
	return FilterPtr(new Filter(p_shader));
}


///////////////////////////////////
// Private functions

Filter::Filter(const ShaderPtr& p_shader)
:
m_shader(p_shader),
m_input(),
m_output(),
m_screenBufferSampler(p_shader->getSamplerIndex("ScreenBuffer")),
m_originalSampler(    p_shader->getSamplerIndex("Original"))
{
}


}
}
}
}
