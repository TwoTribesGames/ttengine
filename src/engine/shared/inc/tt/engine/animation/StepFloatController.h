#if !defined(INC_TT_ENGINE_ANIMATION_STEPFLOATCONTROLLER_H)
#define INC_TT_ENGINE_ANIMATION_STEPFLOATCONTROLLER_H

#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/fs/types.h>
#include <tt/code/ErrorStatus.h>


namespace tt {
namespace engine {
namespace animation {


struct StepFloatKey
{
	real time;
	real value;
	
	inline StepFloatKey()
	:
	time(0.0f),
	value(0.0f)
	{ }
};


class StepFloatController
{
public:
	StepFloatController();
	~StepFloatController();
	
	real getValue(real p_time) const;
	
	inline real getStartTime() const {return m_keys.front().time;}
	inline real getEndTime()   const {return m_keys.back().time;}
	
	void load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus);
	
private:
	typedef std::vector<StepFloatKey> KeyContainer;
	KeyContainer m_keys;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_ANIMATION_STEPFLOATCONTROLLER_H
