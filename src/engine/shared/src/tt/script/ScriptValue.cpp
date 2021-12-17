#include <tt/script/ScriptValue.h>

#include <tt/code/fwd.h>
#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/script/helpers.h>
#include <tt/str/str.h>


namespace tt {
namespace script {

//--------------------------------------------------------------------------------------------------
// Public member functions


ScriptValue::ScriptValue()
:
m_type(OT_NULL),
m_simpleValue(),
m_array(),
m_table()
{
}


ScriptValue ScriptValue::create(HSQUIRRELVM p_vm, s32 p_idx)
{
	// Validate if value is of correct type
	if (isValidValue(sq_gettype(p_vm, p_idx)))
	{
		return getValueFromStack(p_vm, p_idx);
	}
	
	return ScriptValue();
}


ScriptValue ScriptValue::createString(const std::string& p_value)
{
	ScriptValue value;
	value.m_type   = OT_STRING;
	value.m_string = p_value;
	return value;
}


ScriptValue ScriptValue::createTable()
{
	ScriptValue value;
	value.m_type   = OT_TABLE;
	return value;
}


void ScriptValue::pushOnStack(HSQUIRRELVM p_vm) const
{
	switch (m_type)
	{
	case OT_NULL:
		sq_pushnull(p_vm);
		return;
		
	case OT_BOOL:
		sq_pushbool(p_vm, m_simpleValue.m_bool);
		return;
		
	case OT_FLOAT:
		sq_pushfloat(p_vm, m_simpleValue.m_float);
		return;
		
	case OT_INTEGER:
		sq_pushinteger(p_vm, m_simpleValue.m_integer);
		return;
		
	case OT_STRING:
		sq_pushstring(p_vm, m_string.c_str(), -1);
		return;
		
	default:
		// Handle other types outside the switch
		break;
	}
	
	if (m_type != OT_ARRAY && m_type != OT_TABLE)
	{
		TT_PANIC("Value type '%s' is not a valid registry value type",
		         sqObjectTypeName(m_type));
		sq_pushnull(p_vm);
		return;
	}
	
	if (m_type == OT_ARRAY)
	{
		sq_newarray(p_vm, static_cast<SQInteger>(m_array.size()));
		SQInteger count = 0;
		for (ArrayValue::const_iterator it = m_array.begin(); it != m_array.end(); ++it, ++count)
		{
			// Push 'key'/value
			sq_pushinteger(p_vm, count);
			(*it).pushOnStack(p_vm);
			
			if (SQ_FAILED(sq_set(p_vm, -3)))
			{
				TT_PANIC("sq_set failed while trying to set a registry value in an array at index '%d'", count);
				sq_pop(p_vm, 2);
			}
		}
	}
	else //if (value.m_type == OT_TABLE)
	{
		sq_newtable(p_vm);
		SQInteger count = 0;
		for (TableValue::const_iterator it = m_table.begin(); it != m_table.end(); ++it, ++count)
		{
			// Push key/value
			(*it).first.pushOnStack(p_vm);
			(*it).second.pushOnStack(p_vm);
			
			if (SQ_FAILED(sq_newslot(p_vm, -3, SQFalse)))
			{
				TT_PANIC("sq_set failed while trying to set a registry value in a table at index '%d'", count);
				sq_pop(p_vm, 2);
			}
		}
	}
}


void ScriptValue::serialize(code::BufferWriteContext* p_context) const
{
	namespace bu = code::bufferutils;
	
	bu::putEnum<u32>(m_type, p_context);
	
	switch (m_type)
	{
	case OT_NULL:
		return;
		
	case OT_BOOL:
		bu::put<u32>(static_cast<u32>(m_simpleValue.m_bool), p_context);
		return;
		
	case OT_FLOAT:
		bu::put(m_simpleValue.m_float, p_context);
		return;
		
	case OT_INTEGER:
		bu::put<s32>(static_cast<s32>(m_simpleValue.m_integer), p_context);
		return;
		
	case OT_STRING:
		bu::put(m_string, p_context);
		return;
		
	default:
		// Handle other types outside the switch
		break;
	}
	
	if (m_type != OT_ARRAY && m_type != OT_TABLE)
	{
		TT_PANIC("Value type '%s' is not a valid registry value type",
		         sqObjectTypeName(m_type));
		return;
	}
	
	if (m_type == OT_ARRAY)
	{
		const u32 entriesCount = static_cast<u32>(m_array.size());
		bu::put(entriesCount, p_context);
		
		for (ArrayValue::const_iterator it = m_array.begin(); it != m_array.end(); ++it)
		{
			(*it).serialize(p_context);
		}
	}
	else //if (value.m_type == OT_TABLE)
	{
		const u32 entriesCount = static_cast<u32>(m_table.size());
		bu::put(entriesCount, p_context);
		
		for (TableValue::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
		{
			(*it).first.serialize(p_context);
			(*it).second.serialize(p_context);
		}
	}
}


ScriptValue ScriptValue::unserialize(code::BufferReadContext* p_context)
{
	ScriptValue result;
	
	namespace bu = code::bufferutils;
	
	result.m_type = bu::getEnum<u32, SQObjectType>(p_context);
	
	switch (result.m_type)
	{
	case OT_NULL:
		return result;
		
	case OT_BOOL:
		result.m_simpleValue.m_bool = bu::get<u32>(p_context);
		return result;
		
	case OT_FLOAT:
		result.m_simpleValue.m_float = bu::get<SQFloat>(p_context);
		return result;
		
	case OT_INTEGER:
		result.m_simpleValue.m_integer = bu::get<s32>(p_context);
		return result;
		
	case OT_STRING:
		result.m_string = bu::get<std::string>(p_context);
		return result;
		
	default:
		// Handle other types outside the switch
		break;
	}
	
	if (result.m_type != OT_ARRAY && result.m_type != OT_TABLE)
	{
		TT_PANIC("Result type '%s' is not a valid registry value type",
		         sqObjectTypeName(result.m_type));
		return ScriptValue();
	}
	
	if (result.m_type == OT_ARRAY)
	{
		const u32 count = bu::get<u32>(p_context);
		result.m_array.reserve(count);
		
		for (u32 i = 0; i < count; ++i)
		{
			result.m_array.push_back(ScriptValue::unserialize(p_context));
		}
	}
	else //if (value.m_type == OT_TABLE)
	{
		const u32 count = bu::get<u32>(p_context);
		result.m_table.reserve(count);
		
		for (u32 i = 0; i < count; ++i)
		{
			const ScriptValue key  (ScriptValue::unserialize(p_context));
			const ScriptValue value(ScriptValue::unserialize(p_context));
			result.m_table.emplace_back(key, value);
		}
	}
	
	return result;
}


std::string ScriptValue::toString() const
{
	switch (m_type)
	{
	case OT_NULL:
		return "(null)";
		
	case OT_BOOL:
		return str::toStr(m_simpleValue.m_bool == SQTrue);
		
	case OT_FLOAT:
		return str::toStr(m_simpleValue.m_float);
		
	case OT_INTEGER:
		return str::toStr(m_simpleValue.m_integer);
		
	case OT_STRING:
		return m_string;
		
	default:
		// Handle other types outside the switch
		break;
	}
	
	if (m_type != OT_ARRAY && m_type != OT_TABLE)
	{
		TT_PANIC("Value type '%s' is not a valid registry value type",
		         sqObjectTypeName(m_type));
		return "";
	}
	
	if (m_type == OT_ARRAY)
	{
		std::string result("[");
		for (ArrayValue::const_iterator it = m_array.begin(); it != m_array.end(); ++it)
		{
			result += (*it).toString();
			
			if (it != m_array.end())
			{
				result += ", ";
			}
		}
		
		result += "]";
		return result;
	}
	else //if (value.m_type == OT_TABLE)
	{
		static s32 indentLevel = 0;
		std::string indent;
		
		for (s32 i = 0; i < indentLevel; ++i)
		{
			indent += "\t";
		}
		++indentLevel;
		std::string result(indent + "{\n");
		for (TableValue::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
		{
			result += indent + "\t";
			result += (*it).first.toString() + " ->";
			
			const SQObjectType valueType = (*it).second.m_type;
			switch (valueType)
			{
			case OT_STRING:
				result += " \"" + (*it).second.toString() + "\"\n";
				break;
				
			case OT_TABLE:
				result += "\n" + (*it).second.toString();
				break;
				
			default:
				result += " " + (*it).second.toString() + "\n";
				break;
			}
		}
		--indentLevel;
		
		result += indent + "}\n";
		return result;
	}
}


const ScriptValue& ScriptValue::findInTable(const std::string& p_key) const
{
	if (m_type != OT_TABLE)
	{
		TT_PANIC("Can't find value in a ScriptValue which isn't a table! Key: '%s'", p_key.c_str());
		return getEmptyScriptValueConst();
	}
	
	for (TableValue::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
	{
		const ScriptValue& key   = (*it).first;
		
		if (key.m_type   == OT_STRING &&
		    key.m_string == p_key)
		{
			return (*it).second;
		}
	}
	
	return getEmptyScriptValueConst();
}


ScriptValue& ScriptValue::findInTable(const std::string& p_key)
{
	if (m_type != OT_TABLE)
	{
		TT_PANIC("Can't find value in a ScriptValue which isn't a table! Key: '%s'", p_key.c_str());
		return getEmptyScriptValue();
	}
	
	for (TableValue::iterator it = m_table.begin(); it != m_table.end(); ++it)
	{
		const ScriptValue& key   = (*it).first;
		
		if (key.m_type   == OT_STRING &&
		    key.m_string == p_key)
		{
			return (*it).second;
		}
	}
	
	return getEmptyScriptValue();
}


bool ScriptValue::setInTable(const std::string& p_key, const ScriptValue& p_value)
{
	if (m_type == OT_NULL) // Null may be upgraded to table.
	{
		m_type = OT_TABLE;
	}
	
	if (m_type != OT_TABLE)
	{
		TT_PANIC("Can't set value in a ScriptValue which isn't a table! Key: '%s'", p_key.c_str());
		return false;
	}
	
	for (TableValue::iterator it = m_table.begin(); it != m_table.end(); ++it)
	{
		const ScriptValue& key   = (*it).first;
		
		if (key.m_type   == OT_STRING &&
		    key.m_string == p_key)
		{
			(*it).second = p_value;
			return true;
		}
	}
	
	m_table.emplace_back(createString(p_key), p_value);
	return true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ScriptValue ScriptValue::getValueFromStack(HSQUIRRELVM p_vm, s32 p_idx)
{
	SqTopRestorerHelper helper(p_vm, true);
	ScriptValue value;
	
	value.m_type = sq_gettype(p_vm, p_idx);
	
	switch (value.m_type)
	{
	case OT_NULL:
		return value;
		
	case OT_BOOL:
		sq_getbool(p_vm, p_idx, &value.m_simpleValue.m_bool);
		return value;
		
	case OT_FLOAT:
		sq_getfloat(p_vm, p_idx, &value.m_simpleValue.m_float);
		return value;
		
	case OT_INTEGER:
		sq_getinteger(p_vm, p_idx, &value.m_simpleValue.m_integer);
		return value;
		
	case OT_STRING:
		{
			const SQChar* strPtr = 0;
			if (SQ_FAILED(sq_getstring(p_vm, p_idx, &strPtr)))
			{
				TT_PANIC("getValueFromStack cannot retrieve string from stack");
				return ScriptValue();
			}
			value.m_string = std::string(strPtr);
			return value;
		}
		
	default:
		// Handle other types outside the switch
		break;
	}
	
	if (value.m_type != OT_ARRAY && value.m_type != OT_TABLE)
	{
		TT_PANIC("Value type '%s' is not a valid registry value type",
		         sqObjectTypeName(value.m_type));
		return ScriptValue();
	}
	
	// Iterate through members of array/table
	sq_pushnull(p_vm);
	while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
	{
		//here -1 is the value and -2 is the key
		if (value.m_type == OT_ARRAY)
		{
			// Only store value (-1)
			value.m_array.emplace_back(getValueFromStack(p_vm, -1));
		}
		else //if (value.m_type == OT_TABLE)
		{
			// Tables don't support null as key
			if (sq_gettype(p_vm, -2) == OT_NULL)
			{
				// Pop key/value and continue with next iteration
				TT_PANIC("Table contains 'null' as key. This should not happen");
				sq_pop(p_vm, 2);
				continue;
			}
			
			// Store key (-2) and value (-1)
			value.m_table.emplace_back(getValueFromStack(p_vm, -2), getValueFromStack(p_vm, -1));
		}
		
		// Pop key/value
		sq_pop(p_vm, 2);
	}
	
	// Pop null iterator
	sq_poptop(p_vm);
	
	return value;
}


// Namespace end
}
}
