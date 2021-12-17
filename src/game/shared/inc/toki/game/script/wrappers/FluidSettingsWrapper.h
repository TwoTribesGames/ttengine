#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_FLUIDSETTINGSWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_FLUIDSETTINGSWRAPPER_H


#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/fluid/FluidSettings.h>


namespace toki   /*! */ {
namespace game   /*! */ {
namespace script /*! */ {
namespace wrappers {

/*! \brief The fluid Wave settings for an entity */
class FluidWaveSettingsWrapper
{
public:
	FluidWaveSettingsWrapper()
	:
	m_settings()
	{}
	
	FluidWaveSettingsWrapper(fluid::FluidSettings::Wave p_settings)
	:
	m_settings(p_settings)
	{}
	
	/*! \brief Construct with values */
	FluidWaveSettingsWrapper(real p_strength, real p_width, real p_duration, real p_positionOffset)
	:
	m_settings(p_strength, p_width, p_duration, p_positionOffset)
	{}
	
	
	/*! \brief Set the strength of the wave to generate.
	    \param p_strength The height the wave will be set at when this wave is triggered. */
	inline void setStrength(real p_strength) { m_settings.strength = p_strength; }
	
	/*! \brief Gets the strength of this wave.
	    \return Strength of the wave */
	inline real getStrength() const { return m_settings.strength; }
	
	/*! \brief Set the width of the wave to generate.
	    \param p_width The width the wave will be set at when this wave is triggered. */
	inline void setWidth(real p_width) { m_settings.width = p_width; }
	
	/*! \brief Gets the strength of this wave.
	    \return Width of the wave */
	inline real getWidth() const { return m_settings.width; }
	
	/*! \brief Set the duration the height and width will be set on the wave.
	    \param p_duration The duration the wave will be set. */
	inline void setDuration(real p_duration) { m_settings.duration = p_duration; }
	
	/*! \brief Gets the duration of this wave.
	    \return duration of the wave */
	inline real getDuration() const { return m_settings.duration; }
	
	/*! \brief Set the horizontal positionOffset this wave will be triggered at.
	    \param p_duration The positionOffset the wave will be set. */
	inline void setpositionOffset(real p_positionOffset) { m_settings.positionOffset = p_positionOffset; }
	
	/*! \brief Gets the positionOffset of this wave.
	    \return positionOffset of the wave */
	inline real getpositionOffset() const { return m_settings.positionOffset; }
	
	
	inline const fluid::FluidSettings::Wave& getSettings() const { return m_settings; }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	fluid::FluidSettings::Wave m_settings;
};


/*! \brief The fluid Particle settings for an entity */
class FluidParticleSettingsWrapper
{
public:
	FluidParticleSettingsWrapper()
	:
	m_settings()
	{}
	
	FluidParticleSettingsWrapper(fluid::FluidSettings::ParticleSettings p_settings)
	:
	m_settings(p_settings)
	{}
	
	/*! \brief Construct with values */
	FluidParticleSettingsWrapper(bool p_enabled, const std::string& p_triggerFileName)
	:
	m_settings(p_enabled, p_triggerFileName)
	{}
	
	
	/*! \brief Set whether this particle is enabled. */
	inline void setEnabled(bool p_enabled) { m_settings.enabled = p_enabled; }
	
	/*! \brief Gets whether this particle is enabled. */
	inline bool isEnabled() const { return m_settings.enabled; }
	
	/*! \brief Set the filename of the particle effect to trigger.
	    \param p_triggerFile The filename of the particle effect to trigger. */
	inline void setTriggerFile(const std::string& p_triggerFile) { m_settings.triggerFileName = p_triggerFile; }
	
	/*! \brief Gets the filename of the particle effect to trigger.
	    \return The filename of the particle effect to trigger */
	inline const std::string& getTriggerFile() const { return m_settings.triggerFileName; }
	
	
	inline const fluid::FluidSettings::ParticleSettings& getSettings() const { return m_settings; }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	fluid::FluidSettings::ParticleSettings m_settings;
};


/*! \brief The fluid settings for an entity */
class FluidSettingsWrapper
{
public:
	explicit FluidSettingsWrapper(fluid::FluidType p_type = fluid::FluidType_Invalid,
	                              entity::EntityHandle p_handle = entity::EntityHandle());
	
	/*! \brief Set whether this entity generates waves.
	    \param p_wavegenerationEnabled Whether this entity generates waves. */
	void setWaveGenerationEnabled(bool p_wavegenerationEnabled);
	
	/*! \brief Gets whether this entity generates waves.
	    \return Whether this entity generates waves */
	bool isWaveGenerationEnabled() const;
	
	/*! \brief Set the wave settings for when this entity falls in a pool
	    \param p_waveSettings Wave settings */
	void setFallInPoolWaveSettings(const FluidWaveSettingsWrapper& p_waveSettings);
	
	/*! \brief Gets the wave settings for when this entity falls in a pool
	    \return the wave settings for when this entity falls in a pool */
	FluidWaveSettingsWrapper getFallInPoolWaveSettings() const;
	
