#if !defined(INC_TT_ENGINE_RENDERER_SPHERE_H)
#define INC_TT_ENGINE_RENDERER_SPHERE_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace renderer {


class Sphere
{
public:
	Sphere();
	explicit Sphere(real p_radius);
	Sphere(const math::Vector3& p_position, real p_radius = 1.0f);
	~Sphere();
	
	bool load(const fs::FilePtr& p_file);
	void print() const;

	bool intersects(const Sphere& p_other) const;
	
	inline math::Vector3 getPosition() const {return m_position;}
	inline real getRadius() const {return m_radius;}
	
	inline void setRadius(real p_radius)                  {m_radius = p_radius;}
	inline void setPosition(real p_x, real p_y, real p_z) {m_position.setValues(p_x, p_y, p_z);}
	inline void setPosition(const math::Vector3& p_pos)   {m_position = p_pos;}
	
private:
	math::Vector3 m_position;
	real          m_radius;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_SPHERE_H
