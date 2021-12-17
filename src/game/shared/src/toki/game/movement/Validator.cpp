#include <toki/game/movement/fwd.h>
#include <toki/game/movement/Validator.h>
#include <toki/game/movement/SurroundingsSurvey.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions


bool Validator::validate(const SurroundingsSurvey& p_survey) const
{
	// One of the must haves must pass if we have mustHaves.
	bool mustHavePassed = false;
	
	for (SurveyResultsContainer::const_iterator it = m_mustHaves.begin(); it != m_mustHaves.end(); ++it)
	{
		if (p_survey.getCheckMask().checkFlags(*it)) // All the flags within a must have must match.
		{
			mustHavePassed = true;
			break; // Found one that's valid. Done.
		}
	}
	
	if (mustHavePassed == false && m_mustHaves.empty() == false)
	{
		return false;
	}
	
	for (SurveyResultsContainer::const_iterator it = m_mustNotHaves.begin(); it != m_mustNotHaves.end(); ++it)
	{
		if (p_survey.getCheckMask().checkFlags((*it)))
		{
			return false;
		}
	}
	
	return true;
}


void Validator::rotateForDown(Direction p_down, bool p_forwardIsLeft)
{
	{
		SurveyResultsContainer rotatedMustHaves;
		for (SurveyResultsContainer::const_iterator it = m_mustHaves.begin(); it != m_mustHaves.end(); ++it)
		{
			rotatedMustHaves.push_back(movement::rotateForDown(*it, p_down, p_forwardIsLeft));
		}
		TT_ASSERT(p_down != Direction_Down || p_forwardIsLeft || m_mustHaves == rotatedMustHaves); // Check that a rotation with down as down hasn't changed anything.
		m_mustHaves = rotatedMustHaves;
	}
	
	{
		SurveyResultsContainer rotatedMustNotHaves;
		for (SurveyResultsContainer::const_iterator it = m_mustNotHaves.begin(); it != m_mustNotHaves.end(); ++it)
		{
			rotatedMustNotHaves.push_back(movement::rotateForDown(*it, p_down, p_forwardIsLeft));
		}
		TT_ASSERT(p_down != Direction_Down || p_forwardIsLeft || m_mustNotHaves == rotatedMustNotHaves); // Check that a rotation with down as down hasn't changed anything.
		m_mustNotHaves = rotatedMustNotHaves;
	}
}




//--------------------------------------------------------------------------------------------------
// Private member functions



// Namespace end
}
}
}
