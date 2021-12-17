#if !defined(INC_TT_CODE_OPTIONALVALUE_H)
#define INC_TT_CODE_OPTIONALVALUE_H

#include <tt/platform/tt_error.h>

namespace tt {
namespace code {


template <typename Type>
class OptionalValue
{
public:
	typedef Type ValueType;
	
	inline OptionalValue() : m_value(), m_valid(false) {}
	inline OptionalValue(ValueType p_value)
	:
	m_value(p_value),
	m_valid(true)
	{}
	
	inline OptionalValue& operator=(const ValueType& p_value)
	{
		m_value = p_value;
		m_valid = true;
		return *this;
	}
	
	inline bool isValid() const { return m_valid;}
	inline ValueType get() const
	{
		TT_ASSERTMSG(isValid(), "Trying to use an Optional value that's invalid!");
		return m_value;
	}
	
	inline operator ValueType() const { return get(); }
	
private:
	ValueType m_value;
	bool      m_valid;
};


template <typename Type>
inline bool operator==(const OptionalValue<Type>& p_lhs, const OptionalValue<Type>& p_rhs)
{
	// Use get() so isValid is checked on both values.
	return p_lhs.get() == p_rhs.get();
}


template <typename Type>
inline bool operator!=(const OptionalValue<Type>& p_lhs, const OptionalValue<Type>& p_rhs)
{
	// Use get() so isValid is checked on both values.
	return p_lhs.get() != p_rhs.get();
}


template <typename Type>
inline bool operator==(const OptionalValue<Type>& p_lhs, const Type& p_rhs)
{
	// Use get() so isValid is checked.
	return p_lhs.get() == p_rhs;
}


template <typename Type>
inline bool operator!=(const OptionalValue<Type>& p_lhs, const Type& p_rhs)
{
	// Use get() so isValid is checked.
	return p_lhs.get() != p_rhs;
}

// End namespace
}
}


#endif // !defined(INC_TT_CODE_OPTIONALVALUE_H)
