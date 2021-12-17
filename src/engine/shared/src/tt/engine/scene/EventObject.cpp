#include <tt/engine/scene/EventObject.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/scene/Instance.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


EventObject::EventObject() 
: 
SceneObject(Type_EventObject),
m_eventMaterialCount(0),
m_eventMaterials(0)
{
}


EventObject::~EventObject()
{
}


bool EventObject::load(const fs::FilePtr& p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}

	// Now we can load our information
	if (p_file->read(&m_eventMaterialCount, sizeof(m_eventMaterialCount)) != 
		sizeof(m_eventMaterialCount) )
	{
		return false;
	}

	for (s32 c = 0; c < 2; ++c)
	{
		if (m_AABB[c].load(p_file) == false)
		{
			return false;
		}
	}

	// We can now load all the data in for these
	m_eventMaterials = new renderer::Material*[m_eventMaterialCount];
	
	for (s32 i = 0; i < m_eventMaterialCount; ++i)
	{
		EngineID matID(0,0);
		
		// Now we can load our information
		if(matID.load(p_file) == false)
		{
			return false;
		}
		
		// Load the material
		if (matID.valid() == false)
		{
			m_eventMaterials[i] = 0;
		}
		else
		{
			m_eventMaterials[i] = (renderer::MaterialCache::get(
				matID, renderer::Material::Flag_DoNotLoadTexture)).get();
		}
	}
	
	return true;
}


void EventObject::calculateBoundingBox()
{
	TT_PANIC("Not implemented");
	/*
	m_boundingBox.reset();

	// Set all valid
	m_boundingBox.setFlags(physics::BoundingBox::Flag_AABBValid | 
                           physics::BoundingBox::Flag_OOBBValid);

	// Go through all the coordinates
	for (s32 c = 0; c < 8; ++c)
	{
		// Get this corner
		math::Vector3 aabb = getAABB(c) * m_worldMatrix;
		
		// Set the OOBB
		m_boundingBox.setOOBB(c, aabb);
	}

	// We can now calculate the sphere
	m_boundingBox.calculateSphere();
	*/
}


void EventObject::renderObject(renderer::RenderContext&)
{
	// Bounding Box Rendering code should be in Renderer if needed

	// We dont really render an event object
}


// Namespace end
}
}
}

