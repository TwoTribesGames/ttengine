
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/pp/Effect.h>
#include <tt/engine/renderer/pp/Filter.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/system/Time.h>


namespace tt {
namespace engine {
namespace renderer {
namespace pp {


PostProcessor::PostProcessor()
:
m_pingTarget(),
m_pongTarget(),
m_aaTarget(),
m_initialized(false),
m_active(false),
m_resetParameters(true),
m_useDepthBuffer(false),
m_frameActive(false),
m_targetID(0),
m_default()
{
}


void PostProcessor::initialize(const FilterPtr& p_default, UseDepthBuffer p_useDepthBuffer)
{
	m_default = p_default;
	
	if (p_default == 0)
	{
		m_active = false;
		return;
	}
	
	m_useDepthBuffer = (p_useDepthBuffer == UseDepthBuffer_Enabled);
	
	m_pingTarget = RenderTarget::createFromBackBuffer(m_useDepthBuffer);
	m_pongTarget = RenderTarget::createFromBackBuffer(false);
	
	m_initialized = true;
}


void PostProcessor::deinitialize()
{
	m_pingTarget.reset();
	m_pongTarget.reset();
	m_aaTarget.reset();

	m_initialized = false;
}


void PostProcessor::handleAntiAliasingChanged(u32 p_sampleCount)
{
	// Basically all this does is create a AA render target if the AA mode has changed
	// We can create a function for this that resembles the functionality better

	if (m_initialized == false)
	{
		TT_ASSERT(m_aaTarget == 0);
		return;
	}
	
	if(p_sampleCount > 1)
	{
		m_aaTarget = RenderTarget::createFromBackBuffer(m_useDepthBuffer, p_sampleCount);
	}
	else
	{
		m_aaTarget.reset();
	}
	
	m_resetParameters = true;
}


void PostProcessor::beginFrame(u32 p_clearFlags)
{
	if(m_initialized == false)
	{
		m_active = false;
	}
	if(isActive() == false) return;
	
	m_targetID = 0;
	
	setRenderTarget(p_clearFlags);
	
	m_frameActive = true;
}


void PostProcessor::endFrame()
{
	if(m_initialized == false || isActive() == false || m_frameActive == false) return;
	
	if(m_resetParameters)
	{
		m_resetParameters = false;
	}
	
	// Restore the RenderTarget that was active before Post-Processing
	RenderTargetStack::pop();
	
	// Set the result of what we just rendered
	if(m_targetID == 0 && m_aaTarget != 0)
	{
		// Use the AA render target
		m_default->setInput(m_aaTarget->getTexture());
	}
	else
	{
		m_default->setInput(m_targetID % 2 == 0 ? m_pingTarget->getTexture() : m_pongTarget->getTexture());
	}
	
	Renderer* renderer = Renderer::getInstance();
	m_default->apply(TargetBlend_Replace);
	renderer->resetCustomBlendModeAlpha();
	renderer->setBlendMode(BlendMode_Blend);
	FixedFunction::setActive();
	
	m_frameActive = false;
}

bool PostProcessor::isActive() const
{
#if !defined(TT_BUILD_FINAL)
	return m_active
	       && (Renderer::getInstance()->getDebug()->wireFrameEnabled() == false)
	       && (Renderer::getInstance()->getDebug()->isOverdrawDebugActive() == false)
	       && (Renderer::getInstance()->getDebug()->isMipmapVisualizerActive() == false);
#else
	return m_active;
#endif
}

RenderTargetPtr PostProcessor::getCurrentTarget()
{
	if (m_targetID == 0 && m_aaTarget != 0)
	{
		return m_aaTarget;
	}
	return (m_targetID % 2 == 0) ? m_pingTarget : m_pongTarget;
}


void PostProcessor::handleClearColorChanged(const ColorRGBA& p_color)
{
	ColorRGBA clearColor(p_color);

#if defined(TT_BUILD_DEV)
	//clearColor = ColorRGB::magenta;
#endif

	if (m_pingTarget != 0) m_pingTarget->setClearColor(clearColor);
	if (m_pongTarget != 0) m_pongTarget->setClearColor(clearColor);
	if (m_aaTarget   != 0) m_aaTarget  ->setClearColor(clearColor);
}


/////////////////////////////////////////
// Private member functions


void PostProcessor::prepare(Effect* p_effect, bool p_isLastEffect)
{
	TT_NULL_ASSERT(p_effect);
	TT_ASSERTMSG(m_frameActive, "Trying to apply effect after post-processing is already done.");

	if (m_initialized == false || m_default == 0) return;

	if (m_resetParameters)
	{
		p_effect->setConstantParameters();
	}

	// Set current RT as input
	if (m_targetID == 0 && m_aaTarget != 0)
	{
		p_effect->setInput(m_aaTarget->getTexture());
	}
	else
	{
		p_effect->setInput(m_targetID % 2 == 0 ? m_pingTarget->getTexture() : m_pongTarget->getTexture());
	}

	// Set next RT as output
	if (p_isLastEffect)
	{
		// Render last effect to framebuffer
		p_effect->setOutput(RenderTargetPtr());
		m_frameActive = false;
	}
	else
	{
		p_effect->setOutput(m_targetID % 2 == 0 ? m_pongTarget : m_pingTarget);
	}

	// Swap buffers
	++m_targetID;
}


void PostProcessor::setRenderTarget(u32 p_clearFlags)
{
	if (m_targetID == 0 && m_aaTarget != 0)
	{
		RenderTargetStack::push(m_aaTarget, p_clearFlags);
	}
	else
	{
		if (m_targetID % 2 == 0)
		{
			RenderTargetStack::push(m_pingTarget, p_clearFlags);
		}
		else
		{
			RenderTargetStack::push(m_pongTarget, p_clearFlags);
		}
	}
}


}
}
}
}
