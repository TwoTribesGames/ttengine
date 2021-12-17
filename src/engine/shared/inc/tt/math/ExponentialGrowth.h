#if !defined(INC_TT_MATH_EXPONENTIALGROWTH_H)
#define INC_TT_MATH_EXPONENTIALGROWTH_H

#include <cmath>

#include <tt/platform/tt_types.h>
//#include <tt/platform/tt_error.h>


namespace tt {
namespace math {


/* \brief Helper to make a exponential equation use delta time but define it as a fixed time.
   
   \note How it works (i.e. Here is the math)
   
    Construction needs p_fixedTimeGrowth and the p_fixedTime per frame.
    Fixed deltaTime code would look like this Something *= growth; which game ticks every p_fixedTime.
    
    -----------------------------------------------------------
    Use Exponential growth/decay math to be able to calcuate with delta time.
    N(t) = N(0)*e^(growth * time)
    
    Explanation of terms:
    N(t) = the amount for time t
    N(0) = the starting amount.
    growth is the growth/decay constant
    time is the fixed delta time
    
    -----------------------------------------------------------
    First step - Find the growth constant with our fixed time.
    Amount at time t = p_fixedTimeGrowth.
    We make starting amount 1.
    Time is p_fixedTime.
    so:
    N(t)                  = N(0) * e^(decay * time       )
    p_fixedTimeGrowth     = 1    * e^(decay * p_fixedTime)
    p_fixedTimeGrowth     =        e^(decay * p_fixedTime)
    ln(p_fixedTimeGrowth) =           decay * p_fixedTime
    
    m_growthConstant = ln(p_fixedTimeGrowth) / p_fixedTime
    This is done in the constructor of the class.
    
    -----------------------------------------------------------
    Second step - Get the growth constant with delta time.
    
    So: Use the growth constant with delta time.
    N(t)         = N(0)         * e^(decay * time)
    newSomething = oldSomething * e^(decay * p_time)
    
    in code this is done in the getGrowth function.
    Client code can do:
    Something *= exponentialGrowth.getGrowth(p_deltaTime); */


class ExponentialGrowth
{
public:
	inline ExponentialGrowth(real p_fixedTimeGrowth, real p_fixedTime)
	:
	m_growthConstant(std::log(p_fixedTimeGrowth) / p_fixedTime)
	{
	}
	
	inline real getGrowth(real p_deltaTime) const
	{
		return std::exp(m_growthConstant * p_deltaTime);
	}
	
private:
	real m_growthConstant;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_EXPONENTIALGROWTH_H)
