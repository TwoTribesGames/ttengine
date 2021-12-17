#if !defined(INC_TT_ENGINE_ANIM2D_POSITIONANIMATION2D_H)
#define INC_TT_ENGINE_ANIM2D_POSITIONANIMATION2D_H

#include <tt/engine/anim2d/Animation2D.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace anim2d {

class PositionAnimation2D : public Animation2D
{
public:
	enum AnimationType 
	{
		AnimationType_Translation,
		AnimationType_GameTranslation,
		AnimationType_Rotation,
		AnimationType_Scale,
		AnimationType_Particles
	};
	static AnimationType animationTypeFromString(const std::string& p_name);
	static std::string animationTypeToString(const AnimationType& p_animationtype);
	
	PositionAnimation2D();
	virtual ~PositionAnimation2D();
	
	struct Transform
	{
		tt::math::Vector3 translation;
		tt::math::Vector2 scale; // x and y scale
		real              rotation;
		
		Transform()
		:
		translation(math::Vector2::zero),
		scale(math::Vector2::allOne),
		rotation(0.0f)
		{}
		
		math::Matrix44 calcMatrix() const
		{
			const real cosAngle(math::cos(rotation));
			const real sinAngle(math::sin(rotation));
			return math::Matrix44(scale.x *  cosAngle, scale.y * sinAngle, 0.0f          , 0.0f,
			                      scale.x * -sinAngle, scale.y * cosAngle, 0.0f          , 0.0f,
			                      0.0f               , 0.0f              , 1.0f          , 0.0f,
			                      translation.x      , translation.y     , translation.z , 1.0f);
		}
		
		math::Matrix44 multiplyWithMatrix(const math::Matrix44& p_rhs) const
		{
			const real cosAngle(math::cos(rotation));
			const real sinAngle(math::sin(rotation));
			
			return math::Matrix44(scale.x *  cosAngle * p_rhs.m_11 + scale.x * sinAngle * p_rhs.m_21,
			                      scale.x *  cosAngle * p_rhs.m_12 + scale.x * sinAngle * p_rhs.m_22,
			                      scale.x *  cosAngle * p_rhs.m_13 + scale.x * sinAngle * p_rhs.m_23,
			                      scale.x *  cosAngle * p_rhs.m_14 + scale.x * sinAngle * p_rhs.m_24,
			                      
			                      scale.y * -sinAngle * p_rhs.m_11 + scale.y * cosAngle * p_rhs.m_21,
			                      scale.y * -sinAngle * p_rhs.m_12 + scale.y * cosAngle * p_rhs.m_22,
			                      scale.y * -sinAngle * p_rhs.m_13 + scale.y * cosAngle * p_rhs.m_23,
			                      scale.y * -sinAngle * p_rhs.m_14 + scale.y * cosAngle * p_rhs.m_24,
			                      
			                      p_rhs.m_31,
			                      p_rhs.m_32,
			                      p_rhs.m_33,
			                      p_rhs.m_34,
			                      
			                      translation.x * p_rhs.m_11 + translation.y * p_rhs.m_21 + 
			                      translation.z * p_rhs.m_31 + p_rhs.m_41,
			                      translation.x * p_rhs.m_12 + translation.y * p_rhs.m_22 + 
			                      translation.z * p_rhs.m_32 + p_rhs.m_42,
			                      translation.x * p_rhs.m_13 + translation.y * p_rhs.m_23 + 
			                      translation.z * p_rhs.m_33 + p_rhs.m_43,
			                      translation.x * p_rhs.m_14 + translation.y * p_rhs.m_24 + 
			                      translation.z * p_rhs.m_34 + p_rhs.m_44);
		}
	};
	
	/*! \brief Apply the transform for the current state of the animation.
	    \param The transform.*/
	virtual void applyTransform(Transform* p_transform) const = 0;
	
	/*! \brief Returns whether or not the animation has a Z animation.
	    \return Whether or not the animation has a Z animation.*/
	virtual bool hasZAnimation() const = 0;
	
	
	/*! \brief Returns whether or not the animation has a Rotation animation.
	    \return Whether or not the animation has a Rotation animation.*/
	virtual bool hasRotationAnimation() const = 0;
	
	/*! \brief Returns the type of the animation
	    \return AnimationType */ 
	virtual AnimationType getAnimationType() const = 0;
	
	virtual PositionAnimation2D* clone() const = 0;
	
protected:
	//Enable copy / disable assignment
	PositionAnimation2D(const PositionAnimation2D& p_rhs);
	PositionAnimation2D& operator=(const PositionAnimation2D&);
	
private:
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_POSITIONANIMATION2D_H)
