#if !defined(INC_TT_ENGINE_PHYSICS_COLLISION_H)
#define INC_TT_ENGINE_PHYSICS_COLLISION_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/renderer/Material.h>


namespace tt {
namespace engine {
namespace physics {

class Collision
{
public:
	inline Collision()
	:
	m_collisionPoint(0.0f, 0.0f, 0.0f),
	m_normal(0.0f, 1.0f, 0.0f),
	m_material(),
	m_time(0.0f)
	{ }
	
	/*! \brief Get the Collision Point
	    \return Returns the math::Vector3 of the collision point. */
	inline const math::Vector3& getCollisionPoint() const {return m_collisionPoint;}
	
	/*! \brief Get the Collision Normal
	    \return Returns the math::Vector3 of the normal of the collision. */
	inline const math::Vector3& getCollisionNormal() const {return m_normal;}
	
	/*! \brief Get the Collision Material
	    \return Returns the material of the collision surface. */
	inline const renderer::MaterialPtr& getMaterial() const {return m_material;}
	
	/*! \brief Get the Collision Time
	    \return Returns the time of intersection. */
	inline real getCollisionTime() const {return m_time;}
	
	inline void setCollisionPoint(const math::Vector3& p_point) {m_collisionPoint = p_point;}
	inline void setCollisionNormal(const math::Vector3& p_normal) {m_normal = p_normal;}
	inline void setCollisionTime(real p_time) {m_time = p_time;}
	inline void setMaterial(const renderer::MaterialPtr& p_material) {m_material = p_material;}
	
private:
	math::Vector3         m_collisionPoint;
	math::Vector3         m_normal;
	renderer::MaterialPtr m_material;
	real                  m_time;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_PHYSICS_COLLISION_H
