#if !defined(INC_TT_ENGINE_PARTICLES_PARTICLETRIGGER_H)
#define INC_TT_ENGINE_PARTICLES_PARTICLETRIGGER_H


#include <string>
#include <vector>

#include <tt/engine/particles/fwd.h>
#include <tt/engine/particles/ParticleEmitter.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>


namespace tt {
namespace engine {
namespace scene2d {
	class Scene2D;
	class SceneInterface;
}
namespace particles {

// Forward Declarations
class WorldObject;
class Camera;


class ParticleTrigger : public scene2d::Scene2D
{
public:
	struct TriggerSettings
	{
		std::string name; // FIXME: Only needed for display name in editor
		TriggerID id;
		s32 maxParticles;
		bool isParticleCullingEnabled;
		EmitterSettingsCollection emitSettings;
		
		inline TriggerSettings()
		:
		name(),
		id(),
		maxParticles(0),
		isParticleCullingEnabled(true),
		emitSettings()
		{ }
	};
	
	// Different trigger types
	enum TriggerType
	{
		TriggerType_Start,  // Start emitters
		TriggerType_Stop,   // Stop emitters
		TriggerType_Kill,   // Kill all particles
		TriggerType_Reload  // Reloads particle settings (for testing)
	};
	
	
	ParticleTrigger(const WorldObject*       p_followObject, u32 p_category);
	ParticleTrigger(scene2d::SceneInterface* p_scene,        u32 p_category);
	
	~ParticleTrigger();
	
	/*! \brief Returns exact copy of original */
	ParticleTrigger* clone(bool p_keepReferenceObject = true);
	
	/*! \brief Send message to this trigger */
	void trigger(TriggerType p_type);
	
	bool load(const std::string& p_filename, real p_scale = 1.0f, s32 p_renderGroup = -1);
	
	/*! \brief Adds delay to this trigger */
	void addDelay(real p_delay);
	
	/*! \brief Modify origin position */
	void setOrigin(real p_x, real p_y);
	
	/*! \brief Modify origin position */
	void setOrigin(const math::Vector2& p_origin);
	
	/*! \brief Sets a new object for this particle effect to follow (set to null stop following). */
	void setFollowObject(const WorldObject* p_object);
	
	/*! \brief Sets whether the particle trigger has ownership of the follow object (WorldObject).
	           Default is false (no ownership).
	           If trigger has ownership, trigger will delete the follow object when the trigger gets destroyed. */
	void setTriggerHasFollowObjectOwnership(bool p_haveOwnership);
	
	/*! \brief Get status of this trigger */
	inline bool isActive() const { return m_active; }
	
	/*! \brief Update all particle emitters */
	void update(real p_deltaTime);
	
	/*! \brief Update all render resources */
	void updateForRender(const tt::math::VectorRect* p_visibilityRect);
	
	/*! \brief Render all Scene2D particles currently alive */
	// FIXME: Add constness to Scene2D::render
	virtual void render();
	
	/*! \brief Render all non-Scene2D particles currently alive */
	void renderAllGroups() const;
	
	/*! \brief Render all non-Scene2D particles from a specific group currently alive */
	void renderGroup(s32 p_group) const;
	
	/*! \brief Render debug info for all particles */
	void renderDebug() const;
	
	virtual real getHeight() const;
	virtual real getWidth() const;
	
	/*! \brief Retrieve name of this trigger */
	inline const std::string& getName() const { return m_name; }
	inline const TriggerID getID() const { return m_triggerID; }
	
	/*! \brief Change the name of the trigger */
	inline void setName(const std::string& p_triggerName) { m_name = p_triggerName; }
	
	/*! \brief Retrieve number of emitters connected to this trigger */
	s32 getEmitterCount() const {return static_cast<s32>(m_emitters.size());}
	
	/*! \brief Get pointer to emitter */
	ParticleEmitter* getEmitter(s32 p_emitter) {return m_emitters.at(p_emitter);}
	const ParticleEmitter* getEmitter(s32 p_emitter) const {return m_emitters.at(p_emitter);}
	
	/*! \brief Add emitter to this trigger */
	inline void addEmitter(ParticleEmitter* p_emitter) { m_emitters.push_back(p_emitter); }
	
	/*! \brief Remove emitter from this trigger */
	void removeEmitter(ParticleEmitter* p_emitter);
	
	/*! \brief Remove emitter from this trigger */
	void removeEmitterByIndex(s32 p_emitterIndex);
	
	/*! \brief Move emitter up */
	void moveEmitterUp(s32 p_emitterIndex);
	
	/*! \brief Move emitter down */
	void moveEmitterDown(s32 p_emitterIndex);
	
	inline bool belongsToCategory(u32 p_category) const
	{ return (m_category & p_category) == p_category; }
	
	renderer::TextureContainer getAndLoadAllUsedTextures() const;
	
	inline s32 getParticleLimit() const { return m_particleLimit; }
	
	void flip(u32 p_flipMask);
	inline virtual real getScale() const             { return m_scale; }
	inline virtual real getScaleForParticles() const { return m_scale; }
	void setScale(real p_scale);
	
	/*! \brief Applies an impulse to the emitter based on angle and power and external_impulse_weight */
	void setInitialExternalImpulse(real p_angle, real p_power);
	
	inline bool isCullingEnabled() const { return m_isCullingEnabled; }
	void setIsCulled(bool p_isCulled);
	inline const math::VectorRect& getBoundingBox() const { return m_boundingBox; }
	inline bool isSceneNode() const { return m_scene != 0; }
	
#ifndef TT_BUILD_FINAL
	inline const std::string& getFileName() const { return m_fileName; }
#endif
	
private:
	using EmitterCollection = std::vector<ParticleEmitter*>;
	
	ParticleTrigger(const ParticleTrigger& p_rhs);
	
	bool loadFromBinary(const std::string& p_filename, s32 p_renderGroup = -1);
	bool setupTrigger(TriggerSettings& p_settings, s32 p_renderGroup);
	void destroyEmitters();
	
	// Disable assignment
	ParticleTrigger& operator=(const ParticleTrigger&);
	
	bool                     m_active;
	bool                     m_isCullingEnabled;
	bool                     m_haveReferenceObjectOwnership;
	TriggerID                m_triggerID;
	scene2d::SceneInterface* m_scene;
	EmitterCollection        m_emitters; // Collection of emitters that are associated with this trigger
	const WorldObject*       m_referenceObject;
	real                     m_scale;
	u32                      m_category;
	s32                      m_particleLimit;
	math::VectorRect         m_boundingBox;
	std::string              m_name;
	
#ifndef TT_BUILD_FINAL
	std::string m_fileName;
#endif
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_PARTICLES_PARTICLETRIGGER_H)