	/*! \brief Set the wave settings at the side this entity is moving towards
	    \param p_waveSettings Wave settings */
	void setForwardInPoolWaveSettings(const FluidWaveSettingsWrapper& p_waveSettings);
	
	/*! \brief Gets the wave settings at the side this entity is moving towards
	    \return Wave settings */
	FluidWaveSettingsWrapper getForwardInPoolWaveSettings() const;
	
	/*! \brief Set the wave settings at the side this entity is moving from
	    \param p_waveSettings Wave settings */
	void setBehindInPoolWaveSettings(const FluidWaveSettingsWrapper& p_waveSettings);
	
	/*! \brief Gets the wave settings at the side this entity is moving from
	    \return Wave settings */
	FluidWaveSettingsWrapper getBehindInPoolWaveSettings() const;
	
	/*! \brief Set the particle settings for the particles at the surface of the fluid.
	    \param p_particleSettings Particle settings */
	void setSurfaceParticleSettings(const FluidParticleSettingsWrapper& p_particleSettings);
	
	/*! \brief Gets the particle settings for the particles at the surface of the fluid.
	    \return Particle settings */
	FluidParticleSettingsWrapper getSurfaceParticleSettings() const;
	
	/*! \brief Set the particle settings for the particles at the position where a waterfall hits the entity.
	    \param p_particleSettings Particle settings */
	void setUnderFallParticleSettings(const FluidParticleSettingsWrapper& p_particleSettings);
	
	/*! \brief Gets the particle settings for the particles at the position where a waterfall hits the entity.
	    \return Particle settings */
	FluidParticleSettingsWrapper getUnderFallParticleSettings() const;
	
	/*! \brief Set the cue of the sound to play when the entity enters a fluid.
	    \param p_enterFluidCue The cue of the sound to play when the entity enters a fluid. */
	void setEnterFluidSoundCue(const std::string& p_enterFluidCue);
	
	/*! \brief Gets the cue of the sound to play when the entity enters a fluid.
	    \return The cue of the sound to play when the entity enters a fluid. */
	std::string getEnterFluidSoundCue() const;
	
	/*! \brief Set the cue of the sound to play when the entity exits a fluid.
	    \param p_exitFluidCue The cue of the sound to play when the entity exits a fluid. */
	void setExitFluidSoundCue(const std::string& p_exitFluidCue);
	
	/*! \brief Gets the cue of the sound to play when the entity exits a fluid.
	    \return The cue of the sound to play when the entity exit a fluid. */
	std::string getExitFluidSoundCue() const;
	
	/*! \brief Set the particle effect to play when the entity enters a fluid.
	    \param p_effectName The particle effect name to play when the entity enters a fluid. */
	void setEnterFluidParticleEffect(const std::string& p_effectName);
	
	/*! \brief Gets the particle effect name that is played when the entity enters a fluid.
	    \return The particle effect name that is played when the entity enters a fluid. */
	std::string getEnterFluidParticleEffect() const;
	
	/*! \brief Set the particle effect to play when the entity exits a fluid.
	    \param p_effectName The particle effect name to play when the entity exits a fluid. */
	void setExitFluidParticleEffect(const std::string& p_effectName);
	
	/*! \brief Gets the particle effect name that is played when the entity exits a fluid.
	    \return The particle effect name that is played when the entity exits a fluid. */
	std::string getExitFluidParticleEffect() const;
	
	/*! \brief Set the cue of the sound to play when the entity is in a fluidpool.
	    \param p_enterFluidCue The cue of the sound to play when the entity is in a fluidpool. */
	void setInFluidPoolSoundCue(const std::string& p_inFluidPoolCue);
	
	/*! \brief Gets the cue of the sound to play when the entity is in a fluidpool.
	    \return The cue of the sound to play when the entity is in a fluidpool. */
	std::string getInFluidPoolSoundCue() const;
	
	/*! \brief Set the cue of the sound to play when the entity is standing under a fluid fall.
	    \param p_enterFluidCue The cue of the sound to play when the entity is standing under a fluid fall. */
	void setUnderFluidFallSoundCue(const std::string& p_underFluidFallCue);
	
	/*! \brief Gets the cue of the sound to play when the entity is standing under a fluid fall.
	    \return The cue of the sound to play when the entity is standing under a fluid fall. */
	std::string getUnderFluidFallSoundCue() const;
	
	void serialize(tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext* p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	const fluid::FluidSettingsPtr getFluidSettings() const;
	fluid::FluidSettingsPtr       getFluidSettings();
	
	fluid::FluidType     m_type;
	entity::EntityHandle m_entityHandle;
};

// Constructor with values in squirrel
FluidWaveSettingsWrapper*     FluidWaveSettingsWrapper_constructor(HSQUIRRELVM v);
FluidParticleSettingsWrapper* FluidParticleSettingsWrapper_constructor(HSQUIRRELVM v);

// Namespace end
}
}
}
}

#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_FLUIDSETTINGSWRAPPER_H)
