#include <limits>
#include <algorithm>

#include <tt/platform/tt_error.h>

#include <tt/math/math.h>

#include <tt/engine/physics/AABoundingBox.h>
#include <tt/engine/physics/Ray.h>

namespace tt {
namespace engine {
namespace physics {

AABoundingBox::AABoundingBox(const math::Vector3& p_min, 
                             const math::Vector3& p_max)
:
m_boxMin(p_min),
m_boxMax(p_max),
m_sphere()
{
	TT_ASSERT(p_min.x <= p_max.x);
	TT_ASSERT(p_min.y <= p_max.y);
	TT_ASSERT(p_min.z <= p_max.z);
	
	m_sphere.calculateSphere(*this);
}


void AABoundingBox::set(const math::Vector3& p_min, 
                        const math::Vector3& p_max)
{
	TT_ASSERT(p_min.x <= p_max.x);
	TT_ASSERT(p_min.y <= p_max.y);
	TT_ASSERT(p_min.z <= p_max.z);
	
	m_boxMin = p_min;
	m_boxMax = p_max;
	m_sphere.calculateSphere(*this);
}


void AABoundingBox::merge(const AABoundingBox& p_box)
{
	m_boxMin.x = std::min(m_boxMin.x, p_box.m_boxMin.x);
	m_boxMin.y = std::min(m_boxMin.y, p_box.m_boxMin.y);
	m_boxMin.z = std::min(m_boxMin.z, p_box.m_boxMin.z);
	m_boxMax.x = std::max(m_boxMax.x, p_box.m_boxMax.x);
	m_boxMax.y = std::max(m_boxMax.y, p_box.m_boxMax.y);
	m_boxMax.z = std::max(m_boxMax.z, p_box.m_boxMax.z);
	
	// Recalculate bounding sphere
	m_sphere.calculateSphere(*this);
}


bool AABoundingBox::checkCollisionRay(const Ray&     p_ray,
                                      real*          p_t_OUT,
                                      math::Vector3* p_normal_OUT) const
{
	return checkCollisionRay(p_ray.getOrigin(),
	                         p_ray.getDirection(),
	                         p_t_OUT,
	                         p_normal_OUT);
}


bool AABoundingBox::checkCollisionRay(const math::Vector3& p_origin,
                                      const math::Vector3& p_dir,
                                      real* p_t_OUT,
                                      tt::math::Vector3* p_normal_OUT) const
{
	// Code from real-time collision detection  (blz 181)
	real tmin = 0.0f; // Set to std::numeric_limits<real>::min() to get first hit on line 
	                  // (Also allow collisions behind the origin by traveling in the negative direction.)
	real tmax = std::numeric_limits<real>::max(); // set to max distance ray can travel (for segment)
	s32 foundOnAxis = -1;
	bool foundAxisHadSwap = false;
	
	// For all three slabs
	for (s32 i = 0; i < 3; ++i)
	{
		if (tt::math::realEqual(p_dir[i], 0))
		{
			// Ray is parallel to slab. no hit if origin not within slab.
			if (p_origin[i] < m_boxMin[i] || p_origin[i] > m_boxMax[i])
			{
				return false;
			}
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab.
			real ood = 1.0f / p_dir[i];
			real t1 = (m_boxMin[i] - p_origin[i]) * ood;
			real t2 = (m_boxMax[i] - p_origin[i]) * ood;
			
			// Make t1 be intersection with near plane, t2 with far plane
			bool swapDone = false;
			if (t1 > t2)
			{
				std::swap(t1, t2);
				swapDone = true;
			}
			
			// Compute the intersection of slab intersection intervals
			if (t1 > tmin)
			{
				tmin = t1;
				foundOnAxis = i;
				foundAxisHadSwap = swapDone;
			}
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
		if (foundOnAxis != -1)
		{
			(*p_normal_OUT)[foundOnAxis] = (foundAxisHadSwap) ? 1.0f : -1.0f;
		}
	}
	return true;
}


bool AABoundingBox::checkCollision(const math::Vector3& v1,
                                   const math::Vector3& v2) const 
{
	return checkCollisionRay(v1, (v2 - v1), 0);
	

	/* OLD ADRIAN CODE
	// TODO: Clean up this function
	real minB[3], maxB[3];
	real origin[3], dir[3];
	bool inside = true;
	u8 quadrant[3];
	s32 whichPlane;
	real maxT[3];
	real candidatePlane[3];
	
	// Get the values
	minB[0] = m_boxMin.x;
	minB[1] = m_boxMin.y;
	minB[2] = m_boxMin.z;
	maxB[0] = m_boxMax.x;
	maxB[1] = m_boxMax.y;
	maxB[2] = m_boxMax.z;
	origin[0] = v1.x;
	origin[1] = v1.y;
	origin[2] = v1.z;
	
	math::Vector3 direction(v2 - v1);
	dir[0] = direction.x;
	dir[1] = direction.y;
	dir[2] = direction.z;
	
	// Find candidate
	for (s32 i = 0; i < 3; ++i)
	{
		if ( origin[i] < minB[i])
		{
			quadrant[i] = 1;
			candidatePlane[i] = minB[i];
			inside = false;
		}
		else if (origin[i] > maxB[i])
		{
			quadrant[i] = 0;
			candidatePlane[i] = maxB[i];
			inside = false;
		}
		else
		{
			quadrant[i] = 2;
		}
	}
	
	// Inside then
	if (inside)
	{
		return true;
	}
	
	// Calculate T
	for (s32 i = 0; i < 3; ++i)
	{
		if ( quadrant[i] != 2 && dir[i] != 0 )
		{
			maxT[i] = ((candidatePlane[i] - origin[i]) / dir[i]);
		}
		else
		{
			maxT[i] = -1;
		}
	}
	
	// Find largest T
	whichPlane = 0;
	
	for (s32 i = 1; i < 3; ++i)
	{
		if ( maxT[whichPlane] < maxT[i] )
		{
			whichPlane = i;
		}
	}
	
	// See if inside
	if ( maxT[whichPlane] < 0)
	{
		return false;
	}
	
	for (s32 i = 0; i < 3; ++i)
	{
		if ( whichPlane != i )
		{
			real temp = ((origin[i] + maxT[whichPlane]) * dir[i]);

			if ( temp < minB[i] || temp > maxB[i] )
			{
				return false;
			}
		}
	}
	
	return true;
	*/
}


bool AABoundingBox::checkCollision(const AABoundingBox& p_box) const
{
	// Quick reject
	if (m_sphere.checkCollision(p_box.m_sphere) == false)
	{
		return false;
	}
	
	// Check X
	if ( m_boxMin.x > p_box.m_boxMax.x ||
		 m_boxMax.x < p_box.m_boxMin.x )
	{
		return false;
	}
	
	// Check Y
	if ( m_boxMin.y > p_box.m_boxMax.y ||
		 m_boxMax.y < p_box.m_boxMin.y )
	{
		return false;
	}
	
	// Check Z
	if ( m_boxMin.z > p_box.m_boxMax.z ||
		 m_boxMax.z < p_box.m_boxMin.z )
	{
		return false;
	}
	return true;
}


bool AABoundingBox::checkCollision(const math::Vector3& p_point) const
{
	// Quick reject
	if (m_sphere.checkCollision(p_point) == false)
	{
		return false;
	}
	
	// Check X
	if (p_point.x > m_boxMax.x || p_point.x < m_boxMin.x)
	{
		return false;
	}
	
	// Check Y
	if (p_point.y > m_boxMax.y || p_point.y < m_boxMin.y)
	{
		return false;
	}
	
	// Check Z
	if ( p_point.z > m_boxMax.z || p_point.z < m_boxMin.z)
	{
		return false;
	}
	return true;
}


// Namespace end
}
}
}

