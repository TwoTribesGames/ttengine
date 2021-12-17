#if !defined(INC_TT_ENGINE_PHYSICS_BOUNDINGSPHERE_H)
#define INC_TT_ENGINE_PHYSICS_BOUNDINGSPHERE_H

#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/renderer/Sphere.h>

namespace tt {
namespace engine {
namespace physics {


// forward declaration.
class AABoundingBox;


class BoundingSphere
{
public:
	inline BoundingSphere()
	:
	m_boundingSphere(),
	m_sphereRadiusSqr(0.0)
	{ }
	
	bool checkCollision(const math::Vector3& p_point) const;
	bool checkCollision(const BoundingSphere& p_sphere) const;
	
	void calculateSphere(const AABoundingBox& p_box);
	
private:
	renderer::Sphere m_boundingSphere;
	real             m_sphereRadiusSqr;
};


inline bool BoundingSphere::checkCollision(const math::Vector3& p_point) const
{
	// IW - converted fromm real64 to real, should only matter on fixed point not floating point!
	// Get the distance between the two points
	real distance = (p_point - m_boundingSphere.getPosition()).lengthSquared();
	return (distance < m_sphereRadiusSqr);
}


inline bool BoundingSphere::checkCollision(const BoundingSphere& p_sphere) const
{
	// Get the distance between the two points
	real distance = (p_sphere.m_boundingSphere.getPosition() -
		m_boundingSphere.getPosition()).lengthSquared();
	return (distance < (p_sphere.m_sphereRadiusSqr + m_sphereRadiusSqr));
}

// End namespace
}
}
}

#endif // !defined(INC_TT_ENGINE_PHYSICS_BOUNDINGSPHERE_H)
