#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/scene2d/shoebox/PlaneFollower.h>


namespace tt {
namespace engine {
namespace particles {

//--------------------------------------------------------------------------------------------------
// Public member functions

ParticleEffect::~ParticleEffect()
{
	if (ParticleMgr::hasInstance())
	{
		ParticleMgr::getInstance()->removeTrigger(m_trigger);
	}
}


void ParticleEffect::play()
{
	if (m_trigger->isActive() == false)
	{
		m_trigger->trigger(engine::particles::ParticleTrigger::TriggerType_Start);
	}
}


void ParticleEffect::setOrigin(const tt::math::Vector2& p_origin)
{
	m_trigger->setOrigin(p_origin);
}


void ParticleEffect::stop(bool p_continueParticles /*= true*/)
{
	if (m_trigger->isActive())
	{
		if (p_continueParticles)
		{
			m_trigger->trigger(engine::particles::ParticleTrigger::TriggerType_Stop);
			ParticleMgr::getInstance()->addEffect(SpawnType_OneShot, cloneAndMoveTrigger(false));
		}
		m_trigger->trigger(engine::particles::ParticleTrigger::TriggerType_Kill);
	}
}


ParticleEffectPtr ParticleEffect::cloneAndMoveTrigger(bool p_keepReferenceObject)
{
	// Create a clone of this ParticleEffect which gets the m_trigger.
	// Create a copy of m_trigger to keep for yourself.
	// (So particle limits are applied to your own copy.)
	if (m_trigger == 0)
	{
		return ParticleEffectPtr();
	}
	
	engine::particles::ParticleTrigger* triggerOwnCopy = m_trigger->clone();
	engine::particles::ParticleTrigger* triggerToMove  = m_trigger;
	if (p_keepReferenceObject == false)
	{
		triggerToMove->setFollowObject(0);
	}
	
	m_trigger = triggerOwnCopy;
	return ParticleEffectPtr(new ParticleEffect(triggerToMove));
}


void ParticleEffect::flip(u32 p_flipMask)
{
	if(m_trigger != 0)
	{
		m_trigger->flip(p_flipMask);
	}
}

//--------------------------------------------------------------------------------------------------
// Private member functions

ParticleEffectPtr ParticleEffect::create(const std::string&                    p_filename,
                                         const math::Vector3&                  p_position,
                                         const engine::particles::WorldObject* p_object,
                                         real                                  p_scale,
                                         u32                                   p_category,
                                         bool                                  p_trigger /*= true*/,
                                         s32                                   p_renderGroup /*= -1*/)
{
	engine::particles::ParticleTrigger* trigger =
		engine::particles::ParticleMgr::getInstance()->addTrigger(
			p_filename, p_object, p_scale, p_category, p_renderGroup);
	if (trigger == 0)
	{
		// NOTE: Panic about being unable to load particle effect is already triggered by ParticleMgr
		return ParticleEffectPtr();
	}
	
	if (p_object == 0)
	{
		trigger->setOrigin(math::Vector2(p_position.x, p_position.y));
		trigger->setDepth(p_position.z);
	}
	
	if (p_trigger)
	{
		trigger->trigger(engine::particles::ParticleTrigger::TriggerType_Start);
	}
	
	return ParticleEffectPtr(new ParticleEffect(trigger));
}


ParticleEffectPtr ParticleEffect::createInScene(const std::string&                                p_filename,
                                                engine::scene2d::SceneInterface*                  p_scene,
                                                const math::Vector3&                              p_position,
                                                const engine::scene2d::shoebox::PlaneFollowerPtr& p_planeFollower,
                                                real                                              p_scale,
                                                u32                                               p_category,
                                                s32                                               p_renderGroup)
{
	engine::particles::ParticleTrigger* trigger =
		engine::particles::ParticleMgr::getInstance()->addTriggerToScene(
			p_filename, p_scene, p_scale, p_category, p_renderGroup);
	
	if (trigger == 0)
	{
		// NOTE: Panic about being unable to load particle effect is already triggered by ParticleMgr
		return ParticleEffectPtr();
	}
	
	if (p_planeFollower == 0)
	{
		trigger->setOrigin(math::Vector2(p_position.x, p_position.y));
	}
	else
	{
		trigger->setFollowObject(p_planeFollower.get());
	}
	trigger->setDepth(p_position.z);
	
	trigger->trigger(engine::particles::ParticleTrigger::TriggerType_Start);
	
	return ParticleEffectPtr(new ParticleEffect(trigger));
}


ParticleEffect::ParticleEffect(engine::particles::ParticleTrigger* p_trigger)
:
m_trigger(p_trigger)
{
}

// Namespace end
}
}
}
