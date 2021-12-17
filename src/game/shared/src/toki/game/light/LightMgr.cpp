#include <algorithm>
#include <iterator>

#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/pp/Filter.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/thread/ThreadedWorkload.h>

#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/light/BlobData.h>
#include <toki/game/light/DarknessMgr.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/light/LightShape.h>
#include <toki/game/light/Polygon.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/level/AttributeLayer.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace light {

#define USE_THREADING 1

tt::engine::renderer::RenderTargetPtr LightMgr::ms_lightGlowsRenderTarget;
tt::engine::renderer::pp::FilterPtr   LightMgr::ms_lightGlowsFilter;
tt::engine::renderer::RenderTargetPtr LightMgr::ms_shadowPing;
tt::engine::renderer::RenderTargetPtr LightMgr::ms_shadowPong;
tt::engine::renderer::pp::FilterPtr   LightMgr::ms_gaussBlurH;
tt::engine::renderer::pp::FilterPtr   LightMgr::ms_gaussBlurV;
tt::engine::renderer::pp::FilterPtr   LightMgr::ms_clearScreen;
ShadowQuality                         LightMgr::ms_shadowQuality = ShadowQuality_High;
s32                                   LightMgr::ms_blurPassCount = 2;

static const real quadSizeCorrection = 1.0f / (2.0f * tt::engine::renderer::Quad2D::quadSize);

//--------------------------------------------------------------------------------------------------
// Public member functions

LightMgr::LightMgr(s32 p_reserveCount, const level::AttributeLayerPtr& p_levelLayer)
:
m_lights(p_reserveCount),
m_visibleLights(),
m_visibleGlows(),
//m_litEntities(),
m_wholeLevelAmbientAlpha(),
m_wholeLevelFullAlpha(),
m_fullScreenMtx(),
m_lightAmbient(0),
m_staticOccluders(),
m_dynamicOccluders(),
m_allOccluders(),
m_levelLayer(p_levelLayer),
m_currentTime(0.0f),
m_dirty(true),
m_isDarkLevel(false),
m_shouldRenderDarkness(false),
m_defaultLightAmbient(128),
#if DO_LIGHT_BLOB_QUAD_DEBUG_RENDER
m_debugQuads(),
#endif
m_sectionProfiler("LightMgr - update")
{
	using tt::engine::renderer::ColorRGB;
	using tt::engine::renderer::ColorRGBA;
	using tt::engine::renderer::Quad2D;
	using tt::engine::renderer::VertexBuffer;
	
	m_wholeLevelAmbientAlpha.reset(new Quad2D(VertexBuffer::Property_Diffuse, ColorRGBA(ColorRGB::black,   0)));
	m_wholeLevelFullAlpha   .reset(new Quad2D(VertexBuffer::Property_Diffuse, ColorRGBA(ColorRGB::white, 255)));
	setLevelLightAmbientImpl(m_defaultLightAmbient);
	
	updateStaticOccluders();
	updateDynamicOccluders();

	mergeStaticWithDynamicOccluders();
}


LightMgr::~LightMgr()
{
	tt::code::helpers::freePointerContainer(   m_staticOccluders);
	tt::code::helpers::freePairSecondContainer(m_dynamicOccluders);
}


LightHandle LightMgr::createLight(const entity::EntityHandle& p_source,
                                  const tt::math::Vector2& p_offset, real p_radius, real p_strength)
{
	return m_lights.create(Light::CreationParams(p_source, p_offset, p_radius, p_strength));
}


void LightMgr::destroyLight(LightHandle& p_handle)
{
	m_lights.destroy(p_handle);
	
	p_handle.invalidate();
}


void LightMgr::onTileChange(const tt::math::Point2& p_position)
{
	(void) p_position;
	m_dirty = true;
}


void LightMgr::handleLevelResized()
{
	m_dirty = true;
}


void LightMgr::resetLevel()
{
	m_dirty = true;
	m_lights.reset();
	//m_litEntities.clear();
	m_darknessRects.clear();
	m_shouldRenderDarkness = false;
	
	m_visibleLights.clear();
	m_visibleGlows.clear();
}


bool LightMgr::isPositionInLight(const tt::math::Vector2& p_position) const
{
#if 0 // No darkness rect logic for rewind
	if (m_isDarkLevel == false)
	{
		if (isInDarknessRect(p_position) == false)
		{
			return true;
		}
	}
#endif
	
	const Light* light = m_lights.getFirst();
	for (s32 i = 0; i < m_lights.getActiveCount(); ++i, ++light)
	{
		if (light->isEnabled() && light->isInLight(p_position))
		{
			return true;
		}
	}
	return false;
}


