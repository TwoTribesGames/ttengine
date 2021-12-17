#if !defined(INC_TT_CODE_DEFAULTVALUE_H)
#define INC_TT_CODE_DEFAULTVALUE_H


#include <tt/code/OptionalValue.h>


namespace tt {
namespace code {


template <typename Type>
class DefaultValue
{
public:
	typedef Type ValueType;
	
	// No default ctor because you WANT a default value when using this class.
	//inline OptionalWithDefault() : m_value(), m_valid(false) {}
	inline DefaultValue(ValueType p_defaultValue)
	:
	m_value(),
	m_defaultValue(p_defaultValue),
	m_valid(false)
	{}

	inline DefaultValue(const DefaultValue& p_value)
		:
		m_value(p_value.m_value),
		m_defaultValue(p_value.m_defaultValue),
		m_valid(p_value.m_valid)
	{}
	
	inline DefaultValue& operator=(const ValueType& p_value)
	{
		m_value = p_value;
		m_valid = true;
		return *this;
	}
	
	inline DefaultValue& operator=(const DefaultValue& p_value)
	{
		m_value = p_value.m_value;
		m_valid = p_value.m_valid;
		return *this;
	}
	
	inline DefaultValue& operator=(const OptionalValue<ValueType>& p_value)
	{
		m_valid = p_value.isValid();
		if (m_valid)
		{
			m_value = p_value.get();
		}
		return *this;
	}
	
	inline bool isValid() const { return m_valid;}
	inline ValueType get() const
	{
		if (isValid()) return m_value;
		else           return m_defaultValue.get();
	}
	
	inline operator ValueType() const { return get(); }
	
private:
	ValueType                      m_value;
	const OptionalValue<ValueType> m_defaultValue;
	bool                           m_valid;
};


template <typename Type>
inline bool operator==(const DefaultValue<Type>& p_lhs, const DefaultValue<Type>& p_rhs)
{
	return p_lhs.get() == p_rhs.get();
}


template <typename Type>
inline bool operator!=(const DefaultValue<Type>& p_lhs, const DefaultValue<Type>& p_rhs)
{
	return p_lhs.get() != p_rhs.get();
}


template <typename Type>
inline bool operator==(const DefaultValue<Type>& p_lhs, const Type& p_rhs)
{
	return p_lhs.get() == p_rhs;
}


template <typename Type>
inline bool operator!=(const DefaultValue<Type>& p_lhs, const Type& p_rhs)
{
	return p_lhs.get() != p_rhs;
}


// End namespace
}
}


#endif // !defined(INC_TT_CODE_DEFAULTVALUE_H)
