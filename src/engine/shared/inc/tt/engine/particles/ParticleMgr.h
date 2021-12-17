#if !defined(INC_TT_ENGINE_PARTICLES_ParticleMgr_H)
#define INC_TT_ENGINE_PARTICLES_ParticleMgr_H

#include <list>
#include <map>

#include <tt/engine/particles/fwd.h>
#include <tt/engine/particles/ParticleEmitter.h>
#include <tt/engine/particles/ParticleTrigger.h>
#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/platform/tt_error.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/thread.h>


namespace tt {
namespace engine {
namespace scene2d {

class SceneInterface;
class Scene2D;

}
namespace particles {

// Constants
enum
{
	Category_All = 0xFFFFFFFF
};


enum SpawnType
{
	SpawnType_OneShot,
	SpawnType_Continuous
};

// Class definition
class ParticleMgr
{
public:
	// Instance management
	static void createInstance();
	static void destroyInstance();
	
	inline static ParticleMgr* getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	
	inline static bool hasInstance()
	{
		return ms_instance != 0;
	}
	
	inline void setTriggerCachingEnabled(bool p_enabled) { m_useTriggerCaching = p_enabled; }
	inline bool isTriggerCachingEnabled() const { return m_useTriggerCaching; }
	inline void clearTriggerCache() { m_triggerSettingsCache.clear(); }
	
	/*! \brief Add a trigger to the collection that is managed by this class.
	    \param p_category Bitmask of the categories the trigger should be part of. */
	ParticleTrigger* addTrigger(const std::string& p_filename,
	                            const WorldObject* p_object      = 0,
	                            real               p_scale       = 1.0f,
	                            u32                p_category    = Category_All, 
	                            s32                p_renderGroup = -1);
	
	/*! \param p_category Bitmask of the categories the trigger should be part of. */
	ParticleTrigger* addTriggerToScene(const std::string&                   p_filename,
	                                   tt::engine::scene2d::SceneInterface* p_scene,
	                                   real                                 p_scale       = 1.0f,
	                                   u32                                  p_category    = Category_All, 
	                                   s32                                  p_renderGroup = -1);
	
	/*! \brief Removes the trigger */
	void removeTrigger(ParticleTrigger* p_trigger);
	
	/*! \brief Update all active particle triggers */
	void update(real p_deltaTime, u32 p_category = Category_All);
	
	/*! \brief Create all resources needed for rendering */
	void updateForRender(const tt::math::VectorRect* p_visibilityRect);
	
	/*! \brief Render all non-Scene2D particles currently alive */
	void renderAllGroups() const;
	
	/*! \brief Render all non-Scene2D particles from a specificed group that are currently alive */
	void renderGroup(s32 p_group) const;
	
	/*! \brief Render debug info */
	void renderDebug() const;
	
	/*! \brief Remove all triggers from the manager, releasing their memory */
	void reset();
	
	
	inline void setTexturePath(const std::string& p_path)
	{
		m_texturePath = p_path;
	}
	inline const std::string& getTexturePath() const { return m_texturePath; }
	
	inline void setHudRenderGroup(s16 p_group) { m_hudGroup = p_group; }
	
	/*! \brief Set a limit for a specific particle game event */
	void setParticleLimit(const std::string& p_triggerName, s32 p_limit, bool p_alwaysSet = false);
	
	/*! \brief Get the particle limit for a specific game event */
	s32 getParticleLimit(const std::string& p_triggerName) const;
	
	// Effects
	/*! \param p_renderGroup If not -1, overwrite the particle effect's render group with the specified value. */
	ParticleEffectPtr createEffect(const std::string&   p_filename,
	                               const math::Vector3& p_position,
	                               u32                  p_category = Category_All,
	                               real                 p_scale = 1.0f,
	                               s32                  p_renderGroup = -1) const;
	
	/*! \param p_renderGroup If not -1, overwrite the particle effect's render group with the specified value. */
	ParticleEffectPtr createEffect(const std::string&            p_filename,
	                               const particles::WorldObject* p_object,
	                               u32                           p_category = Category_All,
	                               real                          p_scale = 1.0f,
	                               s32                           p_renderGroup = -1) const;
	
