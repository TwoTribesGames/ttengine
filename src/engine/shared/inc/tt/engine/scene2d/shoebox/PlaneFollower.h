#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_PLANEFOLLOWER_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_PLANEFOLLOWER_H

#include <tt/engine/particles/WorldObject.h>
#include <tt/engine/scene2d/fwd.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

class PlaneFollower : public particles::WorldObject
{
public:
	PlaneFollower(PlaneScene* p_parent = 0, const tt::math::Vector3& p_offset = tt::math::Vector3::zero)
	:
	m_offset(p_offset),
	m_parent(p_parent)
	{ }
	
	virtual tt::math::Vector3 getPosition() const;
	inline virtual real       getScale() const             { return 1.0f; }
	inline virtual real       getScaleForParticles() const { return -1.0f; }
	virtual bool              isCulled() const;
	
private:
	tt::math::Vector3 m_offset;
	PlaneScene*       m_parent;
};

// Namespace end
}
}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_PLANEFOLLOWER_H)
