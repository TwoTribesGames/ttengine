#if !defined(INC_TOKI_GAME_FLUID_FLUIDSETTINGS_H)
#define INC_TOKI_GAME_FLUID_FLUIDSETTINGS_H


#include <tt/code/fwd.h>
#include <tt/platform/tt_error.h>

#include <toki/game/fluid/fwd.h>
#include <toki/game/fluid/types.h>


namespace toki {
namespace game {
namespace fluid {


struct FluidSettings
{
	struct Wave
	{
		inline Wave(real p_strength, real p_width, real p_duration, real p_positionOffset)
		:
		strength(p_strength), width(p_width), duration(p_duration), positionOffset(p_positionOffset)
		{}
		
		inline Wave()
		:
		strength(0.4f), width(2.0f), duration(0.05f), positionOffset(0.0f)
		{}
		
		real strength;
		real width;
		real duration;
		real positionOffset;
		
		void serialize(tt::code::BufferWriteContext* p_context) const;
		static FluidSettings::Wave unserialize(tt::code::BufferReadContext* p_context);
	};
	
	struct ParticleSettings
	{
		bool        enabled;
		std::string triggerFileName;
		
		inline ParticleSettings(bool p_enabled, const std::string& p_triggerFileName)
		:
		enabled(p_enabled), triggerFileName(p_triggerFileName)
		{}
		
		inline ParticleSettings()
		:
		enabled(false), triggerFileName("particles/placeholder_continuous.trigger")
		{}
		
		void serialize(tt::code::BufferWriteContext* p_context) const;
		static FluidSettings::ParticleSettings unserialize(tt::code::BufferReadContext* p_context);
	};
	
	
	bool waveGenerationEnabled;
	
	Wave fallInPool;
	Wave forwardInPool;
	Wave behindInPool;
	
	std::string enterSound;
	std::string exitSound;
	std::string enterParticleEffect;
	std::string exitParticleEffect;
	
	ParticleSettings particles[EntityFluidEffectType_Count];
	std::string      sound    [EntityFluidEffectType_Count];
	
	inline ParticleSettings& modifyParticles(EntityFluidEffectType p_type)
	{
		if (isValidEntityFluidEffectType(p_type) == false)
		{
			TT_PANIC("Incorrect EntityFluidEffectType: %d", p_type);
			return particles[0]; // Need to return something;
		}
		return particles[p_type];
	}
	inline const ParticleSettings& getParticles(EntityFluidEffectType p_type) const
	{
		if (isValidEntityFluidEffectType(p_type) == false)
		{
			TT_PANIC("Incorrect EntityFluidEffectType: %d", p_type);
			return particles[0]; // Need to return something;
		}
		return particles[p_type];
	}
	
	inline std::string& modifySound(EntityFluidEffectType p_type)
	{
		if (isValidEntityFluidEffectType(p_type) == false)
		{
			TT_PANIC("Incorrect EntityFluidEffectType: %d", p_type);
			return sound[0]; // Need to return something;
		}
		return sound[p_type];
	}
	
	inline const std::string& getSound(EntityFluidEffectType p_type) const
	{
		if (isValidEntityFluidEffectType(p_type) == false)
		{
			TT_PANIC("Incorrect EntityFluidEffectType: %d", p_type);
			return sound[0]; // Need to return something;
		}
		return sound[p_type];
	}
	
	inline FluidSettings(bool p_waveGenerationEnabled,
	                     Wave p_fallInPool, Wave p_forwardInPool, Wave p_behindInPool, 
	                     ParticleSettings p_underFallParticles, ParticleSettings p_surfaceParticles)
	:
	waveGenerationEnabled(p_waveGenerationEnabled),
	fallInPool(p_fallInPool),
	forwardInPool(p_forwardInPool),
	behindInPool(p_behindInPool),
	enterSound(),
	exitSound(),
	enterParticleEffect(),
	exitParticleEffect()
	{
		modifyParticles(EntityFluidEffectType_Fall)    = p_underFallParticles;
		modifyParticles(EntityFluidEffectType_Surface) = p_surfaceParticles;
	}
	
	FluidSettings();
	
	void serialize(tt::code::BufferWriteContext* p_context) const;
	static FluidSettingsPtr unserialize(tt::code::BufferReadContext*  p_context);
};



// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_FLUIDSETTINGS_H)
