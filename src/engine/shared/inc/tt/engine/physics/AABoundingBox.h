#if !defined(INC_TT_ENGINE_PHYSICS_AABOUNDINGBOX_H)
#define INC_TT_ENGINE_PHYSICS_AABOUNDINGBOX_H

#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/physics/BoundingSphere.h>

namespace tt {
namespace engine {
namespace physics {

class Ray;

class AABoundingBox
{
public:
	AABoundingBox(const math::Vector3& p_min = math::Vector3::zero, 
	              const math::Vector3& p_max = math::Vector3::zero);
	
	void set(const math::Vector3& p_min, 
	         const math::Vector3& p_max);
	
	inline const math::Vector3& getMin() const {return m_boxMin;}
	inline const math::Vector3& getMax() const {return m_boxMax;}
	
	void merge(const AABoundingBox& p_box);
	
	bool checkCollisionRay(const Ray&         p_ray,
	                       real*              p_t_OUT = 0,
	                       tt::math::Vector3* p_normal_OUT = 0) const;
	bool checkCollisionRay(const math::Vector3& p_origin,
	                       const math::Vector3& p_dir,
	                       real* p_t_OUT = 0,
	                       tt::math::Vector3* p_normal_OUT = 0) const;
	bool checkCollision(const math::Vector3& p_v1, const math::Vector3& p_v2) const;
	bool checkCollision(const AABoundingBox& p_box) const;
	bool checkCollision(const math::Vector3& p_point) const;
	
private:
	math::Vector3 m_boxMin;
	math::Vector3 m_boxMax;
	BoundingSphere m_sphere;
};

// End namespace
}
}
}

#endif // !defined(INC_TT_ENGINE_PHYSICS_AABOUNDINGBOX_H)
