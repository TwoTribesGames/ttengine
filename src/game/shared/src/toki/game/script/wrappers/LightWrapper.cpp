#include <tt/math/math.h>

#include <toki/game/light/Glow.h>
#include <toki/game/light/JitterEffect.h>
#include <toki/game/light/Light.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/script/wrappers/LightWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

LightWrapper::LightWrapper(const light::LightHandle& p_lightHandle)
:
m_lightHandle(p_lightHandle)
{
}


bool LightWrapper::isEnabled() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->isEnabled() : false;
}


void LightWrapper::setEnabled(bool p_enabled)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setEnabled(p_enabled);
	}
}


void LightWrapper::setDirection(real p_angle)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		// Translate 'shape/pres' angles to proper math angles.
		real angle = -tt::math::degToRad(p_angle) + tt::math::halfPi;
		if (angle < 0.0f)
		{
			angle += tt::math::twoPi;
		}
		light->setDirection(angle);
	}
}


bool LightWrapper::affectsEntities() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->affectsEntities() : false;
}


void LightWrapper::setAffectsEntities(bool p_enabled)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setAffectsEntities(p_enabled);
	}
}


void LightWrapper::setTextureRotationSpeed(real p_speedInDegrees)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setTextureRotationSpeed(tt::math::degToRad(p_speedInDegrees));
	}
}


tt::math::Vector2 LightWrapper::getOffset() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->getOffset() : tt::math::Vector2::zero;
}


tt::math::Vector2 LightWrapper::getCurrentOffset() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->getCurrentOffset() : tt::math::Vector2::zero;
}


void LightWrapper::setOffset(const tt::math::Vector2& p_offset, real p_duration)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setOffset(p_offset, p_duration);
	}
}


real LightWrapper::getRadius() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->getRadius() : 0.0f;
}


real LightWrapper::getCurrentRadius() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->getCurrentRadius() : 0.0f;
}


void LightWrapper::setRadius(real p_radius, real p_duration)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setRadius(p_radius, p_duration);
	}
}


void LightWrapper::setSpread(real p_spread, real p_duration)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setSpread(tt::math::degToRad(p_spread), p_duration);
	}
}


real LightWrapper::getSpread() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? tt::math::radToDeg(light->getSpread()) : 0.0f;
}


real LightWrapper::getCurrentSpread() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? tt::math::radToDeg(light->getCurrentSpread()) : 0.0f;
}


real LightWrapper::getStrength() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->getStrength() : 0.0f;
}


real LightWrapper::getCurrentStrength() const
{
	const light::Light* light = m_lightHandle.getPtr();
	return (light != 0) ? light->getCurrentStrength() : 0.0f;
}


void LightWrapper::setStrength(real p_strength, real p_duration)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setStrength(p_strength, p_duration);
	}
}


void LightWrapper::setColor(const tt::engine::renderer::ColorRGBA& p_color)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setColor(p_color);
	}
}


void LightWrapper::setTexture(const std::string p_textureName)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->setTexture(p_textureName);
	}
}


void LightWrapper::createGlow(const std::string& p_imageName, real p_scale,
                              real p_minRadius, real p_maxRadius, real p_fadeRadius)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light->createGlow(p_imageName, p_scale, p_minRadius, p_maxRadius, p_fadeRadius);
	}
}


void LightWrapper::setJitterEffect(real p_scaleFrequency,     real p_scaleAmplitude,
                                   real p_positionXFrequency, real p_positionXAmplitude,
                                   real p_positionYFrequency, real p_positionYAmplitude)
{
	light::Light* light = m_lightHandle.getPtr();
	if (light != 0)
	{
		light::JitterEffectPtr jitterEffectPtr(
			new light::JitterEffect(p_scaleFrequency, p_scaleAmplitude,
			                        p_positionXFrequency, p_positionXAmplitude,
			                        p_positionYFrequency, p_positionYAmplitude));
		light->setJitterEffect(jitterEffectPtr);
	}
}


bool LightWrapper::isLevelDark()
{
	return AppGlobal::getGame()->getLightMgr().isDarkLevel();
}


