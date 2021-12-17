#include <tt/engine/scene/Scene.h>
#include <tt/engine/scene/Fog.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/renderer/RenderContext.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/fs/File.h>
#include <tt/code/helpers.h>


namespace tt {
namespace engine {
namespace scene {


Scene::Scene(const EngineID& p_id, u32) 
: 
m_id(p_id),
m_fogTable(),
m_lights(),
m_cameras(),
m_instances()
{
}


Scene::~Scene()
{
	code::helpers::freeContainer(m_fogTable);
	code::helpers::freeContainer(m_lights);
	code::helpers::freeContainer(m_cameras);
}


Scene* Scene::create(const fs::FilePtr&, const EngineID& p_id, u32 p_flags)
{
	return new Scene(p_id, p_flags);
}


bool Scene::load(const fs::FilePtr& p_file)
{
	s32 fogCount(0);
	if(p_file->read(&fogCount, sizeof(fogCount)) != sizeof(fogCount))
	{
		return false;
	}

	m_fogTable.reserve(static_cast<FogTable::size_type>(fogCount));
	for(s32 i = 0; i < fogCount; ++i)
	{
		FogPtr fogEntry(new Fog);
		if(fogEntry->load(p_file) == false)
		{
			return false;
		}
		m_fogTable.push_back(fogEntry);
	}

	return true;
}


void Scene::update()
{
	for(Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		// Render
		(*it)->update();
	}
}


void Scene::render()
{
	// Create render context
	renderer::RenderContext renderContext(this);

	for(Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		// Render
		(*it)->render(&renderContext);
	}
}


void Scene::setFog(s32 p_fogIndex)
{
	// Find & set fog by reference
	for(FogTable::iterator it = m_fogTable.begin(); it != m_fogTable.end(); ++it)
	{
		if((*it)->getReference() == p_fogIndex)
		{
			renderer::Renderer::getInstance()->setFog(*it);
			return;
		}
	}

	// Not found -- disable fog
	renderer::Renderer::getInstance()->setFog(FogPtr());
}


// Namespace end
}
}
}

