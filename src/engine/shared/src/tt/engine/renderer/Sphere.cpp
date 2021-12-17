#include <tt/engine/renderer/Sphere.h>
#include <tt/platform/tt_printf.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace renderer {


Sphere::Sphere()
:
m_position(),
m_radius(1.0f)
{
}


Sphere::Sphere(real p_radius)
:
m_position(),
m_radius(p_radius)
{
}


Sphere::Sphere(const math::Vector3& p_position, real p_radius)
:
m_position(p_position),
m_radius(p_radius)
{
}


Sphere::~Sphere()
{
}


void Sphere::print() const
{
	TT_Printf("Sphere [%f, %f, %f] R: %f \n", 
		realToFloat(m_position.x), 
		realToFloat(m_position.y), 
		realToFloat(m_position.z), 
		realToFloat(m_radius));
}

bool Sphere::load(const fs::FilePtr& p_file)
{
	// Attempt to load the sphere
	if (m_position.load(p_file) == false)
	{
		return false;
	}

	if (p_file->read(&m_radius, sizeof(m_radius)) != sizeof(m_radius))
	{
		return false;
	}

	return true;
}


bool Sphere::intersects(const Sphere& p_other) const
{
	return (m_radius + p_other.getRadius()) * (m_radius + p_other.getRadius()) <
		math::distanceSquared(m_position, p_other.getPosition());
}


// Namespace end
}
}
}

