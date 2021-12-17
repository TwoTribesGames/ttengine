#include <tt/engine/renderer/gpu_capabilities.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ShadowVolumeManager.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/scene/Instance.h>


namespace tt {
namespace engine {
namespace renderer {

MaterialPtr                        ShadowVolumeManager::ms_material;
u32                                ShadowVolumeManager::ms_colorMask         = 0;
bool                               ShadowVolumeManager::ms_depthEnabled      = false;
bool                               ShadowVolumeManager::ms_depthWriteEnabled = false;
ShadowVolumeManager::ShadowVolumes ShadowVolumeManager::ms_shadowVolumes;
bool                               ShadowVolumeManager::ms_initialized       = false;


bool ShadowVolumeManager::init(const ColorRGBA& p_shadowColor)
{
	TT_ASSERTMSG(ms_initialized == false, "Double initalization of ShadowVolumeManager!");
	
	if (hasStencilBufferSupport() == false)
	{
		return false;
	}
	
	ms_material.reset(new Material(p_shadowColor, p_shadowColor));
	ms_initialized = true;
	
	return true;
}


void ShadowVolumeManager::destroy()
{
	TT_ASSERTMSG(ms_initialized, "Can't destroy uninitalized ShadowVolumeManager!");
	
	ms_material.reset();
	ms_shadowVolumes.clear();
	
	ms_initialized = false;
}


void ShadowVolumeManager::queueForRender(const scene::InstancePtr& p_model)
{
	// Don't add models if the queue will not get cleared.
	if (ms_initialized)
	{
		ms_shadowVolumes.push_back(p_model);
	}
}


void ShadowVolumeManager::renderShadows()
{
	// Early out if not shadow volumes are registered
	if (ms_initialized == false || ms_shadowVolumes.empty()) return;
	
	Renderer* renderer = Renderer::getInstance();
	
	// Make sure no transparents are waiting to be rendered.
	renderer->renderTransparents();
	
	// Step 1: Setup stencil buffer
	initShadowVolumePass();
	
	// Step 2: Draw all shadow volumes
	renderer::RenderContext shadowRC;
	shadowRC.pass = RenderPass_ShadowVolumes;
	
	ms_material->select(shadowRC);
	
	const bool cullingEnabled = renderer->isCullingEnabled();
	renderer->setCullingEnabled(false);
	
	for (ShadowVolumes::iterator it = ms_shadowVolumes.begin(); it != ms_shadowVolumes.end(); ++it)
	{
		(*it)->render(&shadowRC);
	}
	
	// Step 3: Draw shadow models again with the shadow color using the stencil buffer to only draw
	//         at areas within a shadow volume, use color multiplication
	renderer->setBlendMode(BlendMode_Modulate);
	
	beginShadowPass();
	
	renderer->setCullingEnabled(true);
	for (ShadowVolumes::iterator it = ms_shadowVolumes.begin(); it != ms_shadowVolumes.end(); ++it)
	{
		// Use shadow RC to prevent custom blend mode from being overridden
		(*it)->render(&shadowRC);
	}
	
	endShadowPass();
	
	// Remove shadow instances
	ms_shadowVolumes.clear();
	
	renderer->setCullingEnabled(cullingEnabled);
	renderer->setBlendMode(BlendMode_Blend);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private

void ShadowVolumeManager::initShadowVolumePass()
{
	Renderer* renderer = Renderer::getInstance();
	
	// Disable all color & depth writes
	ms_colorMask = renderer->getColorMask();
	renderer->setColorMask(ColorMask_None);
	
	// Enable Z buffer, but disable Z writes
	ms_depthEnabled      = renderer->isZBufferEnabled();
	ms_depthWriteEnabled = renderer->isDepthWriteEnabled();
	
	renderer->setZBufferEnabled   (true);
	renderer->setDepthWriteEnabled(false);
	
	// Clear the stencil buffer to half maximum value (8-bits)
	renderer->clearStencil(128);
	
	// Enable and configure the stencil test
	renderer->setStencilEnabled(true);
	
	// Specify the stencil comparison function to always pass
	renderer->setStencilFunction(StencilSide_Front, StencilTestFunction_Always, 0, 255);
	renderer->setStencilFunction(StencilSide_Back , StencilTestFunction_Always, 0, 255);
	
	// Increase stencil value on depth fail for CW faces
	renderer->setStencilOperation(
		StencilSide_Front, StencilOperation_Keep, StencilOperation_Increment, StencilOperation_Keep);
	
	// Decrease stencil value on depth fail for CCW faces
	renderer->setStencilOperation(
		StencilSide_Back, StencilOperation_Keep, StencilOperation_Decrement, StencilOperation_Keep);
}


void ShadowVolumeManager::beginShadowPass()
{
	Renderer* renderer = Renderer::getInstance();
	renderer->setColorMask(ColorMask_Color);
	
	// Configure stencil test to accept front facing polygons in shadowed areas
	renderer->setStencilFunction(StencilSide_Front, StencilTestFunction_NotEqual, 128, 255);
	renderer->setStencilFunction(StencilSide_Back , StencilTestFunction_Never   , 128, 255);
	
	// Replace if passes to prevent writing same pixel multiple times
	renderer->setStencilOperation(
		StencilSide_Front, StencilOperation_Keep, StencilOperation_Keep, StencilOperation_Replace);
	renderer->setStencilOperation(
		StencilSide_Back, StencilOperation_Keep, StencilOperation_Keep, StencilOperation_Replace);
}


void ShadowVolumeManager::endShadowPass()
{
	Renderer* renderer = Renderer::getInstance();
	renderer->setStencilEnabled   (false);
	renderer->setColorMask        (ms_colorMask);
	renderer->setZBufferEnabled   (ms_depthEnabled);
	renderer->setDepthWriteEnabled(ms_depthWriteEnabled);
}




// Namespace end
}
}
}
