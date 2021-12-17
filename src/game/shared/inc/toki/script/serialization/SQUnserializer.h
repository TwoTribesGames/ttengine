#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_UNSQSERIALIZER_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_UNSQSERIALIZER_H

#include <vector>
#include <string>

#include <squirrel/squirrel.h>

#include <tt/code/fwd.h>

#include <toki/script/serialization/fwd.h>

namespace toki {
namespace script {
namespace serialization {

class SQUnserializer
{
public:
	inline SQUnserializer(HSQUIRRELVM p_vm, const SQCachePtr& p_cache)
	:
	m_vm(p_vm),
	m_cache(p_cache),
	m_objectsCreated(false)
	{
	}
	
	~SQUnserializer();
	
	void unserialize(tt::code::BufferReadContext* p_context);
	
	HSQOBJECT unserializeObject(tt::code::BufferReadContext* p_context) const;
	
	inline HSQUIRRELVM getVM() const { return m_vm; }
	
private:
	class Reference
	{
	public:
		Reference(const HSQOBJECT& p_object, const ProcessedObject& p_key, const ProcessedObject& p_value)
		:
		m_object(p_object),
		m_key(p_key),
		m_value(p_value)
		{
			TT_ASSERT(m_key.isIndexedType() || m_value.isIndexedType());
		}
		
		void createReference(const SQUnserializer& p_unserializer) const;
		
	private:
		HSQOBJECT       m_object;
		ProcessedObject m_key;
		ProcessedObject m_value;
	};
	
	typedef std::vector<Reference> References;
	typedef std::vector<HSQOBJECT> HSQOBJECTS;
	
	ProcessedObject unserializeProcessedObject(tt::code::BufferReadContext* p_context) const;
	void pushOnStack(const ProcessedObject& p_object) const;
	void setSlotOfStackObject(const ProcessedObject& p_key, const ProcessedObject& p_value) const;
	void removeSquirrelReferences(HSQOBJECTS& p_objects);
	
	
	HSQUIRRELVM m_vm;
	SQCachePtr  m_cache;
	
	HSQOBJECTS  m_strings;
	HSQOBJECTS  m_classes;
	HSQOBJECTS  m_closures;
	HSQOBJECTS  m_arrays;
	HSQOBJECTS  m_instances;
	HSQOBJECTS  m_tables;
	
	bool        m_objectsCreated;
	References  m_references;
};


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_UNSQSERIALIZER_H)
