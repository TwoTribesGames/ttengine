#include <string>

#include <tt/script/helpers.h>

#include <toki/script/serialization/SQCache.h>

namespace toki {
namespace script {
namespace serialization {

//--------------------------------------------------------------------------------------------------
// Public member functions

SQCache::SQCache(HSQUIRRELVM p_vm)
{
	tt::script::SqTopRestorerHelper tmp(p_vm, true);
	
	sq_pushroottable(p_vm);
	
	HSQOBJECT rootTable;
	sq_getstackobj(p_vm, -1, &rootTable);
	
	// Cache root
	buildCache(p_vm, "");
	
	// Cache usertypes
	UserType::buildCache(m_userTypeCache);
	
	sq_poptop(p_vm);
}


UserType::Type SQCache::getUserType(SQUserPointer p_ptr) const
{
	UserType::Cache::const_iterator it = m_userTypeCache.find(p_ptr);
	
	if (it == m_userTypeCache.end())
	{
		return UserType::Type_Invalid;
	}
	
	return(*it).second;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

// Builds cache of the object on stack
void SQCache::buildCache(HSQUIRRELVM p_vm, const std::string& p_namespace)
{
	tt::script::SqTopRestorerHelper tmp(p_vm, true);
	SQObjectType type = sq_gettype(p_vm, -1);
	TT_ASSERT(type == OT_TABLE || type == OT_CLASS);
	
	HSQOBJECT object;
	sq_getstackobj(p_vm, -1, &object);
	
	sq_pushnull(p_vm); //null iterator
	while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
	{
		SQObjectType valueType = sq_gettype(p_vm, -1);
		if (valueType == OT_NATIVECLOSURE ||
		   (valueType != OT_CLASS && valueType != OT_TABLE && valueType != OT_ARRAY && isCachedMemberType(valueType) == false))
		{
			// Not an object of interest, continue with next iteration
			sq_pop(p_vm, 2);
			continue;
		}
		
		HSQOBJECT key;
		sq_getstackobj(p_vm, -2, &key);
		HSQOBJECT value;
		sq_getstackobj(p_vm, -1, &value);
		
		TT_ASSERTMSG(sq_gettype(p_vm, -2) == OT_STRING, "Key should be of type OT_STRING but is %s",
			tt::script::sqObjectTypeName(sq_gettype(p_vm, -2)));
		const SQChar* keyPtr;
		if (SQ_FAILED(sq_getstring(p_vm, -2, &keyPtr)))
		{
			TT_PANIC("sq_getstring failed!");
			keyPtr = "";
		}
		const std::string keyName(keyPtr);
		
		if (valueType == OT_CLASS)
		{
			if (m_classCache.contains(value._unVal.pClass) == false)
			{
				const std::string name(p_namespace + keyName);
				m_classCache.add(value._unVal.pClass, ClassDefinition(name));
				m_inverseClassCache.add(name, value);
			}
		}
		else if (valueType == OT_CLOSURE)
		{
			if (m_closureCache.contains(value._unVal.pClosure) == false)
			{
				const std::string name(p_namespace + keyName);
				m_closureCache.add(value._unVal.pClosure, name);
				m_inverseClosureCache.add(name, value);
			}
		}
		
		if (valueType == OT_CLASS || valueType == OT_TABLE)
		{
			buildCache(p_vm, p_namespace + keyName + ".");
		}
		
		// Continue with next element
		sq_pop(p_vm, 2);
	}
	
	// Pop null iterator
	sq_poptop(p_vm);
}


// Namespace end
}
}
}
