#include <tt/engine/physics/CollisionData.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace physics {


CollisionData::CollisionData()
:
m_method(ReturnMethod_First),
m_collisionData()
{
	// Allocate space in container
	m_collisionData.reserve(16);
}


void CollisionData::reset()
{
	// Clear all collisions
	m_collisionData.clear();
}


void CollisionData::addCollision(const Collision& p_collision)
{
	// First see if we need to add a new p_collision or not
	if (m_method == ReturnMethod_First)
	{
		if(m_collisionData.empty())
		{
			// If no other collisions, insert it
			m_collisionData.push_back(p_collision);
		}
		else
		{
			// Otherwise, overwrite first element
			m_collisionData[0] = p_collision;
		}
	}
	else if (m_method == ReturnMethod_Closest)
	{
		// We are only returning the closest, so compare with first
		if (m_collisionData.empty() == false && 
			 math::fabs(p_collision.getCollisionTime()) > math::fabs(m_collisionData[0].getCollisionTime()))
		{
			// Current element is closer, do not replace
			return;
		}
		else
		{
			if(m_collisionData.empty())
			{
				// If no other collisions, insert it
				m_collisionData.push_back(p_collision);
			}
			else
			{
				// Otherwise, overwrite first element
				m_collisionData[0] = p_collision;
			}
		}
	}
	else
	{
		// Just add to the container
		m_collisionData.push_back(p_collision);
	}
}


// Namespace end
}
}
}

