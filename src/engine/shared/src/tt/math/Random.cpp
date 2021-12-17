#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <tt/math/Random.h>
#include <tt/system/Time.h>


namespace tt {
namespace math {

//#define DEBUG_RANDOM

#ifdef DEBUG_RANDOM
	#define Rand_Printf TT_Printf
#else
	#define Rand_Printf(...)
#endif

Random::Random(u32 p_seed, bool p_print, u32 p_break)
:
m_context(),
m_seed(0)
#if !defined(TT_BUILD_FINAL)
,
m_print(p_print),
m_break(p_break)
#endif // !defined(TT_BUILD_FINAL)
{
#if defined(TT_BUILD_FINAL)
	(void) p_print;
	(void) p_break;
#endif // defined(TT_BUILD_FINAL)
	
	if (p_seed == 0)
	{
		// Set initial seed based on time
		setSeed(static_cast<u32>(system::Time::getInstance()->getMicroSeconds()));
	}
	else
	{
		setSeed(p_seed);
	}
}


// Get a random number up to (NOT INCLUDING) p_max
u32 Random::getNext(u32 p_max)
{
	// MATH_Rand32() from SDK
	m_context.x = m_context.mul * m_context.x + m_context.add;
	
	u32 ret = 0;
	// If the "max" argument is a constant, optimized by compiler.
	if (p_max == 0)
	{
		ret = static_cast<u32>((m_context.x >> 32));
	}
	else
	{
		ret = static_cast<u32>(((m_context.x >> 32) * p_max) >> 32);
	}
	
#if !defined(TT_BUILD_FINAL)
	if (m_print)
	{
		Rand_Printf("Random::getNext-%p-(max:%d): returns: %d\n", &m_context, p_max, ret);
	}
	if (m_break != 0)
	{
		TT_ASSERTMSG(ret != m_break, "ret: %d, m_break: %d", ret, m_break);
	}
#endif // !defined(TT_BUILD_FINAL)
	
	return ret;
}


void Random::setSeed(u32 p_seed)
{
	m_seed = p_seed;
	Rand_Printf("Random::setSeed-%p-(%d)\n", &m_context, p_seed );
	
	// MATH_InitRand32() from SDK
	m_context.x = m_seed;
	m_context.mul = (1566083941LL << 32) + 1812433253LL;
	m_context.add = 2531011;
}

// Namespace end
}
}

