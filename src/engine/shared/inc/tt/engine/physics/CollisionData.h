#if !defined(INC_TT_ENGINE_PHYSICS_COLLISIONDATA_H)
#define INC_TT_ENGINE_PHYSICS_COLLISIONDATA_H


#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/engine/physics/Collision.h>
#include <tt/engine/renderer/Material.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace engine {
namespace physics {


class CollisionData
{
public:
	enum ReturnMethod
	{
		ReturnMethod_First = 0,
		ReturnMethod_Closest,
		ReturnMethod_All
	};
	
	CollisionData();
	
	inline ReturnMethod getMethod() const {return m_method;}
	inline void setMethod(ReturnMethod p_method) {m_method = p_method;}
	
	void reset();
	void addCollision(const Collision& p_collision);
	
	inline u32 getCollisionCount() const {return static_cast<u32>(m_collisionData.size());}
	
	inline Collision* getCollision(u32 p_index = 0)
	{
		return &m_collisionData[p_index];
	}
	
	inline const math::Vector3& getCollisionPoint(u32 p_index = 0) const
	{
		return m_collisionData[p_index].getCollisionPoint();
	}
	
	inline const math::Vector3& getCollisionNormal(u32 p_index = 0) const
	{
		return m_collisionData[p_index].getCollisionNormal();
	}
	
	inline const renderer::MaterialPtr& getMaterial(u32 p_index = 0) const
	{
		return m_collisionData[p_index].getMaterial();
	}
	
	inline real getCollisionTime(u32 p_index = 0) const
	{
		return m_collisionData[p_index].getCollisionTime();
	}
	
private:
	ReturnMethod m_method;
	
	typedef std::vector<Collision> CollisionContainer;
	CollisionContainer m_collisionData;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_PHYSICS_COLLISIONDATA_H
