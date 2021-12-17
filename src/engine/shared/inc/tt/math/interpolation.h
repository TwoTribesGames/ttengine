#if !defined(INC_TT_MATH_EASING_H)
#define INC_TT_MATH_EASING_H

#include <tt/math/math.h>
#include <tt/platform/tt_error.h>

namespace tt            /*! */ {
namespace math          /*! */ {
namespace interpolation /*! */ {

/*! \brief EasingType. See http://gizma.com/easing/ for a visual representation of the
           difference between the various easing types */
enum EasingType
{
	EasingType_Linear,           //!< EasingType_Linear
	EasingType_QuadraticIn,      //!< EasingType_QuadraticIn
	EasingType_QuadraticOut,     //!< EasingType_QuadraticOut
	EasingType_QuadraticInOut,   //!< EasingType_QuadraticInOut
	EasingType_CubicIn,          //!< EasingType_CubicIn
	EasingType_CubicOut,         //!< EasingType_CubicOut
	EasingType_CubicInOut,       //!< EasingType_CubicInOut
	EasingType_QuarticIn,        //!< EasingType_QuarticIn
	EasingType_QuarticOut,       //!< EasingType_QuarticOut
	EasingType_QuarticInOut,     //!< EasingType_QuarticInOut
	EasingType_QuinticIn,        //!< EasingType_QuinticIn
	EasingType_QuinticOut,       //!< EasingType_QuinticOut
	EasingType_QuinticInOut,     //!< EasingType_QuinticInOut
	EasingType_SinusoidalIn,     //!< EasingType_SinusoidalIn
	EasingType_SinusoidalOut,    //!< EasingType_SinusoidalOut
	EasingType_SinusoidalInOut,  //!< EasingType_SinusoidalInOut
	EasingType_ExponentialIn,    //!< EasingType_ExponentialIn
	EasingType_ExponentialOut,   //!< EasingType_ExponentialOut
	EasingType_ExponentialInOut, //!< EasingType_ExponentialInOut
	EasingType_CircularIn,       //!< EasingType_CircularIn
	EasingType_CircularOut,      //!< EasingType_CircularOut
	EasingType_CircularInOut,    //!< EasingType_CircularInOut
	EasingType_BackIn,           //!< EasingType_BackIn
	EasingType_BackOut,          //!< EasingType_BackOut
	EasingType_BackInOut,        //!< EasingType_BackInOut
	
	EasingType_Count,
	EasingType_Invalid
};

const char* getEasingTypeName(EasingType p_type);

/*! \brief Returns the EasingType corresponding with its stringname. */
EasingType getEasingTypeFromName(const std::string& p_name);


template <typename Type>
class Easing
{
public:
	Easing()
	:
	m_beginValue(),
	m_endValue(),
	m_duration(0.0f),
	m_time(0.0f),
	m_type(EasingType_Invalid),
	m_difference(),
	m_value()
	{ }
	
	Easing(const Type& p_beginValue, const Type& p_endValue, real p_duration, EasingType p_type)
	:
	m_beginValue(p_beginValue),
	m_endValue(p_endValue),
	m_duration(p_duration),
	m_time(0.0f),
	m_type(p_type),
	m_difference(p_endValue - p_beginValue),
	m_value(p_beginValue)
	{
		TT_ASSERT(m_duration >= 0.0f);
		
		if (m_duration == 0.0f)
		{
			m_value = m_endValue;
		}
	}
	
	static Easing createFromData(const Type& p_beginValue, const Type& p_endValue,
	                             real p_duration, real p_time, EasingType p_type)
	{
		TT_ASSERT(p_time >= 0.0f);
		
		Easing easing(p_beginValue, p_endValue, p_duration, p_type);
		easing.m_time = p_time;
		return easing;
	}
	
	inline const Type& getBeginValue() const { return m_beginValue;  }
	inline const Type& getEndValue()   const { return m_endValue;  }
	inline real        getDuration()    const { return m_duration;    }
	inline EasingType  getType()        const { return m_type;        }
	
	inline const Type& getValue()       const { return m_value;  }
	inline real        getTime()        const { return m_time; }
	
	inline void setType(EasingType p_type)
	{
		TT_ASSERT(p_type < EasingType_Count);
		m_type = p_type;
	}
	
	inline void update(real p_deltaTime)
	{
		if (m_duration == 0.0f)
		{
			return;
		}
		
		m_time += p_deltaTime;
		
		if (m_time >= m_duration)
		{
			m_time = m_duration;
			
			// Stop condition
			m_duration = 0.0f;
			m_value = m_endValue;
		}
		else
		{
			m_value = getValue(m_beginValue, m_difference, m_time, m_duration, m_type);
		}
	}
	
