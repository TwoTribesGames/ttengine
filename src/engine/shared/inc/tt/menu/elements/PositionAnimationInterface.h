#if !defined(INC_TT_MENU_ELEMENTS_POSITIONANIMATIONINTERFACE_H)
#define INC_TT_MENU_ELEMENTS_POSITIONANIMATIONINTERFACE_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Interface class for position animations. */
class PositionAnimationInterface
{
public:
	PositionAnimationInterface() { }
	virtual ~PositionAnimationInterface() { }
	
	virtual void update() = 0;
	
	virtual void setSpeed(s32 p_speed) = 0;
	virtual s32  getSpeed() const = 0;
	
	virtual s32 getX() const = 0;
	virtual s32 getY() const = 0;
	
	virtual void start() = 0;
	
private:
	// No copying
	PositionAnimationInterface(const PositionAnimationInterface&);
	const PositionAnimationInterface& operator=(const PositionAnimationInterface&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_POSITIONANIMATIONINTERFACE_H)
