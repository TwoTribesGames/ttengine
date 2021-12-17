//////////////////////////////////////////////////////////////////////////
// Fixed.h
// Fixedpoint class.
// Copyright (c) 2005 - 2007 Two Tribes. All rights reserved.
//////////////////////////////////////////////////////////////////////////

#ifndef INC_TT_MATH_FIXED_H
#define INC_TT_MATH_FIXED_H


#include <iostream>
#include <limits>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/code/TypeWithByteSize.h>
#include <tt/streams/fwd.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


namespace tt {
namespace math {

//! \brief Fixed-point number presentation class
//!	 
//!	- bitSize   width of number in bits, must be word-limited  (16,32,64)  
template<int bitSize, int fractionBits>
class Fixed
{
public:
	// Bitsize template parameter must be divisible by 8.
	TT_STATIC_ASSERT(bitSize % 8 == 0 && bitSize != 8);
	
	// The nunber of fraction bits can not be greater than the bitSize of the 
	// actual number.
	TT_STATIC_ASSERT(bitSize >= fractionBits);
	
	typedef typename tt::code::TypeWithBitSize<bitSize>::type ValueType;
	
	TT_STATIC_ASSERT(std::numeric_limits<ValueType>::is_integer &&
	                 std::numeric_limits<ValueType>::is_signed);
	
	// --------------------------------
	// Various constructors
	Fixed()
	:
	m_value(0)
	{ }
	
	Fixed(const Fixed& p_a)
	:
	m_value(p_a.m_value)
	{ }
	
	template<int otherBitSize, int otherFractionBits>
	Fixed(const Fixed<otherBitSize, otherFractionBits>& p_a)
	:
	m_value(p_a.convert<bitSize>(fractionBits))
	{
	}
	
	Fixed(ValueType p_a)
	:
	m_value(p_a << FIXSHIFT)
	{ }
	
	
	// No implicit ctor because of possible data loss.
	explicit Fixed(u32 p_a)
	:
	m_value(static_cast<ValueType>(p_a) << FIXSHIFT)
	{ }
	
	Fixed(float p_a)
	:
	m_value(static_cast<ValueType>(p_a * static_cast<float>(FIXUNIT)))
	{ }
	
	explicit Fixed(double p_a)
	:
	m_value(static_cast<ValueType>(p_a * static_cast<double>(FIXUNIT)))
	{ }
	
	// --------------------------------
	// The cast operators, one for each built in type	
	
	inline operator u8() const
	{
		return static_cast<u8>(m_value >> FIXSHIFT);
	}
	
	inline operator s8() const
	{
		return static_cast<s8>(m_value / FIXUNIT);
	}
	
	inline operator u16() const
	{
		return static_cast<u16>(m_value >> FIXSHIFT);
	}
	
	inline operator s16() const
	{
		return static_cast<s16>(m_value / FIXUNIT);
	}
	
	inline operator u32() const
	{
		return static_cast<u32>(m_value >> FIXSHIFT);
	}
	
	inline operator s32() const
	{
		return static_cast<s32>(m_value / FIXUNIT);
	}
	
	inline operator u64() const
	{
		return static_cast<u64>(m_value >> FIXSHIFT);
	}
	
	inline operator s64() const
	{
		return static_cast<s64>(m_value / FIXSHIFT);
	}
	
	// NO IMPLICIT CONVERSION TO FLOAT!!
	// This causes too much trouble on DS
	
	inline float toFloat() const
	{
		return static_cast<float>(m_value) / FIXUNIT;
	}
	
	inline double toDouble() const
	{
		return static_cast<double>(m_value) / FIXUNIT;
	}
	
	
	inline int toInt() const
	{
		return static_cast<int>(m_value / FIXUNIT);
	}
	
	// --------------------------------
	// All the operators =
	
	inline Fixed& operator=(const Fixed& p_a)
	{
		m_value = p_a.m_value;
		return *this;
	}
	
	template<typename T>
	inline Fixed& operator=(T p_a)
	{
		m_value = Fixed(p_a).m_value;
		return *this;
	}
	
	// --------------------------------
	// The operators +=, -=, *=, /=
	inline Fixed& operator+=(const Fixed& p_a)
	{
		m_value += p_a.m_value;
		return *this;
	}
	
	template<typename T>
	inline Fixed& operator+=(T p_a)
	{
		m_value += Fixed(p_a).m_value;
		return *this;
	}
	
	inline Fixed& operator-=(const Fixed& p_a)
	{
		m_value -= p_a.m_value;
		return *this;
	}
	
	template<typename T>
	inline Fixed& operator-=(T p_a)
	{
		m_value -= Fixed(p_a).m_value;
		return *this;
	}
	
	inline Fixed& operator*=(const Fixed& p_a)
	{
		typedef typename tt::code::TypeWithBitSize<bitSize>::doubleType DType;
		m_value = static_cast<ValueType>(static_cast<DType>(m_value) * p_a.m_value 
		                                 >> FIXSHIFT);
		
		return *this;
	}
	