bool LightMgr::isEntityInLight(const entity::Entity& p_entity) const
{
	return isEntityInLightImpl(p_entity);
	
#if 0 // No darkness rect logic for rewind
	if (isEntityInLightImpl(p_entity) == false)
	{
		return false;
	}
	
	if (m_isDarkLevel == false)
	{
		if (isInDarknessRect(p_entity.getWorldRect()))
		{
			m_litEntities.insert(p_entity.getHandle());
			m_entitiesInShadowRect.insert(p_entity.getHandle());
		}
	}
	else
	{
		m_litEntities.insert(p_entity.getHandle());
	}
	return true;
#endif
}


void LightMgr::update(real p_deltaTime)
{
	m_currentTime += p_deltaTime;
	
	m_sectionProfiler.startFrameUpdate();
	
	// Update all lights first
	Light* light = m_lights.getFirst();
	for (s32 i = 0; i < m_lights.getActiveCount(); ++i, ++light)
	{
		light->update(p_deltaTime);
	}
	
	// Check for early exit
	if (m_isDarkLevel == false)
	{
		updateDarkness(); // Get (fresh) darkness rects from DarknessMgr.
		
#if 0 // No darkness rect logic for rewind
		if (shouldDoLight() == false)
		{
			return;
		}
#endif
	}

	updateSensors();
	
	m_sectionProfiler.stopFrameUpdate();
}


void LightMgr::updateForRender(const tt::math::VectorRect& p_visibilityRect, s32 p_lightAmbient)
{
	m_visibleLights.clear();
	m_visibleGlows.clear();

	m_wholeLevelAmbientAlpha->update();
	m_wholeLevelFullAlpha   ->update();

	const real visibleWidth  = p_visibilityRect.getWidth()  * quadSizeCorrection;
	const real visibleHeight = p_visibilityRect.getHeight() * quadSizeCorrection;
	const real quadSize = std::max(visibleWidth, visibleHeight) * 2;
	
	m_fullScreenMtx.setIdentity();
	m_fullScreenMtx.translate(p_visibilityRect.getCenterPosition());
	m_fullScreenMtx.scale(quadSize, quadSize);
	
#if 0 // No darkness rect only rendering in rewind
	// Check if we need to render lights:
	// > Only in dark levels or in light levels if a darkness rectangle is on screen
	if(shouldDoLight() == false)
	{
		m_shouldRenderDarkness = false;
		return;
	}
#endif
	
	setLevelLightAmbientImpl(p_lightAmbient);
	
	Light* light = m_lights.getFirst();
	for (s32 i = 0; i <  m_lights.getActiveCount(); ++i, ++light)
	{
		const tt::math::Vector2& pos = light->getWorldPosition();
		const real radius            = light->getRadius();
		
		// On screen culling.
		if (p_visibilityRect.intersects(pos, radius))
		{
			// Rewind had light rendering outside of the rects.
			//if (shouldDoRectChecks() == false || isInDarknessRect(pos, radius)) // Inside darkness
			{
				m_visibleLights.push_back(light);
			}
			m_visibleGlows.push_back(light);
		}
	}
	
#if 0 // old toki tori 2 code replace for rewind (Always render lights)
	// Check if there is darkness on screen, otherwise only glows need to be rendered
	if(m_isDarkLevel || isInDarknessRect(p_visibilityRect))
	{
		m_shouldRenderDarkness = true;
	}
	else
	{
		m_shouldRenderDarkness = false;
		return;
	}
#else
	m_shouldRenderDarkness = m_isDarkLevel || isInDarknessRect(p_visibilityRect);
#endif

	if (m_visibleLights.empty() == false)
	{
		using namespace toki::utils;
		bool needMerge = false;
		
		m_sectionProfiler.startFrameUpdateSection(LightMgrSection_StaticOccluders);
		if (m_dirty)
		{
			updateStaticOccluders();
			needMerge = true;
		}
		
		m_sectionProfiler.startFrameUpdateSection(LightMgrSection_DynamicOccluders);
		needMerge = updateDynamicOccluders() || needMerge;
		
		m_sectionProfiler.startFrameUpdateSection(LightMgrSection_MergeOccluders);
		if (needMerge)
		{
			mergeStaticWithDynamicOccluders();
		}
	}
	
#if USE_THREADING
	tt::thread::ThreadedWorkload work(m_visibleLights.size(),
		std::bind(&LightMgr::updateLightShape, this, std::placeholders::_1, m_allOccluders));
	work.startAndWaitForCompletion();
#else
	for (LightPtrs::iterator it = m_visibleLights.begin(); it != m_visibleLights.end(); ++it)
	{
		(*it)->updateLightShape(m_allOccluders);
	}
#endif
}


