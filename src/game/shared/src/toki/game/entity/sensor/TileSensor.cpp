#include <algorithm>
#include <iterator>

#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/entity/sensor/TileSensor.h>
#include <toki/game/entity/sensor/Shape.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>

// FIXME: Remove debug rendering
#include <toki/game/DebugView.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

//--------------------------------------------------------------------------------------------------
// Public member functions

TileSensor::TileSensor(const CreationParams& p_creationParams, const TileSensorHandle& p_ownHandle)
:
m_inLocalSpace(true),
m_translation(tt::math::Vector2::zero),
m_enabled(true),
m_suspended(false),
m_ownHandle(p_ownHandle),
m_parent(p_creationParams.source),
m_shape(p_creationParams.shape),
m_ignoreOwnCollision(false),
m_isDirty(false)
{
	TT_NULL_ASSERT(m_shape);
}


TileSensor::~TileSensor()
{
	// Should we do a similair exit for tiles?
	/*
	if (m_sensedEntities.empty() == false)
	{
		Entity* sourceEntity = m_parent.getPtr();
		if (sourceEntity != 0 && sourceEntity->isInitialized())
		{
			// Handle onexit callbacks
			script::EntityBasePtr script(sourceEntity->getEntityScript());
			for (EntityHandles::const_iterator it = m_sensedEntities.begin();
			     it != m_sensedEntities.end(); ++it)
			{
				handleOnExit(script, (*it));
			}
		}
	}
	*/
}


void TileSensor::update()
{
	Entity* parent = m_parent.getPtr();
	if (parent == 0 || parent->isSuspended() || m_shape == 0 || m_suspended)
	{
		return;
	}
	
	m_isDirty = false;
	if (m_shape != 0)
	{
		m_shape->updateTransform(*parent, getWorldPosition());
	}
	
	if (isEnabled() && isActive(parent))
	{
		level::CollisionTypes newCollisionTouching;
		fluid::FluidTypes     newFluidTouching;
		fluid::FluidTypes     newFallTouching;
		
		// FIXME: Work on tiles not rects for non-box shapes.
		tt::math::PointRect tileRect(m_shape->getBoundingTileRect());
		const tt::math::Point2 tileMin = tileRect.getMin();
		const tt::math::Point2 tileMax = tileRect.getMaxInside();
		
		fluid::FluidType  notFallingInside;
		fluid::FluidType  fallInside; // Waterfall etc
		
		Game* game = AppGlobal::getGame();
		TT_NULL_ASSERT(game);
		game->getFluidMgr().getFluidTypes(tileMin,
		                                  tileMax,
		                                  newFluidTouching,
		                                  notFallingInside,
		                                  newFallTouching,
		                                  fallInside);
		
		
		const level::AttributeLayerPtr& layer  = AppGlobal::getGame()->getAttributeLayer();
		const EntityTiles* entityTilesToIgnore = (m_ignoreOwnCollision) ? parent->getCollisionTiles() : 0;
		
		for (tt::math::Point2 pos = tileMin; pos.y <= tileMax.y; ++pos.y)
		{
			for (pos.x = tileMin.x; pos.x <= tileMax.x; ++pos.x)
			{
				level::CollisionTypes collisionTypes = 
					game->getTileRegistrationMgr().getCollisionTypesFromRegisteredTiles(pos, layer, entityTilesToIgnore);
				newCollisionTouching.setFlags(collisionTypes);
			}
		}
		
		doCallbacks(newCollisionTouching, newFluidTouching, newFallTouching);
	}
}


