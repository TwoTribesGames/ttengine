#include <tt/engine/physics/BoundingBox.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <algorithm>

namespace tt {
namespace engine {
namespace physics {


BoundingBox::BoundingBox()
:
m_flags(Flag_None),
m_screenPosition(),
m_screenWidth(0),
m_screenHeight(0),
m_boundingSphere(),
m_sphereRadiusSqr(0)
{
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::reset()
{
	// Clear the flags
	m_flags = Flag_None;

	// Clear the values
	m_AABB[0].setValues(0, 0, 0);
	m_AABB[1].setValues(0, 0, 0);
}


void BoundingBox::setOOBB(s32 p_corner, const math::Vector3& p_point)
{
	// Set the corner
	m_OOBB[p_corner] = p_point;

	// Now work out min AABB
	m_AABB[0].x = std::min(m_AABB[0].x, p_point.x);
	m_AABB[0].y = std::min(m_AABB[0].y, p_point.y);
	m_AABB[0].z = std::min(m_AABB[0].z, p_point.z);

	// Now work out max AABB
	m_AABB[1].x = std::max(m_AABB[1].x, p_point.x);
	m_AABB[1].y = std::max(m_AABB[1].y, p_point.y);
	m_AABB[1].z = std::max(m_AABB[1].z, p_point.z);
}


void BoundingBox::calculateSphere()
{
	// Basically choose a sphere based on the aabb
	m_boundingSphere.setPosition((m_AABB[0] + m_AABB[1]) / 2.0f);

	m_sphereRadiusSqr = (m_AABB[0] - m_boundingSphere.getPosition()).lengthSquared();
	
	m_boundingSphere.setRadius(static_cast<real>(math::sqrt(m_sphereRadiusSqr)));

	// Set the sphere as valid
	setFlag(Flag_SphereValid);
}


const BoundingBox& BoundingBox::operator=(const BoundingBox& p_rhs)
{
	// Copy flags
	m_flags = p_rhs.m_flags;

	// Copy screen info
	m_screenPosition = p_rhs.m_screenPosition;
	m_screenWidth	 = p_rhs.m_screenWidth;
	m_screenHeight	 = p_rhs.m_screenHeight;

	// Copy OOBB
	for (s32 i = 0; i < 8; ++i)
	{
		m_OOBB[i] = p_rhs.m_OOBB[i];
	}

	// Copy AABB
	m_AABB[0] = p_rhs.m_AABB[0];
	m_AABB[1] = p_rhs.m_AABB[1];

	// Copy bounding sphere
	m_boundingSphere = p_rhs.m_boundingSphere;
	m_sphereRadiusSqr = p_rhs.m_sphereRadiusSqr;

	return *this;
}


bool BoundingBox::checkCollision(const math::Vector3& p_point, 
								 const CollisionType p_type) const 
{
	// Handle the type
	switch (p_type)
	{
		case CollisionType_Sphere:
		{
			if (checkFlag(Flag_SphereValid))
			{
				return checkSphereCollision(p_point);
			}
			break;
		}

		case CollisionType_OOBB:
		{
			if (checkFlag(Flag_OOBBValid))
			{
				return checkOOBBCollision(p_point);
			}
			break;
		}

		case CollisionType_AABB:
		{
			if ( checkFlag(Flag_AABBValid))
			{
				return checkAABBCollision(p_point);
			}
			break;
		}

		case CollisionType_Screen:
		{
			if ( checkFlag(Flag_ScreenValid) )
			{
				return checkScreenCollision(p_point);
			}
			break;
		}

		default:
		{
			TT_PANIC("Invalid collision type");
		}
	}
	return false;
}


bool BoundingBox::checkCollision(const BoundingBox& p_box, 
								 const CollisionType p_type) const 
{
	// Handle the type
	switch (p_type)
	{
		case CollisionType_Sphere:
		{
			if (checkFlag(Flag_SphereValid))
			{
				return checkSphereCollision(p_box);
			}
			break;
		}

		case CollisionType_OOBB:
		{
			if (checkFlag(Flag_OOBBValid))
			{
				return checkOOBBCollision(p_box);
			}
			break;
		}

		case CollisionType_AABB:
		{
			if ( checkFlag(Flag_AABBValid))
			{
				return checkAABBCollision(p_box);
			}
			break;
		}

		case CollisionType_Screen:
		{
			if ( checkFlag(Flag_ScreenValid) )
			{
				return checkScreenCollision(p_box);
			}
			break;
		}

		default:
		{
			TT_PANIC("Invalid collision type");
		}
	}
	return false;
}




bool BoundingBox::checkCollision(const math::Vector3& v1,
                                 const math::Vector3& v2) const 
{
	// TODO: Clean up this function
	real minB[3], maxB[3];
	real origin[3], dir[3];
	bool inside = true;
	u8 quadrant[3];
	s32 whichPlane;
	real maxT[3];
	real candidatePlane[3];

	// Get the values
	minB[0] = m_AABB[0].x;
	minB[1] = m_AABB[0].y;
	minB[2] = m_AABB[0].z;
	maxB[0] = m_AABB[1].x;
	maxB[1] = m_AABB[1].y;
	maxB[2] = m_AABB[1].z;
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
}




bool BoundingBox::checkAABBCollision(const math::Vector3& p_point) const
{
	// Quick reject
	if (checkSphereCollision(p_point) == false)
	{
		return false;
	}

	// Check X
	if (p_point.x > getAABBMax().x || p_point.x < getAABBMin().x)
	{
		return false;
	}
	// Check Y
	if (p_point.y > getAABBMax().y || p_point.y < getAABBMin().y)
	{
		return false;
	}
	// Check Z
	if ( p_point.z > getAABBMax().z || p_point.z < getAABBMin().z)
	{
		return false;
	}
	return true;
}


bool BoundingBox::checkOOBBCollision(const math::Vector3&) const
{
	TT_PANIC("This function has not been implemented");
	return false;
}


bool BoundingBox::checkScreenCollision(const math::Vector3& p_point) const
{
	// Check screen
	if ( p_point.x < m_screenPosition.x ||
		 p_point.x > (m_screenPosition.x + m_screenWidth) )
	{
		return false;
	}
	if ( p_point.y < m_screenPosition.y ||
		 p_point.y > (m_screenPosition.y + m_screenHeight) )
	{
		return false;
	}
	return true;
}




bool BoundingBox::checkAABBCollision(const BoundingBox& p_box) const
{
	// Quick reject
	if (checkSphereCollision(p_box) == false)
	{
		return false;
	}

	// Check X
	if ( getAABBMin().x > p_box.getAABBMax().x ||
		 getAABBMax().x < p_box.getAABBMin().x )
	{
		return false;
	}

	// Check Y
	if ( getAABBMin().y > p_box.getAABBMax().y ||
		 getAABBMax().y < p_box.getAABBMin().y )
	{
		return false;
	}

	// Check Z
	if ( getAABBMin().z > p_box.getAABBMax().z ||
		 getAABBMax().z < p_box.getAABBMin().z )
	{
		return false;
	}
	return true;
}


bool BoundingBox::checkOOBBCollision(const BoundingBox& p_box) const
{
	// Quick reject
	if (checkSphereCollision(p_box) == false)
	{
		return false;
	}

	TT_WARN("This function has not been fully implemented");
	return false;
}


bool BoundingBox::checkScreenCollision(const BoundingBox&) const
{
	TT_PANIC("This function has not been implemented");
	return false;
}


void BoundingBox::merge(const BoundingBox& p_box)
{
	// Check the AABB flag
	if (checkFlag(Flag_AABBValid))
	{
		// Merge
		m_AABB[0].x = std::min(m_AABB[0].x, p_box.m_AABB[0].x);
		m_AABB[0].y = std::min(m_AABB[0].y, p_box.m_AABB[0].y);
		m_AABB[0].z = std::min(m_AABB[0].z, p_box.m_AABB[0].z);
		m_AABB[1].x = std::max(m_AABB[1].x, p_box.m_AABB[1].x);
		m_AABB[1].y = std::max(m_AABB[1].y, p_box.m_AABB[1].y);
		m_AABB[1].z = std::max(m_AABB[1].z, p_box.m_AABB[1].z);
	}
	else if(p_box.checkFlag(Flag_AABBValid))
	{
		// Use AABB from parameter
		setFlag(Flag_AABBValid);

		m_AABB[0] = p_box.m_AABB[0];
		m_AABB[1] = p_box.m_AABB[1];
	}
	// Recalculate bounding sphere
	calculateSphere();
}


// Namespace end
}
}
}