	template<typename T>
	inline Fixed& operator*=(T p_a)
	{
		return operator*=(Fixed(p_a));
	}
	
	inline Fixed& operator/=(const Fixed& p_a)
	{
		TT_ASSERT(p_a.m_value != 0);
		if (p_a.m_value != 0 )
		{
			typedef typename tt::code::TypeWithBitSize<bitSize>::doubleType DType;
			m_value = static_cast<ValueType>((static_cast<DType>(m_value) << FIXSHIFT)
			                                 / p_a.m_value);
		}
		else
		{
			m_value = 0;
		}
		return *this;
	}
	
	template<typename T>
	inline Fixed& operator/=(T p_a)
	{
		return operator/=(Fixed(p_a));
	}
	
	
	// prefix increment/decrement
	inline Fixed& operator++() { m_value += FIXONE; return *this; }
	inline Fixed& operator--() { m_value -= FIXONE; return *this; }
	
	
	// postfix increment/decrement (aren't allowed)
	inline Fixed operator++(int /* dummy */);
//	{
//		Fixed temp(*this);
//		m_value += FIXONE;
//		return temp;
//	}
	
	inline Fixed operator--(int /* dummy */);
//	{
//		Fixed temp(*this);
//		m_value -= FIXONE;
//		return temp;
//	}
	
	
	// The negation operator
	inline Fixed operator-() const
	{
		Fixed temp(*this);
		temp.m_value = -temp.m_value;
		return temp;
	}
	
	
	// Bit shift operators
	inline Fixed operator>>(int p_n) const
	{
		Fixed temp;
		temp.m_value = m_value >> p_n;
		return temp;
	}
	
	
	inline Fixed operator<<(int p_n) const
	{
		Fixed temp;
		temp.m_value = m_value << p_n;
		return temp;
	}
	
	
	inline Fixed& operator>>=(int p_n)
	{
		m_value >>= p_n;
		return *this;
	}
	
	
	inline Fixed& operator<<=(int p_n)
	{
		m_value <<= p_n;
		return *this;
	}
	
	
	// special unsigned int operator.
	inline bool operator==(unsigned int p_a) const
	{
		if (m_value < 0)
		{
			return false;
		}
		
		return static_cast<unsigned int>(m_value) == (p_a << FIXSHIFT);
	}
	
	
	//! \brief less than
	inline bool operator<(const Fixed& p_other) const
	{
		return (m_value < p_other.m_value);
	}
	
	
	template<typename T>
	inline bool operator<(T p_other) const
	{
		// return (m_value < Fixed<bitSize,fractionBits>(p_other).m_value);
		return (m_value < Fixed(p_other).m_value);
	}
	
	
	//! \brief Equal
	inline bool operator==(const Fixed& p_other) const
	{
		return (m_value == p_other.m_value);
	}
	
	
	template<typename T>
	inline bool operator==(const T& p_other) const
	{
		return (m_value == Fixed(p_other).m_value);
	}
	
	
	//! \brief Not equal
	inline bool operator!=(const Fixed& p_other) const
	{
		return (m_value !=  p_other.m_value);
	}
	
	template<typename T>
	inline bool operator!=(T p_other) const
	{
		return (m_value != Fixed(p_other).m_value);
	}
	
	
	//! \brief Greater than
	inline bool operator>(const Fixed& p_other) const
	{
		return (m_value > p_other.m_value);
	}
	
	template<typename T>
	inline bool operator>(T p_other) const
	{
		return (m_value > Fixed(p_other).m_value);
	}
	
	
	//! \brief Less than or equal
	inline bool operator<=(const Fixed& p_other) const
	{
		return (m_value <= p_other.m_value);
	}
	
	template<typename T>
	inline bool operator<=(T p_other) const
	{
		return (m_value <= Fixed(p_other).m_value);
	}
	
	
	//! \brief Greater than or equal
	inline bool operator>=(const Fixed& p_other) const
	{
		return (m_value >= p_other.m_value);
	}
	
	
	template<typename T>
	inline bool operator>=(T p_other) const
	{
		return (m_value >= Fixed(p_other).m_value);
	}
	
	
	////////////////////////////////////////////////////////////////////////////
	// extra functions
	////////////////////////////////////////////////////////////////////////////
	
	inline ValueType getValue() const
	{
		return m_value;
	}
	
	inline void setValue(ValueType p_value)
	{
		m_value = p_value;
	}
	
	template<int otherBitSize>
	inline typename tt::code::TypeWithBitSize<otherBitSize>::type convert(int p_bits) const
	{
		typedef typename tt::code::TypeWithBitSize<otherBitSize>::type OtherValueType;
		OtherValueType ret = static_cast<OtherValueType>(m_value);
		
		if (p_bits < FIXSHIFT)
		{
			return static_cast<OtherValueType>(ret >> (FIXSHIFT - p_bits));
		}
		if (p_bits > FIXSHIFT)
		{
			return static_cast<OtherValueType>(ret << (p_bits - FIXSHIFT));
		}
		
		return ret;
	}
	
	
	//! \return A fixed number defined by the parameter
	static Fixed createFromRawFixed(ValueType p_val)
	{
		return Fixed(p_val, 0);
	}
	
	
private:
	static const ValueType FIXSHIFT = fractionBits;
	static const ValueType FIXUNIT  = ValueType(1) << FIXSHIFT;
	static const ValueType FIXONE   = ValueType(1) << FIXSHIFT;
	static const ValueType FIXZERO  = ValueType(0) << FIXSHIFT;
	
