#if !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_PHYSICSSETTINGS_H)
#define INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_PHYSICSSETTINGS_H


#include <string>

#include <tt/code/fwd.h>
#include <tt/platform/tt_types.h>
#include <tt/math/Vector2.h>

#include <toki/game/entity/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {

struct PhysicsSettings
{
	// Used within physics intergation
	real mass;
	real drag;
	real collisionDrag;
	
	// Used to modify input for integration
	real thrust;
	tt::math::Vector2 extraForce;
	
	// Used by Higher level code.
	real easeOutDistance;
	real moveEndDistance; // How close the the end point is send as enough. (movement end)
	
	real speedFactor; // HACK: Multiple the final speed with this.
	real bouncyness;
	
	bool hasCollisionWithSolid;
	bool isCollisionMovementFailure;
	bool shouldTurn;
	
	std::string turnAnimationName;
	std::string presentationAnimationName;
	
	tt::math::Vector2 pathOffset; // Extra offset to target position for path finding. (own orientation is applied)
	
	inline PhysicsSettings(real p_mass,
	                       real p_drag,
	                       real p_thrust,
	                       real p_easeOutDistance,
	                       real p_moveEndDistance)
	:
	mass(p_mass),
	drag(p_drag),
	collisionDrag(p_drag),
	thrust(p_thrust),
	extraForce(tt::math::Vector2::zero),
	easeOutDistance(p_easeOutDistance),
	moveEndDistance(p_moveEndDistance),
	speedFactor(1.0f),
	bouncyness(0.0f),
	hasCollisionWithSolid(true),
	isCollisionMovementFailure(false),
	shouldTurn(false),
	turnAnimationName(),
	presentationAnimationName(),
	pathOffset(tt::math::Vector2::zero),
	m_clampType(Clamp_None),
	m_clampPos(0.0f, 0.0f),
	m_clampOther(0.0f, 0.0f)
	{ }
	
	inline PhysicsSettings()
	:
	mass(1.0f),
	drag(2.0f),
	collisionDrag(2.0f),
	thrust(16.0f),
	extraForce(tt::math::Vector2::zero),
	easeOutDistance(1.0f),
	moveEndDistance(0.01f),
	speedFactor(1.0f),
	hasCollisionWithSolid(true),
	isCollisionMovementFailure(true),
	shouldTurn(false),
	turnAnimationName(),
	presentationAnimationName(),
	pathOffset(tt::math::Vector2::zero),
	m_clampType(Clamp_None),
	m_clampPos(0.0f, 0.0f),
	m_clampOther(0.0f, 0.0f)
	{ }
	
	void                   serialize  (tt::code::BufferWriteContext* p_context) const;
	static PhysicsSettings unserialize(tt::code::BufferReadContext*  p_context);
	
	bool clampEntity(Entity* p_entity, const tt::math::Vector2& p_oldPos, tt::math::Vector2* p_speed) const;
	
	inline void clearClamp() { m_clampType = Clamp_None; }
	inline void setRectClamp(const tt::math::VectorRect& p_rect)
	{
		m_clampType  = Clamp_Rect;
		m_clampPos   = p_rect.getMin();
		m_clampOther = p_rect.getMaxEdge();
	}
	inline void setCircleClamp(const tt::math::Vector2& p_centerPos, real p_radius)
	{
		m_clampType    = Clamp_Circle;
		m_clampPos     = p_centerPos;
		m_clampOther.x = p_radius;
		m_clampOther.y = p_radius * p_radius;
	}
	
	enum Clamp
	{
		Clamp_None,
		Clamp_Rect,
		Clamp_Circle
	};
	
	Clamp                m_clampType;
	tt::math::Vector2    m_clampPos;   // Min for rect, centerpos for circle.
	tt::math::Vector2    m_clampOther; // Max for rect, x is radius and y is radius^2 for circle.
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_PHYSICSSETTINGS_H)
