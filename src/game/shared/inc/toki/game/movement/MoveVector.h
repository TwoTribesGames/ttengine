#if !defined(INC_TOKI_GAME_MOVEMENT_MOVEVECTOR_H)
#define INC_TOKI_GAME_MOVEMENT_MOVEVECTOR_H

#include <tt/math/Vector2.h>

#include <toki/game/movement/MoveBase.h>
#include <toki/game/movement/fwd.h>


namespace toki {
namespace game {
namespace movement {


class MoveVector : public MoveBase
{
public:
	MoveVector(const Directions& p_directions, entity::LocalDir p_localDir,
	           const tt::math::Vector2& p_speed, const Validator& p_validator, 
	           const Flags& p_flags, s32 p_priority, 
	           const tt::pres::Tags& p_animationTags,
	           const std::string& p_animationName,
	           const std::string& p_name,
	           const std::string& p_startCallback,
	           const std::string& p_endCallback,
	           bool               p_always);
	virtual ~MoveVector() { }
	
	virtual const tt::math::Vector2& getSpeed() const { return m_speed; }
	virtual void startAllPresentationObjects(entity::Entity& p_entity,
                                             entity::movementcontroller::DirectionalMovementController& p_movementController) const;
	
	virtual MoveBasePtr createRotatedLocalDirForDown(Direction p_down, bool p_forwardIsLeft = false) const;
	
private:
	tt::math::Vector2 m_speed;
	
	const MoveVector& operator=(const MoveVector&); // Disable assigment
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_MOVEVECTOR_H)
