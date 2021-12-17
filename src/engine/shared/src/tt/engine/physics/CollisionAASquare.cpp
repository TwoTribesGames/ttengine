#include <limits>

#include <tt/engine/physics/CollisionAASquare.h>
#include <tt/engine/physics/Ray.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace physics {

CollisionAASquare::CollisionAASquare()
:
m_pos(math::Vector3::zero),
m_size(0.0f),
m_axis(-1),
m_positiveNormal(true)
{
}


CollisionAASquare::CollisionAASquare(const math::Vector3& p_pos,
                                     real p_size,
                                     const math::Vector3& p_normal)
:
m_pos(p_pos),
m_size(p_size),
m_axis(-1),
m_positiveNormal(true)
{
	setNormalAxis(p_normal);
}


bool CollisionAASquare::checkCollisionRay(const Ray&     p_ray,
                                          real*          p_t_OUT,
                                          math::Vector3* p_normal_OUT) const
{
	return checkCollisionRay(p_ray.getOrigin(),
	                         p_ray.getDirection(),
	                         p_t_OUT,
	                         p_normal_OUT);
}


bool CollisionAASquare::checkCollisionRay(const math::Vector3& p_origin,
                                          const math::Vector3& p_dir,
                                          real* p_t_OUT,
                                          tt::math::Vector3* p_normal_OUT) const
{
	if (m_axis == -1)
	{
		TT_PANIC("Invalid normal axis");
		return false;
	}
	
	math::Vector3 min(getMin());
	math::Vector3 max(getMax());
	
	// Code from real-time collision detection  (blz 181)
	real tmin = 0.0f; // Set to std::numeric_limits<real>::min() to get first hit on line 
	                  // (Also allow collisions behind the origin by traveling in the negative direction.)
	real tmax = std::numeric_limits<real>::max(); // set to max distance ray can travel (for segment)
	
	// For all three slabs
	for (s8 i = 0; i < 3; ++i)
	{
		if (tt::math::realEqual(p_dir[i], 0))
		{
			// Ray is parallel to slab. 
			// No hit if parallel to plane or origin not within slab.
			if (i == m_axis || p_origin[i] < min[i] || p_origin[i] > max[i])
			{
				return false;
			}
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab.
			real ood = 1.0f / p_dir[i];
			real t1 = (min[i] - p_origin[i]) * ood;
			real t2 = t1;
			if (i != m_axis)
			{
				t2 = (max[i] - p_origin[i]) * ood;
				
				// Make t1 be intersection with near plane, t2 with far plane
				if (t1 > t2)
				{
					std::swap(t1, t2);
				}
			}
			// Compute the intersection of slab intersection intervals
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax)
			{
				return false;
			}
		}
	}
	
	// Ray intersects all 3 slabs. (== Collision)
	
	if (p_t_OUT != 0)
	{
		(*p_t_OUT) = tmin;
	}
	if (p_normal_OUT != 0)
	{
		(*p_normal_OUT) = tt::math::Vector3::zero;
		if (p_origin[m_axis] > m_pos[m_axis])
		{
			TT_ASSERT(p_dir[m_axis] < 0);
			(*p_normal_OUT)[m_axis] = 1;
		}
		else if (p_origin[m_axis] < m_pos[m_axis])
		{
			TT_ASSERT(p_dir[m_axis] > 0);
			(*p_normal_OUT)[m_axis] = -1;
		}
		else
		{
			// Origin is at the same pos as square.
			TT_ASSERT(math::realEqual(tmin, 0) );
			// Can't return collsion normal,
			// Just return a normal in the other direction as p_dir.
			if (p_dir[m_axis] > 0)
			{
				(*p_normal_OUT)[m_axis] = -1;
			}
			else
			{
				(*p_normal_OUT)[m_axis] = 1;
			}
		}
		TT_ASSERT((*p_normal_OUT) != tt::math::Vector3::zero);
	}
	return true;
}


bool CollisionAASquare::checkCollision(const math::Vector3& v1, const math::Vector3& v2) const
{
	return checkCollisionRay(v1, (v2 - v1), 0);
}


bool CollisionAASquare::checkCollision(const AABoundingBox& p_box) const
{
	math::Vector3 min(getMin());
	math::Vector3 max(getMax());
	
	math::Vector3 boxMin(p_box.getMin());
	math::Vector3 boxMax(p_box.getMax());
	
	for (s8 i = 0; i < 3; ++i)
	{
		if (boxMax[i] < min[i] || boxMin[i] > max[i])
		{
			return false;
		}
	}
	
	return true;
}


bool CollisionAASquare::checkCollision(const math::Vector3& p_point) const
{
	math::Vector3 min(getMin());
	math::Vector3 max(getMax());
	
	for (s8 i = 0; i < 3; ++i)
	{
		if (p_point[i] < min[i] || p_point[i] > max[i])
		{
			return false;
		}
	}
	
	return true;
}


math::Vector3 CollisionAASquare::getMin() const
{
	TT_ASSERTMSG(m_axis != -1, "Invalid normal axis");
	
	real halfSize = m_size * 0.5f;
	math::Vector3 halfSizeVec(math::Vector3::zero);
	for (int i = 0; i < 3; ++i)
	{
		if (i != m_axis)
		{
			halfSizeVec[i] = halfSize;
		}
	}
	
	return m_pos - halfSizeVec;
}


math::Vector3 CollisionAASquare::getMax() const
{
	TT_ASSERTMSG(m_axis != -1, "Invalid normal axis");
	
	real halfSize = m_size * 0.5f;
	math::Vector3 halfSizeVec(math::Vector3::zero);
	for (int i = 0; i < 3; ++i)
	{
		if (i != m_axis)
		{
			halfSizeVec[i] = halfSize;
		}
	}
	
	return m_pos + halfSizeVec;
}


void CollisionAASquare::setNormalAxis(const math::Vector3& p_normal)
{
	// Find which axis has -1 or 1.
	m_axis = -1;
	
	for (s8 i = 0; i < 3; ++i)
	{
		if (math::realEqual(p_normal[i], real(1)) ||
			math::realEqual(p_normal[i], real(-1)))
		{
			m_axis = i;
			m_positiveNormal = p_normal[i] > 0;
			break;
		}
	}
	
	if (m_axis == -1)
	{
		TT_PANIC("Invalid normal. No axis with -1 or 1");
		return;
	}
	
	// The other two axis should be 0.
	for (s8 i = 0; i < 3; ++i)
	{
		if (i != m_axis &&
			math::realEqual(p_normal[i], real(0)) == false)
		{
			TT_PANIC("Invalid normal. (%f, %f, %f). "
			         "One axis (index: %d) is -1 or 1, but the other two need to be 0.",
			         realToFloat(p_normal.x), 
			         realToFloat(p_normal.y), 
			         realToFloat(p_normal.z), 
			         m_axis);
			
			m_axis = -1;
			return;
		}
	}
}

// Namespace end
}
}
}
