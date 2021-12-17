#if !defined(INC_TT_ENGINE_PHYSICS_COLLISIONAASQUARE_H)
#define INC_TT_ENGINE_PHYSICS_COLLISIONAASQUARE_H


#include <tt/engine/physics/AABoundingBox.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace physics {

class Ray;

class CollisionAASquare
{
public:
	CollisionAASquare();
	CollisionAASquare(const math::Vector3& p_pos,
	                  real p_size,
	                  const math::Vector3& p_normal);
	
	inline void setPos(const math::Vector3& p_pos)       { m_pos = p_pos;           }
	inline void setSize(real p_size)                     { m_size = p_size;         }
	inline void setNormal(const math::Vector3& p_normal) { setNormalAxis(p_normal); }
	
	inline const math::Vector3& getPos() const { return m_pos;  }
	inline real getSize() const                { return m_size; }
	inline math::Vector3 getNormal() const
	{
		math::Vector3 result(math::Vector3::zero);
		if (m_axis >= 0 && m_axis < 3)
		{
			result[m_axis] = (m_positiveNormal) ? 1.0f : -1.0f;
		}
		return result;
	}
	
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
	
	math::Vector3 getMin() const;
	math::Vector3 getMax() const;
	
private:
	void setNormalAxis(const math::Vector3& p_normal);
	
	
	math::Vector3 m_pos;
	real          m_size;
	s8            m_axis; //!< Vector3 Index of the axis of the planes normal.
	bool          m_positiveNormal;
};

// End namespace
}
}
}


#endif  // !defined(INC_TT_ENGINE_PHYSICS_COLLISIONAASQUARE_H)
