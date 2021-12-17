#include <tt/code/bufferutils.h>

#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/script/wrappers/CameraEffectWrapper.h>
#include <toki/game/Game.h>
#include <toki/game/CameraEffect.h>
#include <toki/game/Camera.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

CameraEffectWrapper::CameraEffectWrapper()
:
m_effect(),
m_cameraType(CameraType_Normal)
{
}


CameraEffectWrapper::CameraEffectWrapper(const CameraEffectPtr& p_effect, CameraType p_cameraType)
:
m_effect(p_effect),
m_cameraType(p_cameraType)
{
	TT_NULL_ASSERT(m_effect);
}


void CameraEffectWrapper::setOffsetInstant(const tt::math::Vector2& p_offset)
{
	if (m_effect != 0)
	{
		m_effect->setOffsetInstant(p_offset);
	}
}


void CameraEffectWrapper::setOffset(const tt::math::Vector2& p_offset, real p_transitionTime,
                                    tt::math::interpolation::EasingType p_easingType)
{
	if (m_effect != 0)
	{
		m_effect->setOffset(p_offset, p_transitionTime, p_easingType);
	}
}


const tt::math::Vector2& CameraEffectWrapper::getActualOffset() const
{
	if (m_effect != 0)
	{
		return m_effect->getActualOffset();
	}
	
	return tt::math::Vector2::zero;
}


const tt::math::Vector2& CameraEffectWrapper::getTargetOffset() const
{
	if (m_effect != 0)
	{
		return m_effect->getTargetOffset();
	}
	
	return tt::math::Vector2::zero;
}


void CameraEffectWrapper::setFOVInstant(real p_fov)
{
	if (m_effect != 0)
	{
		m_effect->setFOVInstant(p_fov);
	}
}


void CameraEffectWrapper::setFOV(real p_fov, real p_transitionTime, tt::math::interpolation::EasingType p_easingType)
{
	if (m_effect != 0)
	{
		m_effect->setFOV(p_fov, p_transitionTime, p_easingType);
	}
}


real CameraEffectWrapper::getActualFOV() const
{
	if (m_effect != 0)
	{
		return m_effect->getActualFOV();
	}
	
	return 0.0f;
}


real CameraEffectWrapper::getTargetFOV() const
{
	if (m_effect != 0)
	{
		return m_effect->getTargetFOV();
	}
	
	return 0.0f;
}


void CameraEffectWrapper::setRotationInstant(real p_rotationInDegrees)
{
	if (m_effect != 0)
	{
		m_effect->setRotationInstant(tt::math::degToRad(p_rotationInDegrees));
	}
}


void CameraEffectWrapper::setRotation(real p_rotationInDegrees, real p_transitionTime, tt::math::interpolation::EasingType p_easingType)
{
	if (m_effect != 0)
	{
		m_effect->setRotation(tt::math::degToRad(p_rotationInDegrees), p_transitionTime, p_easingType);
	}
}


real CameraEffectWrapper::getActualRotation() const
{
	if (m_effect != 0)
	{
		return m_effect->getActualRotation();
	}
	
	return 0.0f;
}


real CameraEffectWrapper::getTargetRotation() const
{
	if (m_effect != 0)
	{
		return m_effect->getTargetRotation();
	}
	
	return 0.0f;
}


void CameraEffectWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	
	const bool wrapperValid = (m_effect != 0);
	bu::put(wrapperValid, p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to save
		return;
	}
	
	bu::putEnum<u8>(m_cameraType, p_context);
	
	m_effect->serialize(p_context);
}


void CameraEffectWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	*this = CameraEffectWrapper();
	
	// Load the effect details
	const bool wrapperValid = bu::get<bool>(p_context);
	
	if (wrapperValid == false)
	{
		// Nothing more to load
		return;
	}
	
	m_cameraType = bu::getEnum<u8, CameraType>(p_context);
	game::Game* game = AppGlobal::getGame();
	TT_NULL_ASSERT(game);
	
	switch (m_cameraType)
	{
	case CameraType_Normal:
		m_effect = game->getCamera().createCameraEffect();
		break;
		
	case CameraType_DRC:
		m_effect = game->getDrcCamera().createCameraEffect();
		break;
		
	default:
		TT_PANIC("Unknown camera type %d", m_cameraType);
		return;
	}
	
	m_effect->unserialize(p_context);
}


void CameraEffectWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(CameraEffectWrapper, "CameraEffect");
	TT_SQBIND_METHOD(CameraEffectWrapper, setOffset);
	TT_SQBIND_METHOD(CameraEffectWrapper, setOffsetInstant);
	TT_SQBIND_METHOD(CameraEffectWrapper, getActualOffset);
	TT_SQBIND_METHOD(CameraEffectWrapper, getTargetOffset);
	TT_SQBIND_METHOD(CameraEffectWrapper, setFOV);
	TT_SQBIND_METHOD(CameraEffectWrapper, setFOVInstant);
	TT_SQBIND_METHOD(CameraEffectWrapper, getActualFOV);
	TT_SQBIND_METHOD(CameraEffectWrapper, getTargetFOV);
	TT_SQBIND_METHOD(CameraEffectWrapper, setRotation);
	TT_SQBIND_METHOD(CameraEffectWrapper, setRotationInstant);
	TT_SQBIND_METHOD(CameraEffectWrapper, getActualRotation);
	TT_SQBIND_METHOD(CameraEffectWrapper, getTargetRotation);
}


// Namespace end
}
}
}
}
