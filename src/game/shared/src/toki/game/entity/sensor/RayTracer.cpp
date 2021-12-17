#include <tt/code/bufferutils.h>
#include <tt/code/TileRayTracer.h>

#include <toki/game/entity/sensor/RayTracer.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/game/Game.h>
#include <toki/level/helpers.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class RayTracerHitTesterExtended
{
public:
	inline RayTracerHitTesterExtended(const level::AttributeLayerPtr&     p_attributeLayer,
	                                  const RayTracer::HitTesterSettings* p_settings)
	:
	m_attributeLayer(p_attributeLayer),
	m_settings(p_settings),
	m_tilesToIgnore(0),
	m_tileRegMgr(AppGlobal::getGame()->getTileRegistrationMgr()),
	m_fluidsLayer(AppGlobal::getGame()->getFluidMgr().getLayer())
	{
		TT_NULL_ASSERT(p_settings);
		const Entity* entityToIgnore = p_settings->entityToIgnore.getPtr();
		m_tilesToIgnore = (entityToIgnore != 0) ? entityToIgnore->getCollisionTiles() : 0;
	}
	
	
	inline bool operator()(const tt::math::Point2& p_location) const
	{
		if (m_attributeLayer->contains(p_location) == false)
		{
			// Hit outside level
			return true;
		}
		
		bool hitSomething = false;
		
		// Check for collision with one of the tiles considered blocking
		const level::CollisionTypes typesAtPos(m_tileRegMgr.getCollisionTypesFromRegisteredTiles(
				p_location, m_attributeLayer, m_tilesToIgnore, EntityHandle(), m_settings->ignoreActiveCollision, true));
		
		if (m_settings->stopOnCollisionTypes.checkAnyFlags(typesAtPos))
		{
			return true;
		}
		
		if (typesAtPos.noOtherFlags(level::g_collisionTypesAir) == false)
		{
			hitSomething = true;
		}
		
		// Check for fluid types
		if (m_settings->stopOnPool.isEmpty() == false ||
		    m_settings->stopOnFall.isEmpty() == false ||
		    m_settings->stopOnEmpty)
		{
			const fluid::FluidFlowType flowType = m_fluidsLayer->getFluidFlowType(p_location);
			if (flowType != fluid::FluidFlowType_None)
			{
				const fluid::FluidType fluidType = m_fluidsLayer->getFluidType(p_location);
				
				if (fluid::isFalling(flowType) == false && m_settings->stopOnPool.checkFlag(fluidType))
				{
					return true;
				}
				
				if (fluid::isFall(flowType) && m_settings->stopOnFall.checkFlag(fluidType))
				{
					return true;
				}
				
				hitSomething = true;
			}
		}
		
		if (m_settings->stopOnEmpty && hitSomething == false)
		{
			return true;
		}
		
		return false;
	}
	
private:
	// No assignment
	RayTracerHitTesterExtended& operator=(const RayTracerHitTesterExtended&);
	
	
	level::AttributeLayerPtr            m_attributeLayer;
	const RayTracer::HitTesterSettings* m_settings;
	const EntityTiles*                  m_tilesToIgnore;
	const level::TileRegistrationMgr&   m_tileRegMgr;
	level::AttributeLayerPtr            m_fluidsLayer;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

void RayTracer::setStopOnEmpty(bool p_stop)
{
	m_hitTesterSettings.stopOnEmpty = p_stop;
}


void RayTracer::setStopOnSolid(bool p_stop)
{
	m_hitTesterSettings.stopOnSolid = p_stop;
	updateCollisionTypeMask();
}


void RayTracer::setStopOnCrystal(bool p_stop)
{
	m_hitTesterSettings.stopOnCrystal = p_stop;
	updateCollisionTypeMask();
}


void RayTracer::setStopOnWaterPool(bool p_stop)
{
	m_hitTesterSettings.stopOnPool.setFlag(fluid::FluidType_Water, p_stop);
}


void RayTracer::setStopOnWaterFall(bool p_stop)
{
	m_hitTesterSettings.stopOnFall.setFlag(fluid::FluidType_Water, p_stop);
}


void RayTracer::setStopOnLavaPool(bool p_stop)
{
	m_hitTesterSettings.stopOnPool.setFlag(fluid::FluidType_Lava, p_stop);
}


void RayTracer::setStopOnLavaFall(bool p_stop)
{
	m_hitTesterSettings.stopOnFall.setFlag(fluid::FluidType_Lava, p_stop);
}


void RayTracer::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	TT_STATIC_ASSERT(sizeof(level::CollisionTypes::ValueType) == sizeof(u32));
	TT_STATIC_ASSERT(sizeof(fluid::FluidTypes::ValueType)     == sizeof(u32));
	
	bu::put      (m_hitLocation,                                                         p_context);
	bu::put      (m_hasCollision,                                                        p_context);
	bu::put      (m_hasHitlocation,                                                      p_context);
	
	bu::put      (m_hitTesterSettings.ignoreActiveCollision,                             p_context);
	bu::putHandle(m_hitTesterSettings.entityToIgnore,                                    p_context);
	bu::put      (static_cast<u32>(m_hitTesterSettings.stopOnCollisionTypes.getFlags()), p_context);
	bu::put      (m_hitTesterSettings.stopOnSolid,                                       p_context);
	bu::put      (m_hitTesterSettings.stopOnCrystal,                                     p_context);
	bu::put      (m_hitTesterSettings.stopOnEmpty,                                       p_context);
	bu::put      (static_cast<u32>(m_hitTesterSettings.stopOnPool.getFlags()),           p_context);
	bu::put      (static_cast<u32>(m_hitTesterSettings.stopOnFall.getFlags()),           p_context);
}


