#include <tt/code/bufferutils.h>
#include <tt/script/helpers.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/script/wrappers/FluidSettingsWrapper.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// FluidWaveSettingsWrapper Public member functions

void FluidWaveSettingsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT_NAME(FluidWaveSettingsWrapper, "FluidWaveSettings");
	TT_SQBIND_SET_CONSTRUCTOR(FluidWaveSettingsWrapper, FluidWaveSettingsWrapper_constructor);
	
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, setStrength);
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, getStrength);
	
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, setWidth);
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, getWidth);
	
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, setDuration);
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, getDuration);
	
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, setpositionOffset);
	TT_SQBIND_METHOD(FluidWaveSettingsWrapper, setpositionOffset);
}

//--------------------------------------------------------------------------------------------------
// FluidParticleSettingsWrapper Public member functions

void FluidParticleSettingsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT_NAME(FluidParticleSettingsWrapper, "FluidParticleSettings");
	TT_SQBIND_SET_CONSTRUCTOR(FluidParticleSettingsWrapper, FluidParticleSettingsWrapper_constructor);
	
	TT_SQBIND_METHOD(FluidParticleSettingsWrapper, setEnabled);
	TT_SQBIND_METHOD(FluidParticleSettingsWrapper, isEnabled);
	
	TT_SQBIND_METHOD(FluidParticleSettingsWrapper, setTriggerFile);
	TT_SQBIND_METHOD(FluidParticleSettingsWrapper, getTriggerFile);
}

//--------------------------------------------------------------------------------------------------
// FluidSettingsWrapper Public member functions

void FluidSettingsWrapper::setWaveGenerationEnabled(bool p_wavegenerationEnabled) 
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->waveGenerationEnabled = p_wavegenerationEnabled; 
	}
}


bool FluidSettingsWrapper::isWaveGenerationEnabled() const 
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->waveGenerationEnabled;
	}
	return false;
}


void FluidSettingsWrapper::setFallInPoolWaveSettings(const FluidWaveSettingsWrapper& p_waveSettings) 
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->fallInPool = p_waveSettings.getSettings();
	}
}


FluidWaveSettingsWrapper FluidSettingsWrapper::getFallInPoolWaveSettings() const 
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return FluidWaveSettingsWrapper(fluidSettings->fallInPool);
	}
	return FluidWaveSettingsWrapper();
}


void FluidSettingsWrapper::setForwardInPoolWaveSettings(const FluidWaveSettingsWrapper& p_waveSettings) 
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->forwardInPool = p_waveSettings.getSettings();
	}
}


FluidWaveSettingsWrapper FluidSettingsWrapper::getForwardInPoolWaveSettings() const 
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return FluidWaveSettingsWrapper(fluidSettings->forwardInPool);
	}
	return FluidWaveSettingsWrapper();
}


void FluidSettingsWrapper::setBehindInPoolWaveSettings(const FluidWaveSettingsWrapper& p_waveSettings) 
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->behindInPool = p_waveSettings.getSettings();
	}
}


FluidWaveSettingsWrapper FluidSettingsWrapper::getBehindInPoolWaveSettings() const 
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return FluidWaveSettingsWrapper(fluidSettings->behindInPool);
	}
	return FluidWaveSettingsWrapper();
}


void FluidSettingsWrapper::setSurfaceParticleSettings(const FluidParticleSettingsWrapper& p_particleSettings) 
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->modifyParticles(fluid::EntityFluidEffectType_Surface) = p_particleSettings.getSettings();
	}
}


FluidParticleSettingsWrapper FluidSettingsWrapper::getSurfaceParticleSettings() const 
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return FluidParticleSettingsWrapper(fluidSettings->getParticles(fluid::EntityFluidEffectType_Surface));
	}
	return FluidParticleSettingsWrapper();
}


void FluidSettingsWrapper::setUnderFallParticleSettings(const FluidParticleSettingsWrapper& p_particleSettings) 
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->modifyParticles(fluid::EntityFluidEffectType_Fall) = p_particleSettings.getSettings();
	}
}


FluidParticleSettingsWrapper FluidSettingsWrapper::getUnderFallParticleSettings() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return FluidParticleSettingsWrapper(fluidSettings->getParticles(fluid::EntityFluidEffectType_Fall));
	}
	return FluidParticleSettingsWrapper();
}


void FluidSettingsWrapper::setEnterFluidSoundCue(const std::string& p_enterFluidCue)
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->enterSound = p_enterFluidCue;
	}
}


std::string FluidSettingsWrapper::getEnterFluidSoundCue() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->enterSound;
	}
	return std::string();
}


void FluidSettingsWrapper::setExitFluidSoundCue(const std::string& p_exitFluidCue)
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->exitSound= p_exitFluidCue;
	}
}


std::string FluidSettingsWrapper::getExitFluidSoundCue() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->exitSound;
	}
	return std::string();
}


void FluidSettingsWrapper::setEnterFluidParticleEffect(const std::string& p_effectName)
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->enterParticleEffect = p_effectName;
	}
}


std::string FluidSettingsWrapper::getEnterFluidParticleEffect() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->enterParticleEffect;
	}
	return std::string();
}


void FluidSettingsWrapper::setExitFluidParticleEffect(const std::string& p_effectName)
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->exitParticleEffect = p_effectName;
	}
}


std::string FluidSettingsWrapper::getExitFluidParticleEffect() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->exitParticleEffect ;
	}
	return std::string();
}


