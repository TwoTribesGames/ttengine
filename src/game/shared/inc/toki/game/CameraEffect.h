#if !defined(INC_TOKI_GAME_CAMERAEFFECT_H)
#define INC_TOKI_GAME_CAMERAEFFECT_H

#include <tt/code/fwd.h>
#include <tt/math/interpolation.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>

namespace toki {
namespace game {


class CameraEffect
{
public:
	CameraEffect();
	~CameraEffect() {}
	
	void update(real p_deltaTime);
	
	void setOffsetInstant(const tt::math::Vector2& p_targetOffset);
	void setOffset(const tt::math::Vector2& p_targetOffset, real p_transitionTime,
	               tt::math::interpolation::EasingType p_easingType);
	
	inline const tt::math::Vector2& getActualOffset() const { return m_easingPosition.getValue();    }
	inline const tt::math::Vector2& getTargetOffset() const { return m_easingPosition.getEndValue(); }
	
	void setFOVInstant(real p_targetFOV);
	void setFOV(real p_targetFOV, real p_transitionTime, tt::math::interpolation::EasingType p_easingType);
	
	inline real                     getActualFOV() const { return m_easingFOV.getValue();    }
	inline real                     getTargetFOV() const { return m_easingFOV.getEndValue(); }
	
	void setRotationInstant(real p_targetRotation);
	void setRotation(real p_targetRotation, real p_transitionTime, tt::math::interpolation::EasingType p_easingType);
	
	inline real                     getActualRotation() const { return m_easingRotation.getValue();    }
	inline real                     getTargetRotation() const { return m_easingRotation.getEndValue(); }
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	tt::math::interpolation::Easing<tt::math::Vector2> m_easingPosition;
	tt::math::interpolation::Easing<real             > m_easingFOV;
	tt::math::interpolation::Easing<real             > m_easingRotation;
};

// Namespace end
}
}

#endif  // !defined(INC_TOKI_GAME_CAMERAEFFECT_H)
