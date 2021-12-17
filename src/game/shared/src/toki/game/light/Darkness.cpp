#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/light/Darkness.h>
#include <toki/game/light/DarknessMgr.h>
#include <toki/game/light/Polygon.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace game {
namespace light {

//--------------------------------------------------------------------------------------------------
// Public member functions

Darkness::Darkness(const CreationParams& p_creationParams, const DarknessHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_source(p_creationParams.source),
m_width(p_creationParams.width),
m_height(p_creationParams.height),
m_enabled(true),
m_poly()
{
	using tt::math::Vector2;
	
	Vertices vertices;
	{
		const real halfWidth  = m_width  * 0.5f;
		const real halfHeight = m_height * 0.5f;
		
		// Create vertices which are larger than level size.
		vertices.push_back(Vector2(-halfWidth, -halfHeight));
		vertices.push_back(Vector2( halfWidth, -halfHeight));
		vertices.push_back(Vector2( halfWidth,  halfHeight));
		vertices.push_back(Vector2(-halfWidth,  halfHeight));
	}
	
	using tt::engine::renderer::ColorRGB;
	using tt::engine::renderer::ColorRGBA;
	
	m_poly = PolygonPtr(new Polygon(Vector2::zero, ColorRGBA(ColorRGB::black, 0), vertices, false));
}


tt::math::VectorRect Darkness::getRect() const
{
	const tt::math::Vector2 centerPos(getWorldPosition());
	m_poly->setPosition(centerPos);
	return tt::math::VectorRect(tt::math::Vector2(centerPos.x - (m_width  * 0.5f),
	                                              centerPos.y - (m_height * 0.5f)),
	                            m_width, m_height);
}


void Darkness::render() const
{
	m_poly->render();
}


void Darkness::setAmbient(u8 p_ambient)
{
	m_poly->setColor(tt::engine::renderer::ColorRGBA(p_ambient, p_ambient, p_ambient, p_ambient));
}


void Darkness::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_source, p_context);
	bu::put      (m_width,  p_context);
	bu::put      (m_height, p_context);
}


Darkness::CreationParams Darkness::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const entity::EntityHandle source = bu::getHandle<entity::Entity>(p_context);
	const real                 width  = bu::get      <real          >(p_context);
	const real                 height = bu::get      <real          >(p_context);
	
	return CreationParams(source, width, height);
}


void Darkness::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_enabled, p_context);
}


void Darkness::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	
	bool enabled = bu::get<bool>(p_context);
	
	setEnabled(enabled);
}


Darkness* Darkness::getPointerFromHandle(const DarknessHandle& p_handle)
{
	if (AppGlobal::hasGame() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getDarknessMgr().getDarkness(p_handle);
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

tt::math::Vector2 Darkness::getWorldPosition() const
{
	const entity::Entity* source = m_source.getPtr();
	if (source != 0)
	{
		return source->getCenterPosition();
	}
	
	TT_PANIC("Darkness has invalid source");
	
	return tt::math::Vector2::zero;
}


// Namespace end
}
}
}