	Fixed(ValueType p_value, int p_dummy)
	:
	m_value(p_value)
	{
		((void)p_dummy);
	}
	
	ValueType m_value;
};



template<int bitSize, int fractionBits>
tt::streams::BOStream& operator<<(
		tt::streams::BOStream& p_stream,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_stream << p_rhs.getValue();
}


template<int bitSize, int fractionBits>
tt::streams::BIStream& operator>>(tt::streams::BIStream& p_stream,
                                  Fixed<bitSize, fractionBits>& p_rhs)
{
	typename Fixed<bitSize, fractionBits>::ValueType temp = 0;
	p_stream >> temp;
	p_rhs.setValue(temp);
	return p_stream;
}


template<int bitSize, int fractionBits>
std::ostream& operator<<(std::ostream& p_stream,
                         const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_stream << p_rhs.toFloat();
}



template<int bitSize, int fractionBits>
std::istream& operator>>(std::istream& p_stream,
                         Fixed<bitSize, fractionBits>& p_rhs)
{
	float temp = 0;
	p_stream >> temp;
	p_rhs = temp;
	return p_stream;
}



// Less than
template<typename T, int bitSize, int fractionBits>
inline bool operator<(T p_lhs,
                      const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) < p_rhs;
}


// Equal
template<typename T, int bitSize, int fractionBits>
inline bool operator==(T p_lhs,
                       const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) == p_rhs;
}


// Not equal
template<typename T, int bitSize, int fractionBits>
inline bool operator!=(T p_lhs,
                       const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) != p_rhs;
}


// Greater than
template<typename T, int bitSize, int fractionBits>
inline bool operator>(T p_lhs,
                      const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize,fractionBits>(p_lhs) > p_rhs;
}


// Less than or equal
template<typename T, int bitSize, int fractionBits>
inline bool operator<=(T p_lhs,
                       const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) <= p_rhs;
}



// Greater than or equal
template<typename T, int bitSize, int fractionBits>
inline bool operator>=(T p_lhs,
                       const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) >= p_rhs;
}


// Arithmetic operators
template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator+(
		T p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) += p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator+(
		Fixed<bitSize,fractionBits> p_lhs,
		T p_rhs)
{
	return p_lhs += Fixed<bitSize, fractionBits>(p_rhs);
}


template<int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator+(
		Fixed<bitSize, fractionBits> p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs += p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator-(
		T p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) -= p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator-(
		Fixed<bitSize, fractionBits> p_lhs,
		T p_rhs)
{
	return p_lhs -= Fixed<bitSize, fractionBits>(p_rhs);
}


template<int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator-(
		Fixed<bitSize, fractionBits> p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs -= p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator*(
		T p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) *= p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator*(
		Fixed<bitSize, fractionBits> p_lhs,
		T p_rhs)
{
	return p_lhs *= Fixed<bitSize, fractionBits>(p_rhs);
}


template<int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator*(
		Fixed<bitSize, fractionBits> p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs *= p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator/(
		T p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return Fixed<bitSize, fractionBits>(p_lhs) /= p_rhs;
}


template<typename T, int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator/(
		Fixed<bitSize, fractionBits> p_lhs,
		T p_rhs)
{
	return p_lhs /= Fixed<bitSize, fractionBits>(p_rhs);
}


template<int bitSize, int fractionBits>
inline const Fixed<bitSize, fractionBits> operator/(
		Fixed<bitSize, fractionBits> p_lhs,
		const Fixed<bitSize, fractionBits>& p_rhs)
{
	return p_lhs /= p_rhs;
}

// Namespace end
}
}


// Numeric Limits for Fixed
namespace std {

template<int bitSize, int fractionBits>
#if defined(__clang__)
struct numeric_limits<tt::math::Fixed<bitSize, fractionBits> >
#else
class numeric_limits<tt::math::Fixed<bitSize, fractionBits> >
#endif
{
public:
	inline static tt::math::Fixed<bitSize, fractionBits> min() throw()
	{
		return tt::math::Fixed<bitSize, fractionBits>::createFromRawFixed(
			numeric_limits<typename tt::math::Fixed<bitSize, fractionBits>::ValueType>::min());
	}
	
	inline static tt::math::Fixed<bitSize, fractionBits> max() throw()
	{
		return tt::math::Fixed<bitSize, fractionBits>::createFromRawFixed(
			numeric_limits<typename tt::math::Fixed<bitSize, fractionBits>::ValueType>::max());
	}
};

}


#endif  // INC_TT_MATH_FIXED_H
