#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_SQSERIALIZER_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_SQSERIALIZER_H

#include <set>

#include <tt/code/fwd.h>
#include <tt/script/VirtualMachine.h>
#include <toki/script/serialization/fwd.h>
#include <toki/script/serialization/ProcessedObject.h>
#include <toki/script/serialization/Registry.h>
#include <toki/script/serialization/SQData.h>
#include <toki/script/serialization/UserType.h>

namespace toki {
namespace script {
namespace serialization {

class SQSerializer
{
public:
	static const s32 reservedMembersCount = 512; // Optimization purposes. Increase this if you hit an assert
	
	inline SQSerializer(const SQCachePtr& p_cache)
	:
	m_cache(p_cache),
	m_rootTableIndex(-1)
	{
	}
	
	void processRoottable(HSQUIRRELVM p_vm);
	
	void serialize(tt::code::BufferWriteContext* p_context) const;
	
	ProcessedObject addObject(HSQUIRRELVM p_vm, const HSQOBJECT& p_object, s32 p_reservedMembers = reservedMembersCount);
	ProcessedObject getProcessedObject(const HSQOBJECT& p_object) const;
	void serializeProcessedObject(const ProcessedObject& p_object, tt::code::BufferWriteContext* p_context) const;
	
private:
	template <typename Type, typename Data>
	ProcessedObject addObject(HSQUIRRELVM p_vm, const HSQOBJECT& p_object, Type p_objectPtr,
	                              Registry<Type, Data>& p_registry, s32 p_reservedMembers);
	
	SQCachePtr                              m_cache;
	
	Registry<SQArray*, SQArrayData>         m_arrays;
	Registry<SQInstance*, SQInstanceData>   m_instances;
	Registry<SQTable*, SQTableData>         m_tables;
	Registry<SQString*, std::string>        m_strings;
	
	Registry<SQClass*, std::string>         m_classes;
	Registry<SQClosure*, std::string>       m_closures;
	
	s32                                     m_rootTableIndex;
};


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_SQSERIALIZER_H)
