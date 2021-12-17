#if !defined(INC_TT_ENGINE_PHYSICS_BOUNDINGBOX_H)
#define INC_TT_ENGINE_PHYSICS_BOUNDINGBOX_H

// Include necessary files
#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/renderer/Sphere.h>


namespace tt {
namespace engine {
namespace physics {

class BoundingBox
{
public:
	// Bounding Box Enumerations
	enum Flag
	{
		Flag_None        = 0,
		Flag_ScreenValid = (1 << 0),
		Flag_OOBBValid   = (1 << 1),
		Flag_AABBValid   = (1 << 2),
		Flag_SphereValid = (1 << 3)
	};
	enum CollisionType
	{
		CollisionType_Sphere = (1 << 0),
		CollisionType_OOBB   = (1 << 1),
		CollisionType_AABB   = (1 << 2),
		CollisionType_Screen = (1 << 3)
	};
	
	BoundingBox();
	~BoundingBox();
	
	inline u32  getFlags() const             { return m_flags; }
	inline void setFlag(Flag p_flag)         { m_flags |= p_flag; }
	inline void setFlags(u32 p_flags)        { m_flags |= p_flags; }
	inline void resetFlag(Flag p_flag)       { m_flags &= ~p_flag; }
	inline bool checkFlag(Flag p_flag) const { return ((m_flags & p_flag) != 0); }
	
	void reset();
	
	inline real getScreenX() const {return m_screenPosition.x;}
	inline real getScreenY() const {return m_screenPosition.y;}
	inline real getScreenZ() const {return m_screenPosition.z;}
	
	inline real getScreenWidth() const  {return m_screenWidth;}
	inline real getScreenHeight() const {return m_screenHeight;}
	
	inline void setScreenX(real p_x) {m_screenPosition.x = p_x;}
	inline void setScreenY(real p_y) {m_screenPosition.y = p_y;}
	inline void setScreenZ(real p_z) {m_screenPosition.z = p_z;}
	inline void setScreenPosition(real p_x, real p_y, real p_z)
	{
		m_screenPosition.setValues(p_x, p_y, p_z);
	}
	
	inline void setScreenWidth(real p_width)   {m_screenWidth  = p_width;}
	inline void setScreenHeight(real p_height) {m_screenHeight = p_height;}
	
	void calculateSphere();
	
	inline math::Vector3 getOOBB(s32 p_corner) const {return m_OOBB[p_corner];}
	inline math::Vector3 getAABBMin() const          {return m_AABB[0];}
	inline math::Vector3 getAABBMax() const          {return m_AABB[1];}
	inline math::Vector3 getSphereCenter() const     {return m_boundingSphere.getPosition();}
	inline real    getSphereRadiusSqr() const {return m_sphereRadiusSqr;}
	inline real    getSphereRadius() const    {return m_boundingSphere.getRadius();}
	
	void setOOBB(s32 p_corner, const math::Vector3& p_point);
	
	void merge(const BoundingBox& p_box);
	
	bool checkCollision(const math::Vector3& p_v1, const math::Vector3& p_v2) const;
	bool checkCollision(real p_x, real p_y) const;
	bool checkCollision(const math::Vector3& p_v,
	                    const CollisionType p_type = CollisionType_AABB) const;
	bool checkCollision(const BoundingBox& p_box,
	                    const CollisionType p_type = CollisionType_AABB) const;
	
	const BoundingBox& operator=(const BoundingBox& p_rhs);
	
private:
	bool checkSphereCollision(const math::Vector3& p_point) const;
	bool checkAABBCollision(const math::Vector3& p_point) const;
	bool checkOOBBCollision(const math::Vector3& p_point) const;
	bool checkScreenCollision(const math::Vector3& p_point) const;
	
	bool checkSphereCollision(const BoundingBox& p_sphere) const;
	bool checkAABBCollision(const BoundingBox& p_box) const;
	bool checkOOBBCollision(const BoundingBox& p_box) const;
	bool checkScreenCollision(const BoundingBox& p_box) const;
	
private:
	// Type of detection
	u32 m_flags;
	
	// Screen space info
	math::Vector3 m_screenPosition; // NOTE: z component not used!
	real          m_screenWidth;
	real          m_screenHeight;
	
	math::Vector3 m_OOBB[8]; // Object Oriented Bounding Box
	math::Vector3 m_AABB[2]; // Axis Aligned Bounding Box
	
	renderer::Sphere m_boundingSphere;
	real             m_sphereRadiusSqr;
};


inline bool BoundingBox::checkSphereCollision(const BoundingBox& p_box) const
{
	// IW - converted fromm real64 to real, should only matter on fixed point not floating point!
	// Get the distance between the two points
	real distance = (p_box.getSphereCenter() -
		this->getSphereCenter()).lengthSquared();
	return (distance < (p_box.getSphereRadiusSqr() + getSphereRadiusSqr()));
}


inline bool BoundingBox::checkSphereCollision(const math::Vector3& p_point) const
{
	// Get the distance between the two points
	// IW - converted fromm real64 to real, should only matter on fixed point not floating point!
	real distance = (p_point - getSphereCenter()).lengthSquared();
	return (distance < m_sphereRadiusSqr);
}


inline bool BoundingBox::checkCollision(real p_x, real p_y) const
{
	if (checkFlag(Flag_ScreenValid))
	{
		// Check screen
		if (p_x < m_screenPosition.x ||
			p_x >(m_screenPosition.x + m_screenWidth))
		{
			return false;
		}
		
		if (p_y < m_screenPosition.y ||
			p_y >(m_screenPosition.y + m_screenHeight))
		{
			return false;
		}
		return true;
	}
	return false;
}

// Namespace end
}
}
}



#endif // INC_TT_ENGINE_PHYSICS_BOUNDINGBOX_H
