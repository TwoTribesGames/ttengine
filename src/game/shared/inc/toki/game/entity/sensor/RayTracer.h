#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_RAYTRACER_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_RAYTRACER_H


#include <tt/code/fwd.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/fluid/types.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class RayTracer
{
public:
	struct HitTesterSettings
	{
		bool                  ignoreActiveCollision;
		EntityHandle          entityToIgnore;
		level::CollisionTypes stopOnCollisionTypes;  // combined checking mask for CollisionTypes that block the hit tester
		bool                  stopOnSolid;           // will be combined into stopOnCollisionTypes
		bool                  stopOnCrystal;         // will be combined into stopOnCollisionTypes
		bool                  stopOnEmpty;           // "air" No collisionType and No fluid.
		fluid::FluidTypes     stopOnPool;            // (isFalling() == false)
		fluid::FluidTypes     stopOnFall;            // (isFall()
		
		
		inline HitTesterSettings()
		:
		ignoreActiveCollision(false),
		entityToIgnore(),
		stopOnCollisionTypes(level::g_collisionTypesSolid),
		stopOnSolid(true),
		stopOnCrystal(true),
		stopOnEmpty(false),
		stopOnPool(),
		stopOnFall()
		{
		}
	};
	
	
	inline explicit RayTracer(const level::AttributeLayerPtr& p_attributeLayer)
	:
	m_hitLocation(tt::math::Vector2::zero),
	m_hasCollision(false),
	m_hasHitlocation(false),
	m_attributeLayer(p_attributeLayer),
	m_hitTesterSettings()
	{
	}
	
	// Ignores the collision tiles of a specific entity
	inline void setIgnoreActiveCollision(bool p_ignoreActiveCollision)
	{
		m_hitTesterSettings.ignoreActiveCollision = p_ignoreActiveCollision;
	}
	
	// Ignores the collision tiles of a specific entity
	inline void setIgnoreEntityCollision(const entity::EntityHandle& p_entityToIgnore)
	{
		m_hitTesterSettings.entityToIgnore = p_entityToIgnore;
	}
	
	void setStopOnEmpty(bool p_stop);
	
	void setStopOnSolid      (bool p_stop);
	void setStopOnCrystal    (bool p_stop);
	
	void setStopOnWaterPool(bool p_stop);
	void setStopOnWaterFall(bool p_stop);
	void setStopOnLavaPool (bool p_stop);
	void setStopOnLavaFall (bool p_stop);
	
	
	inline bool                        shouldIgnoreActiveCollision() const { return m_hitTesterSettings.ignoreActiveCollision; }
	inline const entity::EntityHandle& getCollisionIgnoreEntity()    const { return m_hitTesterSettings.entityToIgnore;        }
	
	inline bool getStopOnEmpty()       const { return m_hitTesterSettings.stopOnEmpty;       }
	inline bool getStopOnSolid()       const { return m_hitTesterSettings.stopOnSolid;       }
	inline bool getStopOnCrystal()     const { return m_hitTesterSettings.stopOnCrystal;     }
	inline bool getStopOnWaterPool()   const { return m_hitTesterSettings.stopOnPool.checkFlag(fluid::FluidType_Water); }
	inline bool getStopOnWaterFall()   const { return m_hitTesterSettings.stopOnFall.checkFlag(fluid::FluidType_Water); }
	inline bool getStopOnLavaPool()    const { return m_hitTesterSettings.stopOnPool.checkFlag(fluid::FluidType_Lava ); }
	inline bool getStopOnLavaFall()    const { return m_hitTesterSettings.stopOnFall.checkFlag(fluid::FluidType_Lava ); }
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	bool trace(const tt::math::Vector2& p_startLocation,
	           const tt::math::Vector2& p_endLocation) const;
	
	inline const tt::math::Vector2& getHitLocation() const { TT_ASSERT(m_hasHitlocation); return m_hitLocation; }
	inline bool                     hasCollision()   const { return m_hasCollision; }
	inline bool                     hasHitlocation() const { return m_hasHitlocation; }
	inline void                     resetHitLocation()     { m_hasHitlocation = false; }
private:
	void updateCollisionTypeMask();
	
	mutable tt::math::Vector2 m_hitLocation;
	mutable bool              m_hasCollision;
	mutable bool              m_hasHitlocation;
	
	level::AttributeLayerPtr  m_attributeLayer;
	HitTesterSettings         m_hitTesterSettings;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_RAYTRACER_H)
