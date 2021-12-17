#include <tt/engine/scene/SceneBlurMgr.h>

#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/pp/Filter.h>

#if defined(TT_PLATFORM_WIN)
#pragma warning(disable : 4592)
#endif

namespace tt {
namespace engine {
namespace scene {


renderer::RenderTargetPtr SceneBlurMgr::ms_renderTarget[2];
renderer::pp::FilterPtr   SceneBlurMgr::ms_filterBlurH;
renderer::pp::FilterPtr   SceneBlurMgr::ms_filterBlurV;
renderer::pp::FilterPtr   SceneBlurMgr::ms_filterUpscale;
renderer::pp::FilterPtr   SceneBlurMgr::ms_filterBoxBlur;
renderer::pp::FilterPtr   SceneBlurMgr::ms_filterTriBlur;
s32                       SceneBlurMgr::ms_activeIndex = 0;



bool SceneBlurMgr::initialize(const renderer::pp::FilterPtr& p_filterBlurH,
                              const renderer::pp::FilterPtr& p_filterBlurV,
                              const renderer::pp::FilterPtr& p_filterUpscale,
                              const renderer::pp::FilterPtr& p_filterBoxBlur,
                              const renderer::pp::FilterPtr& p_filterTriBlur,
                              real                           p_renderTargetRatio)
{
	if (p_filterBlurH   == 0 || p_filterBlurV == 0   ||
		p_filterUpscale == 0 || p_filterBoxBlur == 0 || p_filterTriBlur == 0)
	{
		return false;
	}
	
	ms_filterBlurH    = p_filterBlurH;
	ms_filterBlurV    = p_filterBlurV;
	ms_filterUpscale  = p_filterUpscale;
	ms_filterBoxBlur  = p_filterBoxBlur;
	ms_filterTriBlur  = p_filterTriBlur;
	
	changeRenderTargetRatio(p_renderTargetRatio);
	
	return true;
}


void SceneBlurMgr::changeRenderTargetRatio(real p_renderTargetRatio)
{
	using renderer::RenderTarget;
	ms_renderTarget[1] = RenderTarget::createFromBackBuffer(false, 0, p_renderTargetRatio);
	ms_renderTarget[0] = RenderTarget::createFromBackBuffer(false, 0, p_renderTargetRatio);
	
	if (ms_renderTarget[0] == 0 || ms_renderTarget[1] == 0)
	{
		return;
	}
	
	// Clear all channels to 0
	const renderer::ColorRGBA clearColor(0,0,0,0);
	ms_renderTarget[0]->setClearColor(clearColor);
	ms_renderTarget[1]->setClearColor(clearColor);
}


void SceneBlurMgr::beginRender()
{
	if (ms_renderTarget[0] == 0)
	{
		TT_WARN("SceneBlurMgr has not been initialized!");
		return;
	}
	
	// Start rendering into blur target
	renderer::Renderer* renderer(renderer::Renderer::getInstance());
	
	if(renderer->getDebug()->isOverdrawDebugActive()) return;
	
	renderer->setColorMask(renderer::ColorMask_All);
	renderer->setCustomBlendModeAlpha(renderer::BlendFactor_One, renderer::BlendFactor_InvSrcAlpha);
	
	ms_activeIndex = (ms_activeIndex + 1) % 2;
	renderer::RenderTargetStack::push(ms_renderTarget[ms_activeIndex], renderer::ClearFlag_All);
}


void SceneBlurMgr::doBlurPass(BlurType p_type)
{
	if(ms_renderTarget[0] == 0)
	{
		TT_WARN("SceneBlurMgr has not been initialized!");
		return;
	}

	renderer::Renderer* renderer(renderer::Renderer::getInstance());
	if(renderer->getDebug()->isOverdrawDebugActive()) return;

	renderer->getDebug()->startRenderGroup("Blur Pass");

	const bool fogEnabled = renderer->isFogEnabled();
	renderer->setFogEnabled(false);
	
	const s32 srcIndex = ms_activeIndex;
	const s32 dstIndex = (ms_activeIndex + 1) % 2;
	
	if (p_type == BlurType_TwoPassConvolution)
	{
		ms_filterBlurH->setInput (ms_renderTarget[srcIndex]->getTexture());
		ms_filterBlurH->setOutput(ms_renderTarget[dstIndex]);
		ms_filterBlurH->apply(renderer::pp::TargetBlend_Replace);
		
		ms_filterBlurV->setInput (ms_renderTarget[dstIndex]->getTexture());
		ms_filterBlurV->setOutput(ms_renderTarget[srcIndex]);
		ms_filterBlurV->apply(renderer::pp::TargetBlend_Replace);
	}
	else
	{
		renderer::pp::FilterPtr blurFilter = (p_type == BlurType_Box) ? ms_filterBoxBlur : ms_filterTriBlur;
		
		blurFilter->setInput (ms_renderTarget[srcIndex]->getTexture());
		blurFilter->setOutput(ms_renderTarget[dstIndex]);
		blurFilter->apply(renderer::pp::TargetBlend_Replace);
		
		// Make sure the target with the blur result is bound for further rendering
		renderer::RenderTargetStack::pop();
		renderer::RenderTargetStack::push(ms_renderTarget[dstIndex], renderer::ClearFlag_DontClear);
		
		ms_activeIndex = dstIndex;
	}

	renderer->setCustomBlendModeAlpha(renderer::BlendFactor_One, renderer::BlendFactor_InvSrcAlpha);
	renderer->setBlendMode(renderer::BlendMode_Blend);
	engine::renderer::FixedFunction::setActive();

	renderer->setFogEnabled(fogEnabled);
	renderer->getDebug()->endRenderGroup();
}


void SceneBlurMgr::endRender()
{
	if (ms_renderTarget[0] == 0)
	{
		TT_WARN("SceneBlurMgr has not been initialized!");
		return;
	}
	
	renderer::Renderer* renderer(renderer::Renderer::getInstance());
	if(renderer->getDebug()->isOverdrawDebugActive()) return;
	
	renderer->getDebug()->startRenderGroup("Upscale Pass");
	
	renderer::RenderTargetStack::pop();
	
	renderer->setColorMask(renderer::ColorMask_Color);
	renderer->resetCustomBlendModeAlpha();
	renderer->setBlendMode(renderer::BlendMode_Premultiplied);
	
	const bool fogEnabled = renderer->isFogEnabled();
	renderer->setFogEnabled(false);
	
	// Draw upscaled result of blur into back buffer
	ms_filterUpscale->setInput(ms_renderTarget[ms_activeIndex]->getTexture());
	ms_filterUpscale->apply();
	engine::renderer::FixedFunction::setActive();
	
	renderer->setBlendMode(renderer::BlendMode_Blend);
	renderer->setFogEnabled(fogEnabled);
	
	renderer->getDebug()->endRenderGroup();
}


void SceneBlurMgr::cleanup()
{
	ms_renderTarget[0].reset();
	ms_renderTarget[1].reset();
	ms_filterBlurH    .reset();
	ms_filterBlurV    .reset();
	ms_filterUpscale  .reset();
	ms_filterBoxBlur  .reset();
	ms_filterTriBlur  .reset();
}


// Namespace end
}
} 
}
