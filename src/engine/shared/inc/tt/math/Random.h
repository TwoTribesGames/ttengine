#ifndef INC_TT_MATH_RANDOM_H
#define INC_TT_MATH_RANDOM_H


#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace math {

class Random
{
public:
	// Get a random number up to (NOT INCLUDING) p_max
	u32  getNext(u32 p_max);
	void setSeed(u32 p_seed);

	inline u32 getNext(u32 p_min, u32 p_max)
	{
		u32 ret = p_min + getNext(p_max - p_min);
		return ret;
	}

	inline u32 getNext()
	{
		u32 ret = getNext(0x7fffffff);
		return ret;
	}
	
	/*! \brief Get random normalized real.
	    \return Random real in range [0.0 - 1.0] */
	inline real getNormalizedNext()
	{
		return getNext(0x7fffffff) / real(0x7ffffffe);
	}
	
	inline real getNextReal(real p_min, real p_max)
	{
		TT_ASSERT(p_min <= p_max);
		return p_min + (getNormalizedNext() * (p_max - p_min));
	}
	
	inline u32 getSeed() const
	{
		return m_seed;
	}

	inline u64 getContextSeedValue() const
	{
		return m_context.x;
	}

	inline void setContextSeedValue(u64 p_x)
	{
		m_context.x = p_x;
	}
	
	inline static Random& getStatic()
	{
		static Random random;
		return random;
	}

	inline static Random& getEffects()
	{
		static Random random;
		return random;
	}
	
	inline void setPrintOn()
	{
#if !defined(TT_BUILD_FINAL)
		m_print = true;
#endif // !defined(TT_BUILD_FINAL)
	}

	inline void setPrintOff()
	{
#if !defined(TT_BUILD_FINAL)
	m_print = false;
#endif // !defined(TT_BUILD_FINAL)
	}

	inline void setBreak(u32 p_value)
	{
#if !defined(TT_BUILD_FINAL)
		m_break = p_value;
#else
		(void) p_value;
#endif // !defined(TT_BUILD_FINAL)
	}

	// WARNING: Made public for Rubiks - Watch out with multiplayer!
//protected:
	explicit Random(u32 p_seed = 0, bool p_print = false, u32 p_break = 0);
	
private:
	
	struct RandContext32
	{
		u64 x;   // Random number value
		u64 mul; // Multiple
		u64 add; // The number to be added
	};
	
	RandContext32 m_context;
	u32           m_seed;
	
#if !defined(TT_BUILD_FINAL)
	bool m_print;
	u32  m_break;
#endif // !defined(TT_BUILD_FINAL)
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_RANDOM_H)

