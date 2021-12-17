#if !defined(INC_TT_ENGINE_PHYSICS_RAY_H)
#define INC_TT_ENGINE_PHYSICS_RAY_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace engine {
namespace physics {

class Ray
{
public:
	Ray(const math::Vector3& p_origin = tt::math::Vector3::zero,
		const math::Vector3& p_end = tt::math::Vector3::forward)
	:
	m_orig(p_origin),
	m_end(p_end),
	m_dir(m_end - m_orig)
	{
		m_dir.normalize();
	}
	
	inline void setOrigin(const math::Vector3& p_orig)
	{
		m_orig = p_orig;
		m_dir = m_end - m_orig;
		m_dir.normalize();
	}
	inline void setOrigin(real p_x, real p_y, real p_z)
	{
		m_orig.setValues(p_x, p_y, p_z);
		m_dir = m_end - m_orig;
		m_dir.normalize();
	}
	
	inline void setEnd(const math::Vector3& p_end)
	{
		m_end = p_end;
		m_dir = m_end - m_orig;
		m_dir.normalize();
	}
	inline void setEnd(real p_x, real p_y, real p_z)
	{
		m_end.setValues(p_x, p_y, p_z);
		m_dir = m_end - m_orig;
		m_dir.normalize();
	}
	
	inline void setOriginEnd(const math::Vector3& p_orig, const math::Vector3& p_end)
	{
		m_orig = p_orig;
		m_end  = p_end;
		m_dir = m_end - m_orig;
		m_dir.normalize();
	}
	
	inline void setOriginEnd(real p_ox, real p_oy, real p_oz, real p_ex, real p_ey, real p_ez)
	{
		m_orig.setValues(p_ox, p_oy, p_oz);
		m_end.setValues(p_ex, p_ey, p_ez);
		m_dir = m_end - m_orig;
		m_dir.normalize();
	}
	
	inline math::Vector3 getOrigin()    const {return m_orig;}
	inline math::Vector3 getEnd()       const {return m_end;}
	inline math::Vector3 getDirection() const {return m_dir;}
	
private:
	math::Vector3 m_orig;
	math::Vector3 m_end;
	math::Vector3 m_dir;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_PHYSICS_RAY_H
