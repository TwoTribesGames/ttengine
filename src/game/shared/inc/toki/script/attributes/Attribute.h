#if !defined(INC_TOKI_SCRIPT_ATTRIBUTES_ATTRIBUTE_H)
#define INC_TOKI_SCRIPT_ATTRIBUTES_ATTRIBUTE_H


#include <vector>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/str/str.h>


namespace toki {
namespace script {
namespace attributes {

class Attribute
{
public:
	enum Type
	{
		Type_None,
		
		Type_Null,
		Type_Integer,
		Type_Float,
		Type_Bool,
		Type_String,
		Type_IntegerArray,
		Type_FloatArray,
		Type_BoolArray,
		Type_StringArray,
		
		Type_Count,
		Type_Invalid
	};
	
	typedef std::vector<s32>  IntegerArray;
	typedef std::vector<real> FloatArray;
	typedef std::vector<bool> BoolArray;
	typedef tt::str::Strings  StringArray;
	
	
	Attribute(const std::string& p_name, const tt::str::Strings& p_value, const std::string& p_type);
	Attribute();
	
	inline const std::string& getName() const { return m_name;              }
	inline Type               getType() const { return m_type;              }
	inline bool               isNull()  const { return m_type == Type_Null; }
	
	static inline bool isArrayType(Type p_type)
	{
		return p_type == Type_IntegerArray || p_type == Type_FloatArray ||
		       p_type == Type_BoolArray    || p_type == Type_StringArray;
	}
	
	void setValue(const tt::str::Strings& p_value);
	void setValue(const std::string&      p_value);
	
	inline s32  getInteger() const      { verifyType(Type_Integer); return m_valueInteger; }
	inline void setInteger(s32 p_value) { m_type = Type_Integer; m_valueInteger = p_value; }
	
	inline real getFloat() const       { verifyType(Type_Float); return m_valueFloat; }
	inline void setFloat(real p_value) { m_type = Type_Float; m_valueFloat = p_value; }
	
	inline bool getBool() const       { verifyType(Type_Bool); return m_valueBool; }
	inline void setBool(bool p_value) { m_type = Type_Bool; m_valueBool = p_value; }
	
	inline const std::string& getString() const       { verifyType(Type_String); return m_valueString; }
	inline void setString(const std::string& p_value) { m_type = Type_String; m_valueString = p_value; }
	
	inline const IntegerArray& getIntegerArray() const       { verifyType(Type_IntegerArray); return m_valueIntegerArray; }
	inline void setIntegerArray(const IntegerArray& p_value) { m_type = Type_IntegerArray; m_valueIntegerArray = p_value; }
	
	inline const FloatArray& getFloatArray() const       { verifyType(Type_FloatArray); return m_valueFloatArray; }
	inline void setFloatArray(const FloatArray& p_value) { m_type = Type_FloatArray; m_valueFloatArray = p_value; }
	
	inline const BoolArray& getBoolArray() const       { verifyType(Type_BoolArray); return m_valueBoolArray; }
	inline void setBoolArray(const BoolArray& p_value) { m_type = Type_BoolArray; m_valueBoolArray = p_value; }
	
	inline const StringArray& getStringArray() const       { verifyType(Type_StringArray); return m_valueStringArray; }
	inline void setStringArray(const StringArray& p_value) { m_type = Type_StringArray; m_valueStringArray = p_value; }
	
	// Returns the value as a string
	std::string getAsString() const;
	
	static inline bool isValidType(Type p_type) { return p_type >= 0 && p_type < Type_Count; }
	static const char* getTypeName(Type p_type);
	static Type        getTypeFromName(const std::string& p_name);
	
private:
	inline bool verifyType(Type p_expectedType) const
	{
		TT_ASSERTMSG(m_type == p_expectedType,
		             "Attribute '%s': Trying to get a value of type '%s', expected type '%s'.",
		             m_name.c_str(), getTypeName(m_type), getTypeName(p_expectedType));
		return m_type == p_expectedType;
	}
	
	
	std::string  m_name;
	Type         m_type;
	
	s32          m_valueInteger;
	real         m_valueFloat;
	bool         m_valueBool;
	std::string  m_valueString;
	IntegerArray m_valueIntegerArray;
	FloatArray   m_valueFloatArray;
	BoolArray    m_valueBoolArray;
	StringArray  m_valueStringArray;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_SCRIPT_ATTRIBUTES_ATTRIBUTE_H)
