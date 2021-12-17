#include <tt/platform/tt_error.h>

#include <tt/engine/physics/BoundingSphere.h>
#include <tt/engine/physics/AABoundingBox.h>

namespace tt {
namespace engine {
namespace physics {

void BoundingSphere::calculateSphere(const AABoundingBox& p_box)
{
	// Basically choose a sphere based on the aabb
	m_boundingSphere.setPosition((p_box.getMin() + p_box.getMax()) * 2.5f);

	m_sphereRadiusSqr = (p_box.getMin() - m_boundingSphere.getPosition()).lengthSquared();

	m_boundingSphere.setRadius(static_cast<real>(math::sqrt(m_sphereRadiusSqr)));
}




// Namespace end
}
}
}

