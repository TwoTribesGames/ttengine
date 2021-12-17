#include <tt/engine/physics/CollisionFace.h>
#include <tt/engine/physics/Collision.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace physics {

CollisionFace::CollisionFace()
:
m_plane(),
m_axis(Axis_X)
{
}


CollisionFace::~CollisionFace()
{
}


bool CollisionFace::create(const math::Vector3& p_v0, const math::Vector3& p_v1, const math::Vector3& p_v2)
{
	// Take a copy of the vectors
	m_vectors[0] = p_v0;
	m_vectors[1] = p_v1;
	m_vectors[2] = p_v2;

	// Now create vectors for the 3 points and create the plane
	m_plane.create(p_v0, p_v1, p_v2);

	// See which axis to check
	real x = math::fabs(m_plane.getNormal().x);
	real y = math::fabs(m_plane.getNormal().y);
	real z = math::fabs(m_plane.getNormal().z);

	if ( x > y && x > z )
	{
		m_axis = Axis_X;
	}
	else if ( y > x && y > z )
	{
		m_axis = Axis_Y;
	}
	else
	{
		m_axis = Axis_Z;
	}
	return true;
}


bool CollisionFace::rayIntersect(const math::Vector3& p_start,
								 const math::Vector3& p_dir,
								 Collision* p_collide) const
{
	// Test for intersection
	math::Vector3 contact;
	real time;

	if (m_plane.rayIntersect(p_start, p_dir, &time, &contact))
	{ 
		if (pointInTriangle(contact))
		{
			p_collide->setCollisionPoint(contact);
			p_collide->setCollisionNormal(m_plane.getNormal());
			p_collide->setCollisionTime(time);
			return true;
		}
	}
	return false;
}


bool CollisionFace::lineIntersect(const math::Vector3& p_start, 
								  const math::Vector3& p_dir, 
								  Collision* p_collide) const
{
	// Throwaway if start point is behind plane
	if ( m_plane.distanceFromPoint(p_start) > 0) 
	{
		return false;
	}

	math::Vector3 contact;
	real time;

	if (m_plane.rayIntersect(p_start, p_dir, &time, &contact))
	{ 
		if (time >= 0.0f && time <= 1.0f)
		{
			if (pointInTriangle(contact))
			{
				p_collide->setCollisionPoint(contact);
				p_collide->setCollisionNormal(m_plane.getNormal());
				p_collide->setCollisionTime(time);
				return true;
			}
		}
	}
	return false;
}


bool CollisionFace::sphereIntersect(const math::Vector3& p_start, real p_radius, Collision* p_collide) const
{

	// Get the distance from the point to the plane
	real distance = m_plane.distanceFromPoint(p_start);

	if ((distance > 0) || math::fabs(distance) > p_radius )
	{
		return false;
	}

	// Work out the intersection point
	real time;
	math::Vector3 contact;

	if(m_plane.rayIntersect(p_start, m_plane.getNormal(), &time, &contact))
	{ 
		if (pointInTriangle(contact))
		{
			p_collide->setCollisionPoint(contact);
			p_collide->setCollisionNormal(m_plane.getNormal());
			p_collide->setCollisionTime(time);
			return true;
		}
	}
	return false;
}


bool CollisionFace::pointInTriangle(const math::Vector3& p_point) const
{
	switch (m_axis)
	{
	case Axis_X:
		return pointInTriangleX(p_point);

	case Axis_Y:
		return pointInTriangleY(p_point);

	case Axis_Z:
		return pointInTriangleZ(p_point);

	default:
		TT_PANIC("Invalid axis enumeration value [%d]", m_axis);
	}
	// Can never get here
	return false;
}


bool CollisionFace::pointInTriangleX(const math::Vector3& p_point) const
{
	// TODO: Clean up code
	real tx = p_point.y;
	real ty = p_point.z;

    const math::Vector3* v0(&m_vectors[2]);
    const math::Vector3* v1(&m_vectors[0]);

    /* get test bit for above/below X axis */
	bool bFlag0 = (v0->z >= ty);
    bool inside = false;

    for ( s32 i = 3 ; i > 0 ; i-- )
	{
		bool bFlag1 = (v1->z >= ty);

		// Possible intersect?
		if ( bFlag0 != bFlag1 )
		{
			real64 cp0 = (v1->z - ty) * (v0->y - v1->y);
			real64 cp1 = (v1->y - tx) * (v0->z - v1->z);

			if ( bFlag1 )
			{
				if ( cp0 >= cp1 )
				{
					inside = !inside;
				}
			}
			else
			{
				if ( cp0 < cp1 )
				{
					inside = !inside;
				}
			}
	    }

		// Move on
		bFlag0 = bFlag1;
		v0 = v1;
		v1++;
    }

    return inside;
}

bool CollisionFace::pointInTriangleY(const math::Vector3& p_point) const
{
	real tx = p_point.x;
	real ty = p_point.z;

    const math::Vector3* v0(&m_vectors[2]);
    const math::Vector3* v1(&m_vectors[0]);

    /* get test bit for above/below X axis */
	bool bFlag0 = (v0->z >= ty);
    bool inside = false;

    for ( s32 i = 3 ; i > 0 ; i-- )
	{
		bool bFlag1 = (v1->z >= ty);

		// Possible intersect?
		if ( bFlag0 != bFlag1 )
		{
			real64 cp0 = (v1->z - ty) * (v0->x - v1->x);
			real64 cp1 = (v1->x - tx) * (v0->z - v1->z);

			if ( bFlag1 )
			{
				if ( cp0 >= cp1 )
				{
					inside = !inside;
				}
			}
			else
			{
				if ( cp0 < cp1 )
				{
					inside = !inside;
				}
			}
	    }

		// Move on
		bFlag0 = bFlag1;
		v0 = v1;
		v1++;
    }

    return inside;
}

bool CollisionFace::pointInTriangleZ(const math::Vector3& p_point) const
{
    real tx = p_point.x;
	real ty = p_point.y;

    const math::Vector3* v0(&m_vectors[2]);
    const math::Vector3* v1(&m_vectors[0]);

    /* get test bit for above/below X axis */
	bool bFlag0 = (v0->y >= ty);
    bool inside = false;

    for ( s32 i = 3 ; i > 0 ; i-- )
	{
		bool bFlag1 = (v1->y >= ty);

		// Possible intersect?
		if ( bFlag0 != bFlag1 )
		{
			real64 cp0 = (v1->y - ty) * (v0->x - v1->x);
			real64 cp1 = (v1->x - tx) * (v0->y - v1->y);

			if ( bFlag1 )
			{
				if ( cp0 >= cp1 )
				{
					inside = !inside;
				}
			}
			else
			{
				if ( cp0 < cp1 )
				{
					inside = !inside;
				}
			}
	    }

		// Move on
		bFlag0 = bFlag1;
		v0 = v1;
		v1++;
	}

    return inside;
}


// Namespace end
}
}
}