void LightMgr::startLightRender(const tt::engine::scene2d::shoebox::ShoeboxPtr& p_shadowMaskShoebox,
                                const tt::math::VectorRect& p_visibilityRect)
{
	if (
#if !defined(TT_BUILD_FINAL)
		AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Light) == false &&
#endif
		m_shouldRenderDarkness)
	{
		using namespace tt::engine::renderer;
		Renderer* renderer(Renderer::getInstance());
		TT_ASSERT(renderer->isZBufferEnabled() == false);
		
		// Start with color buffer as is, and clear the alpha channel to 0.
		//TT_ASSERT(renderer->getClearColor().a == 0);
		//renderer->setClearColor(ColorRGBA(ColorRGB::blue, 0));
		renderer->setColorMask(ColorMask_Alpha);
		renderer->setCustomBlendMode(BlendFactor_One, BlendFactor_Zero);
		renderer->resetCustomBlendModeAlpha();
		
		// Add lightmask shoebox background
		renderer->setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_One);
		if (p_shadowMaskShoebox != 0)
		{
			p_shadowMaskShoebox->renderBackground();
			(void) p_visibilityRect;
		}
		
		// All following calls should only write to the color buffer, but not the alpha.
		renderer->setColorMask(ColorMask_Color);
		renderer->setBlendMode(BlendMode_Blend); // Restore the default blend mode.
	}
}


void LightMgr::addLightsToRender()
{
	if (
#if !defined(TT_BUILD_FINAL)
		AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Light) == false &&
#endif
		(m_shouldRenderDarkness || m_visibleLights.empty() == false))
	{
		using namespace tt::engine::renderer;
		Renderer* renderer(Renderer::getInstance());
		
		renderer->setColorMask(ColorMask_All);
		renderer->setCustomBlendMode(BlendFactor_One, BlendFactor_One);
		renderer->setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_One);
		
		if(m_visibleLights.empty() == false)
		{
			// Render lights into separate render target
			// And apply a gaussian blur to achieve nice soft shadows
			
			bool renderTargetRestored(false);
			if (ms_shadowPing != 0 && ms_gaussBlurH != 0 && ms_gaussBlurV != 0)
			{
				RenderTargetStack::push(ms_shadowPing, ClearFlag_All);
			}
			
			tt::engine::renderer::MatrixStack::getInstance()->resetTextureMatrix();
			for (LightPtrs::iterator it = m_visibleLights.begin(); it != m_visibleLights.end(); ++it)
			{
				(*it)->render(m_currentTime);
			}
			
			if (ms_shadowPing != 0 && ms_gaussBlurH != 0 && ms_gaussBlurV != 0
#if !defined(TT_BUILD_FINAL)
			    && Renderer::getInstance()->getDebug()->isOverdrawDebugActive() == false
#endif
			)
			{
				for(s32 i = 0; i < ms_blurPassCount; ++i)
				{
					ms_gaussBlurH->setInput(ms_shadowPing->getTexture());
					ms_gaussBlurH->setOutput(ms_shadowPong);
					ms_gaussBlurH->apply(pp::TargetBlend_Replace);
					
					if(i == (ms_blurPassCount - 1))
					{
						// Add result of blurred lights to framebuffer alpha channel
						RenderTargetStack::pop();
						renderTargetRestored = true;
						renderer->setCustomBlendMode(BlendFactor_One, BlendFactor_One);
						renderer->setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_One);
						
						ms_gaussBlurV->setOutput(RenderTargetPtr());
						ms_gaussBlurV->setInput(ms_shadowPong->getTexture());
						ms_gaussBlurV->apply(pp::TargetBlend_Blend);
					}
					else
					{
						ms_gaussBlurV->setOutput(ms_shadowPing);
						ms_gaussBlurV->setInput(ms_shadowPong->getTexture());
						ms_gaussBlurV->apply(pp::TargetBlend_Replace);
					}
				}
			}
			
			// Finished using shader effects
			FixedFunction::setActive();
			
			if (renderTargetRestored == false)
			{
				RenderTargetStack::pop();
			}
		}
		
		// Restore the defaults for normal renders after this.
		renderer->setColorMask(ColorMask_All);
		renderer->setBlendMode(BlendMode_Blend); // Restore the default blend mode.
		renderer->setCustomBlendModeAlpha(BlendFactor_Zero, BlendFactor_InvSrcAlpha);
		
#if !defined(TT_BUILD_FINAL)
		/* // Debug rendering
		for (Polygons::const_iterator it = m_occluders.begin(); it != m_occluders.end(); ++it)
		{
			(*it)->render();
			//(*it)->renderDebug();
		}
		// */
#endif
	}
}


