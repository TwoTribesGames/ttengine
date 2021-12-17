 #include <tt/code/bufferutils.h>

#include <toki/game/entity/movementcontroller/PhysicsSettings.h>
#include <toki/game/entity/Entity.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {

//--------------------------------------------------------------------------------------------------
// Public member functions

void PhysicsSettings::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(mass,                       p_context);
	bu::put(drag,                       p_context);
	bu::put(collisionDrag,              p_context);
	bu::put(thrust,                     p_context);
	bu::put(extraForce,                 p_context);
	bu::put(easeOutDistance,            p_context);
	bu::put(moveEndDistance,            p_context);
	bu::put(speedFactor,                p_context);
	bu::put(bouncyness,                 p_context);
	bu::put(hasCollisionWithSolid,      p_context);
	bu::put(isCollisionMovementFailure, p_context);
	bu::put(shouldTurn                , p_context);
	bu::put(turnAnimationName         , p_context);
	bu::put(presentationAnimationName,  p_context);
	bu::put(pathOffset,                 p_context);
	bu::putEnum<u8, Clamp>(m_clampType, p_context);
	bu::put(m_clampPos,                 p_context);
	bu::put(m_clampOther,               p_context);
}


PhysicsSettings PhysicsSettings::unserialize(tt::code::BufferReadContext*  p_context)
{
	PhysicsSettings settings;
	
	namespace bu = tt::code::bufferutils;
	
	settings.mass                       = bu::get<real             >(p_context);
	settings.drag                       = bu::get<real             >(p_context);
	settings.collisionDrag              = bu::get<real             >(p_context);
	settings.thrust                     = bu::get<real             >(p_context);
	settings.extraForce                 = bu::get<tt::math::Vector2>(p_context);
	settings.easeOutDistance            = bu::get<real             >(p_context);
	settings.moveEndDistance            = bu::get<real             >(p_context);
	settings.speedFactor                = bu::get<real             >(p_context);
	settings.bouncyness                 = bu::get<real             >(p_context);
	settings.hasCollisionWithSolid      = bu::get<bool             >(p_context);
	settings.isCollisionMovementFailure = bu::get<bool             >(p_context);
	settings.shouldTurn                 = bu::get<bool             >(p_context);
	settings.turnAnimationName          = bu::get<std::string      >(p_context);
	settings.presentationAnimationName  = bu::get<std::string      >(p_context);
	settings.pathOffset                 = bu::get<tt::math::Vector2>(p_context);
	settings.m_clampType                = bu::getEnum<u8, Clamp    >(p_context);
	settings.m_clampPos                 = bu::get<tt::math::Vector2>(p_context);
	settings.m_clampOther               = bu::get<tt::math::Vector2>(p_context);
	
	return settings;
}


bool PhysicsSettings::clampEntity(Entity* p_entity, 
								  const tt::math::Vector2& p_oldPos, 
								  tt::math::Vector2* p_speed) const
{
	TT_NULL_ASSERT(p_entity);
	TT_NULL_ASSERT(p_speed);
	
	if (m_clampType == Clamp_None)
	{
		return false;
	}
	
	const tt::math::VectorRect entityRect(p_entity->calcWorldRect());
	const tt::math::Vector2& min = entityRect.getMin();
	const tt::math::Vector2  max = entityRect.getMaxEdge();
	
	// Do clamp if needed.
	switch (m_clampType)
	{
	case Clamp_Rect:
		{
			const bool clampX = (min.x < m_clampPos.x || max.x > m_clampOther.x);
			const bool clampY = (min.y < m_clampPos.y || max.y > m_clampOther.y);
			
			if (clampX)
			{
				p_entity->modifyPosition().x = p_oldPos.x;
				p_speed->x = 0.0f;
			}
			if (clampY)
			{
				p_entity->modifyPosition().y = p_oldPos.y;
				p_speed->y = 0.0f;
			}
			if (clampX || clampY)
			{
				// Note: Separate if and return instead of returning result of if so we can add a breakpoint to clamp.
				return true;
			}
		}
		break;
	case Clamp_Circle:
		{
			const tt::math::Vector2 centerPos = p_entity->getCenterPosition();
			
			const tt::math::Vector2 diff(centerPos- m_clampPos);
			const real diffLenSqrt = diff.lengthSquared();
			
			const real& radiusSqrt = m_clampOther.y;
			if (diffLenSqrt > radiusSqrt)
			{
				const tt::math::Vector2 diffNorm(diff.getNormalized());
				{
					const real diffLen = tt::math::sqrt(diffLenSqrt);
					const real& radius = m_clampOther.x;
					const real tooFarDist = diffLen - radius;
					TT_ASSERT(tooFarDist >= 0.0f);
					
					p_entity->modifyPosition() -= diffNorm * tooFarDist;
					
					// Project speed on the line perpendicular to the circle normal.
					real distance = tt::math::dotProduct(diffNorm, *p_speed);
					const tt::math::Vector2 projectionOnCircle = (*p_speed) - (diffNorm * (distance + 0.1f));
					//TT_Printf("Speed %f, %f -> %f, %f (tooFarDist: %f)\n", p_speed->x, p_speed->y, projectionOnCircle.x, projectionOnCircle.y, tooFarDist);
					(*p_speed) = projectionOnCircle;
				}
				return true;
			}
		}
		break;
	default:
		TT_PANIC("Unknown shape: %d\n", m_clampType);
		break;
	}
	return false;
}


// Namespace end
}
}
}
}
