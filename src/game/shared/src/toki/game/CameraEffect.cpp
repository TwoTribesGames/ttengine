#include <tt/code/bufferutils.h>

#include <toki/game/CameraEffect.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

CameraEffect::CameraEffect()
:
m_easingPosition(),
m_easingFOV(),
m_easingRotation()
{
}


void CameraEffect::update(real p_deltaTime)
{
	m_easingPosition.update(p_deltaTime);
	m_easingFOV.update(p_deltaTime);
	m_easingRotation.update(p_deltaTime);
}


void CameraEffect::setOffsetInstant(const tt::math::Vector2& p_targetOffset)
{
	m_easingPosition = tt::math::interpolation::Easing<tt::math::Vector2>(
		tt::math::Vector2::zero, p_targetOffset, 0.0f, tt::math::interpolation::EasingType_Invalid);
}


void CameraEffect::setOffset(const tt::math::Vector2& p_targetOffset, real p_transitionTime,
                             tt::math::interpolation::EasingType p_easingType)
{
	const tt::math::Vector2 prevOffset(getActualOffset());
	
	m_easingPosition = tt::math::interpolation::Easing<tt::math::Vector2>(
		prevOffset, p_targetOffset, p_transitionTime, p_easingType);
}


void CameraEffect::setFOVInstant(real p_targetFOV)
{
	m_easingFOV = tt::math::interpolation::Easing<real>(0.0f, p_targetFOV, 0.0f,
		tt::math::interpolation::EasingType_Invalid);
}


void CameraEffect::setFOV(real p_targetFOV, real p_transitionTime,
                          tt::math::interpolation::EasingType p_easingType)
{
	const real prevFOV(getActualFOV());
	
	m_easingFOV = tt::math::interpolation::Easing<real>(
		prevFOV, p_targetFOV, p_transitionTime, p_easingType);
}


void CameraEffect::setRotationInstant(real p_targetRotation)
{
	m_easingRotation = tt::math::interpolation::Easing<real>(0.0f, p_targetRotation, 0.0f,
		tt::math::interpolation::EasingType_Invalid);
}


void CameraEffect::setRotation(real p_targetRotation, real p_transitionTime,
                               tt::math::interpolation::EasingType p_easingType)
{
	const real prevRotation(getActualRotation());
	
	m_easingRotation = tt::math::interpolation::Easing<real>(
		prevRotation, p_targetRotation, p_transitionTime, p_easingType);
}


void CameraEffect::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	bu::putEasing(m_easingPosition, p_context);	// tt::math::interpolation::Easing
	bu::putEasing(m_easingFOV,      p_context);	// tt::math::interpolation::Easing
	bu::putEasing(m_easingRotation, p_context);	// tt::math::interpolation::Easing
}


void CameraEffect::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	*this = CameraEffect();
	
	namespace bu = tt::code::bufferutils;
	m_easingPosition = bu::getEasing<tt::math::Vector2>(p_context);
	m_easingFOV      = bu::getEasing<real             >(p_context);
	m_easingRotation = bu::getEasing<real             >(p_context);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

// Namespace end
}
}