void LightMgr::endLightRender(const tt::engine::scene2d::shoebox::ShoeboxPtr& p_shadowMaskShoebox,
                              const tt::math::VectorRect& p_visibilityRect)
{
	if (
#if !defined(TT_BUILD_FINAL)
		AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Light) == false &&
#endif
		m_shouldRenderDarkness)
	{
		using namespace tt::engine::renderer;
		Renderer* renderer(Renderer::getInstance());
		
		// Add default ambient light or shoebox light mask.
		renderer->setColorMask(ColorMask_Alpha);
		renderer->setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_One);
		
		if (ms_clearScreen != 0)
		{
			const tt::math::Vector4 clearColor(0, 0, 0, m_lightAmbient / 255.0f);
			ms_clearScreen->getShader()->setParameter("clearColor", clearColor);
			ms_clearScreen->apply();
			FixedFunction::setActive();
		}
		else
		{
			renderFullScreenQuad2D(m_wholeLevelAmbientAlpha);
		}
		
		if (p_shadowMaskShoebox != 0)
		{
			p_shadowMaskShoebox->renderBackgroundZero(p_visibilityRect);
			
			// Render lightmask shoebox foreground.
			p_shadowMaskShoebox->renderForegroundZeroBack(p_visibilityRect);
			p_shadowMaskShoebox->renderForegroundZeroFront(p_visibilityRect);
			p_shadowMaskShoebox->renderForeground();
		}
		
		// Blend the existing color in the framebuffer with the alpha channel that's there.
		renderer->setColorMask(ColorMask_All);
		renderer->setCustomBlendMode(BlendFactor_Zero, BlendFactor_DstAlpha);
		renderer->setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_Zero);
		
		if (shouldDoRectChecks())
		{
			// FIXME: Render rects
			AppGlobal::getGame()->getDarknessMgr().renderDarkness();
		}
		else
		{
			if (ms_clearScreen != 0)
			{
				const tt::math::Vector4 clearColor(0, 0, 0, 1.0f);
				ms_clearScreen->getShader()->setParameter("clearColor", clearColor);
				ms_clearScreen->apply();
				FixedFunction::setActive();
			}
			else
			{
				renderFullScreenQuad2D(m_wholeLevelFullAlpha);
			}
		}
		
		// Restore all masks for clear.
		renderer->setColorMask(ColorMask_Color);
		renderer->setBlendMode(BlendMode_Blend); // Restore the default blend mode.
		renderer->resetCustomBlendModeAlpha();
		
#if DO_LIGHT_BLOB_QUAD_DEBUG_RENDER
		// DEBUG rendering
		tt::engine::renderer::DebugRendererPtr debug = tt::engine::renderer::Renderer::getInstance()->getDebug();
		for (DebugQuads::const_iterator it = m_debugQuads.begin(); it != m_debugQuads.end(); ++it)
		{
			debug->renderRect(it->color, it->rect);
		}
#endif
		
#if !defined(TT_BUILD_FINAL)
		/* // Outline rendering of polygons.
		for (Polygons::const_iterator it = m_allOccluders.begin(); it != m_allOccluders.end(); ++it)
		{
			(*it)->render();
		}
		// */
#endif
	}
}


void LightMgr::renderLightGlows() const
{
	if (ms_lightGlowsRenderTarget == 0 || m_visibleGlows.empty()) return;
	
	using namespace tt::engine::renderer;
	Renderer* renderer = Renderer::getInstance();
	
	// Properly render to alpha channel if clear alpha is 0
	if (ms_lightGlowsFilter != 0)
	{
		renderer->setBlendMode(BlendMode_Blend);
		renderer->setCustomBlendModeAlpha(BlendFactor_InvDstAlpha, BlendFactor_One);
		renderer->setColorMask(ColorMask_All);
	
		RenderTargetStack::push(ms_lightGlowsRenderTarget, ClearFlag_All);
	}
	
	for (LightPtrs::const_iterator it = m_visibleGlows.begin(); it != m_visibleGlows.end(); ++it)
	{
		(*it)->renderGlow();
	}
	
	if (ms_lightGlowsFilter != 0)
	{
		RenderTargetStack::pop();
		
		renderer->setColorMask(ColorMask_Color);
		renderer->resetCustomBlendModeAlpha();
		renderer->setBlendMode(BlendMode_Premultiplied);
		
		const bool fogEnabled = renderer->isFogEnabled();
		renderer->setFogEnabled(false);
		
		ms_lightGlowsFilter->setInput(ms_lightGlowsRenderTarget->getTexture());
		ms_lightGlowsFilter->apply();
		
		// Finished using shader effects
		FixedFunction::setActive();
		
		renderer->setFogEnabled(fogEnabled);
	}
}