void TileSensor::renderDebug() const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	{
		Game* game = AppGlobal::getGame();
		toki::DebugRenderMask mask = AppGlobal::getDebugRenderMask();
		
		if (mask.checkFlag(DebugRender_EntityTileSensorShapes) == false)
		{
			return;
		}
		
		const tt::math::Vector2 sensorPosition(getWorldPosition());
		
		DebugView& debugView = game->getDebugView();
		
		const Entity* parent = m_parent.getPtr();
		if (parent == 0)
		{
			return;
		}
		
		// Render debug texts
		{
			using namespace tt::engine::renderer;
			Camera& cam = AppGlobal::getGame()->getCamera();
			tt::math::Vector2 scrPos(cam.worldToScreen(sensorPosition));
			
			s32 textCount = 0;
			if (m_ignoreOwnCollision)    ++textCount;
			if (m_inLocalSpace == false) ++textCount;
			
			const s32 textHeight = 15;
			real yOffset = -(textCount * textHeight) / 2.0f - 5;
			
			if (m_inLocalSpace == false)
			{
				debugView.registerText(
					DebugView::TextInfo(scrPos + tt::math::Vector2(-25.0f, yOffset), "worldspace", true,
					                    tt::engine::renderer::ColorRGB::green, 0.0f));
			}
			yOffset += textHeight;
			
			if (m_ignoreOwnCollision)
			{
				debugView.registerText(
					DebugView::TextInfo(scrPos + tt::math::Vector2(-20.0f, yOffset), "ign own col", true,
					                    tt::engine::renderer::ColorRGB::green, 0.0f));
			}
			yOffset += textHeight;
		}
		
		if (m_shape != 0)
		{
			m_shape->visualize(getDebugColor());
		}
	}
#endif
}


void TileSensor::setWorldPosition(const tt::math::Vector2& p_position)
{
	m_inLocalSpace = false;
	m_translation  = p_position;
}


void TileSensor::setOffset(const tt::math::Vector2& p_offset)
{
	m_inLocalSpace = true;
	m_translation  = p_offset;
}


tt::math::Vector2 TileSensor::getWorldPosition() const
{
	if (m_inLocalSpace == false)
	{
		return m_translation;
	}
	
	const entity::Entity* parent = m_parent.getPtr();
	if (parent != 0)
	{
		return parent->getCenterPosition() + parent->applyOrientationToVector2(m_translation);
	}
	
	TT_PANIC("TileSensor has invalid parent");
	
	return tt::math::Vector2::zero;
}


void TileSensor::setIgnoreOwnCollision(bool p_ignoreOwnCollision)
{
	m_ignoreOwnCollision = p_ignoreOwnCollision;
}


tt::engine::renderer::ColorRGBA TileSensor::getDebugColor() const
{
	const entity::Entity* source = m_parent.getPtr();
	TT_NULL_ASSERT(source);
	
	using tt::engine::renderer::ColorRGBA;
	
	if (isActive(source) && isEnabled())
	{
		return ColorRGBA(100, 155,  55, 255);
	}
	return ColorRGBA(100, 100, 100, 255);
}


void TileSensor::setShape(const ShapePtr& p_shape)
{
	TT_NULL_ASSERT(p_shape);
	if (p_shape != 0)
	{
		m_shape = p_shape;
	}
}


void TileSensor::setEnabled(bool p_enabled)
{
	if (m_enabled == p_enabled)
	{
		return;
	}
	
	if (p_enabled)
	{
		m_isDirty = true;
	}
	else
	{
		doCallbacks(level::CollisionTypes(), fluid::FluidTypes(), fluid::FluidTypes());
	}
	m_enabled = p_enabled;
}


void TileSensor::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_parent, p_context);
	
	const bool haveShape = (m_shape != 0);
	bu::put(haveShape, p_context);
	if (haveShape)
	{
		m_shape->serialize(p_context);
	}
}


TileSensor::CreationParams TileSensor::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const EntityHandle source = bu::getHandle<Entity>(p_context);
	
	const bool haveShape = bu::get<bool>(p_context);
	ShapePtr shape;
	if (haveShape)
	{
		shape = Shape::unserialize(p_context);
	}
	
	return CreationParams(source, shape);
}


void TileSensor::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_inLocalSpace,          p_context);
	bu::put(m_translation,           p_context);
	bu::put(m_enabled,               p_context);
	bu::put(m_suspended,             p_context);
	
	//m_ownHandle // Done by client code
	//m_parent    // Done in serializeCreationParams
	//m_shape     // Done in serializeCreationParams
	bu::put(m_ignoreOwnCollision,    p_context);
	bu::put(m_isDirty,               p_context);
	
	bu::putBitMask(m_collisionTouching, p_context);
	bu::putBitMask(m_fluidTouching,     p_context);
	bu::putBitMask(m_fallTouching,      p_context);
}


