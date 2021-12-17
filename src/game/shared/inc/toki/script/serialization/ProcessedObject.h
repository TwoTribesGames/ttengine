#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_PROCESSEDOBJECT_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_PROCESSEDOBJECT_H

#include <squirrel/squirrel.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace script {
namespace serialization {


class ProcessedObject
{
public:
	inline ProcessedObject()
	:
	m_type(static_cast<SQObjectType>(-1)),
	m_isValueValid(false),
	m_isWeakRef(false)
	{
	}
	
	static inline ProcessedObject createInteger(SQInteger p_integer)
	{
		return ProcessedObject(p_integer);
	}
	
	static inline ProcessedObject createFloat(SQFloat p_float)
	{
		return ProcessedObject(p_float);
	}
	
	static inline ProcessedObject createBool(bool p_bool)
	{
		return ProcessedObject(p_bool);
	}
	
	static inline ProcessedObject createIndexed(SQObjectType p_type, s32 p_index)
	{
		return ProcessedObject(p_type, p_index, false);
	}
	
	static inline ProcessedObject createWeakRef(SQObjectType p_type, s32 p_index)
	{
		return ProcessedObject(p_type, p_index, true);
	}
	
	static inline ProcessedObject createNull()
	{
		return ProcessedObject(OT_NULL);
	}
	
	inline SQObjectType getType() const { return m_type; }
	inline bool isValid() const { return m_isValueValid; }
	inline bool isWeakRef() const { return m_isWeakRef; }
	
	inline SQFloat getFloat() const             { TT_ASSERT(m_type == OT_FLOAT && m_isValueValid); return m_float; }
	
	inline SQInteger getInteger() const         { TT_ASSERT(m_type == OT_INTEGER && m_isValueValid); return m_integer; }
	
	inline bool getBool() const                 { TT_ASSERT(m_type == OT_BOOL && m_isValueValid); return m_bool; }
	
	inline s32 getIndex() const                 { TT_ASSERT(isIndexedType() && m_isValueValid); return m_index; }
	
	inline bool isIndexedType() const
	{
		return m_type == OT_ARRAY || m_type == OT_CLASS || m_type == OT_CLOSURE || m_type == OT_INSTANCE ||
		       m_type == OT_NATIVECLOSURE || m_type == OT_STRING || m_type == OT_TABLE;
	}
	
private:
	inline ProcessedObject(bool p_bool)
	:
	m_type(OT_BOOL),
	m_bool(p_bool),
	m_isValueValid(true),
	m_isWeakRef(false)
	{
	}
	
	inline ProcessedObject(SQInteger p_integer)
	:
	m_type(OT_INTEGER),
	m_integer(p_integer),
	m_isValueValid(true),
	m_isWeakRef(false)
	{
	}
	
	inline ProcessedObject(SQFloat p_float)
	:
	m_type(OT_FLOAT),
	m_float(p_float),
	m_isValueValid(true),
	m_isWeakRef(false)
	{
	}
	
	inline ProcessedObject(SQObjectType p_type, s32 p_index, bool p_isWeakRef)
	:
	m_type(p_type),
	m_index(p_index),
	m_isValueValid(p_index >= 0),
	m_isWeakRef(p_isWeakRef)
	{
		TT_ASSERT(isIndexedType());
	}
	
	inline ProcessedObject(SQObjectType p_type)
	:
	m_type(p_type),
	m_isValueValid(true),
	m_isWeakRef(false)
	{
		// Used for OT_NULL and potential other types with no value
	}
	
	SQObjectType m_type;
	union
	{
		SQFloat   m_float;
		SQInteger m_integer;
		bool      m_bool;
		s32       m_index;
	};
	bool m_isValueValid;
	bool m_isWeakRef;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_PROCESSEDOBJECT_H)
