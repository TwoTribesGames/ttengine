#include <tt/engine/renderer/Plane.h>
#include <tt/platform/tt_printf.h>
#include <tt/fs/File.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace renderer {

Plane::Plane()
:
m_normal(0,1,0),
m_distance(0)
{
}

Plane::Plane(const math::Vector3& p_v0, const math::Vector3& p_v1, const math::Vector3& p_v2)
:
m_normal(),
m_distance(0)
{
	create(p_v0, p_v1, p_v2);
}


Plane::Plane(const math::Vector3& p_normal, real p_distance)
:
m_normal(p_normal),
m_distance(p_distance)
{
}


Plane::~Plane()
{
}


void Plane::print() const
{
	TT_Printf("0x%08x, 0x%08x, 0x%08x - 0x%08x\n",
		getNormal().x, getNormal().y, getNormal().z, getDistance());
}


bool Plane::load(const fs::FilePtr& p_file)
{
	// Attempt to load the plane
	if (m_normal.load(p_file) == false)
	{
		return false;
	}
	
	if (p_file->read(&m_distance, sizeof(m_distance)) != sizeof(m_distance))
	{
		return false;
	}

	return true;
}


void Plane::create(const math::Vector3& p_v0, const math::Vector3& p_v1, const math::Vector3& p_v2)
{
	// Create the direction vectors
	math::Vector3 dir1 = p_v1 - p_v0;
	math::Vector3 dir2 = p_v2 - p_v0;

	// Create the normal
	m_normal = crossProduct(dir1, dir2);
	m_normal.normalize();

	m_distance = -dotProduct(p_v0, m_normal);
}


void Plane::create(const math::Vector3& p_normal, const math::Vector3& p_vector)
{
	// Copy the normal
	m_normal = p_normal;

	// Calculate distance
	m_distance = -math::dotProduct(p_vector, m_normal);
}


real Plane::distanceFromPoint(const math::Vector3& p_point) const
{
	return math::dotProduct(m_normal, p_point) + m_distance;
}


bool Plane::rayIntersect(const math::Vector3& p_start, 
						 const math::Vector3& p_dir, 
						 real* p_time,
						 math::Vector3* p_contact) const
{
	real angle = math::dotProduct(m_normal, p_dir);

	if (math::realEqual(angle, 0))
	{
		// Lines are parallel
		return false;
	}

	// Calculate intersection time
	real numerator = -(math::dotProduct(m_normal, p_start) + m_distance);
	real time = numerator / angle;

	if (p_time != 0)
	{
		*p_time = time;
	}

	if (p_contact != 0)
	{
		*p_contact = p_start + (p_dir * time);
	}

	return true;
}


bool Plane::lineSegmentIntersect(const math::Vector3& p_start, 
								 const math::Vector3& p_end, 
								 math::Vector3* p_contact) const
{
	// Calculate direction of the line
	math::Vector3 dir = p_end - p_start;

	// Get the intersection of the line segment and the plane
	real time;

	if (rayIntersect(p_start, dir, &time, p_contact))
	{
		// Make sure the intersection is between the start and the end
		if (time >= 0 && time <= 1.0f)
		{
			return true;
		}
	}
	return false;
}


// Namespace end
}
}
}

