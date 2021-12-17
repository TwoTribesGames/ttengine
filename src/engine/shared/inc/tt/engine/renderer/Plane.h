#if !defined(INC_TT_ENGINE_RENDERER_PLANE_H)
#define INC_TT_ENGINE_RENDERER_PLANE_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace renderer {


class Plane
{
public:
	Plane();
	Plane(const math::Vector3& p_v0, const math::Vector3& p_v1, const math::Vector3& p_v2);
	explicit Plane(const math::Vector3& p_normal, real p_distance = 0.0f);
	~Plane();
	
	bool load(const fs::FilePtr& p_file);
	void print() const;
	
	inline math::Vector3 getNormal() const {return m_normal;}
	
	inline void setNormalX(real p_x) {m_normal.x = p_x;}
	inline void setNormalY(real p_y) {m_normal.y = p_y;}
	inline void setNormalZ(real p_z) {m_normal.z = p_z;}
	inline void setNormal(const math::Vector3& p_normal) {m_normal = p_normal;}
	
	inline real getDistance() const {return m_distance;}
	inline void setDistance(real p_distance) {m_distance = p_distance;}
	
	void create(const math::Vector3& p_v0, const math::Vector3& p_v1, const math::Vector3& p_v2);
	void create(const math::Vector3& p_normal, const math::Vector3& p_vector);
	
	real distanceFromPoint(const math::Vector3& p_point) const;

	bool rayIntersect(const math::Vector3& p_start, const math::Vector3& p_dir,
		real* p_time = 0, math::Vector3* p_contact = 0) const;
	
	bool lineSegmentIntersect(const math::Vector3& p_start,
		const math::Vector3& p_end, math::Vector3* p_contact) const;
	
private:
	math::Vector3 m_normal;
	real          m_distance;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_PLANE_H
