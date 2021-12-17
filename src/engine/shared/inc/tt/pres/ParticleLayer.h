#if !defined(INC_TT_PRES_PARTICLELAYER_H)
#define INC_TT_PRES_PARTICLELAYER_H
#include <tt/platform/tt_types.h>

#include <tt/engine/particles/ParticleMgr.h>
#include <tt/pres/RenderableInterface.h>


namespace tt {
namespace pres {


class ParticleLayer : public RenderableInterface
{
public:
	ParticleLayer(s32 p_renderGroup)
	:
	m_renderGroup(p_renderGroup)
	{}
	virtual ~ParticleLayer(){}
	
	void render() const
	{
		engine::particles::ParticleMgr::getInstance()->renderGroup(m_renderGroup);
	}
	
	
private:
	
	s32 m_renderGroup;
	
	// Disable Copy/assignment
	ParticleLayer(const ParticleLayer&);
	ParticleLayer& operator=(const ParticleLayer&);	
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_PARTICLELAYER_H)
