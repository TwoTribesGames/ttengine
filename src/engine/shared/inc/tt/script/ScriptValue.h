#if !defined(INC_TT_SCRIPT_SCRIPTVALUE_H)
#define INC_TT_SCRIPT_SCRIPTVALUE_H

#include <string>
#include <vector>

#include <squirrel/squirrel.h>

#include <tt/code/fwd.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace script {

class ScriptValue
{
public:
	ScriptValue();
	
	static ScriptValue create(HSQUIRRELVM p_vm, s32 p_idx);
	static ScriptValue createString(const std::string& p_value);
	static ScriptValue createTable();
	
	void pushOnStack(HSQUIRRELVM p_vm) const;
	
	void                 serialize(code::BufferWriteContext* p_context) const;
	static ScriptValue unserialize(code::BufferReadContext*  p_context);
	
	std::string toString() const;
	
	inline bool isNull() const { return m_type == OT_NULL; }
	const ScriptValue& findInTable(const std::string& p_key) const;
	      ScriptValue& findInTable(const std::string& p_key);
	bool setInTable(const std::string& p_key, const ScriptValue& p_value);
	
	static inline       ScriptValue& getEmptyScriptValue()      { static ScriptValue empty; TT_ASSERT(empty.isNull()); return empty; }
	static inline const ScriptValue& getEmptyScriptValueConst() { static ScriptValue empty;                            return empty; }
	
private:
	typedef std::vector<ScriptValue>                          ArrayValue;
	typedef std::string                                       StringValue;
	typedef std::vector<std::pair<ScriptValue, ScriptValue> > TableValue;
	
	static inline bool isValidValue(SQObjectType p_valueType)
	{
		return p_valueType == OT_ARRAY   || p_valueType == OT_BOOL || p_valueType == OT_FLOAT ||
		       p_valueType == OT_INTEGER || p_valueType == OT_NULL ||
		       p_valueType == OT_STRING  || p_valueType == OT_TABLE;
	}
	static ScriptValue getValueFromStack(HSQUIRRELVM p_vm, s32 p_idx);
	
	union SimpleValue
	{
		SQBool    m_bool;
		SQFloat   m_float;
		SQInteger m_integer;
	};
	SQObjectType m_type;
	
	SimpleValue  m_simpleValue;
	StringValue  m_string;
	ArrayValue   m_array;
	TableValue   m_table;
};


// Namespace end
}
}

#endif  // !defined(INC_TT_SCRIPT_SCRIPTVALUE_H)