	static Type getValue(const Type& p_beginValue, const Type& p_difference, real p_time,
	                     real p_duration, EasingType p_type)
	{
		const Type& b(p_beginValue);
		const Type& c(p_difference);
		real t = p_time;
		real d = p_duration;
		
		switch (p_type)
		{
		case EasingType_Linear:
			{
				return ((c * t) / d) + b;
			}
			
		case EasingType_QuadraticIn:
			{
				t /= d;
				return c * t * t + b;
			}
			
		case EasingType_QuadraticOut:
			{
				t /= d;
				return -c * t * (t - 2) + b;
			}
			
		case EasingType_QuadraticInOut:
			{
				t /= d / 2;
				if (t < 1)
				{
					return ((c / 2) * (t * t)) + b;
				}
				--t;
				return -c / 2 * (((t - 2) * t) - 1) + b;
			}
			
		case EasingType_CubicIn:
			{
				t /= d;
				return c * t * t * t + b;
			}
			
		case EasingType_CubicOut:
			{
				t = (t / d) - 1;
				return c * (t * t * t + 1) + b;
			}
			
		case EasingType_CubicInOut:
			{
				t /= d / 2;
				if (t < 1)
				{
					return c / 2 * t * t * t + b;
				}
				t -= 2;
				return c / 2 * (t * t * t + 2) + b;
			}
			
		case EasingType_QuarticIn:
			{
				t /= d;
				return c * t * t * t * t + b;
			}
			
		case EasingType_QuarticOut:
			{
				t = (t / d) - 1;
				return -c * (t * t * t * t - 1) + b;
			}
			
		case EasingType_QuarticInOut:
			{
				t /= d / 2;
				if (t < 1)
				{
					return c / 2 * t * t * t * t + b;
				}
				t -= 2;
				return -c / 2 * (t * t * t * t - 2) + b;
			}
			
		case EasingType_QuinticIn:
			{
				t /= d;
				return c * t * t * t * t * t + b;
			}
		
		case EasingType_QuinticOut:
			{
				t = (t / d) - 1;
				return c * (t * t * t * t * t + 1) + b;
			}
			
		case EasingType_QuinticInOut:
			{
				t /= d / 2;
				if (t < 1)
				{
					return c / 2 * t * t * t * t * t + b;
				}
				t -= 2;
				return c / 2 * (t * t * t * t * t + 2) + b;
			}
			
		case EasingType_SinusoidalIn:
			{
				return -c * cos(t / d * (pi / 2)) + c + b;
			}
			
		case EasingType_SinusoidalOut:
			{
				return c * sin(t / d * (pi / 2)) + b;
			}
			
		case EasingType_SinusoidalInOut:
			{
				return -c / 2 * (cos(pi * t / d) - 1) + b;
			}
			
		case EasingType_ExponentialIn:
			{
				return (t == 0) ? b : c * pow(2.0f, 10.0f * (t / d - 1.0f)) + b;
			}
			
		case EasingType_ExponentialOut:
			{
				return (t == d) ? b + c : c * ( -pow(2.0f, -10.0f * t / d) + 1) + b;
			}
			
		case EasingType_ExponentialInOut:
			{
				if (t == 0) return b;
				if (t == d) return b + c;
				t /= d / 2;
				if (t < 1)
				{
					return c / 2 * pow(2.0f, 10.0f * (t - 1)) + b;
				}
				--t;
				return c / 2 * (-pow(2.0f, -10.0f * t) + 2) + b;
			}
			
		case EasingType_CircularIn:
			{
				t /= d;
				return -c * (sqrt(1 - t * t) - 1) + b;
			}
			
		case EasingType_CircularOut:
			{
				t = (t / d) - 1;
				return c * sqrt(1 - t * t) + b;
			}
			
		case EasingType_CircularInOut:
			{
				t /= d / 2;
				if (t < 1)
				{
					return -c / 2 * (sqrt(1 - t * t) - 1) + b;
				}
				t -= 2;
				return c / 2 * (sqrt(1 - t * t) + 1) + b;
			}
			
		case EasingType_BackIn:
			{
				real s = 1.70158f;
				t /= d;
				return c * t * t * ((s + 1) * t - s) + b;
			}
			
		case EasingType_BackOut:
			{
				real s = 1.70158f;
				t = t / d - 1;
				return c * (t * t * ((s + 1) * t + s) + 1) + b;
			}
			
		case EasingType_BackInOut:
			{
				real s = 1.70158f * 1.525f;
				t /= d / 2;
				if (t < 1) return c / 2 * (t * t * ((s + 1) * t - s)) + b;
				t -= 2;
				return c / 2 * (t * t * ((s + 1) * t + s) + 2) + b;
			}
			
		default:
			TT_PANIC("Invalid EasingType '%d'", p_type);
			break;
		}
		
		return b;
	}
	
private:
	// Startup values
	Type       m_beginValue;
	Type       m_endValue;
	real       m_duration;
	real       m_time;
	EasingType m_type;
	
	// Calculated values
	Type       m_difference;
	Type       m_value;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MATH_EASING_H)
