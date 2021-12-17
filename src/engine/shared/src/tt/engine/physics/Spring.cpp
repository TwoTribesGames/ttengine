#include <tt/engine/physics/Spring.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace physics {

Spring::Spring()
:
m_position(0,0,0),
m_target(0,0,0),
m_velocity(0,0,0),
m_spring(40.0f),
m_dampen(0.0f)
{
	// Compute dampen from spring
	calculateDampenFromSpring();
}

Spring::Spring(real p_spring)
:
m_position(0,0,0),
m_target(0,0,0),
m_velocity(0,0,0),
m_spring(p_spring),
m_dampen(0.0f)
{
	// Compute dampen from spring
	calculateDampenFromSpring();
}

Spring::~Spring()
{
}

void Spring::calculateDampenFromSpring()
{
	// Compute dampen from spring
	m_dampen = 2.0f * math::sqrt(m_spring);
}

void Spring::update(real p_update)
{
	// Divide by 64 - its about accurate enough for a spring environment
	//update = (update << 12) >> 6;
	//p_update /= 64;

	// Recalculate velocity
	math::Vector3 distance = m_target - m_position;
	math::Vector3 springAcceleration = (distance * m_spring) - (m_velocity * m_dampen);
	
	m_velocity += springAcceleration * p_update;
	
	// TODO: Check if translated correctly from fixed point arithmetic
	m_position.x += m_velocity.x;
	m_position.y += m_velocity.y;
	m_position.z += m_velocity.z;
}


// Namespace end
}
}
}

