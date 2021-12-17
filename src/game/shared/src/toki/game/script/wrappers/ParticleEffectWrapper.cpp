#include <algorithm>
#include <vector>

#include <tt/code/bufferutils.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/platform/tt_error.h>
#include <tt/script/ScriptEngine.h>

#include <toki/game/script/wrappers/ParticleEffectWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

ParticleEffectWrapper::ParticleEffectWrapper()
:
m_particleEffect(),
m_spawnInfo(),
m_flipMask(tt::engine::particles::FlipAxis_None),
m_scale(1.0f),
m_shouldSerializeEmitterSettings(false)
{
}


ParticleEffectWrapper::ParticleEffectWrapper(const tt::engine::particles::ParticleEffectPtr& p_particleEffect,
                                             const SpawnInfo&                                p_spawnInfo)
:
m_particleEffect(p_particleEffect),
m_spawnInfo(p_spawnInfo),
m_flipMask(tt::engine::particles::FlipAxis_None),
m_scale(1.0f),
m_shouldSerializeEmitterSettings(false)
{
	// Retrieve the correct scale
	if (p_particleEffect != 0 && p_particleEffect->getTrigger() != 0)
	{
		m_scale = p_particleEffect->getTrigger()->getScale();
	}
}


void ParticleEffectWrapper::play()
{
	if (m_particleEffect != 0)
	{
		m_particleEffect->play();
	}
}


void ParticleEffectWrapper::stop(bool p_continueParticles)
{
	if (m_particleEffect != 0)
	{
		m_particleEffect->stop(p_continueParticles);
	}
}


void ParticleEffectWrapper::setPosition(const tt::math::Vector2& p_worldPosition)
{
	if (m_particleEffect != 0)
	{
		m_particleEffect->setOrigin(p_worldPosition);
	}
}


void ParticleEffectWrapper::spawn()
{
	if (m_particleEffect != 0 && m_spawnInfo.oneShot == false)
	{
		m_particleEffect->spawn();
		if (m_particleEffect->getTrigger() != 0)
		{
			m_scale = m_particleEffect->getTrigger()->getScale();
		}
	}
}


bool ParticleEffectWrapper::isActive() const
{
	return (m_particleEffect != 0) ? m_particleEffect->isActive() : false;
}


void ParticleEffectWrapper::flipX()
{
	if (m_particleEffect != 0)
	{
		// Toggle X-flip bit
		m_flipMask ^= tt::engine::particles::FlipAxis_X;
		m_particleEffect->flip(tt::engine::particles::FlipAxis_X);
	}
}


void ParticleEffectWrapper::flipY()
{
	if (m_particleEffect != 0)
	{
		// Toggle Y-flip bit
		m_flipMask ^= tt::engine::particles::FlipAxis_Y;
		m_particleEffect->flip(tt::engine::particles::FlipAxis_Y);
	}
}


real ParticleEffectWrapper::getScale() const
{
	return m_scale;
}


void ParticleEffectWrapper::setScale(real p_scale)
{
	if (m_particleEffect != 0)
	{
		tt::engine::particles::ParticleTrigger *trigger = m_particleEffect->getTrigger();
		if (trigger == 0)
		{
			return;
		}
		m_scale = p_scale;
		trigger->setScale(m_scale);
	}
}


void ParticleEffectWrapper::setInitialExternalImpulse(real p_angle, real p_power)
{
	if (m_particleEffect != 0)
	{
		tt::engine::particles::ParticleTrigger *trigger = m_particleEffect->getTrigger();
		if (trigger == 0)
		{
			return;
		}
		trigger->setInitialExternalImpulse(p_angle, p_power);
	}
}


