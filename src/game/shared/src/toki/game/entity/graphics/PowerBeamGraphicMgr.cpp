#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/entity/graphics/PowerBeamGraphicMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

//--------------------------------------------------------------------------------------------------
// Public member functions

PowerBeamGraphicMgr::PowerBeamGraphicMgr(s32 p_reserveCount)
:
m_powerBeamGraphics(p_reserveCount),
m_graphicsNeedUpdate(false)
{
}


PowerBeamGraphicHandle PowerBeamGraphicMgr::createPowerBeamGraphic(
		PowerBeamType               p_type,
		const sensor::SensorHandle& p_source)
{
	if (isValidPowerBeamType(p_type) == false)
	{
		TT_PANIC("Invalid power beam type specified: %d. Not creating a power beam.", p_type);
		return PowerBeamGraphicHandle();
	}
	
	return m_powerBeamGraphics.create(PowerBeamGraphic::CreationParams(p_type, p_source));
}


void PowerBeamGraphicMgr::destroyPowerBeamGraphic(PowerBeamGraphicHandle& p_handle)
{
	m_powerBeamGraphics.destroy(p_handle);
	
	p_handle.invalidate();
}


void PowerBeamGraphicMgr::update(real p_elapsedTime)
{
	PowerBeamGraphic* powerBeam = m_powerBeamGraphics.getFirst();
	for (s32 i = 0; i <  m_powerBeamGraphics.getActiveCount(); ++i, ++powerBeam)
	{
		powerBeam->update(p_elapsedTime);
	}
	m_graphicsNeedUpdate = true;
}


void PowerBeamGraphicMgr::updateForRender(const tt::math::VectorRect& p_visibilityRect)
{
	if (m_graphicsNeedUpdate)
	{
		PowerBeamGraphic* powerBeam = m_powerBeamGraphics.getFirst();
		for (s32 i = 0; i <  m_powerBeamGraphics.getActiveCount(); ++i, ++powerBeam)
		{
			powerBeam->updateForRender(p_visibilityRect);
		}
	}
	m_graphicsNeedUpdate = false;
}


void PowerBeamGraphicMgr::render(const tt::math::VectorRect& p_visibilityRect) const
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	const bool fogWasEnabled = renderer->isFogEnabled();
	renderer->setFogEnabled(false);
	// Disable fog for all beam graphics.
	
	const PowerBeamGraphic* powerBeam = m_powerBeamGraphics.getFirst();
	for (s32 i = 0; i < m_powerBeamGraphics.getActiveCount(); ++i, ++powerBeam)
	{
		powerBeam->render(p_visibilityRect);
	}
	
	// Restore fog setting
	renderer->setFogEnabled(fogWasEnabled);
}


void PowerBeamGraphicMgr::renderLightmask(const tt::math::VectorRect& p_visibilityRect) const
{
	using namespace tt::engine::renderer;
	Renderer* renderer(Renderer::getInstance());
	
	// Lights are only rendered to alpha channel.
	renderer->setColorMask(ColorMask_Alpha);
	renderer->setCustomBlendMode(BlendFactor_One, BlendFactor_One);
	renderer->resetCustomBlendModeAlpha();
	
	// NOTE: For now, the power beam light masks are the same as the normal graphics, so just call render()
	render(p_visibilityRect);
	
	// Restore the defaults for normal renders after this.
	renderer->setColorMask(ColorMask_All);
	renderer->setBlendMode(BlendMode_Blend); // Restore the default blend mode.
	renderer->setCustomBlendModeAlpha(BlendFactor_Zero, BlendFactor_InvSrcAlpha);
}


void PowerBeamGraphicMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(
			serialization::Section_PowerBeamGraphicMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the PowerBeamGraphicMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	tt::code::serializeHandleArrayMgr(m_powerBeamGraphics, &context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void PowerBeamGraphicMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(
			serialization::Section_PowerBeamGraphicMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the PowerBeamGraphicMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	tt::code::unserializeHandleArrayMgr(&m_powerBeamGraphics, &context);
}

// Namespace end
}
}
}
}
