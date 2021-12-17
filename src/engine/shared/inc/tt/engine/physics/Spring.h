#if !defined(INC_TT_ENGINE_PHYSICS_SPRING_H)
#define INC_TT_ENGINE_PHYSICS_SPRING_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace engine {
namespace physics {

class Spring
{
public:
	Spring();
	explicit Spring(real p_spring);
	~Spring();
	
	void update(real p_update);
	
	inline void setPosition(const math::Vector3& p_pos) {m_position = p_pos;}
	inline void setPosition(real p_x, real p_y, real p_z)
	{
		m_position.setValues(p_x, p_y, p_z);
	}
	inline void setPositionX(real p_x) {m_position.x = p_x;}
	inline void setPositionY(real p_y) {m_position.y = p_y;}
	inline void setPositionZ(real p_z) {m_position.z = p_z;}
	
	inline void setTarget(const math::Vector3& p_target) {m_target = p_target;}
	inline void setTarget(real p_x, real p_y, real p_z)
	{
		m_target.setValues(p_x, p_y, p_z);
	}
	inline void setTargetX(real p_x) {m_target.x = p_x;}
	inline void setTargetY(real p_y) {m_target.y = p_y;}
	inline void setTargetZ(real p_z) {m_target.z = p_z;}
	
	inline void setSpring(real p_spring) {m_spring = p_spring;}
	void calculateDampenFromSpring();
	inline void setDampen(real p_dampen) {m_dampen = p_dampen;}
	
	inline math::Vector3 getPosition() const {return m_position;}
	inline real getPositionX() const {return m_position.x;}
	inline real getPositionY() const {return m_position.y;}
	inline real getPositionZ() const {return m_position.z;}
	
	inline math::Vector3 getTarget() const {return m_position;}
	inline real getTargetX() const {return m_target.x;}
	inline real getTargetY() const {return m_target.y;}
	inline real getTargetZ() const {return m_target.z;}
	
	inline real getSpring() const {return m_spring;}
	inline real getDampen() const {return m_dampen;}
	
private:
	math::Vector3 m_position;
	math::Vector3 m_target;
	math::Vector3 m_velocity;
	real          m_spring;
	real          m_dampen;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_PHYSICS_SPRING_H
