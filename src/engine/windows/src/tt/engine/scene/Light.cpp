///////////////////////////////////////////////////////////////////////////////
///
///  File name     : Light.cpp
///  Platform      : Shared
///  Project       : Rubiks' Cube
///  Author        : Adrian Brown, M.C. Bouterse (marco@twotribes.com)
///  Company       : Two Tribes (www.twotribes.com)
///  Created On    : 2008-01-22
///  Description   : Light Class
///

// Include complement header file
#include <tt/engine/scene/Light.h>

// include additional files
#include <tt/engine/file/File.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>

// namespace definition
namespace tt {
namespace engine {
namespace scene {


Light::Light() 
: 
SceneObject(Type_Light),
m_color(255,255,255),
m_direction(0,-1,0),
m_range(100),
m_attenuation(1,0,0),
m_type(Type_Point),
m_enabled(false),
m_index(-1)
{
	updateD3DStruct();
}

Light::~Light()
{
}


bool Light::load(file::File* p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}
	return true;
}


void Light::update(Instance*, animation::Animation*)
{
	if (m_instance != 0)
	{
		m_instance->render();
		m_direction = m_instance->getActualPosition() - getActualPosition();
	}
	
	// Make sure the direction is normalised
	m_direction.normalize();

	if(m_index != -1)
	{
		// DEBUG - Updating every frame
		updateD3DStruct();

		//TT_Printf("Light {%f, %f, %f}\n", getPosition().x, getPosition().y, getPosition().z);

		DXUTGetD3D9Device()->SetLight(m_index, &m_light);
		DXUTGetD3D9Device()->LightEnable(m_index, TRUE);
	}
}

void Light::setEnabled(bool p_enabled)
{
	if(p_enabled && m_index != -1)
	{
		// Enable the light
		DXUTGetD3D9Device()->LightEnable(m_index, TRUE);
	}
	else if(m_index != -1)
	{
		DXUTGetD3D9Device()->LightEnable(m_index, FALSE);
	}
	m_enabled = p_enabled;
}


void Light::renderObject(Instance*)
{
	// We dont really render a light

	using renderer::MatrixStack;
	using renderer::Renderer;

	MatrixStack::getInstance()->push();
	MatrixStack::getInstance()->translate(getPosition());
	Renderer::getInstance()->getLightManager().renderLightModel();
	MatrixStack::getInstance()->pop();
}


/////////////////////////
// Private

void Light::updateD3DStruct()
{
	// Clear light structure
	ZeroMemory(&m_light, sizeof(D3DLIGHT9));

	// Type of light
	m_light.Type = static_cast<D3DLIGHTTYPE>(m_type);

	// Diffuse & Specular color
	m_light.Specular.a = 1.0f;
	m_light.Specular.r = m_light.Diffuse.r = m_color.r / 255.0f;
	m_light.Specular.g = m_light.Diffuse.g = m_color.g / 255.0f;
	m_light.Specular.b = m_light.Diffuse.b = m_color.b / 255.0f;

	if(m_type == Type_Point || m_type == Type_Spot)
	{
		// Set position
		m_light.Position.x = getPosition().x;
		m_light.Position.y = getPosition().y;
		m_light.Position.z = getPosition().z;

		// Set attenuation
		m_light.Range = m_range;
		m_light.Attenuation0 = m_attenuation.x;
		m_light.Attenuation1 = m_attenuation.y;
		m_light.Attenuation2 = m_attenuation.z;
	}
	if(m_type == Type_Directional || m_type == Type_Spot)
	{
		// Set direction
		m_light.Direction.x = m_direction.x;
		m_light.Direction.y = m_direction.y;
		m_light.Direction.z = m_direction.z;
	}
	if(m_type == Type_Spot)
	{
		// Not supported yet
		m_light.Falloff = 1.0f;
		m_light.Theta = 0.0f;
		m_light.Phi = 1.0f;
	}
}

} // End of namespace scene
} // End of namespace engine
} // End of namespace tt

