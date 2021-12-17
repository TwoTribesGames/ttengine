#include <toki/game/entity/Entity.h>
#include <toki/game/movement/MoveBase.h>
#include <toki/pres/PresentationObject.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions

bool MoveBase::validate(const SurroundingsSurvey& p_survey, entity::LocalDir p_localDir, 
                        movement::Direction p_down, bool p_forwardIsLeft) const
{
	// None is same as down for our logic.
	if (p_down == movement::Direction_None)
	{
		p_down = movement::Direction_Down;
	}
	
	// The entity orientation down is the same as this moves.
	if (m_orientationDown != p_down)
	{
		return false;
	}
	
	if (hasLocalDir())
	{
		// This is a local dir move
		
		if (m_localDir == entity::LocalDir_None)
		{
			if (m_forwardIsLeft)
			{
				return false;
			}
		}
		else
		{
			// The local moves also have a specific 'flip' which should be the same.
			if (m_forwardIsLeft != p_forwardIsLeft)
			{
				// The local move doesn't the same.
				return false;
			}
		}
		
		if (m_localDir != p_localDir && // The entity's local dir isn't the same as this moves.
		    m_always == false)
		{
			return false;
		}
	}
	
	return m_validator.validate(p_survey);
}


void MoveBase::startPresentationObjectImpl(toki::pres::PresentationObject& p_pres, bool p_forceRestart) const
{
	if ((m_animationTags.empty() == false || m_animationName.empty() == false) &&
	     p_pres.isAffectedByMovement())
	{
		if (p_forceRestart)
		{
			p_pres.stop();
		}
		p_pres.start(m_animationName, m_animationTags, false);
	}
}


void MoveBase::startAllPresentationObjectsImpl(entity::Entity& p_entity,
                                               entity::movementcontroller::DirectionalMovementController& p_movementController, bool p_forceRestart) const
{
	if ((m_animationTags.empty() == false || m_animationName.empty() == false))
	{
		if (p_forceRestart)
		{
			p_entity.stopAllPresentationObjectsForMovement();
		}
		tt::pres::Tags tags(p_movementController.getTags());
		tt::pres::Tags standOnTags(p_entity.getStandOnTags());
		tags.insert(standOnTags.begin(), standOnTags.end());
		tags.insert(m_animationTags.begin(), m_animationTags.end());
		
		p_entity.startAllPresentationObjectsForMovement(m_animationName, tags);
	}
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

void MoveBase::rotateForDown(Direction p_down, bool p_forwardIsLeft)
{
	TT_ASSERT(m_directions.isEmpty());
	TT_ASSERT(hasLocalDir());
	TT_ASSERT(m_forwardIsLeft == false);
	TT_ASSERT(m_orientationDown == movement::Direction_Down);
	m_forwardIsLeft   = p_forwardIsLeft;
	TT_ASSERT(movement::isValidDirection(p_down));
	TT_ASSERT(p_down != movement::Direction_None);
	m_orientationDown = p_down;
	
	m_validator.rotateForDown(p_down, p_forwardIsLeft);
	
	if (p_down == Direction_Right ||
	    p_down == Direction_Left)
	{
		Flags rotated;
		
		// X needs to become Y and vice versa 
		if (m_flags.checkFlag(Flag_StartNeedsXPositionSnap))
		{
			rotated.setFlag(Flag_StartNeedsYPositionSnap);
		}
		if (m_flags.checkFlag(Flag_StartNeedsYPositionSnap))
		{
			rotated.setFlag(Flag_StartNeedsXPositionSnap);
		}
		if (m_flags.checkFlag(Flag_EndNeedsXPositionSnap))
		{
			rotated.setFlag(Flag_EndNeedsYPositionSnap);
		}
		if (m_flags.checkFlag(Flag_EndNeedsYPositionSnap))
		{
			rotated.setFlag(Flag_EndNeedsXPositionSnap);
		}
		
		// Other flags don't need rotation, but do need to stay.
		if (m_flags.checkFlag(Flag_PersistentMove))
		{
			rotated.setFlag(Flag_PersistentMove);
		}
		if (m_flags.checkFlag(Flag_IgnoreCollision))
		{
			rotated.setFlag(Flag_IgnoreCollision);
		}
		
		m_flags = rotated;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions



// Namespace end
}
}
}
