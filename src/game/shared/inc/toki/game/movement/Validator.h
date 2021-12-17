#if !defined(INC_TOKI_GAME_MOVEMENT_VALIDATOR_H)
#define INC_TOKI_GAME_MOVEMENT_VALIDATOR_H


#include <vector>

#include <toki/game/movement/fwd.h>


namespace toki {
namespace game {
namespace movement {


class Validator
{
public:
	bool validate(const SurroundingsSurvey& p_survey) const;
	
	inline Validator() {}
	
	inline void addMustHave(const SurveyResults& p_mustHave)
	{
		m_mustHaves.push_back(p_mustHave);
	}
	
	inline void addMustNotHave(const SurveyResults& p_mustNotHave)
	{
		// If empty they nothing to check.
		if (p_mustNotHave.isEmpty() == false)
		{
			m_mustNotHaves.push_back(p_mustNotHave);
		}
	}
	
	void rotateForDown(Direction p_down, bool p_forwardIsLeft);
	
private:
	typedef std::vector<SurveyResults> SurveyResultsContainer;
	SurveyResultsContainer m_mustHaves;
	SurveyResultsContainer m_mustNotHaves;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_VALIDATOR_H)
