#ifndef INC_TT_ENGINE_PARTICLES_PARTICLEEFFECT_H
#define INC_TT_ENGINE_PARTICLES_PARTICLEEFFECT_H


#include <tt/engine/particles/ParticleTrigger.h>
#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace particles {

class ParticleEffect;
typedef tt_ptr<ParticleEffect>::shared ParticleEffectPtr;


class ParticleEffect
{
public:
	~ParticleEffect();
	
	/*! \brief Play and stop commands */
	void play();
	void stop(bool p_continueParticles = true);
	
	void setOrigin(const tt::math::Vector2& p_origin);
	
	inline ParticleEffectPtr clone(bool p_keepReferenceObject = true) const
	{
		return ParticleEffectPtr(new ParticleEffect((m_trigger == 0) ? 0 : m_trigger->clone(p_keepReferenceObject)));
	}
	
	ParticleEffectPtr cloneAndMoveTrigger(bool p_keepReferenceObject = true);
	
	void spawn() const
	{
		if (m_trigger != 0) m_trigger->trigger(engine::particles::ParticleTrigger::TriggerType_Start);
	}
	
	bool isActive() const 
	{
		return m_trigger != 0 ? m_trigger->isActive() : false;
	}
	
	bool isCulled() const
	{
		return m_trigger != 0 ? m_trigger->isCulled() : true;
	}
	
	inline tt::engine::particles::ParticleTrigger* getTrigger() const { return m_trigger; }
	
	void flip(u32 p_flipMask);
	
private:
	static ParticleEffectPtr create(const std::string&                    p_filename,
	                                const math::Vector3&                  p_position,
	                                const engine::particles::WorldObject* p_object,
	                                real                                  p_scale,
	                                u32                                   p_category,
	                                bool                                  p_trigger = true,
	                                s32                                   p_renderGroup = -1);
	
	static ParticleEffectPtr createInScene(const std::string&                                p_filename,
	                                       engine::scene2d::SceneInterface*                  p_scene,
	                                       const math::Vector3&                              p_position, // Ignored when p_planeFollower is passed.
	                                       const engine::scene2d::shoebox::PlaneFollowerPtr& p_planeFollower,
	                                       real                                              p_scale,
	                                       u32                                               p_category,
	                                       s32                                               p_renderGroup);
	
	explicit ParticleEffect(engine::particles::ParticleTrigger* p_trigger);
	
	engine::particles::ParticleTrigger* m_trigger;
	
	
	// No copying
	ParticleEffect(const ParticleEffect&);
	ParticleEffect& operator=(const ParticleEffect&);
	
	friend class ParticleMgr;
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_PARTICLES_PARTICLEEFFECT_H
