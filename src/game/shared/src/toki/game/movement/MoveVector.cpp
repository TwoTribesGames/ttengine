#include <toki/game/entity/Entity.h>
#include <toki/game/movement/MoveVector.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions


MoveVector::MoveVector(const Directions&        p_directions,
                       entity::LocalDir         p_localDir,
                       const tt::math::Vector2& p_speed,
                       const Validator&         p_validator,
                       const Flags&             p_flags,
                       s32                      p_priority,
                       const tt::pres::Tags&    p_animationTags,
                       const std::string&       p_animationName,
                       const std::string&       p_name,
                       const std::string&       p_startCallback,
                       const std::string&       p_endCallback,
                       bool                     p_always)
:
MoveBase(p_directions, p_localDir, p_validator, p_flags, p_priority, p_animationTags, p_animationName,
         p_name, p_startCallback, p_endCallback, p_always),
m_speed(p_speed)
{
}


void MoveVector::startAllPresentationObjects(entity::Entity& p_entity,
                                             entity::movementcontroller::DirectionalMovementController& p_movementController) const
{
	startAllPresentationObjectsImpl(p_entity, p_movementController, false);
}


MoveBasePtr MoveVector::createRotatedLocalDirForDown(Direction p_down, bool p_forwardIsLeft) const
{
	MoveVector* copy = new MoveVector(*this);
	copy->rotateForDown(p_down, p_forwardIsLeft);
	copy->m_speed = entity::applyOrientationToVector2(copy->m_speed, p_down, p_forwardIsLeft);
	
	return MoveBasePtr(copy);
}


//--------------------------------------------------------------------------------------------------
// Private member functions



// Namespace end
}
}
}
