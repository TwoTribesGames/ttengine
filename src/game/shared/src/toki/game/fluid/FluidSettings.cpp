
#include <tt/code/bufferutils.h>

#include <toki/game/fluid/FluidSettings.h>


namespace toki {
namespace game {
namespace fluid {

void FluidSettings::Wave::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(strength,       p_context);
	bu::put(width,          p_context);
	bu::put(duration,       p_context);
	bu::put(positionOffset, p_context);
}


FluidSettings::Wave FluidSettings::Wave::unserialize(tt::code::BufferReadContext*  p_context)
{
	FluidSettings::Wave settings;
	
	namespace bu = tt::code::bufferutils;
	
	settings.strength       = bu::get<real>(p_context);
	settings.width          = bu::get<real>(p_context);
	settings.duration       = bu::get<real>(p_context);
	settings.positionOffset = bu::get<real>(p_context);
	
	return settings;
}


void FluidSettings::ParticleSettings::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(enabled,         p_context);
	bu::put(triggerFileName, p_context);
}


FluidSettings::ParticleSettings FluidSettings::ParticleSettings::unserialize(tt::code::BufferReadContext*  p_context)
{
	FluidSettings::ParticleSettings settings;
	
	namespace bu = tt::code::bufferutils;
	
	settings.enabled         = bu::get<bool>(p_context);
	settings.triggerFileName = bu::get<std::string>(p_context);
	
	return settings;
}


void FluidSettings::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(waveGenerationEnabled, p_context);
	
	fallInPool.serialize(p_context);
	forwardInPool.serialize(p_context);
	behindInPool.serialize(p_context);
	
	bu::put(enterSound, p_context);
	bu::put(exitSound,  p_context);
	bu::put(enterParticleEffect, p_context);
	bu::put(exitParticleEffect,  p_context);
	for (s32 i = 0; i < EntityFluidEffectType_Count; ++i)
	{
		bu::put(sound[i], p_context);
	}
	for (s32 i = 0; i < EntityFluidEffectType_Count; ++i)
	{
		particles[i].serialize(p_context);
	}
}


FluidSettingsPtr FluidSettings::unserialize(tt::code::BufferReadContext*  p_context)
{
	FluidSettingsPtr settings = FluidSettingsPtr(new FluidSettings());
	
	namespace bu = tt::code::bufferutils;
	
	settings->waveGenerationEnabled = bu::get<bool>(p_context);
	
	settings->fallInPool          = FluidSettings::Wave::unserialize(p_context);
	settings->forwardInPool       = FluidSettings::Wave::unserialize(p_context);
	settings->behindInPool        = FluidSettings::Wave::unserialize(p_context);
	
	settings->enterSound          = bu::get<std::string>(p_context);
	settings->exitSound           = bu::get<std::string>(p_context);
	settings->enterParticleEffect = bu::get<std::string>(p_context);
	settings->exitParticleEffect  = bu::get<std::string>(p_context);
	
	for (s32 i = 0; i < EntityFluidEffectType_Count; ++i)
	{
		EntityFluidEffectType type = static_cast<EntityFluidEffectType>(i);
		settings->modifySound(type) = bu::get<std::string>(p_context);
	}
	for (s32 i = 0; i < EntityFluidEffectType_Count; ++i)
	{
		EntityFluidEffectType type = static_cast<EntityFluidEffectType>(i);
		settings->modifyParticles(type) = FluidSettings::ParticleSettings::unserialize(p_context);
	}
	
	return settings;
}


FluidSettings::FluidSettings()
:
waveGenerationEnabled(false),
// real p_strength, real p_width, real p_duration, real p_positionOffset
fallInPool(   0.5f, 2.0f, 0.5f, 0.0f),
forwardInPool(0.25f, 0.5f, 0.3f , 2.0f),
behindInPool( 0.1f, 0.5f, 0.05f, 0.5f),
enterSound(),
exitSound(),
enterParticleEffect(),
exitParticleEffect()
{}


// Namespace end
}
}
}