void RayTracer::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	TT_STATIC_ASSERT(sizeof(level::CollisionTypes::ValueType) == sizeof(u32));
	TT_STATIC_ASSERT(sizeof(fluid::FluidTypes::ValueType)     == sizeof(u32));
	
	m_hitLocation                             = bu::get<tt::math::Vector2>(p_context);
	m_hasCollision                            = bu::get<bool>(p_context);
	m_hasHitlocation                          = bu::get<bool>(p_context);
	
	m_hitTesterSettings.ignoreActiveCollision = bu::get<bool>(p_context);
	m_hitTesterSettings.entityToIgnore        = bu::getHandle<Entity>(p_context);
	m_hitTesterSettings.stopOnCollisionTypes  = level::CollisionTypes(bu::get<u32>(p_context));
	m_hitTesterSettings.stopOnSolid           = bu::get<bool>(p_context);
	m_hitTesterSettings.stopOnCrystal         = bu::get<bool>(p_context);
	m_hitTesterSettings.stopOnEmpty           = bu::get<bool>(p_context);
	m_hitTesterSettings.stopOnPool            = fluid::FluidTypes(bu::get<u32>(p_context));
	m_hitTesterSettings.stopOnFall            = fluid::FluidTypes(bu::get<u32>(p_context));
}


bool RayTracer::trace(const tt::math::Vector2& p_startLocation,
                      const tt::math::Vector2& p_endLocation) const
{
	RayTracerHitTesterExtended hitTester(m_attributeLayer, &m_hitTesterSettings);
	m_hasCollision = tt::code::tileRayTrace(p_startLocation, p_endLocation, hitTester, &m_hitLocation) == false;
	
	// Now the hitlocation is valid
	m_hasHitlocation = true;
	
	return m_hasCollision == false;
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

void RayTracer::updateCollisionTypeMask()
{
	m_hitTesterSettings.stopOnCollisionTypes = level::CollisionTypes();
	
	// First pass: enable all types that should be enabled
	if (m_hitTesterSettings.stopOnSolid)       m_hitTesterSettings.stopOnCollisionTypes.setFlags(level::g_collisionTypesSolid);
	if (m_hitTesterSettings.stopOnCrystal)     m_hitTesterSettings.stopOnCollisionTypes.setFlags(level::g_collisionTypesCrystal);
	
	// Second pass: remove all types that should not be enabled
	if (m_hitTesterSettings.stopOnSolid       == false) m_hitTesterSettings.stopOnCollisionTypes.resetFlags(level::g_collisionTypesSolid);
	if (m_hitTesterSettings.stopOnCrystal     == false) m_hitTesterSettings.stopOnCollisionTypes.resetFlags(level::g_collisionTypesCrystal);
}

// Namespace end
}
}
}
}
