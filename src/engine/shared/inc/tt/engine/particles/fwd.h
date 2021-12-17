#if !defined(INC_TT_ENGINE_PARTICLES_FWD_H)
#define INC_TT_ENGINE_PARTICLES_FWD_H

#include <tt/platform/tt_types.h>
#include <tt/math/hash/Hash.h>

namespace tt {
namespace engine {
namespace particles {

struct Particle;
class WorldObject;
class ParticleMgr;

class ParticleEmitter;
typedef tt_ptr<ParticleEmitter>::shared ParticleEmitterPtr;
typedef tt_ptr<ParticleEmitter>::weak   ParticleEmitterWeakPtr;

class ParticleTrigger;
typedef tt_ptr<ParticleTrigger>::shared ParticleTriggerPtr;
typedef tt_ptr<ParticleTrigger>::weak   ParticleTriggerWeakPtr;

class ParticleEffect;
typedef tt_ptr<ParticleEffect>::shared ParticleEffectPtr;

typedef math::hash::Hash<32> TriggerID;

}
}
}

#endif // !defined(INC_TT_ENGINE_PARTICLES_FWD_H)
