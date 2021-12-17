#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_PARTICLEEFFECTWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_PARTICLEEFFECTWRAPPER_H


#include <string>

#include <tt/code/fwd.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/math/Vector2.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/types.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'ParticleEffect' in Squirrel. */
class ParticleEffectWrapper
{
public:
	struct SpawnInfo
	{
		bool                 oneShot;                 // One-shot or continuous?
		std::string          filename;                // Filename with which effect was spawned
		tt::math::Vector2    position;                // Position (or position offset) at which effect was spawned
		bool                 followEntity;            // Whether this effect should follow an entity
		entity::EntityHandle spawningEntity;          // The Entity that spawned this effect
		real                 spawnDelay;              // Delay for emitters to start spawning
		bool                 positionIsInWorldSpace;  // World space or entity-local space?
		ParticleLayer        particleLayer;           // Which layer the effect was put in
		real                 scale;                   // The scale of the effect (default is 1.0f)
		
		
		inline SpawnInfo()
		:
		oneShot(true),
		filename(),
		position(tt::math::Vector2::zero),
		followEntity(false),
		spawningEntity(),
		spawnDelay(0.0f),
		positionIsInWorldSpace(false),
		particleLayer(ParticleLayer_Invalid),
		scale(1.0)
		{ }
	};
	
	
	ParticleEffectWrapper();
	ParticleEffectWrapper(const tt::engine::particles::ParticleEffectPtr& p_particleEffect,
	                      const SpawnInfo&                                p_spawnInfo);
	
	inline const tt::engine::particles::ParticleEffectPtr& getEffect() const { return m_particleEffect; }
	
	// Squirrel bindings:
	
	/*! \brief Starts the particle effect if it was not active yet. */
	void play();
	
	/*! \brief Stops the particle effect.
	    \param p_continueParticles Whether existing particles should continue (just stops the emitter),
	                               or all existing particles should be forcibly removed. */
	void stop(bool p_continueParticles);
	
	/*! \brief Sets the worldposition of this effect. NOTE: Only works when effect doesn't have followEntity!
	    \param p_worldPosition The worldposition. */
	void setPosition(const tt::math::Vector2& p_worldPosition);
	
	/*! \brief Starts or restarts the particle effect unconditionally. */
	void spawn();
	
	/*! \brief Indicates whether the particle effect is currently active. */
	bool isActive() const;
	
	/*! \brief Flips the effect in the X direction. */
	void flipX();
	
	/*! \brief Flips the effect in the Y direction. */
	void flipY();
	
	/*! \brief Gets the scale of the effect. */
	real getScale() const;
	
	/*! \brief Scales the effect uniformly. */
	void setScale(real p_scale);
	
	/*! \brief Sets the initial external impulse for this effect. */
	void setInitialExternalImpulse(real p_angle, real p_power);
	
	/*! \brief Sets the emitter properties of a specific emitter of this particle effect.
	    \param p_index [Integer] The index of the emitter of which the settings need to be set.
	    \param p_settings [Table] A table containing all settings (key, value) that need to be changed.
	           Supported settings:<br />
	           particles (real); the number of emitted particles.<br />
	           lifetime (real); the lifetime of the emitted particles.<br />
	           origin (Vector2); the origin of the emitted particles.<br />
	           area_type (string); the area type of this emitter; 'circle' or 'rectangle'. Note: make sure this is set correctly before using rect_width/height or radius.<br />
	           rect_width (real); width of the emission rectangle<br />
	           rect_height (real); width of the emission rectangle<br />
	           inner_radius (real); inner radius of the emission circle<br />
	           radius (real); radius of the emission circle<br />
	           valid_rect (VectorRect); sets the valid area of the emitter. Use 'null' to reset.<br />
	           velocity_x (real); changes the base velocity x of the emitter (not the range).<br />
	           velocity_y (real); changes the base velocity y of the emitter (not the range).<br />
	           start_rotation (real); sets the start rotation (in degrees) of the emitted particles.<br />
	           fps (real); sets the frames per second for all animations in this emitter.*/
	int setEmitterProperties(HSQUIRRELVM p_vm);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	typedef tt::engine::particles::Range<real> RealRange;
	
	// Helper functions for setEmitterProperties
	real getReal(HSQUIRRELVM p_vm, s32 p_index) const;
	RealRange getRealRange(HSQUIRRELVM p_vm, s32 p_index) const;
	tt::math::Vector2 getVector2(HSQUIRRELVM p_vm, s32 p_index) const;
	tt::math::VectorRect getVectorRect(HSQUIRRELVM p_vm, s32 p_index) const;
	std::string getString(HSQUIRRELVM p_vm, s32 p_index) const;
	
	tt::engine::particles::ParticleEffectPtr m_particleEffect;
	SpawnInfo                                m_spawnInfo;
	u32                                      m_flipMask;
	real                                     m_scale;
	bool                                     m_shouldSerializeEmitterSettings;
};


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_PARTICLEEFFECTWRAPPER_H)
