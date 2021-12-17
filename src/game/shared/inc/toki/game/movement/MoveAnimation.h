#if !defined(INC_TOKI_GAME_MOVEMENT_MOVEANIMATION_H)
#define INC_TOKI_GAME_MOVEMENT_MOVEANIMATION_H

#include <vector>

#include <tt/math/Point2.h>

#include <toki/game/movement/fwd.h>
#include <toki/game/movement/MoveBase.h>


namespace toki {
namespace game {
namespace movement {


class MoveAnimation : public MoveBase
{
public:
	MoveAnimation(const Directions& p_directions, entity::LocalDir p_localDir,
	              const SubSteps& p_subSteps, real p_animationTimeLength,
	              const Validator& p_validator, const Flags& p_flags,
	              s32 p_priority, const tt::pres::Tags& p_animationTags,
	              const std::string& p_animationName,
	              const std::string& p_name,
	              const std::string& p_startCallback,
	              const std::string& p_endCallback,
	              bool               p_always);
	virtual ~MoveAnimation() { }
	
	virtual const tt::math::Vector2& getSpeed() const { return tt::math::Vector2::zero; }
	virtual void startAllPresentationObjects(entity::Entity& p_entity,
                                             entity::movementcontroller::DirectionalMovementController& p_movementController) const;
	virtual inline bool isAnimation() const { return true; }
	virtual const MoveAnimation* getMoveAnimation() const { return this; }
	
	virtual MoveBasePtr createRotatedLocalDirForDown(Direction p_down, bool p_forwardIsLeft = false) const;
	
	const tt::math::Point2& getStep()       const { return m_step;       }
	const SubSteps&         getSubSteps()   const { return m_subSteps;   }
	real                    getTimeLength() const { return m_timeLength; }
	
private:
	tt::math::Point2 m_step; // Where you'll move to (relativly) after this move. (In tiles.)
	SubSteps m_subSteps;
	real     m_timeLength;
	
	const MoveAnimation& operator=(const MoveAnimation&); // Disable assigment
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_MOVEANIMATION_H)
