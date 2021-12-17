#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_SQCACHE_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_SQCACHE_H

#include <set>
#include <string>

#include <squirrel/squirrel.h>

#include <toki/script/serialization/Registry.h>
#include <toki/script/serialization/UserType.h>

namespace toki {
namespace script {
namespace serialization {


class SQCache
{
public:
	struct ClassDefinition
	{
		ClassDefinition(const std::string& p_name)
		:
		name(p_name)
		{}
		
		std::string name;
	};
	typedef Registry<SQClass*, ClassDefinition>     ClassCache;
	typedef Registry<SQClosure*, std::string>       ClosureCache;
	typedef Registry<std::string, HSQOBJECT>        ObjectCache;
	
	SQCache(HSQUIRRELVM p_vm);
	
	static inline bool isCachedMemberType(SQObjectType p_type)
	{
		return p_type == OT_CLOSURE || p_type == OT_NATIVECLOSURE ||
		       p_type == OT_INTEGER || p_type == OT_FLOAT;
	}
	
	const ClassCache&         getClassCache() const         { return m_classCache; }
	const ClosureCache&       getClosureCache() const       { return m_closureCache; }
	
	const ObjectCache&        getInverseClassCache() const         { return m_inverseClassCache; }
	const ObjectCache&        getInverseClosureCache() const       { return m_inverseClosureCache; }
	
	// FIXME: Move this functionality to UserType?
	UserType::Type getUserType(SQUserPointer p_ptr) const;
	
private:
	void buildCache(HSQUIRRELVM p_vm, const std::string& p_namespace);
	
	ClassCache         m_classCache;
	ClosureCache       m_closureCache;
	ObjectCache        m_inverseClassCache;
	ObjectCache        m_inverseClosureCache;
	UserType::Cache    m_userTypeCache;
};


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_SQCACHE_H)
