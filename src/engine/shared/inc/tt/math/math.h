#if !defined(INC_TT_MATH_MATH_H)
#define INC_TT_MATH_MATH_H


#include <cmath>
#include <limits>
#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/code/Int2Type.h>

// The tt::math namespace with several helper functions.
#include <tt/math/tt_cmath.h>
#include <tt/math/tt_cmath_fixed.h>
#include <tt/math/Fixed.h>


namespace tt {
namespace math {

// Constants
static const real pi            =  3.1415927f;
static const real twoPi         =  6.2831853f;
static const real oneOverPi     =  0.3183099f;
static const real halfPi        =  1.5707963f;
static const real oneAndAHalfPi =  4.7123889f;
static const real toRadians     =  0.01745329f;
static const real toDegrees     = 57.29577951f;

static const float floatTolerance        = 0.0005f;
static const Fixed<32,12> fixedTolerance = 0.005f;

enum Axis
{
	Axis_X,
	Axis_Y,
	Axis_Z
};


// Angle conversion
inline real degToRad(real p_deg)
{
	return p_deg * toRadians;
}


inline real radToDeg(real p_rad)
{
	return p_rad * toDegrees;
}


template<int bitSize, int fractionBits>
inline bool clamp(Fixed<bitSize, fractionBits>&       p_value,
                  const Fixed<bitSize, fractionBits>& p_minValue,
                  const Fixed<bitSize, fractionBits>& p_maxValue)
{
	TT_ASSERT(p_minValue <= p_maxValue);
	
	if (p_value < p_minValue)
	{
		p_value = p_minValue;
		return true;
	}
	else if (p_value > p_maxValue)
	{
		p_value = p_maxValue;
		return true;
	}
	return false;
}


template<int bitSize, int fractionBits, typename T>
inline bool clamp(Fixed<bitSize, fractionBits>& p_value,
                  T                             p_minValue,
                  T                             p_maxValue)
{
	TT_ASSERT(p_minValue <= p_maxValue);
	
	if (p_value < p_minValue)
	{
		p_value = p_minValue;
		return true;
	}
	else if (p_value > p_maxValue)
	{
		p_value = p_maxValue;
		return true;
	}
	return false;
}


/*! \brief Clamps the specified value to the specified range.
    \param p_value The value to clamp.
    \param p_minValue The minimum value in the range.
    \param p_maxValue The maximum value in the range. */
template<typename T>
inline bool clamp(T& p_value,
                  T  p_minValue,
                  T  p_maxValue)
{
	TT_ASSERTMSG(p_minValue <= p_maxValue,
	             "tt::math::clamp min (%f) is greater than max (%f)! (While clamping %f.)\n",
	             float(p_minValue), float(p_maxValue), float(p_value));
	
	if (p_value < p_minValue)
	{
		p_value = p_minValue;
		return true;
	}
	else if (p_value > p_maxValue)
	{
		p_value = p_maxValue;
		return true;
	}
	return false;
}


template<int bitSize, int fractionBits>
inline void wrap(Fixed<bitSize, fractionBits>&       p_value,
                 const Fixed<bitSize, fractionBits>& p_minValue,
                 const Fixed<bitSize, fractionBits>& p_maxValue)
{
	while (p_value < p_minValue)
	{
		p_value = p_maxValue + (p_value - p_minValue);
	}
	while (p_value > p_maxValue)
	{
		p_value = p_minValue + (p_value - p_maxValue);
	}
}


template<int bitSize, int fractionBits, typename T>
inline void wrap(Fixed<bitSize, fractionBits>& p_value,
                 T                             p_minValue,
                 T                             p_maxValue)
{
	while (p_value < p_minValue)
	{
		p_value = p_maxValue + (p_value - p_minValue);
	}
	while (p_value > p_maxValue)
	{
		p_value = p_minValue + (p_value - p_maxValue);
	}
}


/*! \brief Wraps the specified value within the specified range.
    \param p_value The value to wrap.
    \param p_minValue The minimum value in the range.
    \param p_maxValue The maximum value in the range. */
template<typename T>
inline void wrap(T& p_value,
                 T  p_minValue,
                 T  p_maxValue)
{
	while (p_value < p_minValue)
	{
		p_value = p_maxValue + (p_value - p_minValue);
	}
	while (p_value > p_maxValue)
	{
		p_value = p_minValue + (p_value - p_maxValue);
	}
}


// Note JL: added this specialization
template<>
inline void wrap(real& p_value,
                 real  p_minValue,
                 real  p_maxValue)
{
	p_value = p_value - (p_maxValue - p_minValue) * tt::math::floor(p_value / (p_maxValue - p_minValue));
}


/*! \brief Check if p_value is power of two
    \param p_value value
    \return true or false */
template<typename T> 
inline bool isPowerOfTwo(T p_value)
{
	TT_STATIC_ASSERT(std::numeric_limits<T>::is_integer);
	return p_value > 0 && ((p_value & (p_value - 1)) == 0);
}


/*! \brief p_value rounded up to a multiple of p_alignment,
    \param p_value value
    \param p_alignment value will be aligned to this (must be power of two)
    \return p_value rounded up to a multiple of p_alignment */
template<typename T> 
inline T roundUp(T p_value, int p_alignment)
{
	TT_STATIC_ASSERT(std::numeric_limits<T>::is_integer);
	TT_ASSERTMSG(isPowerOfTwo(p_alignment),
	             "Alignment %d is not a power of two!", p_alignment);

	return static_cast<T>((static_cast<int>(p_value) + p_alignment - 1) &
	                      ~(p_alignment - 1));
}


/*! \brief p_value is checked whether it is aligned to p_alignment,
    \param p_value value
    \param p_alignment value (must be power of two)
    \return is p_value aligned to p_alignment */
template<typename T> 
inline bool isAligned(T p_value, int p_alignment)
{
	TT_STATIC_ASSERT(std::numeric_limits<T>::is_integer);
	TT_ASSERTMSG(isPowerOfTwo(p_alignment),
	             "Alignment %d is not a power of two!", p_alignment);

	return (static_cast<int>(p_value) & (p_alignment - 1)) == 0;
}


/*! \brief Find power of two of p_value
    \param p_value value (should be power of two)
    \return the number of shifts required to get p_value */
template<typename T> 
inline int findPowerOfTwo(T p_value)
{
	TT_STATIC_ASSERT(std::numeric_limits<T>::is_integer);
	TT_ASSERTMSG(isPowerOfTwo(p_value), "%d is not a power of two!", p_value);
	
	// count number of shifts
	int rval = 0;
	while (p_value > 0)
	{
		p_value >>= 1;
		++rval;
	}
	
	return (rval - 1);
}


/*! \brief Rounds integer types up to a power of 2.
    \param p_num The integer to round up.
    \return The integer p_num rounded up to a power of 2. */
template<typename T>
inline T roundToPowerOf2(T p_num)
{
	TT_ASSERT(p_num > 0);
	TT_STATIC_ASSERT(std::numeric_limits<T>::is_integer);
	T msb(static_cast<T>(-1));
	T prevmsb(static_cast<T>(-1));
	T one(1);
	T zero(0);
	
	const T bitCount(sizeof(T) * 8);
	for (T i = 0; i < bitCount; ++i)
	{
		if ((p_num & one) == one)
		{
			prevmsb = msb;
			msb     = i;
		}
		p_num >>= one;
		
		if (p_num == zero)
		{
			break;
		}
	}
	
	if (prevmsb == T(-1))
	{
		p_num = one << msb;
	}
	else
	{
		p_num = one << (msb + one);
	}
	
	return p_num;
}


/*! \brief Rounds a floating point number up to a chosen amount of decimals.
    \param p_input The floating point number to round
    \param p_decimals The number of decimals to round up to. */
inline real round(real p_input, s32 p_decimals)
{
	real add = 0.5f;
	real multiply = tt::math::pow(real(10.0f), real(p_decimals));

	if(p_input < 0.0f) add = -0.5f;
	return static_cast<real>(static_cast<s32>((p_input * multiply) + add) / multiply);
}



/*! \brief Rounds a floating point number up to 0 decimals.
    \param p_input The floating point number to round.*/
inline real round(real p_input)
{
	return floor(p_input + 0.5f);
}


/*! \brief Checks if two floating point numbers are (roughly) equal.
    \param p_a The first number to compare.
    \param p_b The second number to compare.
    \return True if numbers are equal, false if numbers are not equal. */
inline bool realEqual(float p_a, float p_b)
{
	return tt::math::fabs(p_a - p_b) < floatTolerance;
}


/*! \brief Checks if two fixed point numbers are (roughly) equal.
    \param p_a The first number to compare.
    \param p_b The second number to compare.
    \return True if numbers are equal, false if numbers are not equal. */
template<int bitSize, int fractionBits>
inline bool realEqual(const Fixed<bitSize, fractionBits>& p_a,
                      const Fixed<bitSize, fractionBits>& p_b)
{
	return tt::math::fabs(p_a - p_b) < fixedTolerance;
}

template<int bitSize, int fractionBits>
inline bool realEqual(const Fixed<bitSize, fractionBits>& p_a,
                      const Fixed<bitSize, fractionBits>& p_b,
                      const Fixed<bitSize, fractionBits>& p_tolerance)
{
	return tt::math::fabs(p_a - p_b) < p_tolerance;
}

inline bool realEqual(float p_a, float p_b, float p_tolerance)
{
	return tt::math::fabs(p_a - p_b) < p_tolerance;
}


template<int bitSize, int fractionBits>
inline bool realEqual(const Fixed<bitSize, fractionBits>& p_a,
                      float p_b)
{
	return tt::math::fabs(p_a - Fixed<bitSize, fractionBits>(p_b)) < fixedTolerance;
}


template<int bitSize, int fractionBits>
inline bool realEqual(float p_a,
                      const Fixed<bitSize, fractionBits>& p_b)
{
	return tt::math::fabs(Fixed<bitSize, fractionBits>(p_a) - p_b) < fixedTolerance;
}


inline bool realGreaterThan(float p_lhs, float p_rhs)
{
	return p_lhs > (p_rhs - floatTolerance);
}

template<int bitSize, int fractionBits>
inline bool realGreaterThan(const Fixed<bitSize, fractionBits>& p_lhs,
                            const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs > (p_rhs - fixedTolerance);
}


inline bool realGreaterEqual(float p_lhs, float p_rhs)
{
	return p_lhs >= (p_rhs - floatTolerance);
}

template<int bitSize, int fractionBits>
inline bool realGreaterEqual(const Fixed<bitSize, fractionBits>& p_lhs,
                             const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs >= (p_rhs - fixedTolerance);
}


inline bool realLessThan(float p_lhs, float p_rhs)
{
	return p_lhs < (p_rhs + floatTolerance);
}

template<int bitSize, int fractionBits>
inline bool realLessThan(const Fixed<bitSize, fractionBits>& p_lhs,
                         const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs < (p_rhs + fixedTolerance);
}


inline bool realLessEqual(float p_lhs, float p_rhs)
{
	return p_lhs <= (p_rhs + floatTolerance);
}

template<int bitSize, int fractionBits>
inline bool realLessEqual(const Fixed<bitSize, fractionBits>& p_lhs,
                          const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs <= (p_rhs + fixedTolerance);
}


//------------------------------------------------------------------------------
// Bit checking functions

/*! \brief Indicates whether the specified bit is set in the specified value.
    \param p_value The value to check.
    \param p_bit The bit to check for.
    \return True if the specified bit is set in p_value, false if it is not. */
template<typename A, typename B>
inline bool isBitSet(A p_value, B p_bit)
{
	return ((p_value & A(p_bit)) == A(p_bit));
}


/*! \brief Indicates whether the any bit in the specified mask is set in the specified value.
    \param p_value The value to check.
    \param p_bits The bits to check for.
    \return True if any of the specified bits is set in p_value, false if none of the specified bits are set. */
template<typename A, typename B>
inline bool isAnyBitSet(A p_value, B p_bits)
{
	return ((p_value & A(p_bits)) != 0);
}


//------------------------------------------------------------------------------
// Byte order helpers

/*! \brief Converts an array of two bytes in big-endian format to an unsigned 16-bit integer.
    \param p_bytes The bytes to convert. Array is assumed to contain at least two bytes.
    \return The bytes converted to unsigned 16-bit integer. */
inline u16 toU16FromBigEndian(const u8* p_bytes)
{
	return static_cast<u16>(static_cast<u16>(p_bytes[0]) << 8 | p_bytes[1]);
}


/*! \brief Converts an array of four bytes in big-endian format to an unsigned 32-bit integer.
    \param p_bytes The bytes to convert. Array is assumed to contain at least four bytes.
    \return The bytes converted to unsigned 32-bit integer. */
inline u32 toU32FromBigEndian(const u8* p_bytes)
{
	return static_cast<u32>(p_bytes[0]) << 24 |
	       static_cast<u32>(p_bytes[1]) << 16 |
	       static_cast<u32>(p_bytes[2]) << 8  |
	       p_bytes[3];
}


inline u32 getLeastSignificantBit(u32 p_x)
{
	return static_cast<u32>(p_x & -static_cast<s32>(p_x));
}


u32 countLeadingZeros(u32 p_x);

inline u32 getMostSignificantBit(u32 p_x)
{
	return static_cast<u32>(p_x & (static_cast<u32>(0x80000000U) >> countLeadingZeros(p_x)));
}


/*! \brief Reflect a bit pattern (mirror)
    \param p_value Value to reflect.
    \return Reflected value.*/
template <typename T>
inline T reflect(T p_value)
{
	T ret(0);
	for (T i = 0; i < sizeof(T) * 8; ++i)
	{
		ret <<= 1;
		if (p_value & 1)
		{
			ret |= 1;
		}
		p_value >>= 1;
	}
	return ret;
}


/*! \brief Rotates a bit pattern left
    \param p_value Value to rotate.
    \param p_amount Number of bits to rotate.
    \return Rotated value.*/
template <typename T, typename B>
inline T rotateLeft(T p_value, B p_amount)
{
	// ensure amount lies within the range [0 - sizeof(T) * 8)
	p_amount &= static_cast<B>((sizeof(T) * 8) - 1);
	
	// shift left, catch the bits that will fall off by shifting right with bitsize - amount
	// make sure to mask the bits that fell off, shifting a negative number right will cause
	// the bit pattern to be padded with 1's
	return T((p_value << p_amount) |
	        ((p_value >> (static_cast<B>(sizeof(T) * 8) - p_amount)) &
	         static_cast<T>(((1 << p_amount) - 1))));
}


/*! \brief Rotates a bit pattern right
    \param p_value Value to rotate.
    \param p_amount Number of bits to rotate.
    \return Rotated value.*/
template <typename T, typename B>
inline T rotateRight(T p_value, B p_amount)
{
	// ensure amount lies within the range [0 - sizeof(T) * 8)
	p_amount &= static_cast<B>((sizeof(T) * 8) - 1);
	
	// shift right, catch the bits that will fall off by shift left with bitsize - amount
	// make sure to mask when shifting right, negative numbers will cause padding with 1's.
	return T((p_value << (static_cast<B>(sizeof(T) * 8) - p_amount)) |
	        ((p_value >> p_amount) &
	         static_cast<T>(((1 << (static_cast<B>(sizeof(T) * 8) - p_amount)) - 1))));
}


template<typename T>
inline T lerp(const T& p_src, const T& p_dst, real p_amount)
{
	return p_src + (p_dst - p_src) * p_amount;
}


template<int bitSize, int fractionBits>
inline void makeHalf(Fixed<bitSize,fractionBits>& p_value)
{
	p_value.setValue(p_value.getValue() >> 1);
}


template <typename T>
inline void makeHalf(T& p_value, code::Int2Type<false>)
{
	p_value *= 0.5f;
}


template <typename T>
inline void makeHalf(T& p_value, code::Int2Type<true>)
{
	p_value >>= 1;
}


template <typename T>
inline void makeHalf(T& p_value)
{
	makeHalf(p_value, code::Int2Type<std::numeric_limits<T>::is_integer>());
	//std::numeric_limits<ValueType>::is_signed
}


template<int bitSize, int fractionBits>
inline Fixed<bitSize,fractionBits> getHalf(Fixed<bitSize,fractionBits> p_value)
{
	return Fixed<bitSize, fractionBits>::createFromRawFixed(p_value.getValue() >> 1);
}


template <typename T>
inline T getHalf(T p_value, code::Int2Type<false>)
{
	return p_value * 0.5f;
}


template <typename T>
inline T getHalf(T p_value, code::Int2Type<true>)
{
	return p_value >> 1;
}


template <typename T>
inline T getHalf(T p_value)
{
	return getHalf(p_value, code::Int2Type<std::numeric_limits<T>::is_integer>());
	//std::numeric_limits<ValueType>::is_signed
}


template<int bitSize, int fractionBits>
inline void makeTimesTwo(Fixed<bitSize,fractionBits>& p_value)
{
	p_value.setValue(p_value.getValue() << 1);
}


template <typename T>
inline void makeTimesTwo(T& p_value)
{
	makeTimesTwo(p_value, code::Int2Type<std::numeric_limits<T>::is_integer>());
	//std::numeric_limits<ValueType>::is_signed
}


template <typename T>
inline void makeTimesTwo(T& p_value, code::Int2Type<false>)
{
	p_value *= 2.0f;
}


template <typename T>
inline void makeTimesTwo(T& p_value, code::Int2Type<true>)
{
	p_value <<= 1;
}


template<int bitSize, int fractionBits>
inline Fixed<bitSize, fractionBits> getTimesTwo(Fixed<bitSize, fractionBits> p_value)
{
	return Fixed<bitSize, fractionBits>::createFromRawFixed(p_value.getValue() << 1);
}


template <typename T>
inline T getTimesTwo(T p_value, code::Int2Type<false>)
{
	return p_value * 2.0f;
}


template <typename T>
inline T getTimesTwo(T p_value, code::Int2Type<true>)
{
	return p_value << 1;
}


template <typename T>
inline T getTimesTwo(T p_value)
{
	return getTimesTwo(p_value, code::Int2Type<std::numeric_limits<T>::is_integer>());
	//std::numeric_limits<ValueType>::is_signed
}


// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_MATH_H)
