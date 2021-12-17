#if !defined(INC_TT_ENGINE_PARTICLES_WORLDOBJECT)
#define INC_TT_ENGINE_PARTICLES_WORLDOBJECT

#include <tt/math/Vector3.h>

namespace tt {
namespace engine {
namespace particles {

class WorldObject
{
public:
	WorldObject() 
	: m_isCulled(false)
	{}
	virtual ~WorldObject() {}
	
	virtual tt::math::Vector3 getPosition() const = 0;
	virtual real getScale() const = 0;
	virtual real getScaleForParticles() const = 0;
	
	// Note JL: made non-virtual
	bool isCulled() const {
		return m_isCulled;
	};

protected:
	bool m_isCulled;
};

// Namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_PARTICLES_WORLDOBJECT)
