#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/pp/PostProcessor.h>


#include <tt/engine/renderer/pp/Effect.h>


namespace tt {
namespace engine {
namespace renderer {
namespace pp {


void Effect::apply(bool p_isLastEffect)
{
	PostProcessorPtr postProcessor(Renderer::getInstance()->getPP());

	if(m_enabled && postProcessor->isActive())
	{
		Renderer* renderer = Renderer::getInstance();
		const bool restoreZWrite = renderer->isDepthWriteEnabled();
		renderer->setDepthWriteEnabled(false);
		
		// Setup the input & output
		postProcessor->prepare(this, p_isLastEffect);
		
		// Pop current post-processing render target
		RenderTargetStack::pop();
		
		// Make sure enough render targets are provided
		TT_ASSERTMSG(m_renderTargets.size() >= (m_filters.size() - 1),
			"Not enough render targets provided, need 1 for each intermediate result!");
		
		const TexturePtr original = m_original.lock();
		TT_NULL_ASSERT(original);
		
		// Execute the filters
		u32 index(0);
		for(Filters::iterator it = m_filters.begin(); it != m_filters.end(); ++it)
		{
			const FilterPtr& filter = (*it);
			
			// Determine output target
			RenderTargetPtr out = (index == (m_filters.size() - 1)) ? m_result.lock() : m_renderTargets[index];
			
			// Provide original input to all filters
			if (filter->hasOriginalSampler())
			{
				original->select(filter->getOriginalSampler());
			}
			
			// Set original input to first filter, intermediate RT to next ones
			filter->setInput( (index == 0) ? original : m_renderTargets[index-1]->getTexture() );
			
			// Set result output target to last filter, intermediate to all previous ones
			filter->setOutput( out );

			// Apply per-frame parameters
			setFrameParameters(index);
	
			// Execute the shader effect
			filter->apply(TargetBlend_Replace);

			++index;
		}

		// Make sure last used render target is activated
		RenderTargetPtr lastRenderTarget = m_result.lock();
		if (lastRenderTarget != 0)
		{
			RenderTargetStack::push(lastRenderTarget, ClearFlag_DontClear);
		}

		// Restore the fixed function pipeline
		FixedFunction::setActive();
		renderer->resetCustomBlendModeAlpha();
		renderer->setBlendMode(BlendMode_Blend);
		renderer->setDepthWriteEnabled(restoreZWrite);
	}
}


void Effect::setEnabled(bool p_enable)
{
	if(m_enabled != p_enable)
	{
		m_enabled = p_enable;
		
		if(m_enabled)
		{
			acquireHandles();
			setConstantParameters();
		}
	}
}

}
}
}
}