void LightMgr::restoreAlphaChannel()
{
	using namespace tt::engine::renderer;
	Renderer* renderer(Renderer::getInstance());
	
	renderer->setColorMask(ColorMask_Alpha);
	renderer->setCustomBlendModeAlpha(BlendFactor_One, BlendFactor_Zero);
	
	if (ms_clearScreen != 0)
	{
		const tt::math::Vector4 clearColor(0, 0, 0, 1.0f);
		ms_clearScreen->getShader()->setParameter("clearColor", clearColor);
		ms_clearScreen->apply();
		FixedFunction::setActive();
	}
	else
	{
		renderFullScreenQuad2D(m_wholeLevelFullAlpha);
	}
	
	renderer->setColorMask(ColorMask_Color);
	renderer->setBlendMode(BlendMode_Blend); // Restore the default blend mode.
	renderer->resetCustomBlendModeAlpha();
}


void LightMgr::debugRender()
{
#if !defined(TT_BUILD_FINAL)
	if (AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Light) && shouldDoLight())
	{
		Light* light = m_lights.getFirst();
		for (s32 i = 0; i <  m_lights.getActiveCount(); ++i, ++light)
		{
			light->debugRender();
			
			// /*
			light->updateLightShape(m_allOccluders);
			light->render();
			// */
		}
	}
#endif
}


s32 LightMgr::getLevelLightAmbient()
{
	return m_defaultLightAmbient;
}


void LightMgr::setLevelLightAmbient(s32 p_ambient)
{
	tt::math::clamp(p_ambient, s32(0), s32(255));
	m_defaultLightAmbient = static_cast<u8>(p_ambient);
}


void LightMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_LightMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the LightMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_isDarkLevel, &context);
	bu::put(m_defaultLightAmbient, &context);
	
	const u32 lightCount = static_cast<u32>(m_lights.getActiveCount());
	bu::put(lightCount, &context);
	
	const Light* light = m_lights.getFirst();
	for (u32 i = 0; i < lightCount; ++i, ++light)
	{
		light->serialize(&context);
	}
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void LightMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_LightMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the LightMgr data.");
		return;
	}
	
	m_lights.reset();
	m_visibleLights.clear();
	m_visibleGlows.clear();
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	m_isDarkLevel         = bu::get<bool>(&context);
	m_defaultLightAmbient = bu::get<u8  >(&context);
	
	const u32 lightCount = bu::get<u32>(&context);
	
	for (u32 i = 0; i < lightCount; ++i)
	{
		Light scratchLight(Light::unserialize(&context));
		m_lights.addWithSpecificHandle(scratchLight, scratchLight.getHandle());
	}
	m_lights.doInternalCleanup();
}


void LightMgr::createStaticResources(bool p_renderLightGlows)
{
	using namespace tt::engine::renderer;
	
	if (p_renderLightGlows && ms_lightGlowsRenderTarget == 0)
	{
		ms_lightGlowsRenderTarget = RenderTarget::createFromBackBuffer(false, 0, 0.5f);
		ms_lightGlowsRenderTarget->setClearColor(ColorRGBA(0,0,0,0));
	}
	
	ms_clearScreen = pp::Filter::create(ShaderCache::get("Clear", "shaders"));
}


void LightMgr::destroyStaticResources()
{
	ms_lightGlowsRenderTarget.reset();
	ms_lightGlowsFilter.reset();
	ms_shadowPing.reset();
	ms_shadowPong.reset();
	ms_gaussBlurH.reset();
	ms_gaussBlurV.reset();
	ms_clearScreen.reset();
}


