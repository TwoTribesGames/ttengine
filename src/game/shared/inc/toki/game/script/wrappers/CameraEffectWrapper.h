#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CAMERAEFFECTWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CAMERAEFFECTWRAPPER_H

#include <tt/code/fwd.h>
#include <tt/math/Vector2.h>
#include <tt/script/helpers.h>

#include <toki/game/fwd.h>
#include <toki/game/CameraEffect.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


/*! \brief 'CameraEffect' in Squirrel. */
class CameraEffectWrapper
{
public:
	enum CameraType
	{
		CameraType_Normal,
		CameraType_DRC
	};
	
	CameraEffectWrapper();
	explicit CameraEffectWrapper(const CameraEffectPtr& p_effect, CameraType p_cameraType);
	
	/*! \brief Sets the (target) offset of this effect with easing
	    \param p_offset the relative offset
	    \param p_transitionTime the time it takes to reach that offset
	    \param p_easingType the easing type */
	void setOffset(const tt::math::Vector2& p_offset, real p_transitionTime,
	               tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Sets the (target) offset of this effect instantly (no easing)
	    \param p_offset the relative offset */
	void setOffsetInstant(const tt::math::Vector2& p_offset);
	
	/*! \brief Returns the actual (current) offset of this effect */
	const tt::math::Vector2& getActualOffset() const;
	
	/*! \brief Returns the target offset of this effect */
	const tt::math::Vector2& getTargetOffset() const;
	
	/*! \brief Sets the (target) FOV of this effect with easing
	    \param p_fov the relative FOV
	    \param p_transitionTime the time it takes to reach that FOV
	    \param p_easingType the easing type */
	void setFOV(real p_fov, real p_transitionTime, tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Sets the (target) FOV of this effect instantly (no easing)
	    \param p_fov the relative FOV */
	void setFOVInstant(real p_fov);
	
	/*! \brief Returns the actual (current) FOV of this effect */
	real getActualFOV() const;
	
	/*! \brief Returns the target FOV of this effect */
	real getTargetFOV() const;
	
	/*! \brief Sets the (target) rotation of this effect with easing
	    \param p_rotation the relative rotation (in degrees)
	    \param p_transitionTime the time it takes to reach that FOV
	    \param p_easingType the easing type */
	void setRotation(real p_rotationInDegrees, real p_transitionTime, tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Sets the (target) rotation of this effect instantly (no easing)
	    \param p_fov the relative rotation (in degrees) */
	void setRotationInstant(real p_rotationInDegrees);
	
	/*! \brief Returns the actual (current) rotation of this effect */
	real getActualRotation() const;
	
	/*! \brief Returns the target rotation of this effect */
	real getTargetRotation() const;
	
	void serialize(tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext* p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	CameraEffectPtr m_effect;
	CameraType      m_cameraType;
};

// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CAMERAEFFECTWRAPPER_H)