void FluidSettingsWrapper::setInFluidPoolSoundCue(const std::string& p_inFluidPoolCue)
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->modifySound(fluid::EntityFluidEffectType_Surface) = p_inFluidPoolCue;
	}
}


std::string FluidSettingsWrapper::getInFluidPoolSoundCue() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->getSound(fluid::EntityFluidEffectType_Surface);
	}
	return std::string();
}


void FluidSettingsWrapper::setUnderFluidFallSoundCue(const std::string& p_underFluidFallCue)
{
	fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		fluidSettings->modifySound(fluid::EntityFluidEffectType_Fall) = p_underFluidFallCue;
	}
}


std::string FluidSettingsWrapper::getUnderFluidFallSoundCue() const
{
	const fluid::FluidSettingsPtr fluidSettings = getFluidSettings();
	if (fluidSettings != 0)
	{
		return fluidSettings->getSound(fluid::EntityFluidEffectType_Fall);
	}
	return std::string();
}


void FluidSettingsWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8>(m_type, p_context);       // fluid::FluidType
	bu::putHandle(m_entityHandle, p_context); // handle
}


void FluidSettingsWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	m_type         = bu::getEnum<u8, fluid::FluidType>(p_context);
	m_entityHandle = bu::getHandle<entity::Entity>(p_context);
}


void FluidSettingsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT_NAME(FluidSettingsWrapper, "FluidSettings");
	
	TT_SQBIND_METHOD(FluidSettingsWrapper, setWaveGenerationEnabled);
	TT_SQBIND_METHOD(FluidSettingsWrapper, isWaveGenerationEnabled);
	
	TT_SQBIND_METHOD(FluidSettingsWrapper, setFallInPoolWaveSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getFallInPoolWaveSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setForwardInPoolWaveSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getForwardInPoolWaveSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setBehindInPoolWaveSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getBehindInPoolWaveSettings);
	
	TT_SQBIND_METHOD(FluidSettingsWrapper, setSurfaceParticleSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getSurfaceParticleSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setUnderFallParticleSettings);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getUnderFallParticleSettings);
	
	TT_SQBIND_METHOD(FluidSettingsWrapper, setEnterFluidSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getEnterFluidSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setExitFluidSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getExitFluidSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setEnterFluidParticleEffect);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getEnterFluidParticleEffect);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setExitFluidParticleEffect);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getExitFluidParticleEffect);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setInFluidPoolSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getInFluidPoolSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, setUnderFluidFallSoundCue);
	TT_SQBIND_METHOD(FluidSettingsWrapper, getUnderFluidFallSoundCue);
	
	
	FluidWaveSettingsWrapper::bind(p_vm);
	FluidParticleSettingsWrapper::bind(p_vm);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


const fluid::FluidSettingsPtr FluidSettingsWrapper::getFluidSettings() const
{
	const entity::Entity* entity = m_entityHandle.getPtr();
	if (entity != 0)
	{
		return entity->getFluidSettings(m_type);
	}
	
	return fluid::FluidSettingsPtr();
}


fluid::FluidSettingsPtr FluidSettingsWrapper::getFluidSettings()
{
	entity::Entity* entity = m_entityHandle.getPtr();
	if (entity != 0)
	{
		return entity->getFluidSettings(m_type);
	}
	
	return fluid::FluidSettingsPtr();
}


FluidSettingsWrapper::FluidSettingsWrapper(fluid::FluidType p_type, entity::EntityHandle p_handle)
:
m_type(p_type),
m_entityHandle(p_handle)
{
}


//--------------------------------------------------------------------------------------------------
// Squirrel constructors

FluidWaveSettingsWrapper* FluidWaveSettingsWrapper_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 4)
	{
		TT_PANIC("FluidWaveSettingsWrapper incorrect number of parameters. (Got: %d, expected: 4)", params);
		return 0; // need 4 params
	}
	
	SQFloat strength;
	SQFloat width;
	SQFloat duration;
	SQFloat positionOffset;
	
	if (SQ_FAILED(sq_getfloat(v, 2, &strength)))
	{
		TT_PANIC("FluidWaveSettingsWrapper 1st parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 3, &width)))
	{
		TT_PANIC("FluidWaveSettingsWrapper 2nd parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 4, &duration)))
	{
		TT_PANIC("FluidWaveSettingsWrapper 3rd parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 5, &positionOffset)))
	{
		TT_PANIC("FluidWaveSettingsWrapper 4th parameter is not a float value");
		return 0;
	}
	
	return new FluidWaveSettingsWrapper(strength, width, duration, positionOffset);
}


FluidParticleSettingsWrapper* FluidParticleSettingsWrapper_constructor(HSQUIRRELVM v) 
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 2)
	{
		TT_PANIC("FluidParticleSettingsWrapper incorrect number of parameters. (Got: %d, expected: 2)", params);
		return 0; // need 2 params
	}
	
	SQBool enabled;
	const SQChar* triggerFile = 0;
	
	if (SQ_FAILED(sq_getbool(v, 2, &enabled)))
	{
		TT_PANIC("FluidParticleSettingsWrapper 1st parameter is not a bool value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getstring(v, 3, &triggerFile)))
	{
		TT_PANIC("FluidParticleSettingsWrapper 2nd parameter is not a string value");
		return 0;
	}
	
	return new FluidParticleSettingsWrapper(enabled != 0, triggerFile);
}

// Namespace end
}
}
}
}
