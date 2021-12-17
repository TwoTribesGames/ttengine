#if !defined(INC_TT_ENGINE_SCENE_FRUSTUM_H)
#define INC_TT_ENGINE_SCENE_FRUSTUM_H


#include <tt/platform/tt_types.h>
#include <tt/engine/scene/fwd.h>
#include <tt/math/fwd.h>
#include <tt/math/Rect.h>


namespace tt {
namespace engine {
namespace scene {


class Frustum
{
public:
	Frustum();
	
	void updateRatio(real p_fov, real p_aspect);
	void updatePlanes(real p_near, real p_far);

	bool containsPoint(const math::Vector3& p_point) const;
	bool containsSphere(const math::Vector3& p_pos, real p_radius) const;

	math::VectorRect getCullRect(real p_distance) const;

	inline void setPass(bool p_top) {m_passTop = p_top;}

	void render(Camera* p_camera) const;

private:
	real m_radarTangent;
	real m_far;
	real m_near;
	real m_aspect;
	bool m_passTop;
};


// Namespace end
}
} 
}

#endif // INC_TT_ENGINE_SCENE_FRUSTUM_H
