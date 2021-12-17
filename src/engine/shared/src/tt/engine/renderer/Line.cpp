#include <tt/engine/renderer/Line.h>
#include <tt/engine/renderer/Sphere.h>
#include <tt/platform/tt_printf.h>
#include <tt/fs/File.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace renderer {


Line::Line()
:
m_start(0,0,0),
m_end(0,0,0)
{
}


Line::Line(const math::Vector3& p_start, const math::Vector3& p_end)
:
m_start(p_start),
m_end(p_end)
{
}


Line::~Line()
{
}


void Line::print() const
{
	TT_Printf("0x%08x, 0x%08x, 0x%08x - 0x%08x, 0x%08x, 0x%08x\n", 
		getStart().x, getStart().y, getStart().z, getEnd().x, getEnd().y, getEnd().z);
}


bool Line::load(const fs::FilePtr& p_file)
{
	// Attempt to load the sphere
	if (p_file->read(&m_start.x, sizeof(m_start.x)) != sizeof(m_start.x))
	{
		return false;
	}
	
	if (p_file->read(&m_start.y, sizeof(m_start.y)) != sizeof(m_start.y))
	{
		return false;
	}

	if (p_file->read(&m_start.z, sizeof(m_start.z)) != sizeof(m_start.z))
	{
		return false;
	}


	// Attempt to load the sphere
	if (p_file->read(&m_end.x, sizeof(m_end.x)) != sizeof(m_end.x))
	{
		return false;
	}
	
	if (p_file->read(&m_end.y, sizeof(m_end.y)) != sizeof(m_end.y))
	{
		return false;
	}

	if (p_file->read(&m_end.z, sizeof(m_end.z)) != sizeof(m_end.z))
	{
		return false;
	}

	return true;
}

bool Line::intersect(Sphere* p_sphere) const
{
	// Get working values
	real x2x1 = m_end.x - m_start.x;
	real y2y1 = m_end.y - m_start.y;
	real z2z1 = m_end.z - m_start.z;
	
	math::Vector3 spherePosition(p_sphere->getPosition());
	real x3x1 = spherePosition.x - m_start.x;
	real y3y1 = spherePosition.y - m_start.y;
	real z3z1 = spherePosition.z - m_start.z;
	
	// Calculate quadratic values
	real a = static_cast<real>(sqrt(x2x1) + sqrt(y2y1) + sqrt(z2z1));
	
	real b = 2 * (x2x1 * x3x1 + y2y1 * y3y1 + z2z1 * z3z1);
	
	real c = static_cast<real>(sqrt(spherePosition.x) + sqrt(spherePosition.y) + sqrt(spherePosition.z) + 
			 sqrt(m_start.x) + sqrt(m_start.y) + sqrt(m_start.z));
	
	c = c - static_cast<real>((2 * (spherePosition.x * m_start.x) + 
				 (spherePosition.y * m_start.y) + 
				 (spherePosition.z * m_start.z) ) - sqrt(p_sphere->getRadius()));
	
	real intersectDist = static_cast<real>(sqrt(b) - (4 * (a * c)));
	
	if (intersectDist < 0)
	{
		return false;
	}
	return true;
}


bool Line::intersect(Line* p_line, real* p_time) const
{
	real denom = ((p_line->getEnd().y - p_line->getStart().y) * (m_end.x - m_start.x)) - 
				 ((p_line->getEnd().x - p_line->getStart().x) * (m_end.y - m_start.y));

	if (math::realEqual(denom, 0))
	{
		// Lines are parallel
		return false;
	}

	// Calculate the numerators
	real num_a = ((p_line->getEnd().x - p_line->getStart().x) * (m_start.y - p_line->getStart().y)) - 
				 ((p_line->getEnd().y - p_line->getStart().y) * (m_start.y - p_line->getStart().x));
	
	// Calculate the time value
	real ta = num_a / denom;

	if (ta < 0.0f || ta > 1.0f)
	{
		return false;
	}

	// And the second p_line
	real num_b = ( (m_end.x - m_start.x) * (m_start.y - p_line->getStart().y) ) - 
				 ( (m_end.y - m_start.y) * (m_start.x - p_line->getStart().x) );

	real tb = num_b / denom;

	// Now if ta and tb are between 0 and 1 then we intersect
	if (tb >= 0.0f && tb <= 1.0f)
	{
		if (p_time)
		{
			*p_time = ta;
		}
		return true;
	}
	return false;
}

real Line::distance(const math::Vector3& p_vector) const
{
	real denom(math::sqrt(m_end.x - m_start.x) + math::sqrt(m_end.y - m_start.y));

	if(math::realEqual(denom, 0))
	{
		// Lines are parallel
		return 0;
	}

	// Calculate the numerators
	real num = (p_vector.x - m_start.x) * (m_end.x - m_start.x) + 
		       (p_vector.y - m_start.y) * (m_end.y - m_start.y);
	real u = num / denom;

	real x = m_start.x + u * (m_end.x - m_start.x);
	real y = m_start.y + u * (m_end.y - m_start.y);

	real d = math::sqrt(math::sqrt(p_vector.x - x) + math::sqrt(p_vector.y - y));

	return d;
}


// Namespace end
}
}
}

