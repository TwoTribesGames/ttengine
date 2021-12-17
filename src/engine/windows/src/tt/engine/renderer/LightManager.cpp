///////////////////////////////////////////////////////////////////////////////
///
///  File name     : LightManager.cpp
///  Platform      : Windows
///  Project       : Rubiks' Cube
///  Author        : Adrian Brown, M.C. Bouterse (marco@twotribes.com)
///  Company       : Two Tribes (www.twotribes.com)
///  Created On    : 2008-01-14
///  Description   : LightManager Class
///

// Include complement header file
#include <tt/engine/renderer/LightManager.h>

// Include additional files
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene/Light.h>

// namespace definition
namespace tt {
namespace engine {
namespace renderer {

LightManager::LightManager()
:
m_activeLightsFlag(0),
m_fog(false)
{
	// Allocate space for max nr of lights
	m_activeLights.resize(maxActiveLights);

	// Create model representing light source
	if(FAILED(D3DXCreateSphere(DXUTGetD3D9Device(), 0.3f, 8, 8, &m_lightModel, 0)))
	{
		TT_PANIC("Could not create sphere");
	}
}

LightManager::~LightManager()
{
	SAFE_RELEASE(m_lightModel);
}


void LightManager::addLight(scene::Light* p_light)
{
	// Find the first open slot
	for(LightContainer::iterator iter = m_activeLights.begin();
		iter != m_activeLights.end(); ++iter)
	{
		if(*iter == 0)
		{
			*iter = p_light;
			p_light->setIndex(std::distance(m_activeLights.begin(), iter));
			p_light->update();
			return;
		}
	}
}


void LightManager::removeLight(scene::Light* p_light)
{
	// Find the light
	for(LightContainer::iterator iter = m_activeLights.begin();
		iter != m_activeLights.end(); ++iter)
	{
		if(*iter == p_light)
		{
			// Invalidate index
			p_light->setEnabled(false);
			p_light->setIndex(-1);

			// Clear entry
			*iter = 0;
			return;
		}
	}
}

void LightManager::update()
{
	// Clear the current flag
	m_activeLightsFlag = 0;

	// Set position - vector mode
	MatrixStack::getInstance()->setMode(MatrixStack::Mode_PositionVector);
	MatrixStack::getInstance()->setIdentity();

	for(LightContainer::iterator iter = m_activeLights.begin();
		iter !=	m_activeLights.end(); ++iter)
	{
		scene::Light* light = *iter;

		if(light != 0 && light->isEnabled())
		{
			// Update the light
			light->update();
		}
	
		//if (light->isEnabled())
		//{
		//	// Mark this as enabled
		//	s32 index = std::distance(m_activeLights.begin(), iter);
		//	m_activeLightsFlag |= (1 << index);

		//	// TODO: Use hardware lights
		//	
		//}
	}
}

void LightManager::setAmbient(const ColorRGB& p_color)
{
	DXUTGetD3D9Device()->SetRenderState(D3DRS_AMBIENT, 
		                        D3DCOLOR_XRGB(p_color.r, p_color.g, p_color.b));
}

void LightManager::renderLightModel()
{
	D3DMATERIAL9 lightMat;
	ZeroMemory(&lightMat, sizeof(D3DMATERIAL9));

	lightMat.Emissive.a = 1.0f;
	lightMat.Emissive.r = 1.0f;
	lightMat.Emissive.g = 1.0f;
	lightMat.Emissive.b = 1.0f;

	lightMat.Diffuse.a = 1.0f;
	lightMat.Diffuse.r = 1.0f;
	lightMat.Diffuse.g = 1.0f;
	lightMat.Diffuse.b = 1.0f;

	lightMat.Ambient.a = 1.0f;
	lightMat.Ambient.r = 1.0f;
	lightMat.Ambient.g = 1.0f;
	lightMat.Ambient.b = 1.0f;

	DXUTGetD3D9Device()->SetMaterial(&lightMat);

	Renderer::getInstance()->setTexture(TexturePtr());

	MatrixStack::getInstance()->updateWorldMatrix();

	m_lightModel->DrawSubset(0);
}

} // End of namespace renderer
} // End of namespace engine
} // End of namespace tt
