#if !defined(INC_TT_MATH_TIMEDLINEARINTERPOLATION_H)
#define INC_TT_MATH_TIMEDLINEARINTERPOLATION_H


//#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace math {

/*! \brief Performs simple numerical linear interpolation to a minimum/maximum value. */
template<typename T>
class TimedLinearInterpolation
{
public:
	inline TimedLinearInterpolation()
	:
	m_startValue(),
	m_endValue(),
	m_value(),
	m_time(0.0f),
	m_endTime(0.0f)
	{
		validate();
	}
	
	inline TimedLinearInterpolation(const T& p_value)
	:
	m_startValue(p_value),
	m_endValue(p_value),
	m_value(p_value),
	m_time(0.0f),
	m_endTime(0.0f)
	{
		validate();
	}
	
	inline TimedLinearInterpolation(const T& p_startValue, const T& p_endValue, real p_duration)
	:
	m_startValue(p_startValue),
	m_endValue(p_endValue),
	m_value(p_startValue),
	m_time(0.0f),
	m_endTime(p_duration)
	{
		validate();
	}
	
	inline void startNewInterpolation(const T& p_endValue, real p_duration)
	{
		m_startValue = m_value;
		m_endValue   = p_endValue;
		m_time       = 0.0f;
		m_endTime    = p_duration;
		validate();
		if (isDone())
		{
			m_value = p_endValue;
		}
	}
	
	inline void update(real p_deltaTime)
	{
		if (isDone())
		{
			return;
		}
		
		m_time += p_deltaTime;
		
		if (isDone() || m_endTime == 0.0f)
		{
			m_value = m_endValue;
			m_time  = m_endTime;
		}
		else
		{
			const real normalizedTime = m_time / m_endTime;
			m_value = m_startValue + ((m_endValue - m_startValue) * normalizedTime);
		}
	}
	
	inline const T& getStartValue() const { return m_startValue;        }
	inline const T& getEndValue()   const { return m_endValue;          }
	inline const T& getValue()      const { return m_value;             }
	inline bool     isDone()        const { return m_time >= m_endTime; }
	
	inline real getCurrentTime() const { return m_time;    }
	inline real getDuration()    const { return m_endTime; }
	
	static inline TimedLinearInterpolation createFromData(const T& p_startValue,
	                                                      const T& p_endValue,
	                                                      real     p_time,
	                                                      real     p_duration)
	{
		return TimedLinearInterpolation(p_startValue, p_endValue, p_time, p_duration);
	}
	
private:
	inline TimedLinearInterpolation(const T& p_startValue,
	                                const T& p_endValue,
	                                real     p_time,
	                                real     p_duration)
	:
	m_startValue(p_startValue),
	m_endValue(p_endValue),
	m_value(p_endValue),
	m_time(p_time),
	m_endTime(p_duration)
	{
		validate();
		update(0.0f);
	}
	
	inline void validate()
	{
		//TT_ASSERT(m_startValue <= m_endValue);
		//TT_ASSERT(m_value >= m_startValue && m_value <= m_endValue);
		TT_ASSERT(m_endTime >= 0.0f);
		//clamp(m_value, m_startValue, m_endValue);
	}
	
	
	T     m_startValue;
	T     m_endValue;
	T     m_value;
	real  m_time;
	real  m_endTime;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_TIMEDLINEARINTERPOLATION_H)
