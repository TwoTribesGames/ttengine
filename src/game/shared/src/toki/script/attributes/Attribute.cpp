#include <tt/code/ErrorStatus.h>

#include <toki/script/attributes/Attribute.h>


namespace toki {
namespace script {
namespace attributes {

//--------------------------------------------------------------------------------------------------
// Public member functions

Attribute::Attribute(const std::string& p_name, const tt::str::Strings& p_value,
                     const std::string& p_type)
:
m_name(p_name),
m_type(Type_None),
m_valueInteger(0),
m_valueFloat(0.0f),
m_valueBool(false),
m_valueString(),
m_valueIntegerArray(),
m_valueFloatArray(),
m_valueBoolArray(),
m_valueStringArray()
{
	m_type = getTypeFromName(p_type);
	if (isValidType(m_type) == false || m_type == Type_None)
	{
		TT_PANIC("Unsupported type '%s' for attribute with name '%s'",
		         p_type.c_str(), m_name.c_str());
		m_type = Type_Invalid;
		return;
	}
	
	if (m_type == Type_Null)
	{
		// Null type has no value
		return;
	}
	
	if (p_value.empty())
	{
		TT_PANIC("Attribute '%s' (type '%s') constructed without a value.",
		         p_name.c_str(), p_type.c_str());
		m_type = Type_None;
		return;
	}
	
	if (isArrayType(m_type))
	{
		setValue(p_value);
	}
	else
	{
		TT_ASSERTMSG(p_value.size() == 1,
		             "Attribute '%s' with type '%s' should not be set with an array value.",
		             p_name.c_str(), getTypeName(m_type));
		setValue(p_value[0]);
	}
}


Attribute::Attribute()
:
m_name(),
m_type(Type_None),
m_valueInteger(0),
m_valueFloat(0.0f),
m_valueBool(false),
m_valueString(),
m_valueIntegerArray(),
m_valueFloatArray(),
m_valueBoolArray(),
m_valueStringArray()
{
}


// setValue non-array types
void Attribute::setValue(const std::string& p_value)
{
	if (isArrayType(m_type))
	{
		TT_PANIC("Cannot set a single value '%s' for attribute '%s' when attribute type is array type '%s'.",
		         p_value.c_str(), m_name.c_str(), getTypeName(m_type));
		return;
	}
	
	TT_ERR_CREATE("Set attribute value");
	
	switch (m_type)
	{
	case Type_Null:    return;
	case Type_Integer: m_valueInteger = tt::str::parseS32 (p_value, &errStatus); break;
	case Type_Float:   m_valueFloat   = tt::str::parseReal(p_value, &errStatus); break;
	case Type_Bool:    m_valueBool    = tt::str::parseBool(p_value, &errStatus); break;
	case Type_String:  m_valueString  = p_value;                                 break;
		
	default:
		TT_PANIC("setValue doesn't support type '%s' for attribute with name '%s'.",
		         getTypeName(m_type), m_name.c_str());
		break;
	}
	
	TT_ASSERTMSG(errStatus.hasError() == false,
	             "Could not parse value '%s' as %s for attribute '%s'.\nReason:\n%s",
	             p_value.c_str(), getTypeName(m_type), m_name.c_str(), errStatus.getErrorMessage().c_str());
}


// setValue array types
void Attribute::setValue(const tt::str::Strings& p_value)
{
	if (isArrayType(m_type) == false)
	{
		TT_PANIC("Cannot set an array value for attribute '%s' when attribute type is a non-array type '%s'.",
		         m_name.c_str(), getTypeName(m_type));
		return;
	}
	
	TT_ERR_CREATE("Set attribute value");
	
	using tt::str::Strings;
	Type elementType = Type_None;
	switch (m_type)
	{
	case Type_IntegerArray:
		elementType = Type_Integer;
		m_valueIntegerArray.clear();
		m_valueIntegerArray.reserve(static_cast<IntegerArray::size_type>(p_value.size()));
		for (Strings::const_iterator it = p_value.begin(); it != p_value.end(); ++it)
		{
			s32 val = tt::str::parseS32((*it), &errStatus);
			m_valueIntegerArray.push_back(val);
		}
		break;
		
	case Type_FloatArray:
		elementType = Type_Float;
		m_valueFloatArray.clear();
		m_valueFloatArray.reserve(static_cast<FloatArray::size_type>(p_value.size()));
		for (Strings::const_iterator it = p_value.begin(); it != p_value.end(); ++it)
		{
			real val = tt::str::parseReal((*it), &errStatus);
			m_valueFloatArray.push_back(val);
		}
		break;
		
	case Type_BoolArray:
		elementType = Type_Bool;
		m_valueBoolArray.clear();
		m_valueBoolArray.reserve(static_cast<BoolArray::size_type>(p_value.size()));
		for (Strings::const_iterator it = p_value.begin(); it != p_value.end(); ++it)
		{
			bool val = tt::str::parseBool((*it), &errStatus);
			m_valueBoolArray.push_back(val);
		}
		break;
		
	case Type_StringArray:
		elementType        = Type_String;
		m_valueStringArray = p_value;
		break;
		
	default:
		TT_PANIC("setValue doesn't support type '%s' for attribute with name '%s'.",
		         getTypeName(m_type), m_name.c_str());
		break;
	}
	
	TT_ASSERTMSG(errStatus.hasError() == false,
	             "Could not parse one or more array values as %s for attribute '%s'.\nReason:\n%s",
	             getTypeName(elementType), m_name.c_str(), errStatus.getErrorMessage().c_str());
}


std::string Attribute::getAsString() const
{
	using tt::str::toStr;
	switch (m_type)
	{
	case Type_Null:         return "null";
	case Type_Integer:      return toStr(m_valueInteger);
	case Type_Float:        return toStr(m_valueFloat);
	case Type_Bool:         return toStr(m_valueBool);
	case Type_String:       return m_valueString;
	//case Type_IntegerArray: return toStr(m_valueIntegerArray);
	//case Type_FloatArray:   return toStr(m_valueFloatArray);
	//case Type_BoolArray:    return toStr(m_valueBoolArray);
	//case Type_StringArray:  return toStr(m_valueStringArray);
	
	default:
		TT_PANIC("Cannot get attribute type '%d' as string.", m_type);
		return std::string();
	}
}


const char* Attribute::getTypeName(Type p_type)
{
	switch (p_type)
	{
	case Type_None:         return "none";
	
	case Type_Null:         return "null";
	case Type_Integer:      return "integer";
	case Type_Float:        return "float";
	case Type_Bool:         return "bool";
	case Type_String:       return "string";
	case Type_IntegerArray: return "integer_array";
	case Type_FloatArray:   return "float_array";
	case Type_BoolArray:    return "bool_array";
	case Type_StringArray:  return "string_array";
		
	default:
		TT_PANIC("Type '%d' not implemented", p_type);
		return "invalid";
	}
}


Attribute::Type Attribute::getTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < Type_Count; ++i)
	{
		const Type type = static_cast<Type>(i);
		if (p_name == getTypeName(type))
		{
			return type;
		}
	}
	
	return Type_Invalid;
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