	ParticleEffectPtr createEffectInScene(const std::string&                        p_filename,
	                                      scene2d::SceneInterface*                  p_scene,
	                                      const math::Vector3&                      p_position, // Ignored when p_planeFollower is passed.
	                                      const scene2d::shoebox::PlaneFollowerPtr& p_planeFollower = scene2d::shoebox::PlaneFollowerPtr(),
	                                      real                                      p_scale         = 1.0f,
	                                      u32                                       p_category      = Category_All,
	                                      s32                                       p_renderGroup   = -1);
	
	void spawnEffect(SpawnType p_type, const ParticleEffectPtr& p_effect);
	
	inline void toggleParticleCulling() { m_particleCullingEnabled = m_particleCullingEnabled == false; }
	inline void setParticleCullingEnabled(bool p_enabled) { m_particleCullingEnabled = p_enabled; }
	inline bool isParticleCullingEnabled() const { return m_particleCullingEnabled; }
	
	// Sets fixed timestep for a specific rendergroup. Useful if game deltatime varies (slomo effect)
	// but for instance HUD particles need to be updated constantly.
	void setFixedTimestepForRenderGroup(s32 p_group, real p_fixedTimestep);
	real getFixedTimestepForRenderGroup(s32 p_group) const; // returns 0 if no fixed timestep was found
	
#if !defined(TT_BUILD_FINAL)
	u32 getTriggerCount() const;
	u32 getEmitterCount() const;
	u32 getParticleCount() const;
	
	u32 getNonCulledTriggerCount() const;
	u32 getNonCulledEmitterCount() const;
	u32 getNonCulledParticleCount() const;
	
	std::string getDebugParticleNames() const;
#endif
	
private:
	typedef std::map<std::string, ParticleTrigger::TriggerSettings> TriggerSettingsCache;
	TriggerSettingsCache m_triggerSettingsCache;
	typedef std::map<s32, real> FixedTimestepSettings;
	
	// Constructor -- only create through createInstance() function
	ParticleMgr();
	~ParticleMgr();
	
	// No copying
	ParticleMgr(const ParticleMgr&);
	ParticleMgr& operator=(const ParticleMgr&);
	
	void addTriggerSettingsToCache(const std::string& p_name,
		const ParticleTrigger::TriggerSettings& p_settings);
	
	ParticleTrigger::TriggerSettings* getTriggerSettingsFromCache(const std::string& p_name);
	
	/*! \brief Request a number of particles for a certain game event
	    \return Number of particles that may be spawned (<= requested count) */
	s32 requestParticles(TriggerID p_triggerID, s32 p_particleCount);
	
	/*! \brief Inform that a number of particles is no longer being used  */
	void releaseParticles(TriggerID p_triggerID, s32 p_particleCount);
	
	void addClonedTrigger(ParticleTrigger* p_clone, ParticleTrigger* p_original);
	
	// Merely adds an effect to the tracking container, without starting it
	void addEffect(SpawnType p_type, const ParticleEffectPtr& p_effect);
	
	// Unique instance of renderer
	static ParticleMgr* ms_instance;
	
	// Collection of triggers managed by the Particle Manager
	using TriggerCollection = std::vector<ParticleTrigger*>;
	TriggerCollection m_triggers;
	TriggerCollection m_sceneTriggers;
	
	using TriggerNameCollection = std::map<ParticleTrigger*, std::string>;
	TriggerNameCollection m_debugTriggerNames;
	TriggerNameCollection m_debugSceneTriggerNames;
	
	// Global limiting for specific particle events
	using ParticleLimits = std::map<TriggerID, s32>;
	ParticleLimits m_limits;
	
	// Root path to search for textures
	std::string m_texturePath;
	
	// Caches the triggers
	bool m_useTriggerCaching;
	
	s16 m_hudGroup;
	
	bool m_particleCullingEnabled;
	
	/*! \brief Effect containers */
	typedef std::pair<SpawnType, ParticleEffectPtr> SpawnedEffect;
	typedef std::vector<SpawnedEffect> SpawnedEffects;
	SpawnedEffects m_effects;
	
	FixedTimestepSettings m_fixedTimestepSettings;
	
	// Threading related methods and members
	tt::thread::Mutex m_particleLock;
	void updateTrigger(size_t p_triggerIndex, real p_deltaTime, u32 p_category);
	void updateTriggerForRender(size_t p_triggerIndex, const tt::math::VectorRect* p_visibilityRect);
	
	friend class ParticleEmitter;
	friend class ParticleEffect;
	friend class ParticleTrigger;
};

// Namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_PARTICLES_ParticleMgr_H)
