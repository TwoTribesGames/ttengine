#if !defined(INC_TT_PRES_ANIM2D_POSITIONANIMATION2D_H)
#define INC_TT_PRES_ANIM2D_POSITIONANIMATION2D_H

#include <tt/pres/anim2d/Animation2D.h>
#include <tt/pres/anim2d/Transform.h>


namespace tt {
namespace pres {
namespace anim2d {

class PositionAnimation2D : public Animation2D
{
public:
	enum AnimationType 
	{
		AnimationType_Translation,
		AnimationType_GameTranslation,
		AnimationType_Rotation,
		AnimationType_Scale
	};
	static AnimationType animationTypeFromString(const std::string& p_name);
	static std::string animationTypeToString(AnimationType p_animationtype);
	
	PositionAnimation2D();
	virtual ~PositionAnimation2D();
	
	
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

#endif // !defined(INC_TT_PRES_ANIM2D_POSITIONANIMATION2D_H)
