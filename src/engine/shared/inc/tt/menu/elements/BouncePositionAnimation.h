#if !defined(INC_TT_MENU_ELEMENTS_BOUNCEPOSITIONANIMATION_H)
#define INC_TT_MENU_ELEMENTS_BOUNCEPOSITIONANIMATION_H


#include <tt/menu/elements/PositionAnimationInterface.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Bounce animation. */
class BouncePositionAnimation : public PositionAnimationInterface
{
public:
	BouncePositionAnimation();
	virtual ~BouncePositionAnimation();
	
	virtual void update();
	
	virtual void setSpeed(s32 p_speed);
	virtual s32  getSpeed() const;
	
	virtual s32 getX() const;
	virtual s32 getY() const;
	
	virtual void start();
	
private:
	// No copying
	BouncePositionAnimation(const BouncePositionAnimation&);
	const BouncePositionAnimation& operator=(const BouncePositionAnimation&);
	
	
	s32  m_speed;
	real m_delta;
	real m_frame;
	real m_size;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_BOUNCEPOSITIONANIMATION_H)
