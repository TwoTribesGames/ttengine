#if !defined(INC_TT_MATH_HASH_HASH_H)
#define INC_TT_MATH_HASH_HASH_H

#include <limits>
#include <string>

#include <tt/platform/tt_types.h>
#include <tt/code/TypeWithByteSize.h>


namespace tt {
namespace math {
namespace hash {

/*! \brief Converts strings to hash values. */
template<int BitSize>
class Hash
{
public:
	typedef typename code::TypeWithBitSize<BitSize>::unsignedType ValueType;
	
	// FIXME: This is evil and needs to be replaced with a proper solution.
	static const ValueType INVALID_HASH = static_cast<ValueType>(-1);
	static const Hash invalid;
	
	explicit Hash(ValueType p_hashValue = INVALID_HASH)
	:
	m_hashValue(p_hashValue)
	{ }
	
	explicit Hash(const std::string& p_string)
	:
	m_hashValue(makeHash(p_string))
	{ }
	
	explicit Hash(const char* p_string)
	:
	m_hashValue(makeHash(p_string))
	{ }
	
	inline ValueType getValue() const { return m_hashValue; }
	
	inline void invalidate() { m_hashValue = INVALID_HASH; }
	
	inline bool isValid() const
	{ return (m_hashValue != INVALID_HASH); }
	
	inline bool operator<(const Hash& p_rhs) const
	{ return m_hashValue < p_rhs.m_hashValue; }
	
	inline bool operator>(const Hash& p_rhs) const
	{ return m_hashValue > p_rhs.m_hashValue; }
	
	inline bool operator<=(const Hash& p_rhs) const
	{ return m_hashValue <= p_rhs.m_hashValue; }
	
	inline bool operator>=(const Hash& p_rhs) const
	{ return m_hashValue >= p_rhs.m_hashValue; }
	
	inline bool operator==(const Hash& p_rhs) const
	{ return m_hashValue == p_rhs.m_hashValue; }
	
	inline bool operator!=(const Hash& p_rhs) const
	{ return !operator==(p_rhs); }
	
private:
	inline static ValueType makeHash(const char* p_string)
	{
		static const ValueType initialHash = 63689U;
		static const ValueType multiplier  = 378551U;
		
		ValueType a      = initialHash;
		ValueType result = 0;
		
		while ((*p_string) != '\0')
		{
			result = (result * a) + *p_string;
			a      = a * multiplier;
			++p_string;
		}
		
		return result;
	}
	
	inline static ValueType makeHash(const std::string& p_string)
	{
		static const ValueType initialHash = 63689U;
		static const ValueType multiplier  = 378551U;
		
		ValueType a      = initialHash;
		ValueType result = 0;
		
		for (std::string::const_iterator it = p_string.begin();
		     it != p_string.end(); ++it)
		{
			result = (result * a) + *it;
			a      = a * multiplier;
		}
		
		return result;
	}
	
	ValueType m_hashValue;
};


template<int BitSize>
const Hash<BitSize> Hash<BitSize>::invalid(INVALID_HASH);

// Namespace end
}
}
}

#endif  // !defined(INC_TT_MATH_HASH_HASH_H)
