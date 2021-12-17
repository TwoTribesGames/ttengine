#include <tt/engine/renderer/LightManager.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/scene/Light.h>


namespace tt {
namespace engine {
namespace renderer {


LightManager::LightManager()
{
	for (s32 i = 0; i < maxActiveLights; ++i)
	{
		m_activeLights[i] = 0;
	}
}


LightManager::~LightManager()
{
	for (s32 i = 0; i < maxActiveLights; ++i)
	{
		if (m_activeLights[i] != 0)
		{
			m_activeLights[i]->setIndex(-1);
		}
	}
}


bool LightManager::addLight(scene::Light* p_light)
{
	// Find the first open slot
	for (s32 i = 0; i < maxActiveLights; ++i)
	{
		if(m_activeLights[i] == 0)
		{
			// Get index in vector
			m_activeLights[i] = p_light;
			
			// Pass to hw light
			p_light->setIndex(i);
			
			return true;
		}
	}
	
	// No free position found
	TT_PANIC("No free light index found");
	return false;
}


bool LightManager::removeLight(scene::Light* p_light)
{
	// Find the light
	for (s32 i = 0; i < maxActiveLights; ++i)
	{
		if(m_activeLights[i] == p_light)
		{
			// Invalidate index
			p_light->setIndex(-1);
			
			// Clear entry
			m_activeLights[i] = 0;
			
			return true;
		}
	}
	
	TT_WARN("Light not found");
	return false;
}


void LightManager::reset()
{
	for (s32 i = 0; i < maxActiveLights; ++i)
	{
		// Restore all active lights
		if(m_activeLights[i] != 0)
		{
			m_activeLights[i]->setIndex(i);
		}
	}
}


}
}
}
