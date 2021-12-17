#if !defined(INC_TT_ENGINE_ANIMATION_HERMITEFLOATCONTROLLER_H)
#define INC_TT_ENGINE_ANIMATION_HERMITEFLOATCONTROLLER_H

#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/fs/types.h>
#include <tt/code/ErrorStatus.h>


namespace tt {
namespace engine {
namespace animation {


struct HermiteFloatKey
{
	real time;
	real value;
	real slope;
	
	inline HermiteFloatKey()
	:
	time(0.0f),
	value(0.0f),
	slope(0.0f)
	{ }
};


class HermiteFloatController
{
public:
	HermiteFloatController();
	~HermiteFloatController();

	real getValue(real p_time) const;

	inline real getStartTime() const {return m_keys.empty() ? real(0) : m_keys.front().time;}
	inline real getEndTime()   const {return m_keys.empty() ? real(0) : m_keys.back().time;}

	void load(const fs::FilePtr& p_file, tt::code::ErrorStatus* p_errStatus);

private:
	real m_defaultValue;
	
	typedef std::vector<HermiteFloatKey> KeyContainer;
	KeyContainer m_keys;
};

// Namespace end
}
}
}


#endif // INC_TT_ENGINE_ANIMATION_HERMITEFLOATCONTROLLER_H