int ParticleEffectWrapper::setEmitterProperties(HSQUIRRELVM p_vm)
{
	tt::script::SqTopRestorerHelper helper(p_vm, true);
	
	if (m_particleEffect == 0)
	{
		return 0;
	}
	
	const SQInteger argc = sq_gettop(p_vm) - 1; // Stack has arguments + context.
	
	if (argc != 2)
	{
		TT_PANIC("ParticleEffectWrapper::setEmitterProperties(integer, table) has %d argument(s), expected 2", argc);
		return 0;
	}
	
	if (sq_gettype(p_vm, 2) != OT_INTEGER)
	{
		TT_PANIC("setEmitterProperties: invalid argument type. First argument should be of type: integer");
		return 0;
	}
	
	if (sq_gettype(p_vm, 3) != OT_TABLE)
	{
		TT_PANIC("setEmitterProperties: invalid argument type. Second argument should be of type: table");
		return 0;
	}
	
	SQInteger index = 0;
	if (SQ_FAILED(sq_getinteger(p_vm, 2, &index)))
	{
		TT_PANIC("setEmitterProperties: invalid argument type. First argument should be of type: integer");
		return 0;
	}
	
	tt::engine::particles::ParticleTrigger *trigger = m_particleEffect->getTrigger();
	if (trigger == 0)
	{
		TT_NULL_ASSERT(trigger);
		return 0;
	}
	
	if (index < 0 || index >= trigger->getEmitterCount())
	{
		TT_PANIC("setEmitterProperties: invalid index '%d'. Expected >= 0 and < %d", index, trigger->getEmitterCount());
		return 0;
	}
	
	m_shouldSerializeEmitterSettings = true;
	
	tt::engine::particles::EmitterSettings& emitterSettings =
		trigger->getEmitter(static_cast<s32>(index))->getSettings();
	
	HSQOBJECT table;
	sq_getstackobj(p_vm, 3, &table);
	
	//if we have a pregeneration time bigger than 0, all of these settings will be ignored. (warn the user)
	TT_ASSERTMSG(emitterSettings.emission.pregeneration_time <= 0.0f,
	             "setEmitterProperties: The emmiter has a pregeneration time > 0, setting properties might not have any effect.");
	
	// Process settings
	sq_pushnull(p_vm); //null iterator
	while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
	{
		const SQChar* strPtr = 0;
		if (SQ_FAILED(sq_getstring(p_vm, -2, &strPtr)))
		{
			TT_PANIC("setEmitterProperties: table should contain entries with keys of type: string");
			return 0;
		}
		const std::string key(strPtr);
		
		// Apply settings to emitters of this trigger
		using namespace tt::engine::particles;
		
		if (key == "origin")
		{
			const tt::math::Vector2 origin = getVector2(p_vm, -1);
			emitterSettings.origin.x = origin.x;
			emitterSettings.origin.y = origin.y;
		}
		else if (key == "particles")
		{
			const real value = getReal(p_vm, -1);
			TT_ASSERTMSG(value >= 0.0f, "setEmitterProperties: particles should be >= 0");
			emitterSettings.emission.particles = value;
		}
		else if (key == "lifetime")
		{
			const real value = getReal(p_vm, -1);
			TT_ASSERTMSG(value >= 0.0f, "setEmitterProperties: lifetime should be >= 0");
			emitterSettings.emission.lifetime = value;
		}
		else if (key == "area_type")
		{
			const std::string type = getString(p_vm, -1);
			if      (type == "circle")    emitterSettings.emission.area_type = EmissionBehavior::AreaType_Circle;
			else if (type == "rectangle") emitterSettings.emission.area_type = EmissionBehavior::AreaType_Rectangle;
			else
			{
				TT_PANIC("Unhandled area_type '%s'; it should be 'circle' or 'rectangle'", type.c_str());
			}
		}
		else if (key == "rect_width")
		{
			const RealRange range = getRealRange(p_vm, -1);
			TT_ASSERTMSG(emitterSettings.emission.area_type == EmissionBehavior::AreaType_Rectangle,
				"setEmitterProperties: cannot set width of non-rectangle emitter");
			emitterSettings.emission.rect_width = range;
		}
		else if (key == "rect_height")
		{
			const RealRange range = getRealRange(p_vm, -1);
			TT_ASSERTMSG(emitterSettings.emission.area_type == EmissionBehavior::AreaType_Rectangle,
				"setEmitterProperties: cannot set height of non-rectangle emitter");
			emitterSettings.emission.rect_height = range;
		}
		else if (key == "inner_radius")
		{
			const real value = getReal(p_vm, -1);
			TT_ASSERTMSG(value >= 0.0f, "setEmitterProperties: innerRadius should be >= 0");
			TT_ASSERTMSG(emitterSettings.emission.area_type == EmissionBehavior::AreaType_Circle,
				"setEmitterProperties: cannot set innerRadius of non-circle emitter");
			emitterSettings.emission.innerRadius = value;
		}
		else if (key == "radius")
		{
			const real value = getReal(p_vm, -1);
			TT_ASSERTMSG(emitterSettings.emission.area_type == EmissionBehavior::AreaType_Circle,
				"setEmitterProperties: cannot set radius of non-circle emitter");
			emitterSettings.emission.radius = value;
		}
		else if (key == "valid_rect")
		{
			if (sq_gettype(p_vm, -1) == OT_NULL)
			{
				trigger->getEmitter(static_cast<s32>(index))->setValidRect(0);
			}
			else
			{
				const tt::math::VectorRect value = getVectorRect(p_vm, -1);
				trigger->getEmitter(static_cast<s32>(index))->setValidRect(&value);
			}
		}
		else if (key == "velocity_x")  // these only change the base velocity, they don't manipulate the range
		{
			const real newMidpoint = getReal(p_vm, -1);
			const real curMidpoint = (emitterSettings.velocity_x.low + emitterSettings.velocity_x.high) / 2;
			emitterSettings.velocity_x.low += newMidpoint - curMidpoint;
			emitterSettings.velocity_x.high += newMidpoint - curMidpoint;
		}
		else if (key == "velocity_y")
		{
			const real newMidpoint = getReal(p_vm, -1);
			const real curMidpoint = (emitterSettings.velocity_y.low + emitterSettings.velocity_y.high) / 2;
			emitterSettings.velocity_y.low += newMidpoint - curMidpoint;
			emitterSettings.velocity_y.high += newMidpoint - curMidpoint;
		}
		else if (key == "start_rotation")
		{
			const real value = getReal(p_vm, -1);
			emitterSettings.particle_creation.start_rotation.low  = value;
			emitterSettings.particle_creation.start_rotation.high = value;
		}
		else if (key == "fps")
		{
			const real fps = getReal(p_vm, -1);
			const real spf = fps > 0.0f ? 1.0f / fps : 0.0f;
			for (tt::engine::particles::ParticleAnimationContainer::iterator it = emitterSettings.animations.begin();
				it != emitterSettings.animations.end(); ++it)
			{
				it->spf = spf;
			}
		}
		else
		{
			TT_PANIC("setEmitterProperties: unsupported key '%s'", key.c_str());
		}
		
		// Next element
		sq_pop(p_vm, 2);
	}
	sq_pop(p_vm, 1);
	
	return 0;
}


void ParticleEffectWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const bool wrapperValid = (m_particleEffect != 0 && m_spawnInfo.spawningEntity.getPtr() != 0);
	bu::put(wrapperValid, p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to save
		return;
	}
	
	bu::put      (m_particleEffect->isActive(),       p_context);
	bu::put      (m_spawnInfo.oneShot,                p_context);
	bu::put      (m_spawnInfo.filename,               p_context);
	bu::put      (m_spawnInfo.position,               p_context);
	bu::put      (m_spawnInfo.followEntity,           p_context);
	bu::putHandle(m_spawnInfo.spawningEntity,         p_context);
	bu::put      (m_spawnInfo.spawnDelay,             p_context);
	bu::put      (m_spawnInfo.positionIsInWorldSpace, p_context);
	
	// Special handling of the "use layer from particle effect" value: it explicitly cannot be
	// translated to/from name (because it is not a real layer), so handle it separately here
	const std::string layerName(
			(m_spawnInfo.particleLayer == ParticleLayer_UseLayerFromParticleEffect) ?
			"from_file" :
			getParticleLayerName(m_spawnInfo.particleLayer));
	bu::put(layerName, p_context);
	
	bu::put(m_flipMask, p_context);
	bu::put(m_scale, p_context);
	
	// Serialize the emitter settings
	bool shouldSerializeEmitterSettings = m_shouldSerializeEmitterSettings;
	if (m_particleEffect->getTrigger() == 0)
	{
		TT_NULL_ASSERT(m_particleEffect->getTrigger());
		shouldSerializeEmitterSettings = false;
	}
	
	bu::put(shouldSerializeEmitterSettings, p_context);
	if (shouldSerializeEmitterSettings)
	{
		const tt::engine::particles::ParticleTrigger* trigger = m_particleEffect->getTrigger();
		bu::put(trigger->getEmitterCount(), p_context);
		for (s32 i = 0; i < trigger->getEmitterCount(); ++i)
		{
			const tt::engine::particles::EmitterSettings& settings = trigger->getEmitter(i)->getSettings();
			
			// Save origin
			bu::put        (settings.origin,                      p_context);
			
			// Save valid area
			const tt::math::VectorRect* validRect = trigger->getEmitter(i)->getValidRect();
			const bool hasValidRect = validRect != 0;
			bu::put        (hasValidRect,                       p_context);
			if (hasValidRect)
			{
				bu::put    (*validRect,                           p_context);
			}
			
			// Save EmissionBehavior
			bu::putEnum<u8>(settings.emission.area_type,          p_context);
			bu::put        (settings.emission.innerRadius,        p_context);
			bu::put        (settings.emission.lifetime,           p_context);
			bu::put        (settings.emission.particles,          p_context);
			bu::put        (settings.emission.pregeneration_step, p_context);
			bu::put        (settings.emission.pregeneration_time, p_context);
			bu::put        (settings.emission.radius,             p_context);
			bu::put        (settings.emission.rect_height.low,    p_context);
			bu::put        (settings.emission.rect_height.high,   p_context);
			bu::put        (settings.emission.rect_width.low,     p_context);
			bu::put        (settings.emission.rect_width.high,    p_context);
			bu::putEnum<u8>(settings.emission.type,               p_context);
		}
	}
}


void ParticleEffectWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	// Start with defaults
	*this = ParticleEffectWrapper();
	
	// Load the particle effect details
	const bool wrapperValid = bu::get<bool>(p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to load
		return;
	}
	
	const bool effectIsActive          = bu::get      <bool             >(p_context);
	m_spawnInfo.oneShot                = bu::get      <bool             >(p_context);
	m_spawnInfo.filename               = bu::get      <std::string      >(p_context);
	m_spawnInfo.position               = bu::get      <tt::math::Vector2>(p_context);
	m_spawnInfo.followEntity           = bu::get      <bool             >(p_context);
	m_spawnInfo.spawningEntity         = bu::getHandle<entity::Entity   >(p_context);
	m_spawnInfo.spawnDelay             = bu::get      <real             >(p_context);
	m_spawnInfo.positionIsInWorldSpace = bu::get      <bool             >(p_context);
	
	// Special handling of the "use layer from particle effect" value: it explicitly cannot be
	// translated to/from name (because it is not a real layer), so handle it separately here
	const std::string layerName = bu::get<std::string>(p_context);
	m_spawnInfo.particleLayer = (layerName == "from_file") ?
			ParticleLayer_UseLayerFromParticleEffect :
			getParticleLayerFromName(layerName);
	
	if (m_spawnInfo.particleLayer != ParticleLayer_UseLayerFromParticleEffect &&
	    isValidParticleLayer(m_spawnInfo.particleLayer) == false)
	{
		TT_PANIC("Loaded unsupported particle layer name from serialization data: '%s', "
		         "for particle effect '%s'\n"
		         "Falling back to 'use layer from particle effect' as layer.",
		         layerName.c_str(), m_spawnInfo.filename.c_str());
		m_spawnInfo.particleLayer = ParticleLayer_UseLayerFromParticleEffect;
	}
	
	// Respawn the particle effect based on the information loaded
	entity::Entity* spawningEntity = m_spawnInfo.spawningEntity.getPtr();
	if (spawningEntity != 0)
	{
		// Have an entity to follow: use that to spawn the effect
		m_particleEffect = spawningEntity->spawnParticle(
				m_spawnInfo.oneShot ? entity::Entity::SpawnType_OneShot : entity::Entity::SpawnType_NoSpawn,
				m_spawnInfo.filename,
				m_spawnInfo.position,
				m_spawnInfo.followEntity,
				m_spawnInfo.spawnDelay,
				m_spawnInfo.positionIsInWorldSpace,
				m_spawnInfo.particleLayer);
	}
	else
	{
		TT_PANIC("Loaded invalid 'spawning entity' handle from serialization data. Particle filename is '%s'.\n"
		         "Cannot respawn particle effect.", 
		         m_spawnInfo.filename.c_str());
		// FIXME: Correctly handle missing loads because of this early exit
		return;
	}
	
	tt::engine::particles::ParticleTrigger* trigger = m_particleEffect->getTrigger();
	if (trigger == 0)
	{
		TT_NULL_ASSERT(m_particleEffect->getTrigger());
		// FIXME: Correctly handle missing loads because of this early exit
		return;
	}
	
	// Handle X/Y flipping
	m_flipMask = bu::get<u32>(p_context);
	if (m_flipMask != tt::engine::particles::FlipAxis_None)
	{
		m_particleEffect->flip(m_flipMask);
	}
	
	// Handle scaling
	m_scale = bu::get<real>(p_context);
	trigger->setScale(m_scale);
	
	// Unserialize the emitter settings
	m_shouldSerializeEmitterSettings = bu::get<bool>(p_context);
	
	if (m_shouldSerializeEmitterSettings)
	{
		s32 emitterCount = bu::get<s32>(p_context);
		
		bool invalidSettings = false;
		if (emitterCount != trigger->getEmitterCount())
		{
			TT_PANIC("ParticleEffectWrapper::unserialize: mismatch between loaded emittercount '%d' and "
			         "spawned effect emittercount %d. Cannot unserialize emitter settings",
			         emitterCount, trigger->getEmitterCount());
			
			// We cannot simply return here, as we need to correctly load the remaining data
			invalidSettings = true;
			m_shouldSerializeEmitterSettings = false;
		}
		
		// Load emitter settings
		for (s32 i = 0; i < emitterCount; ++i)
		{
			using namespace tt::engine::particles;
			
			// Load origin
			const tt::math::Vector2 origin = bu::get<tt::math::Vector2>(p_context);
			
			// Load validrect
			tt::math::VectorRect validRect;
			bool hasValidRect = bu::get<bool>(p_context);
			if (hasValidRect)
			{
				validRect = bu::get<tt::math::VectorRect>(p_context);
			}
			
			// Load EmissionBehavior
			EmissionBehavior settings;
			settings.area_type          = bu::getEnum<u8, EmissionBehavior::AreaType    >(p_context);
			settings.innerRadius        = bu::get    <real                              >(p_context);
			settings.lifetime           = bu::get    <real                              >(p_context);
			settings.particles          = bu::get    <real                              >(p_context);
			settings.pregeneration_step = bu::get    <real                              >(p_context);
			settings.pregeneration_time = bu::get    <real                              >(p_context);
			settings.radius             = bu::get    <real                              >(p_context);
			settings.rect_height.low    = bu::get    <real                              >(p_context);
			settings.rect_height.high   = bu::get    <real                              >(p_context);
			settings.rect_width.low     = bu::get    <real                              >(p_context);
			settings.rect_width.high    = bu::get    <real                              >(p_context);
			settings.type               = bu::getEnum<u8, EmissionBehavior::EmissionType>(p_context);
			
			if (invalidSettings == false)
			{
				// Overwrite existing settings
				tt::engine::particles::ParticleEmitter* emitter = trigger->getEmitter(i);
				emitter->getSettings().origin = origin;
				emitter->getSettings().emission = settings;
				hasValidRect ? emitter->setValidRect(&validRect) : emitter->setValidRect(0);
			}
		}
	}
	
	// Check if we have to spawn the particle.
	if (m_particleEffect != 0 && effectIsActive)
	{
		// Give the particle some pregen settings so it doesn't look like it's just triggered.
		const s32 emitterCount = trigger->getEmitterCount();
		typedef std::vector<real> reals;
		reals pregen_steps(emitterCount, 0.0f);
		reals pregen_time( emitterCount, 0.0f);
		
		for (s32 i = 0; i < emitterCount; ++i)
		{
			tt::engine::particles::EmissionBehavior& settings = trigger->getEmitter(i)->getSettings().emission;
			// Remember pregen settings so we can restore it.
			pregen_steps[i] = settings.pregeneration_step;
			pregen_time [i] = settings.pregeneration_time;
			
			const real unserializePreGenTime = 1.5f;
			// No need to override if it's already larger.
			if (settings.pregeneration_time <= unserializePreGenTime)
			{
				settings.pregeneration_step = 0.025f;
				settings.pregeneration_time = unserializePreGenTime;
			}
		}
		
		spawn(); // Spawn the particle
		
		// Restore pregen settings.
		for (s32 i = 0; i < emitterCount; ++i)
		{
			tt::engine::particles::EmissionBehavior& settings = trigger->getEmitter(i)->getSettings().emission;
			settings.pregeneration_step = pregen_steps[i];
			settings.pregeneration_time = pregen_time [i];
		}
	}
}


void ParticleEffectWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(ParticleEffectWrapper, "ParticleEffect");
	TT_SQBIND_METHOD(ParticleEffectWrapper, play);
	TT_SQBIND_METHOD(ParticleEffectWrapper, stop);
	TT_SQBIND_METHOD(ParticleEffectWrapper, setPosition);
	TT_SQBIND_METHOD(ParticleEffectWrapper, spawn);
	TT_SQBIND_METHOD(ParticleEffectWrapper, isActive);
	TT_SQBIND_METHOD(ParticleEffectWrapper, flipX);
	TT_SQBIND_METHOD(ParticleEffectWrapper, flipY);
	TT_SQBIND_METHOD(ParticleEffectWrapper, getScale);
	TT_SQBIND_METHOD(ParticleEffectWrapper, setScale);
	TT_SQBIND_METHOD(ParticleEffectWrapper, setInitialExternalImpulse);
	TT_SQBIND_SQUIRREL_METHOD(ParticleEffectWrapper, setEmitterProperties);
}


//--------------------------------------------------------------------------------------------------
// Public member functions

real ParticleEffectWrapper::getReal(HSQUIRRELVM p_vm, s32 p_index) const
{
	SQFloat value = 0.0f;
	if (SQ_FAILED(sq_getfloat(p_vm, -1, &value)))
	{
		TT_PANIC("getReal: failed to retrieve real from stack at index '%d'", p_index);
	}
	return static_cast<real>(value);
}


ParticleEffectWrapper::RealRange ParticleEffectWrapper::getRealRange(HSQUIRRELVM p_vm, s32 p_index) const
{
	const real value = getReal(p_vm, p_index);
	tt::engine::particles::Range<real> rangeValue;
	const real halfValue = value * 0.5f;
	rangeValue.high =  halfValue;
	rangeValue.low  = -halfValue;
	return rangeValue;
}


tt::math::Vector2 ParticleEffectWrapper::getVector2(HSQUIRRELVM p_vm, s32 p_index) const
{
	return SqBind<tt::math::Vector2>::get(p_vm, p_index);
}


tt::math::VectorRect ParticleEffectWrapper::getVectorRect(HSQUIRRELVM p_vm, s32 p_index) const
{
	return SqBind<tt::math::VectorRect>::get(p_vm, p_index);
}


std::string ParticleEffectWrapper::getString(HSQUIRRELVM p_vm, s32 p_index) const
{
	return SqBind<std::string>::get(p_vm, p_index);
}

// Namespace end
}
}
}
}
