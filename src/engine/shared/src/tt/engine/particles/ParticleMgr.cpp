#include <algorithm>

#include <tt/code/helpers.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/scene2d/SceneInterface.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/fs/utils/utils.h>
#include <tt/str/str.h>
#include <tt/thread/ThreadedWorkload.h>

namespace tt {
namespace engine {
namespace particles {

#define USE_THREADING 1

// Unique instance of particle manager
ParticleMgr* ParticleMgr::ms_instance = 0;


//--------------------------------------------------------------------------------------------------
// Public member functions

void ParticleMgr::createInstance()
{
	TT_ASSERT(ms_instance == 0);
	ms_instance = new ParticleMgr();
}


void ParticleMgr::destroyInstance()
{
	TT_NULL_ASSERT(ms_instance);
	
	delete ms_instance;
	ms_instance = 0;
}


ParticleTrigger* ParticleMgr::addTrigger(const std::string& p_filename,
                                         const WorldObject* p_object,
                                         real               p_scale,
                                         u32                p_category,
                                         s32                p_renderGroup)
{
	// Try to construct a new particle trigger object
	ParticleTrigger* trig = new ParticleTrigger(p_object, p_category);
	
	// Load the settings from XML or Binary
	if (trig->load(p_filename, p_scale, p_renderGroup) == false)
	{
		delete trig;
		TT_PANIC("Creating particle effect '%s' failed. Particle effect file may not exist.",
		         p_filename.c_str());
		return 0;
	}
	
	// Add trigger to collection
	m_triggers.push_back(trig);
	
#if !defined(TT_BUILD_FINAL)
	m_debugTriggerNames[trig] = trig->getFileName();
#endif
	
	if (trig->getParticleLimit() != -1)
	{
		// settings.name was m_name.
		setParticleLimit(trig->getName(), trig->getParticleLimit());
	}
	
	// Return a pointer to it
	return trig;
}


ParticleTrigger* ParticleMgr::addTriggerToScene(const std::string&                   p_filename,
                                                tt::engine::scene2d::SceneInterface* p_scene,
                                                real                                 p_scale,
                                                u32                                  p_category,
                                                s32                                  p_renderGroup)
{
	TT_NULL_ASSERT(p_scene);
	
	// Try to construct a new particle trigger object
	ParticleTrigger* trig = new ParticleTrigger(p_scene, p_category);
	
	// Load the settings from XML or Binary
	if (trig->load(p_filename, p_scale, p_renderGroup) == false)
	{
		delete trig;
		TT_PANIC("Creating particle effect '%s' failed. Particle effect file may not exist.",
		         p_filename.c_str());
		return 0;
	}
	
	// Add trigger to collection
	m_sceneTriggers.push_back(trig);
	
#if !defined(TT_BUILD_FINAL)
	m_debugTriggerNames[trig] = trig->getFileName();
#endif
	
	if (trig->getParticleLimit() != -1)
	{
		// settings.name was m_name.
		setParticleLimit(trig->getName(), trig->getParticleLimit());
	}
	
	// Return a pointer to it
	return trig;
}


void ParticleMgr::removeTrigger(ParticleTrigger* p_trigger)
{
	TT_NULL_ASSERT(p_trigger);
	TriggerCollection& triggers(p_trigger->isSceneNode() ? m_sceneTriggers : m_triggers);
	
	// Remove the specified trigger from the collection
	TriggerCollection::iterator trig = std::find(triggers.begin(), triggers.end(), p_trigger);
	if (trig != triggers.end())
	{
		// Free Memory for trigger
		delete (*trig);
		
		// Remove from collection
		//tt::code::helpers::unorderedErase(triggers, trig);
		triggers.erase(trig);
		
		return;
	}

#if !defined(TT_BUILD_FINAL)
	// If we receive an unrecognized pointer, things can go horribly wrong in final builds
	// if there are new triggers loaded (possibly re-using the stale pointers)
	if (triggers.empty() == false)
	{
		TriggerNameCollection& names(p_trigger->isSceneNode() ? m_debugSceneTriggerNames : m_debugTriggerNames);
		std::string name("<unknown>");
		TriggerNameCollection::const_iterator it = names.find(p_trigger);
		if (it != names.end())
		{
			name = it->second;
		}
		
		TT_PANIC("Trigger pointer '%s' is outdated, fix removal order", name.c_str());
	}
#endif
}


void ParticleMgr::update(real p_deltaTime, u32 p_category)
{
	// Update all active particle triggers
	{
#if USE_THREADING
		tt::thread::ThreadedWorkload work(m_triggers.size(),
			std::bind(&ParticleMgr::updateTrigger, this, std::placeholders::_1, p_deltaTime, p_category));
		work.startAndWaitForCompletion();
#else
		TriggerCollection::iterator end = m_triggers.end();
		for (TriggerCollection::iterator it = m_triggers.begin(); it != end; ++it)
		{
			TT_ASSERT((*it)->isSceneNode() == false);
			if ((*it)->belongsToCategory(p_category) && (*it)->isActive())
			{
				(*it)->update(p_deltaTime);
			}
		}
#endif
	}
	
	// Clean up spawned effects
	for (SpawnedEffects::iterator it(m_effects.begin()); it != m_effects.end();)
	{
		// Check if oneshot effect is still active
		if ((*it).first == SpawnType_OneShot && ((*it).second->isActive() == false || (*it).second->isCulled()))
		{
			it = code::helpers::unorderedErase(m_effects, it);
		}
		else
		{
			++it;
		}
	}
}


void ParticleMgr::updateForRender(const tt::math::VectorRect* p_visibilityRect)
{
#if USE_THREADING
	tt::thread::ThreadedWorkload work(m_triggers.size(),
		std::bind(&ParticleMgr::updateTriggerForRender, this, std::placeholders::_1, p_visibilityRect));
	work.startAndWaitForCompletion();
#else
	TriggerCollection::const_iterator end = m_triggers.end();
	for (TriggerCollection::const_iterator it = m_triggers.begin(); it != end; ++it)
	{
		TT_ASSERT((*it)->isSceneNode() == false);
		if ((*it)->isActive())
		{
			(*it)->updateForRender(p_visibilityRect);
		}
	}
#endif
}


void ParticleMgr::renderAllGroups() const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Particles");
	
