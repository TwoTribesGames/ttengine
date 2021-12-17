#include <tt/code/bufferutils.h>

#include <toki/game/entity/effect/FogEffectMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {


//--------------------------------------------------------------------------------------------------
// Public member functions

FogEffectMgr::FogEffectMgr()
:
m_colorEasingType(tt::math::interpolation::EasingType_Invalid),
m_colorTime(0.0f),
m_colorDuration(0.0f),
m_colorBegin(),
m_colorEnd(),
m_defaultColor(),
m_defaultColorSet(false),
m_nearFarEasingType(tt::math::interpolation::EasingType_Invalid),
m_nearFarTime(0.0f),
m_nearFarDuration(0.0f),
m_nearBegin(0.0f),
m_nearEnd(0.0f),
m_farBegin(0.0f),
m_farEnd(0.0f),
m_defaultNear(0.0f),
m_defaultFar(0.0f),
m_defaultNearFarSet(false)
{
}


void FogEffectMgr::update(real p_deltaTime)
{
	if (m_colorDuration > 0.0f)
	{
		m_colorTime += p_deltaTime;
		if (m_colorTime >= m_colorDuration)
		{
			m_colorDuration = 0.0f;
			m_defaultColor  = m_colorEnd;
		}
		else
		{
			const real value = tt::math::interpolation::Easing<real>::getValue(
					0.0f, 1.0f, m_colorTime, m_colorDuration, m_colorEasingType);
			
			using tt::engine::renderer::ColorRGB;
			m_defaultColor = ColorRGB::mix(m_colorBegin, m_colorEnd, value);
		}
	}
	
	if (m_nearFarDuration > 0.0f)
	{
		m_nearFarTime += p_deltaTime;
		if (m_nearFarTime >= m_nearFarDuration)
		{
			m_nearFarDuration = 0.0f;
			m_defaultNear     = m_nearEnd;
			m_defaultFar      = m_farEnd;
		}
		else
		{
			const real delta = tt::math::interpolation::Easing<real>::getValue(
					0.0f, 1.0f, m_nearFarTime, m_nearFarDuration, m_nearFarEasingType);
			
			const real oneMinusDelta = 1.0f - delta;
			
			m_defaultNear = (m_nearBegin * oneMinusDelta) + m_nearEnd * delta;
			m_defaultFar  = (m_farBegin  * oneMinusDelta) + m_farEnd  * delta;
		}
	}
}


void FogEffectMgr::setDefaultColor(const tt::engine::renderer::ColorRGB& p_color, real p_duration,
                                   tt::math::interpolation::EasingType p_easingType)
{
	m_defaultColorSet = true;
	
	if (p_duration <= 0.0f)
	{
		m_defaultColor  = p_color;
		m_colorBegin    = p_color;
		m_colorEnd      = p_color;
		m_colorDuration = 0.0f;
	}
	else
	{
		m_colorEasingType = p_easingType;
		m_colorTime       = 0.0f;
		m_colorDuration   = p_duration;
		m_colorBegin      = m_defaultColor;
		m_colorEnd        = p_color;
	}
}


void FogEffectMgr::setDefaultNearFar(real p_near, real p_far, real p_duration,
                                     tt::math::interpolation::EasingType p_easingType)

{
	m_defaultNearFarSet = true;
	
	if (p_duration <= 0.0f)
	{
		m_defaultNear = p_near;
		m_defaultFar  = p_far;
		m_nearBegin   = p_near;
		m_farBegin    = p_far;
		m_nearEnd     = p_near;
		m_farEnd      = p_far;
		m_nearFarDuration = 0.0f;
	}
	else
	{
		m_nearFarEasingType = p_easingType;
		m_nearFarTime       = 0.0f;
		m_nearFarDuration   = p_duration;
		m_nearBegin         = m_defaultNear;
		m_nearEnd           = p_near;
		m_farBegin          = m_defaultFar;
		m_farEnd            = p_far;
	}
}


void FogEffectMgr::resetSettings()
{
	m_defaultColorSet   = false;
	m_defaultNearFarSet = false;
}


void FogEffectMgr::applySettingsIfNotSet(const tt::engine::renderer::ColorRGB& p_color, real p_near, real p_far)
{
	if (m_defaultColorSet == false)
	{
		m_defaultColor  = p_color;
		m_colorEnd      = m_defaultColor;
		m_colorDuration = 0.0f;
	}
	
	if (m_defaultNearFarSet == false)
	{
		m_defaultNear     = p_near;
		m_defaultFar      = p_far;
		m_nearEnd         = m_defaultNear;
		m_farEnd          = m_defaultFar;
		m_nearFarDuration = 0.0f;
	}
}


void FogEffectMgr::serialize  (tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_defaultColorSet, p_context);
	if (m_defaultColorSet)
	{
		bu::putEnum<u8, tt::math::interpolation::EasingType>(m_colorEasingType, p_context);
		bu::put(m_colorTime    , p_context);
		bu::put(m_colorDuration, p_context);
		bu::put(m_colorBegin   , p_context);
		bu::put(m_colorEnd     , p_context);
		bu::put(m_defaultColor , p_context);
	}
	
	bu::put(m_defaultNearFarSet, p_context);
	if (m_defaultNearFarSet)
	{
		bu::putEnum<u8, tt::math::interpolation::EasingType>(m_nearFarEasingType, p_context);
		bu::put(m_nearFarTime    , p_context);
		bu::put(m_nearFarDuration, p_context);
		bu::put(m_nearBegin      , p_context);
		bu::put(m_nearEnd        , p_context);
		bu::put(m_farBegin       , p_context);
		bu::put(m_farEnd         , p_context);
		bu::put(m_defaultNear    , p_context);
		bu::put(m_defaultFar     , p_context);
	}
}


void FogEffectMgr::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_defaultColorSet = bu::get<bool>(p_context);
	if (m_defaultColorSet)
	{
		m_colorEasingType = bu::getEnum<u8, tt::math::interpolation::EasingType>(p_context);
		m_colorTime       = bu::get<real>(p_context);
		m_colorDuration   = bu::get<real>(p_context);
		m_colorBegin      = bu::get<tt::engine::renderer::ColorRGB>(p_context);
		m_colorEnd        = bu::get<tt::engine::renderer::ColorRGB>(p_context);
		m_defaultColor    = bu::get<tt::engine::renderer::ColorRGB>(p_context);
	}
	
	m_defaultNearFarSet = bu::get<bool>(p_context);
	if (m_defaultNearFarSet)
	{
		m_nearFarEasingType = bu::getEnum<u8, tt::math::interpolation::EasingType>(p_context);
		m_nearFarTime       = bu::get<real>(p_context);
		m_nearFarDuration   = bu::get<real>(p_context);
		m_nearBegin         = bu::get<real>(p_context);
		m_nearEnd           = bu::get<real>(p_context);
		m_farBegin          = bu::get<real>(p_context);
		m_farEnd            = bu::get<real>(p_context);
		m_defaultNear       = bu::get<real>(p_context);
		m_defaultFar        = bu::get<real>(p_context);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
