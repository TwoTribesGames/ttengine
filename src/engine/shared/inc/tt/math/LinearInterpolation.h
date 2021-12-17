#if !defined(INC_TT_MATH_LINEARINTERPOLATION_H)
#define INC_TT_MATH_LINEARINTERPOLATION_H


#include <tt/math/math.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace math {

/*! \brief Performs simple numerical linear interpolation to a minimum/maximum value. */
template<typename T>
class LinearInterpolation
{
public:
	enum State
	{
		State_None,
		State_ToMax,
		State_ToMin
	};
	
	
	LinearInterpolation()
	:
	m_state(State_None),
	m_minValue(0),
	m_maxValue(100),
	m_value(0),
	m_delta(1)
	{
		validate();
	}
	
	LinearInterpolation(T p_minValue, T p_maxValue, T p_initialValue, T p_delta)
	:
	m_state(State_None),
	m_minValue(p_minValue),
	m_maxValue(p_maxValue),
	m_value(p_initialValue),
	m_delta(p_delta)
	{
		validate();
	}
	
	inline void setState(State p_state) { m_state = p_state; }
	inline void setDelta(T p_delta)
	{
		TT_ASSERT(p_delta > 0);
		m_delta = p_delta;
	}
	inline void setMinValue(T p_value) { m_minValue = p_value; }
	inline void setMaxValue(T p_value) { m_maxValue = p_value; }
	
	inline void interpolateToMin(bool p_resetValue = false)
	{
		m_state = State_ToMin;
		if (p_resetValue) m_value = m_maxValue;
	}
	
	inline void interpolateToMax(bool p_resetValue = false)
	{
		m_state = State_ToMax;
		if (p_resetValue) m_value = m_minValue;
	}
	
	inline void forceToMin() { m_state = State_None; m_value = m_minValue; }
	inline void forceToMax() { m_state = State_None; m_value = m_maxValue; }
	
	inline void update()
	{
		switch (m_state)
		{
		case State_ToMax:
			m_value += m_delta;
			if (m_value >= m_maxValue)
			{
				m_state = State_None;
			}
			clamp(m_value, m_minValue, m_maxValue);
			break;
			
		case State_ToMin:
			m_value -= m_delta;
			if (m_value <= m_minValue)
			{
				m_state = State_None;
			}
			clamp(m_value, m_minValue, m_maxValue);
			break;
			
		default: break;
		}
	}
	
	inline State getState()    const { return m_state;               }
	inline T     getMinValue() const { return m_minValue;            }
	inline T     getMaxValue() const { return m_maxValue;            }
	inline T     getValue()    const { return m_value;               }
	inline T     getDelta()    const { return m_delta;               }
	inline bool  isDone()      const { return m_state == State_None; }
	
private:
	inline void validate()
	{
		TT_ASSERT(m_minValue <= m_maxValue);
		TT_ASSERT(m_value >= m_minValue && m_value <= m_maxValue);
		TT_ASSERT(m_delta > 0);
		clamp(m_value, m_minValue, m_maxValue);
	}
	
	
	State m_state;
	T     m_minValue;
	T     m_maxValue;
	T     m_value;
	T     m_delta;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_LINEARINTERPOLATION_H)
