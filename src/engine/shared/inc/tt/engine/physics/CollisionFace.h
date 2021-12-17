#if !defined(INC_TT_ENGINE_PHYSICS_COLLISIONFACE_H)
#define INC_TT_ENGINE_PHYSICS_COLLISIONFACE_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/renderer/Plane.h>


namespace tt {
namespace engine {
namespace physics {

// Forward declarations
class Collision;

class CollisionFace
{
private:
	enum Axis
	{
		Axis_X = 0,
		Axis_Y,
		Axis_Z
	};
	
public:
	CollisionFace();
	~CollisionFace();
	
	bool create(const math::Vector3& p_v0, const math::Vector3& p_v1, const math::Vector3& p_v2);
	bool rayIntersect(const math::Vector3& p_start, const math::Vector3& p_dir, Collision* p_collide) const;
	bool lineIntersect(const math::Vector3& p_start, const math::Vector3& p_dir, Collision* p_collide) const;
	bool sphereIntersect(const math::Vector3& p_start, real p_radius, Collision* p_collide) const;
	
	inline Axis          getAxis()   const {return m_axis;}
	inline math::Vector3 getNormal() const {return m_plane.getNormal();}
	
// Private functions
private:
	bool pointInTriangle(const math::Vector3& p_point) const;
	bool pointInTriangleX(const math::Vector3& p_point) const;
	bool pointInTriangleY(const math::Vector3& p_point) const;
	bool pointInTriangleZ(const math::Vector3& p_point) const;
	
private:
	math::Vector3   m_vectors[3];
	renderer::Plane m_plane;
	Axis            m_axis;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_PHYSICS_COLLISIONFACE_H
