#include <toki/game/entity/Entity.h>
#include <toki/game/movement/MoveAnimation.h>
#include <toki/level/helpers.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions


MoveAnimation::MoveAnimation(const Directions& p_directions, entity::LocalDir p_localDir,
                             const SubSteps& p_subSteps, real p_animationTimeLength,
                             const Validator& p_validator, const Flags& p_flags,
                             s32 p_priority, const tt::pres::Tags& p_animationTags,
                             const std::string& p_animationName,
                             const std::string& p_name,
                             const std::string& p_startCallback,
                             const std::string& p_endCallback,
                             bool               p_always)
:
MoveBase(p_directions, p_localDir, p_validator, p_flags, p_priority, p_animationTags, p_animationName,
         p_name, p_startCallback, p_endCallback, p_always),
m_subSteps(p_subSteps),
m_timeLength(p_animationTimeLength)
{
	if (m_timeLength < 0.0f)
	{
		TT_PANIC("Can't have an animation with a negative time: %d", m_timeLength);
		m_timeLength = 0.0f;
	}
	
	// Where does this move end.
	for (SubSteps::const_iterator it = m_subSteps.begin(); it != m_subSteps.end(); ++it)
	{
		// FIXME: m_step should be value in file format. (The substeps are a temp. feature.)
		//        All supsteps should have the value as m_step is. So this can be a DEV-only assert.
		m_step += (*it);
	}
}


void MoveAnimation::startAllPresentationObjects(entity::Entity& p_entity,
                                                entity::movementcontroller::DirectionalMovementController& p_movementController) const
{
	startAllPresentationObjectsImpl(p_entity, p_movementController, true);
}


MoveBasePtr MoveAnimation::createRotatedLocalDirForDown(Direction p_down, bool p_forwardIsLeft) const
{
	MoveAnimation* copy(new MoveAnimation(*this));
	copy->rotateForDown(p_down, p_forwardIsLeft);
	copy->m_step = entity::applyOrientationToPoint2(copy->m_step, p_down, p_forwardIsLeft);
	
	copy->m_subSteps.clear();
	for (SubSteps::const_iterator it = m_subSteps.begin(); it != m_subSteps.end(); ++it)
	{
		copy->m_subSteps.push_back(entity::applyOrientationToPoint2(*it, p_down, p_forwardIsLeft));
	}
	
	return MoveBasePtr(copy);
}



//--------------------------------------------------------------------------------------------------
// Private member functions



// Namespace end
}
}
}
