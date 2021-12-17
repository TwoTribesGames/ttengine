#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>

#if !defined(TT_BUILD_FINAL)
#	include <tt/engine/debug/DebugRenderer.h>
#	include <tt/engine/renderer/Renderer.h>
#endif

#include <toki/game/entity/effect/EffectRectMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

//--------------------------------------------------------------------------------------------------
// Public member functions

EffectRectMgr::EffectRectMgr(s32 p_reserveCount)
:
m_effectRects(p_reserveCount),
m_latestContext(tt::math::Vector2::zero, tt::math::Vector2::zero)
{
}


EffectRectHandle EffectRectMgr::createEffectRect(EffectRectTarget            p_targetType,
                                                 const entity::EntityHandle& p_owner)
{
	if (isValidEffectRectTarget(p_targetType) == false)
	{
		TT_PANIC("Invalid effect rect target specified: %d. Not creating a Effect Rect.", p_targetType);
		return EffectRectHandle();
	}
	
	EffectRectHandle handle = m_effectRects.create(EffectRect::CreationParams(p_targetType, p_owner));
	
	EffectRect* rect = getEffectRect(handle);
	rect->update(0.0f, m_latestContext);
	
	return handle;
}


void EffectRectMgr::destroyEffectRect(EffectRectHandle& p_handle)
{
	m_effectRects.destroy(p_handle);
	
	p_handle.invalidate();
}


void EffectRectMgr::update(real p_elapsedTime, const Camera& p_camera)
{
	const tt::math::Vector2 cameraPos(p_camera.getCurrentPositionWithEffects());
	
	entity::Entity* entity = p_camera.getFollowEntity().getPtr();
	
	const tt::math::Vector2 controllingEntityPos( (entity != 0 && entity->isInitialized() ) ?
	                                               entity->getCenterPosition() : tt::math::Vector2::zero );
	
	m_latestContext = EffectRect::EffectRectContext(cameraPos, controllingEntityPos);
	
	EffectRect* effectRect = m_effectRects.getFirst();
	for (s32 i = 0; i <  m_effectRects.getActiveCount(); ++i, ++effectRect)
	{
		effectRect->update(p_elapsedTime, m_latestContext);
	}
	
	m_effectMgr.update(p_elapsedTime);
}


void EffectRectMgr::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	using tt::engine::renderer::Renderer;
	using tt::engine::debug::DebugRendererPtr;
	const DebugRendererPtr& debug = Renderer::getInstance()->getDebug();
	
	const EffectRect* effectRect = m_effectRects.getFirst();
	for (s32 i = 0; i <  m_effectRects.getActiveCount(); ++i, ++effectRect)
	{
		effectRect->renderDebug();
	}
	
	tt::engine::renderer::ColorRGB color(0, 0, 128);
	color.r = 255; // Camera
	debug->renderCircle(color, m_latestContext.cameraPos,            0.15f);
	color.r = 0;   // Reset
	color.g = 255; // Controlling Entity.
	debug->renderCircle(color,  m_latestContext.controllingEntityPos, 0.15f);
#endif
}


void EffectRectMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(
			serialization::Section_EffectRectMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the EffectRectMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::serializeHandleArrayMgr(m_effectRects, &context);
	
	// If we add more to context then we might want to give it a serialize function.
	bu::put(m_latestContext.cameraPos,            &context);
	bu::put(m_latestContext.controllingEntityPos, &context);
	
	m_effectMgr.serialize(&context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void EffectRectMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(
			serialization::Section_EffectRectMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the EffectRectMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::unserializeHandleArrayMgr(&m_effectRects, &context);
	
	const tt::math::Vector2 camPos        = bu::get<tt::math::Vector2>(&context);
	const tt::math::Vector2 ctrlEntityPos = bu::get<tt::math::Vector2>(&context);
	m_latestContext = EffectRect::EffectRectContext(camPos, ctrlEntityPos);
	
	m_effectMgr.unserialize(&context);
}

// Namespace end
}
}
}
}