void TileSensor::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_inLocalSpace          = bu::get<bool             >(p_context);
	m_translation           = bu::get<tt::math::Vector2>(p_context);
	m_enabled               = bu::get<bool             >(p_context);
	m_suspended             = bu::get<bool             >(p_context);
	//m_ownHandle // Done by client code 
	//m_parent    // Done in unserializeCreationParams
	//m_shape     // Done in unserializeCreationParams
	m_ignoreOwnCollision    = bu::get<bool             >(p_context);
	m_isDirty               = bu::get<bool             >(p_context);
	
	m_collisionTouching     = bu::getBitMask<level::CollisionTypes::Flag,
	                                         level::CollisionTypes::FlagCount>(p_context);
	m_fluidTouching         = bu::getBitMask<fluid::FluidTypes::Flag,
	                                         fluid::FluidTypes::FlagCount>(p_context);
	m_fallTouching          = bu::getBitMask<fluid::FluidTypes::Flag,
	                                         fluid::FluidTypes::FlagCount>(p_context);
}


TileSensor* TileSensor::getPointerFromHandle(const TileSensorHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getTileSensorMgr().getSensor(p_handle);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void TileSensor::doCallbacks(const level::CollisionTypes& p_newCollisionTouching,
                             const fluid::FluidTypes&     p_newFluidTouching,
                             const fluid::FluidTypes&     p_newFallTouching)
{
	Entity* parent = m_parent.getPtr();
	script::EntityBasePtr script(parent->getEntityScript());
	TT_NULL_ASSERT(script);
	
	{
		// Collision
		if (p_newCollisionTouching.checkAnyFlags(level::g_collisionTypesSolid) &&
		    m_collisionTouching.checkAnyFlags(level::g_collisionTypesSolid) == false)
		{
			script->onTileSensorSolidTouchEnter(m_ownHandle);
		}
		else if (p_newCollisionTouching.checkAnyFlags(level::g_collisionTypesSolid) == false &&
		         m_collisionTouching.checkAnyFlags(level::g_collisionTypesSolid))
		{
			script->onTileSensorSolidTouchExit(m_ownHandle);
		}
		
		// Fluid - Water
		if (p_newFluidTouching.checkFlag(fluid::FluidType_Water) &&
		    m_fluidTouching.checkFlag(fluid::FluidType_Water) == false)
		{
			script->onTileSensorWaterTouchEnter(m_ownHandle);
		}
		else if (p_newFluidTouching.checkFlag(fluid::FluidType_Water) == false &&
		         m_fluidTouching.checkFlag(fluid::FluidType_Water))
		{
			script->onTileSensorWaterTouchExit(m_ownHandle);
		}
		
		if (p_newFallTouching.checkFlag(fluid::FluidType_Water) &&
		    m_fallTouching.checkFlag(fluid::FluidType_Water) == false)
		{
			script->onTileSensorWaterfallTouchEnter(m_ownHandle);
		}
		else if (p_newFallTouching.checkFlag(fluid::FluidType_Water) == false &&
		         m_fallTouching.checkFlag(fluid::FluidType_Water))
		{
			script->onTileSensorWaterfallTouchExit(m_ownHandle);
		}
		
		// Fluid - Lava
		if (p_newFluidTouching.checkFlag(fluid::FluidType_Lava) &&
		    m_fluidTouching.checkFlag(fluid::FluidType_Lava) == false)
		{
			script->onTileSensorLavaTouchEnter(m_ownHandle);
		}
		else if (p_newFluidTouching.checkFlag(fluid::FluidType_Lava) == false &&
		         m_fluidTouching.checkFlag(fluid::FluidType_Lava))
		{
			script->onTileSensorLavaTouchExit(m_ownHandle);
		}
		
		if (p_newFallTouching.checkFlag(fluid::FluidType_Lava) &&
		    m_fallTouching.checkFlag(fluid::FluidType_Lava) == false)
		{
			script->onTileSensorLavafallTouchEnter(m_ownHandle);
		}
		else if (p_newFallTouching.checkFlag(fluid::FluidType_Lava) == false &&
		         m_fallTouching.checkFlag(fluid::FluidType_Lava))
		{
			script->onTileSensorLavafallTouchExit(m_ownHandle);
		}
	}
	
	m_collisionTouching = p_newCollisionTouching;
	m_fluidTouching     = p_newFluidTouching;
	m_fallTouching      = p_newFallTouching;
}


bool TileSensor::isActive(const Entity* p_parent) const
{
	if (p_parent->isSuspended() || m_suspended)
	{
		return false;
	}
	
	if (m_shape == 0)
	{
		return false;
	}
	
	return true;
}



// Namespace end
}
}
}
}