	// Render all active particles
	{
		TriggerCollection::const_iterator end = m_triggers.end();
		for (TriggerCollection::const_iterator it = m_triggers.begin(); it != end; ++it)
		{
			TT_ASSERT((*it)->isSceneNode() == false);
			if ((*it)->isActive())
			{
				(*it)->renderAllGroups();
			}
		}
	}
	
	// Restore texture transform to play nice with other elements that are not
	// using it...
	renderer::MatrixStack::getInstance()->resetTextureMatrix();
	renderer::Renderer::getInstance()->setBlendMode(renderer::BlendMode_Blend);
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void ParticleMgr::renderGroup(s32 p_group) const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Particles");
	
	// Render all active particles
	{
		TriggerCollection::const_iterator end = m_triggers.end();
		for (TriggerCollection::const_iterator it = m_triggers.begin(); it != end; ++it)
		{
			const ParticleTrigger* pt = *it;
			TT_ASSERT(pt->isSceneNode() == false);
			if (pt->isActive())
			{
				pt->renderGroup(p_group);
			}
		}
	}
	
	// Restore texture transform to play nice with other elements that are not
	// using it...
	renderer::MatrixStack::getInstance()->resetTextureMatrix();
	renderer::Renderer::getInstance()->setBlendMode(renderer::BlendMode_Blend);
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void ParticleMgr::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	// FIXME MR: Careful; when calling this method, no other thread should access the sceneTriggers
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Particles Debug");
	
	if (m_particleCullingEnabled == false)
	{
		return;
	}
	
