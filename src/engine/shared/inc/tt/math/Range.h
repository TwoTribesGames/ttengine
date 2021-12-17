#if !defined(INC_TT_MATH_RANGE_H)
#define INC_TT_MATH_RANGE_H

#include <tt/math/Random.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace math {

class Range
{
public:
	explicit Range(real p_minMax = 0.0f);
	Range(real p_min, real p_max);
	
	inline real getMin()   const { return m_min;         }
	inline real getMax()   const { return m_max;         }
	inline real getRange() const { return m_max - m_min; }
	
	void setMinMax(real p_minMax);
	void setMinMax(real p_min, real p_max);
	
	inline real getRandom(tt::math::Random& p_rng) const;
	
private:
	real m_min;
	real m_max;
};


inline Range::Range(real p_minMax)
:
m_min(p_minMax),
m_max(p_minMax)
{}


inline Range::Range(real p_min, real p_max)
:
m_min(p_min),
m_max(p_max)
{
	TT_ASSERT(p_min <= p_max);
}


inline void Range::setMinMax(real p_min, real p_max)
{
	TT_ASSERT(p_min <= p_max);
	m_min = p_min;
	m_max = p_max;
}


inline void Range::setMinMax(real p_minMax)
{
	m_min = p_minMax;
	m_max = p_minMax;
}


inline real Range::getRandom(tt::math::Random& p_rng) const
{
	if (m_max - m_min <= 0)
	{
		return m_min;
	}
	return (p_rng.getNormalizedNext() * (m_max - m_min)) + m_min;
}


inline bool operator==(const Range& p_lhs, const Range& p_rhs)
{
	return p_lhs.getMin() == p_rhs.getMin() &&
	       p_lhs.getMax() == p_rhs.getMax();
};


//namespace end
}
}

#endif // !defined(INC_TT_MATH_RANGE_H)
