#include <cfloat>

#include <tt/engine/scene/Light.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


Light::Light() 
: 
m_color(renderer::ColorRGB::white),
m_direction(0,0,0),
m_position(0,0,0),
m_distanceAttenuation(0,0,0),
m_type(LightType_Point),
m_enabled(false),
m_index(-1)
{
	static const float maxRange = std::sqrt(FLT_MAX);
	m_properties.setRange(maxRange);
	setDistanceAttenuation();
}


Light::~Light()
{
	setEnabled(false);
}


void Light::update()
{
	if (m_enabled)
	{
		m_properties.setColor(m_color);
		
		if (m_type == LightType_Point)
		{
			m_properties.setPosition(m_position);
		}
		else
		{
			m_properties.setDirection(m_direction);
		}
		
		renderer::FixedFunction::setLight(m_index, m_properties);
	}
}


void Light::debugRender() const
{
#ifndef TT_BUILD_FINAL
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	
	if (getType() == LightType_Point || getType() == LightType_Spot)
	{
		math::Vector3 halfSize(math::Vector3::allOne);
		renderer->getDebug()->renderSolidAABox(getColor(), m_position + halfSize, m_position - halfSize);
	}
	else
	{
		renderer->getDebug()->renderLine(getColor(), math::Vector3::zero, m_direction * 25);
	}
#endif
}


void Light::setDistanceAttenuation(const math::Vector3& p_attenuation)
{
	setDistanceAttenuation(p_attenuation.x, p_attenuation.y, p_attenuation.z);
}


void Light::setDistanceAttenuation(real p_constant, real p_linear, real p_quadratic)
{
	TT_WARNING(getType() == LightType_Point || getType() == LightType_Spot,
		"Distance attenuation has no effect on directional lights");

	m_distanceAttenuation.setValues(p_constant, p_linear, p_quadratic);
	m_properties.setAttenuation(m_distanceAttenuation);
}


void Light::setType(LightType p_type)
{
	TT_ASSERTMSG(isValidLightType(p_type), "Not a valid light type: %d\n", p_type);
	
	if(p_type != m_type)
	{
		TT_ASSERTMSG(m_type != LightType_Spot, "Spot light is not yet supported.");
		m_type = p_type;
	}
}


void Light::setEnabled(bool p_enabled)
{
	if (m_enabled != p_enabled)
	{
		renderer::LightManager& lightMgr = renderer::Renderer::getInstance()->getLightManager();
		
		p_enabled ? lightMgr.addLight(this) : lightMgr.removeLight(this);
		m_enabled = p_enabled;
	}
}


void Light::setIndex(s32 p_index)
{
	if (p_index == -1)
	{
		renderer::FixedFunction::setLightEnabled(m_index, false);
		m_enabled = false;
		m_index = -1;
	}
	else
	{
		m_index = p_index;
		renderer::FixedFunction::setLightEnabled(m_index, true);
	}
}


}
}
}