	// Render all active particles
	for (auto& it : m_triggers)
	{
		it->renderDebug();
	}
	for (auto& it : m_sceneTriggers)
	{
		it->renderDebug();
	}
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
#endif
}


void ParticleMgr::reset()
{
	// FIXME MR: Careful; when calling this method, no other thread should access the sceneTriggers
	m_effects.clear();
	{
		for (auto& it : m_triggers)
		{
			delete it;
		}
		for (auto& it : m_sceneTriggers)
		{
			delete it;
		}
	}
	m_triggers.clear();
	m_sceneTriggers.clear();
	m_limits.clear();
	
#if !defined(TT_BUILD_FINAL)
	// Martijn: Don't clear the names because an outdated trigger pointer outdated can survive a reset
	//m_debugTriggerNames.clear();
	//m_debugSceneTriggerNames.clear();
#endif
}


void ParticleMgr::setParticleLimit(const std::string& p_triggerName, s32 p_limit, bool p_alwaysSet)
{
	TriggerID triggerID(p_triggerName);
	ParticleLimits::iterator it = m_limits.find(triggerID);
	
	if (it == m_limits.end())
	{
		// Not found: insert new one
		m_limits.insert(ParticleLimits::value_type(triggerID, p_limit));
	}
	else if (p_limit <= 0)
	{
		// Remove limit
		m_limits.erase(it);
	}
	else
	{
		// Already have this event name
		if (p_alwaysSet)
		{
			it->second = p_limit;
		}
		// Check if the new limit is lower
		else if ((*it).second > p_limit)
		{
			TT_WARN("Different limit for same game event name (%s). Found: %d, using new/lowest: %d.",
					p_triggerName.c_str(), (*it).second, p_limit);
			(*it).second = p_limit;
		}
	}
}


s32 ParticleMgr::getParticleLimit(const std::string& p_triggerName) const
{
	ParticleLimits::const_iterator it = m_limits.find(TriggerID(p_triggerName));

	return (it == m_limits.end()) ? -1 : it->second;
}


ParticleEffectPtr ParticleMgr::createEffect(const std::string& p_filename,
                                            const math::Vector3&   p_position,
                                            u32                    p_category,
                                            real                   p_scale,
                                            s32                    p_renderGroup) const
{
	return ParticleEffect::create(p_filename, p_position, 0, p_scale, p_category, false, p_renderGroup);
}


ParticleEffectPtr ParticleMgr::createEffect(const std::string&                    p_filename,
                                            const engine::particles::WorldObject* p_object,
                                            u32                                   p_category,
                                            real                                  p_scale,
                                            s32                                   p_renderGroup) const
{
	return ParticleEffect::create(p_filename, tt::math::Vector3::zero,
	                              p_object, p_scale, p_category, false, p_renderGroup);
}


ParticleEffectPtr ParticleMgr::createEffectInScene(const std::string&                        p_filename,
                                                   scene2d::SceneInterface*                  p_scene,
                                                   const math::Vector3&                      p_position,
                                                   const scene2d::shoebox::PlaneFollowerPtr& p_planeFollower,
                                                   real                                      p_scale,
                                                   u32                                       p_category,
                                                   s32                                       p_renderGroup)
{
	return ParticleEffect::createInScene(p_filename, p_scene, p_position, p_planeFollower, p_scale, p_category, p_renderGroup);
}


void ParticleMgr::spawnEffect(SpawnType p_type, const ParticleEffectPtr& p_effect)
{
	addEffect(p_type, p_effect);
	p_effect->spawn();
}


void ParticleMgr::setFixedTimestepForRenderGroup(s32 p_group, real p_fixedTimestep)
{
	TT_ASSERT(p_fixedTimestep > 0.0f);
	m_fixedTimestepSettings[p_group] = p_fixedTimestep;
}


real ParticleMgr::getFixedTimestepForRenderGroup(s32 p_group) const
{
	FixedTimestepSettings::const_iterator it = m_fixedTimestepSettings.find(p_group);
	return it == m_fixedTimestepSettings.end() ? 0.0f : it->second;
}


#if !defined(TT_BUILD_FINAL)
u32 ParticleMgr::getTriggerCount() const
{
	// FIXME: MR: This is a debug function that can crash if the scene inserts/erases a particle effect during this call
	return static_cast<u32>(m_triggers.size()) + static_cast<u32>(m_sceneTriggers.size());
}


u32 ParticleMgr::getEmitterCount() const
{
	// FIXME: MR: This is a debug function that can crash if the scene inserts/erases a particle effect during this call
	u32 result = 0;
	for (auto& it : m_triggers)
	{
		result += it->getEmitterCount();
	}
	for (auto& it : m_sceneTriggers)
	{
		result += it->getEmitterCount();
	}
	return result;
}


u32 ParticleMgr::getParticleCount() const
{
	// FIXME: MR: This is a debug function that can crash if the scene inserts/erases a particle effect during this call
	u32 result = 0;
	for (auto& it : m_triggers)
	{
		const s32 emitterCount(it->getEmitterCount());
		for (s32 i = 0; i < emitterCount; ++i)
		{
			result += it->getEmitter(i)->getParticleCount();
		}
	}
	for (auto& it : m_sceneTriggers)
	{
		const s32 emitterCount(it->getEmitterCount());
		for (s32 i = 0; i < emitterCount; ++i)
		{
			result += it->getEmitter(i)->getParticleCount();
		}
	}
	return result;
}


u32 ParticleMgr::getNonCulledTriggerCount() const
{
	// FIXME: MR: This is a debug function that can crash if the scene inserts/erases a particle effect during this call
	u32 result = 0;
	for (auto& it : m_triggers)
	{
		if (it->isCulled())
		{
			result += it->getEmitterCount();
		}
	}
	for (auto& it : m_sceneTriggers)
	{
		if (it->isCulled())
		{
			result += it->getEmitterCount();
		}
	}
	return result;
}


u32 ParticleMgr::getNonCulledEmitterCount() const
{
	// FIXME: MR: This is a debug function that can crash if the scene inserts/erases a particle effect during this call
	u32 result = 0;
	for (auto& it : m_triggers)
	{
		if (it->isCulled() == false)
		{
			result += it->getEmitterCount();
		}
	}
	for (auto& it : m_sceneTriggers)
	{
		if (it->isCulled() == false)
		{
			result += it->getEmitterCount();
		}
	}
	return result;
}


u32 ParticleMgr::getNonCulledParticleCount() const
{
	u32 result = 0;
	TriggerCollection::const_iterator end = m_triggers.end();
	for (TriggerCollection::const_iterator it = m_triggers.begin(); it != end; ++it)
	{
		if ((*it)->isCulled() == false) 
		{
			const s32 emitterCount((*it)->getEmitterCount());
			for (s32 i = 0; i < emitterCount; ++i)
			{
				result += (*it)->getEmitter(i)->getParticleCount();
			}
		}
	}
	return result;
}


std::string ParticleMgr::getDebugParticleNames() const
{
	typedef std::map<std::string, s32> EffectMap;
	std::string result;
	{
		EffectMap effects;
		s32 total = 0;
		{
			for (TriggerCollection::const_iterator it = m_triggers.begin(); it != m_triggers.end(); ++it)
			{
				if ((*it)->isCulled() == false) 
				{
					std::pair<EffectMap::iterator, bool> const& r =
						effects.insert(std::make_pair((*it)->getFileName(), 1));
					if (r.second == false)
					{
						++r.first->second;
					}
					++total;
				}
			}
		}
		
		char buf[256];
		sprintf(buf, "Active Particle Triggers (%d):\n", total);
		result += buf;
		for (EffectMap::const_iterator it = effects.begin(); it != effects.end(); ++it)
		{
			sprintf(buf, "%5d %s\n", it->second, it->first.c_str());
			result += buf;
		}
	}
	result += "\n\n\n";
	{
		EffectMap effects;
		s32 total = 0;
		{
			for (TriggerCollection::const_iterator it = m_sceneTriggers.begin(); it != m_sceneTriggers.end(); ++it)
			{
				if ((*it)->isCulled() == false) 
				{
					std::pair<EffectMap::iterator, bool> const& r =
						effects.insert(std::make_pair((*it)->getFileName(), 1));
					if (r.second == false)
					{
						++r.first->second;
					}
					++total;
				}
			}
		}
		
		char buf[256];
		sprintf(buf, "Active Scene Particle Triggers (%d):\n", total);
		result += buf;
		for (EffectMap::const_iterator it = effects.begin(); it != effects.end(); ++it)
		{
			sprintf(buf, "%5d %s\n", it->second, it->first.c_str());
			result += buf;
		}
	}
	
	return result;
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

ParticleMgr::ParticleMgr()
:
m_triggerSettingsCache(),
m_triggers(),
m_sceneTriggers(),
m_debugTriggerNames(),
m_debugSceneTriggerNames(),
m_limits(),
m_texturePath(),
m_useTriggerCaching(false),
m_hudGroup(-1),
m_particleCullingEnabled(true),
m_effects(),
m_fixedTimestepSettings(),
m_particleLock()
{
}

ParticleMgr::~ParticleMgr()
{
	reset();
}


void ParticleMgr::addTriggerSettingsToCache(const std::string& p_name,
                                            const ParticleTrigger::TriggerSettings& p_settings)
{
	m_triggerSettingsCache.insert(std::make_pair(p_name, p_settings));
}


ParticleTrigger::TriggerSettings* ParticleMgr::getTriggerSettingsFromCache(const std::string& p_name)
{
	TriggerSettingsCache::iterator it(m_triggerSettingsCache.find(p_name));
	if (it != m_triggerSettingsCache.end())
	{
		return &(*it).second;
	}
	else
	{
		return 0;
	}
}


s32 ParticleMgr::requestParticles(TriggerID p_triggerID, s32 p_particleCount)
{
	tt::thread::CriticalSection guard(&m_particleLock);

	ParticleLimits::iterator it = m_limits.find(p_triggerID);

	if(it == m_limits.end())
	{
		// Not limited
		return p_particleCount;
	}
	else
	{
		if((*it).second >= p_particleCount)
		{
			// Decrease budget
			(*it).second -= p_particleCount;
			return p_particleCount;
		}
		else
		{
			// Not enough left
			s32 particlesLeft = (*it).second;
			(*it).second = 0;
			return particlesLeft;
		}
	}
}


void ParticleMgr::releaseParticles(TriggerID p_triggerID, s32 p_particleCount)
{
	tt::thread::CriticalSection guard(&m_particleLock);

	ParticleLimits::iterator it = m_limits.find(p_triggerID);

	if(it != m_limits.end())
	{
		// Increase budget again
		(*it).second += p_particleCount;
	}
}


void ParticleMgr::addClonedTrigger(ParticleTrigger* p_clone, ParticleTrigger* p_original)
{
	TT_NULL_ASSERT(p_clone);
	TT_NULL_ASSERT(p_original);
	
	TriggerCollection& triggers(p_clone->isSceneNode() ? m_sceneTriggers : m_triggers);
	
	triggers.push_back(p_clone);
	
#if !defined(TT_BUILD_FINAL)
	TriggerNameCollection& names(p_clone->isSceneNode() ? m_debugSceneTriggerNames : m_debugTriggerNames);
	names[p_clone] = p_original->getFileName() + " [CLONE]";
#endif
}


void ParticleMgr::addEffect(SpawnType p_type, const ParticleEffectPtr& p_effect)
{
	m_effects.emplace_back(p_type, p_effect);
}


//--------------------------------------------------------------------------------------------------
// Threading related methods

void ParticleMgr::updateTrigger(size_t p_triggerIndex, real p_deltaTime, u32 p_category)
{
	TT_ASSERT(p_triggerIndex < m_triggers.size());
	
	ParticleTrigger* trigger = m_triggers[p_triggerIndex];
	TT_ASSERT(trigger->isSceneNode() == false);
	if (trigger->belongsToCategory(p_category) && trigger->isActive())
	{
		trigger->update(p_deltaTime);
	}
}


void ParticleMgr::updateTriggerForRender(size_t p_triggerIndex, const tt::math::VectorRect* p_visibilityRect)
{
	TT_ASSERT(p_triggerIndex < m_triggers.size());
	
	ParticleTrigger* trigger = m_triggers[p_triggerIndex];
	TT_ASSERT(trigger->isSceneNode() == false);
	if (trigger->isActive())
	{
		trigger->updateForRender(p_visibilityRect);
	}
}


// Namespace end
}
}
}