void LightWrapper::setLevelLightMode(LevelLightMode p_mode)
{
	Game* game = AppGlobal::getGame();
	switch (p_mode)
	{
	case LevelLightMode_Light:
		game->getLightMgr().setDarkLevel(false);
		game->setStopLightRenderingOnSplit(true);
		break;
		
	case LevelLightMode_Dark:
		game->getLightMgr().setDarkLevel(true);
		game->setStopLightRenderingOnSplit(false);
		break;
		
	case LevelLightMode_LightWithNoStopOnSplit:
		game->getLightMgr().setDarkLevel(false);
		game->setStopLightRenderingOnSplit(false);
		break;
		
	case LevelLightMode_DarkWithStopOnSplit:
		game->getLightMgr().setDarkLevel(true);
		game->setStopLightRenderingOnSplit(true);
		break;
		
	default:
		TT_PANIC("Invalid LevelLightMode %d", p_mode);
		break;
	}
}


s32 LightWrapper::getLevelLightAmbient()
{
	return AppGlobal::getGame()->getLightMgr().getLevelLightAmbient();
}


void LightWrapper::setLevelLightAmbient(s32 p_ambient)
{
	AppGlobal::getGame()->getLightMgr().setLevelLightAmbient(p_ambient);
}


std::string LightWrapper::getLevelLightModeName(LevelLightMode p_mode)
{
	switch (p_mode)
	{
	case LevelLightMode_Light:                  return "Light";
	case LevelLightMode_Dark:                   return "Dark";
	case LevelLightMode_LightWithNoStopOnSplit: return "LightWithNoStopOnSplit";
	case LevelLightMode_DarkWithStopOnSplit:    return "DarkWithStopOnSplit";
	default:
		TT_PANIC("LevelLightMode '%d' not implemented");
		break;
	}
	return "invalid";
}


LightWrapper::LevelLightMode LightWrapper::getLevelLightModeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < LevelLightMode_Count; ++i)
	{
		const LevelLightMode mode= static_cast<LevelLightMode>(i);
		if (p_name == getLevelLightModeName(mode))
		{
			return mode;
		}
	}
	
	return LevelLightMode_Invalid;
}


void LightWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_CONSTANT(LevelLightMode_Light);
	TT_SQBIND_CONSTANT(LevelLightMode_Dark);
	TT_SQBIND_CONSTANT(LevelLightMode_LightWithNoStopOnSplit);
	TT_SQBIND_CONSTANT(LevelLightMode_DarkWithStopOnSplit);
	
	TT_SQBIND_INIT_NAME(LightWrapper, "Light");
	TT_SQBIND_METHOD(LightWrapper, isEnabled);
	TT_SQBIND_METHOD(LightWrapper, setEnabled);
	TT_SQBIND_METHOD(LightWrapper, setDirection);
	TT_SQBIND_METHOD(LightWrapper, affectsEntities);
	TT_SQBIND_METHOD(LightWrapper, setAffectsEntities);
	TT_SQBIND_METHOD(LightWrapper, setTextureRotationSpeed);
	TT_SQBIND_METHOD(LightWrapper, getOffset);
	TT_SQBIND_METHOD(LightWrapper, getCurrentOffset);
	TT_SQBIND_METHOD(LightWrapper, setOffset);
	TT_SQBIND_METHOD(LightWrapper, getRadius);
	TT_SQBIND_METHOD(LightWrapper, getCurrentRadius);
	TT_SQBIND_METHOD(LightWrapper, setRadius);
	TT_SQBIND_METHOD(LightWrapper, getSpread);
	TT_SQBIND_METHOD(LightWrapper, getCurrentSpread);
	TT_SQBIND_METHOD(LightWrapper, setSpread);
	TT_SQBIND_METHOD(LightWrapper, getStrength);
	TT_SQBIND_METHOD(LightWrapper, getCurrentStrength);
	TT_SQBIND_METHOD(LightWrapper, setStrength);
	TT_SQBIND_METHOD(LightWrapper, setColor);
	TT_SQBIND_METHOD(LightWrapper, setTexture);
	TT_SQBIND_METHOD(LightWrapper, createGlow);
	TT_SQBIND_METHOD(LightWrapper, setJitterEffect);
	TT_SQBIND_METHOD(LightWrapper, equals);
	TT_SQBIND_METHOD(LightWrapper, getHandleValue);
	
	// Static methods
	TT_SQBIND_STATIC_METHOD(LightWrapper, isLevelDark);
	TT_SQBIND_STATIC_METHOD(LightWrapper, setLevelLightMode);
	TT_SQBIND_STATIC_METHOD(LightWrapper, getLevelLightAmbient);
	TT_SQBIND_STATIC_METHOD(LightWrapper, setLevelLightAmbient);
	
	TT_SQBIND_STATIC_METHOD(LightWrapper, getLevelLightModeName);
	TT_SQBIND_STATIC_METHOD(LightWrapper, getLevelLightModeFromName);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
