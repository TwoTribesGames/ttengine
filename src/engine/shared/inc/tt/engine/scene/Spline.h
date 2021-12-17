#if !defined(INC_TT_ENGINE_SCENE_SPLINE_H)
#define INC_TT_ENGINE_SCENE_SPLINE_H


#include <tt/platform/tt_types.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/math/Spline.h>


namespace tt {
namespace engine {
namespace scene {


class Spline : public SceneObject
{
public:
	inline Spline()
	:
	SceneObject(SceneObject::Type_Spline),
	m_spline()
	{ }
	virtual ~Spline() {}

	inline math::Vector3 model(real p_u, math::Spline::Mode p_mode = math::Spline::Mode_Time) const
	{
		return m_spline.model(p_u, p_mode);
	}

private:
	math::Spline m_spline;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_SPLINE_H
