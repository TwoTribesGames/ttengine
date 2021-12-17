#if !defined(INC_TT_ENGINE_PHYSICS_COLLISIONMODEL_H)
#define INC_TT_ENGINE_PHYSICS_COLLISIONMODEL_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/scene/fwd.h>
#include <tt/engine/physics/fwd.h>


namespace tt {
namespace engine {
namespace physics {


class CollisionModel
{
public:
	CollisionModel();
	~CollisionModel();
	
	bool generate(scene::Model* p_model);
	bool rayIntersect(const math::Vector3& p_start,
	                  const math::Vector3& p_dir,
	                  math::Vector3* p_collide);
	bool lineIntersect(const math::Vector3& p_start,
	                   const math::Vector3& p_dir,
	                   math::Vector3* p_collide);
	
private:
	s32  countTriangles(const scene::SceneObjectPtr& p_object,
		                s32 p_currentCount = 0) const;
	void extractPlane(const scene::SceneObjectPtr& p_object, 
		              s32 currentCount = 0);
	
	// No copying
	CollisionModel(const CollisionModel&);
	CollisionModel& operator=(const CollisionModel&);
	
	
	scene::Model* m_model;
	s32           m_collisionFaceCount;
	s32           m_collidedFace;
	
	CollisionFace* m_collisionFaces;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_PHYSICS_COLLISIONMODEL_H