void LightMgr::setShadowQuality(ShadowQuality p_quality)
{
	if (tt::engine::renderer::Renderer::hasInstance() == false) return;
	
	if (ms_shadowPing == 0 || p_quality != ms_shadowQuality)
	{
		s32  aaSamples(0);
		real blurTargetSize(0.5f);
		
		switch (p_quality)
		{
		case ShadowQuality_Low:
			ms_shadowPing.reset();
			ms_shadowPong.reset();
			break;
			
		case ShadowQuality_Medium:
			aaSamples = 0;
			blurTargetSize = 0.25f;
			ms_blurPassCount = 1;
			break;
			
		case ShadowQuality_High:
			aaSamples = 4;
			blurTargetSize = 0.3f;
			ms_blurPassCount = 1;
			break;
		}
		
		if (p_quality > ShadowQuality_Low)
		{
			using tt::engine::renderer::ColorRGBA;
			using tt::engine::renderer::RenderTarget;
			
			ms_shadowPing = RenderTarget::createFromBackBuffer(false, aaSamples, blurTargetSize);
			ms_shadowPing->setClearColor(ColorRGBA(0,0,0,0));
			
			ms_shadowPong = RenderTarget::createFromBackBuffer(false, 0, blurTargetSize);
			ms_shadowPong->setClearColor(ColorRGBA(0,0,0,0));
		}
		
		ms_shadowQuality = p_quality;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void LightMgr::updateStaticOccluders()
{
	m_dirty = false;
	const s32 width   = m_levelLayer->getWidth();
	const s32 height  = m_levelLayer->getHeight();
	const u8* dataPtr = m_levelLayer->getRawData();
	
	static BlobData blobData;
	blobData.reset(width);
	
	// Go through tiles.
	// (Starting at bottom left, go right on that row and continue with the one above after that.)
	for (tt::math::Point2 pos(0, 0); pos.y < height; ++pos.y)
	{
		for (pos.x = 0; pos.x < width; ++pos.x, ++dataPtr)
		{
			const u8 value = *dataPtr;
			
#if defined(TT_BUILD_DEV)
			//* // Asserts to make sure this usage for the raw data is correct.
			TT_ASSERT(m_levelLayer->getCollisionType(pos) == level::getCollisionType(value));
			// */
#endif
			
			const bool blockLight = level::isLightBlocking(level::getCollisionType(value));
			
			doGrowBlobData(blobData, pos, blockLight);
		}
		//TT_Printf("\n");
	}
	
	// --- Go over the prev* one more them to flush the latest data. ---
	for (s32 index = 0; index < width; ++index)
	{
		doGrowBlobDataFlush(blobData, index, height);
	}
	// Check for Right Top tile.
	blobData.checkForQuadMerge(width - 1, width - 1, -1);
	
#if DO_LIGHT_BLOB_QUAD_DEBUG_RENDER
	m_debugQuads.clear();
#endif
	const size_t oldSize = m_staticOccluders.size();
	tt::code::helpers::freePointerContainer(m_staticOccluders);
	m_staticOccluders.reserve(oldSize);
	
	for (BlobData::Blobs::const_iterator blobIt = blobData.allBlobs.begin();
	     blobIt != blobData.allBlobs.end(); ++blobIt)
	{
#if DO_LIGHT_BLOB_QUAD_DEBUG_RENDER
		m_debugQuads.push_back(DebugQuad(tt::math::Vector2(blobIt->getMin().x + 0.1f,
		                                                   blobIt->getMin().y + 0.1f), 
		                                 tt::math::Vector2(blobIt->getMax().x + 0.9f,
		                                                   blobIt->getMax().y + 0.9f),
		                                 tt::engine::renderer::ColorRGB::blue));
#endif
		
		const SubQuads& quads = blobIt->getSubQuads();
		for (SubQuads::const_iterator it = quads.begin(); it != quads.end(); ++it)
		{
			const SubQuad& subQuad = (*it);
			
#if DO_LIGHT_BLOB_QUAD_DEBUG_RENDER
			m_debugQuads.push_back(DebugQuad(tt::math::Vector2(subQuad.min.x + 0.2f,
			                                                   subQuad.min.y + 0.2f), 
			                                 tt::math::Vector2(subQuad.max.x + 0.8f,
			                                                   subQuad.max.y + 0.8f),
			                                 tt::engine::renderer::ColorRGB::magenta));
#endif
			
			using tt::math::Vector2;
			const tt::math::Vector2 min(subQuad.min);
			const tt::math::Vector2 max(subQuad.max + tt::math::Point2::allOne);
			
			Vertices vertices;
			vertices.push_back(min);
			vertices.push_back(Vector2(max.x, min.y));
			vertices.push_back(max);
			vertices.push_back(Vector2(min.x, max.y));
			
			m_staticOccluders.push_back(new Polygon(Vector2(0.0f, 0.0f), 
			                                        tt::engine::renderer::ColorRGB::green,
			                                        vertices, true));
		}
	}
}


bool LightMgr::updateDynamicOccluders()
{
	if (AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return false;
	}
	
	Vertices vertices(4); // Reserve for the 4 quad vertices
	
	EntityPolygons updatedList;
	
	const entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	const entity::Entity* entity = entityMgr.getFirstEntity();
	for (s32 i = 0; i < entityMgr.getActiveEntitiesCount(); ++i, ++entity)
	{
		if (entity != 0 && entity->isLightBlocking())
		{
			const entity::EntityHandle handle = entity->getHandle();
			
			// Check if we already have it.
			EntityPolygons::iterator polyIt = m_dynamicOccluders.find(handle);
			if (polyIt != m_dynamicOccluders.end())
			{
				// Found it
				Polygon* poly = polyIt->second;
				poly->setPosition(entity->getWorldRect().getPosition());
				updatedList[handle] = poly;
				m_dynamicOccluders.erase(polyIt);
			}
			else
			{
				// Not found, create new one.
				const tt::math::VectorRect& rect(entity->getWorldRect());
				
				tt::math::Vector2 min(0.0f, 0.0f);
				tt::math::Vector2 max(rect.getWidth(), rect.getHeight());
				
				vertices[0] = min;
				vertices[1].setValues(max.x, min.y);
				vertices[2] = max;
				vertices[3].setValues(min.x, max.y);
				
				updatedList[handle] = new Polygon(rect.getPosition(),
				                                  tt::engine::renderer::ColorRGB::green,
				                                  vertices, true);
			}
		}
	}
	
	tt::code::helpers::freePairSecondContainer(m_dynamicOccluders);
	std::swap(m_dynamicOccluders, updatedList);
	
	return true;
}


void LightMgr::mergeStaticWithDynamicOccluders()
{
	m_allOccluders.clear();
	m_allOccluders.reserve(m_staticOccluders.size() + m_dynamicOccluders.size());
	m_allOccluders.insert(m_allOccluders.end(), m_staticOccluders.begin(), m_staticOccluders.end());
	for (EntityPolygons::const_iterator it = m_dynamicOccluders.begin(); it != m_dynamicOccluders.end(); ++it)
	{
		m_allOccluders.push_back(it->second);
	}
}


void LightMgr::updateLightShape(size_t p_index, const Polygons& p_occluders)
{
	TT_ASSERT(p_index < m_visibleLights.size());
	
	m_visibleLights[p_index]->updateLightShape(p_occluders);
}


void LightMgr::updateDarkness()
{
	AppGlobal::getGame()->getDarknessMgr().updateRects(&m_darknessRects, 255);
}


void LightMgr::updateSensors()
{
	m_sectionProfiler.startFrameUpdateSection(toki::utils::LightMgrSection_Sensors);
/*
	// NO LIGHT SENSORS IN REWIND
	const entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	
	entity::EntityHandleSet currentEntitiesInShadowRects;
	
#if 0 // No darkness rect logic for rewind
	if (shouldDoRectChecks())
	{
		const level::TileRegistrationMgr& tileMgr = AppGlobal::getGame()->getTileRegistrationMgr();
		const level::AttributeLayerPtr&   level   = AppGlobal::getGame()->getAttributeLayer();

		const tt::math::Point2 maxLevel(level->getWidth() - 1, level->getHeight() - 1);

		// Get all entities with in our shadow rects.
		for (Rects::const_iterator it = m_darknessRects.begin(); it != m_darknessRects.end(); ++it)
		{
			const tt::math::PointRect tileRect = level::worldToTile(*it);

			const tt::math::Point2 minPos = tt::math::pointMax(tileRect.getMin(), tt::math::Point2::zero);
			const tt::math::Point2 maxPos = tt::math::pointMin(tileRect.getMaxInside(), maxLevel);

			tt::math::Point2 pos;
	
			for (pos.y = minPos.y; pos.y <= maxPos.y; ++pos.y)
			{
				for (pos.x = minPos.x; pos.x <= maxPos.x; ++pos.x)
				{
					if(tileMgr.hasEntityAtPosition(pos.x, pos.y))
					{
						const entity::EntityHandleSet& entities = tileMgr.getRegisteredEntityHandles(pos);

						for (entity::EntityHandleSet::const_iterator entityIt = entities.begin();
						     entityIt != entities.end(); ++entityIt)
						{
							if(currentEntitiesInShadowRects.find(*entityIt) == currentEntitiesInShadowRects.end())
							{
								currentEntitiesInShadowRects.insert(*entityIt);
							}
						}
					}
				}
			}
		}
		
		// Entities entered rects
		{
			entity::EntityHandles difference;
			std::set_difference(
				currentEntitiesInShadowRects.begin(), currentEntitiesInShadowRects.end(),
				m_entitiesInShadowRect.begin(), m_entitiesInShadowRect.end(), std::back_inserter(difference));
			
			for (entity::EntityHandles::const_iterator it = difference.begin(); it != difference.end(); ++it)
			{
				entity::Entity* target = entityMgr.getEntity(*it);
				if (target != 0)
				{
					if (isEntityInLight(*target) == false)
					{
						target->onLightExit();
					}
				}
			}
		}
		
		// Entities exited rects
		{
			entity::EntityHandles difference;
			std::set_difference(
				m_entitiesInShadowRect.begin(), m_entitiesInShadowRect.end(),
				currentEntitiesInShadowRects.begin(), currentEntitiesInShadowRects.end(), std::back_inserter(difference));
			
			for (entity::EntityHandles::const_iterator it = difference.begin(); it != difference.end(); ++it)
			{
				entity::EntityHandleSet::iterator litIt = m_litEntities.find(*it);
				if (litIt != m_litEntities.end())
				{
					m_litEntities.erase(litIt);
				}
				
				entity::Entity* target = entityMgr.getEntity(*it);
				if (target != 0)
				{
					target->onLightEnter();
				}
			}
		}
		
		std::swap(m_entitiesInShadowRect, currentEntitiesInShadowRects);
	}
#endif
	
	// Light check
	
	entity::EntityHandleSet currentLitEntities;
	
	// No darkness rect logic for rewind
	//if (m_isDarkLevel)
	{
		Light* light = m_lights.getFirst();
		for (s32 i = 0; i < m_lights.getActiveCount(); ++i, ++light)
		{
			if (light->isEnabled())
			{
				light->getAffectedEntities(currentLitEntities, m_levelLayer);
			}
		}
	}
#if 0 // No darkness rect logic for rewind
	else
	{
		for (entity::EntityHandleSet::const_iterator it = m_entitiesInShadowRect.begin(); it != m_entitiesInShadowRect.end(); ++it)
		{
			const entity::Entity* entity = it->getPtr();
			if (entity != 0 && isEntityInLightImpl(*entity))
			{
				currentLitEntities.insert(*it);
			}
		}
	}
#endif
	
	// Entities entered light
	{
		// FIXME: Fragmented allocation?
		entity::EntityHandles difference;
		std::set_difference(
			currentLitEntities.begin(), currentLitEntities.end(),
			m_litEntities.begin(), m_litEntities.end(), std::back_inserter(difference));
		
		for (entity::EntityHandles::const_iterator it = difference.begin();
		     it != difference.end(); ++it)
		{
			entity::Entity* target = entityMgr.getEntity(*it);
			if (target != 0)
			{
				target->onLightEnter();
			}
		}
	}
	
	// Entities exited light
	{
		// FIXME: Fragmented allocation?
		entity::EntityHandles difference;
		std::set_difference(
			m_litEntities.begin(), m_litEntities.end(),
			currentLitEntities.begin(), currentLitEntities.end(), std::back_inserter(difference));
		
		for (entity::EntityHandles::const_iterator it = difference.begin();
		     it != difference.end(); ++it)
		{
			entity::Entity* target = entityMgr.getEntity(*it);
			if (target != 0)
			{
				target->onLightExit();
			}
		}
	}
	
	std::swap(m_litEntities, currentLitEntities);
	*/
}


bool LightMgr::isInDarknessRect(const tt::math::Vector2& p_position, real p_radius) const
{
	for (Rects::const_iterator it = m_darknessRects.begin(); it != m_darknessRects.end(); ++it)
	{
		const tt::math::VectorRect& rect = (*it);
		if (rect.intersects(p_position, p_radius))
		{
			return true;
		}
	}
	return false;
}


bool LightMgr::isInDarknessRect(const tt::math::VectorRect& p_rect) const
{
	for (Rects::const_iterator it = m_darknessRects.begin(); it != m_darknessRects.end(); ++it)
	{
		const tt::math::VectorRect& rect = (*it);
		if (rect.intersects(p_rect))
		{
			return true;
		}
	}
	return false;
}


bool LightMgr::isEntityInLightImpl(const entity::Entity& p_entity) const
{
	if (p_entity.isSuspended())
	{
		return false;
	}
	
	bool isInLight = false;
	const entity::Entity::DetectionPoints& points(p_entity.getLightDetectionPoints());
	const tt::math::Vector2 position(p_entity.getCenterPosition());
	for (entity::Entity::DetectionPoints::const_iterator it = points.begin(); it != points.end(); ++it)
	{
		if (isPositionInLight(position + (*it)))
		{
			isInLight = true;
			break;
		}
	}
	
	if (isInLight == false)
	{
		return false;
	}
	return true;
}


void LightMgr::setLevelLightAmbientImpl(s32 p_ambient)
{
	TT_NULL_ASSERT(m_wholeLevelAmbientAlpha);
	tt::math::clamp(p_ambient, s32(0), s32(255));
	if (m_lightAmbient != p_ambient)
	{
		m_lightAmbient = p_ambient;
		u8 newAmbient  = static_cast<u8>(p_ambient);
		
		using tt::engine::renderer::ColorRGBA;
		m_wholeLevelAmbientAlpha->setColor(ColorRGBA(newAmbient, newAmbient, newAmbient, newAmbient));
	}
}


void LightMgr::renderFullScreenQuad2D(const tt::engine::renderer::Quad2DPtr& p_quad)
{
	using tt::engine::renderer::Renderer;
	using tt::engine::renderer::MatrixStack;
	Renderer*    renderer = Renderer::getInstance();
	MatrixStack* stack    = MatrixStack::getInstance();
	
	renderer->setTexture(tt::engine::renderer::TexturePtr());
	
	stack->push();
	stack->multiply44(m_fullScreenMtx);
	stack->updateWorldMatrix();
	
	p_quad->render();
	
	stack->pop();
}


// Namespace end
}
}
}

