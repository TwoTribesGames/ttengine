#include <tt/code/bufferutils.h>
#include <tt/math/Vector2.h>
#include <tt/str/str.h>
#include <tt/script/ScriptObject.h>

#include <toki/script/serialization/SQCache.h>
#include <toki/script/serialization/SQSerializer.h>

namespace toki {
namespace script {
namespace serialization {

//--------------------------------------------------------------------------------------------------
// Public member functions

void SQSerializer::processRoottable(HSQUIRRELVM p_vm)
{
	TT_ASSERTMSG(m_rootTableIndex == -1, "Roottable should only be processed once");
	
	sq_pushroottable(p_vm);
	
	HSQOBJECT table;
	sq_getstackobj(p_vm, -1, &table);
	
	ProcessedObject processedTable =
		addObject<SQTable*, SQTableData>(p_vm, table, table._unVal.pTable, m_tables, 2048);
	
	m_rootTableIndex = processedTable.getIndex();
	TT_ASSERT(m_rootTableIndex >= 0);
	
	sq_poptop(p_vm);
}


void SQSerializer::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	// Serialize strings
	{
		const s32 size = static_cast<s32>(m_strings.size());
		bu::put(size, p_context);
		
		for (s32 i = 0; i < size; ++i)
		{
			bu::put(m_strings[i], p_context);
		}
	}
	
	// Serialize used classes
	{
		const s32 size = static_cast<s32>(m_classes.size());
		bu::put(size, p_context);
		
		for (s32 i = 0; i < size; ++i)
		{
			bu::put(m_classes[i], p_context);
		}
	}
	
	// Serialize used closures
	{
		const s32 size = static_cast<s32>(m_closures.size());
		bu::put(size, p_context);
		
		for (s32 i = 0; i < size; ++i)
		{
			bu::put(m_closures[i], p_context);
		}
	}
	
	// Serialize arrays
	{
		const s32 size = static_cast<s32>(m_arrays.size());
		bu::put(size, p_context);
		
		for (s32 i = 0; i < size; ++i)
		{
			const SQArrayData& data = m_arrays[i];
			
			bu::put(static_cast<s32>(data.entries.size()), p_context);
			
			// Iterate through all members
			for (SQArrayData::Entries::const_iterator it = 
				data.entries.begin(); it != data.entries.end(); ++it)
			{
				serializeProcessedObject(*it, p_context);
			}
		}
	}
	
	// Serialize tables
	{
		const s32 size = static_cast<s32>(m_tables.size());
		
		// Roottable index
		TT_ASSERT(m_rootTableIndex >= 0 && m_rootTableIndex < size);
		bu::put(m_rootTableIndex, p_context);
		
		bu::put(size, p_context);
		
		for (s32 i = 0; i < size; ++i)
		{
			const SQTableData& data = m_tables[i];
			
			bu::put(static_cast<s32>(data.entries.size()), p_context);
			
			for (SQTableData::Entries::const_iterator it = 
				data.entries.begin(); it != data.entries.end(); ++it)
			{
				serializeProcessedObject((*it).first, p_context);
				serializeProcessedObject((*it).second, p_context);
			}
		}
	}
	
	// Serialize instances
	{
		const s32 size = static_cast<s32>(m_instances.size());
		bu::put(size, p_context);
		
		for (s32 i = 0; i < size; ++i)
		{
			const SQInstanceData& data = m_instances[i];
			
			bu::putEnum<s32>(data.userType, p_context);
			
			if (data.userType != UserType::Type_None)
			{
				UserType::serialize(*this, data.userType, data.userPtr, p_context);
			}
			else
			{
				bu::put(data.classID, p_context);
			}
			
			// Serialize all members
			bu::put(static_cast<s32>(data.members.size()), p_context);
			for (SQInstanceData::Members::const_iterator it = 
				data.members.begin(); it != data.members.end(); ++it)
			{
				// Serialize key
				serializeProcessedObject((*it).first, p_context);
				
				// Serialize value
				serializeProcessedObject((*it).second, p_context);
			}
		}
	}
}


ProcessedObject SQSerializer::addObject(HSQUIRRELVM p_vm, const HSQOBJECT& p_object, s32 p_reservedMembers)
{
	// Check for early out if this object has already been processed
	{
		ProcessedObject result(getProcessedObject(p_object));
		if (result.isValid())
		{
			return result;
		}
	}
	
	s32 idx = -1;
	switch (p_object._type)
	{
	case OT_ARRAY:
		return addObject<SQArray*, SQArrayData>(p_vm, p_object, p_object._unVal.pArray, m_arrays, p_reservedMembers);
		
	case OT_INSTANCE:
		return addObject<SQInstance*, SQInstanceData>(p_vm, p_object, p_object._unVal.pInstance, m_instances, p_reservedMembers);
		
	case OT_TABLE:
		return addObject<SQTable*, SQTableData>(p_vm, p_object, p_object._unVal.pTable, m_tables, p_reservedMembers);
		
	case OT_CLASS:
		{
			SQClass* ptr = p_object._unVal.pClass;
			idx = m_classes.indexOf(ptr);
			TT_ASSERT(idx < 0);
			if (idx < 0)
			{
				const SQCache::ClassCache& classCache = m_cache->getClassCache();
				idx = classCache.indexOf(ptr);
				TT_ASSERT(idx >= 0);
				idx = m_classes.add(ptr, classCache[idx].name);
			}
		}
		break;
		
	case OT_CLOSURE:
		{
			SQClosure* ptr = p_object._unVal.pClosure;
			idx = m_closures.indexOf(ptr);
			TT_ASSERT(idx < 0);
			if (idx < 0)
			{
				const SQCache::ClosureCache& closureCache = m_cache->getClosureCache();
				idx = closureCache.indexOf(ptr);
				TT_ASSERT(idx >= 0);
				idx = m_closures.add(ptr, closureCache[idx]);
			}
		}
		break;
		
	case OT_STRING:
		{
			SQString* ptr = p_object._unVal.pString;
			idx = m_strings.indexOf(ptr);
			TT_ASSERT(idx < 0);
			if (idx < 0)
			{
				sq_pushobject(p_vm, p_object);
				const SQChar* str;
				if (SQ_FAILED(sq_getstring(p_vm, -1, &str)))
				{
					TT_PANIC("sq_getstring failed");
					str = "";
				}
				sq_poptop(p_vm);
				idx = m_strings.add(ptr, std::string(str));
			}
		}
		break;
		
	case OT_WEAKREF:
		{
			SQWeakRef* ptr = p_object._unVal.pWeakRef;
			TT_NULL_ASSERT(ptr);
			if (ptr == 0)
			{
				return ProcessedObject();
			}
			
			// Push the weak reference on the stack and reference it
			sq_pushobject(p_vm, p_object);
			if (SQ_FAILED(sq_getweakrefval(p_vm, -1)))
			{
				sq_poptop(p_vm);
				return ProcessedObject();
			}
			
			if (sq_gettype(p_vm, -1) == OT_NULL)
			{
				sq_poptop(p_vm);
				return ProcessedObject::createNull();
			}
			
			HSQOBJECT refObject;
			sq_getstackobj(p_vm, -1, &refObject);
			sq_pop(p_vm, 2); // No need to keep things on the stack
			
			ProcessedObject processedRefObject = addObject(p_vm, refObject, p_reservedMembers);
			
			return ProcessedObject::createWeakRef(processedRefObject.getType(), processedRefObject.getIndex());
		}
		
	default:
		TT_PANIC("Unhandled type '%s'", tt::script::sqObjectTypeName(p_object._type));
	}
	
	return ProcessedObject::createIndexed(p_object._type, idx);
}


ProcessedObject SQSerializer::getProcessedObject(const HSQOBJECT& p_object) const
{
	switch (p_object._type)
	{
	case OT_ARRAY:
		return ProcessedObject::createIndexed(OT_ARRAY, m_arrays.indexOf(p_object._unVal.pArray));
		
	case OT_BOOL:
		return ProcessedObject::createBool(p_object._unVal.nInteger != 0);
		
	case OT_CLASS:
		return ProcessedObject::createIndexed(OT_CLASS, m_classes.indexOf(p_object._unVal.pClass));
		
	case OT_CLOSURE:
		return ProcessedObject::createIndexed(OT_CLOSURE, m_closures.indexOf(p_object._unVal.pClosure));
		
	case OT_FLOAT:
		return ProcessedObject::createFloat(p_object._unVal.fFloat);
		
	case OT_INSTANCE:
		return ProcessedObject::createIndexed(OT_INSTANCE, m_instances.indexOf(p_object._unVal.pInstance));
		
	case OT_INTEGER:
		return ProcessedObject::createInteger(p_object._unVal.nInteger);
		
	case OT_NULL:
		return ProcessedObject::createNull();
		
	case OT_STRING:
		return ProcessedObject::createIndexed(OT_STRING, m_strings.indexOf(p_object._unVal.pString));
		
	case OT_TABLE:
		return ProcessedObject::createIndexed(OT_TABLE, m_tables.indexOf(p_object._unVal.pTable));
		
	case OT_WEAKREF:
		{
			HSQOBJECT refObject = sq_getrealval(&p_object);
			switch (refObject._type)
			{
			case OT_NULL:
				return ProcessedObject::createNull();
				
			case OT_ARRAY:
				return ProcessedObject::createWeakRef(OT_ARRAY, m_arrays.indexOf(refObject._unVal.pArray));
				
			case OT_INSTANCE:
				return ProcessedObject::createWeakRef(OT_INSTANCE, m_instances.indexOf(refObject._unVal.pInstance));
				
			case OT_CLASS:
				return ProcessedObject::createWeakRef(OT_CLASS, m_classes.indexOf(refObject._unVal.pClass));
				
			case OT_TABLE:
				return ProcessedObject::createWeakRef(OT_TABLE, m_tables.indexOf(refObject._unVal.pTable));
				
			case OT_STRING:
				return ProcessedObject::createWeakRef(OT_STRING, m_strings.indexOf(p_object._unVal.pString));
				
			default:
				TT_PANIC("Unhandled type weakref '%s'", tt::script::sqObjectTypeName(refObject._type));
				break;
			}
			return ProcessedObject();
		}
		
	default:
		TT_PANIC("Unhandled type '%s'", tt::script::sqObjectTypeName(p_object._type));
	}
	
	return ProcessedObject();
}


void SQSerializer::serializeProcessedObject(const ProcessedObject& p_object,
                                            tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u32>(p_object.getType(), p_context);
	
	switch (p_object.getType())
	{
	case OT_BOOL:
		bu::put(p_object.getBool(), p_context);
		break;
		
	case OT_FLOAT:
		bu::put(p_object.getFloat(), p_context);
		break;
		
	case OT_INTEGER:
		bu::put<s32>(static_cast<s32>(p_object.getInteger()), p_context);
		break;
		
	case OT_NULL:
		// Don't serialize anything
		break;
		
	case OT_ARRAY:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < m_arrays.size());
		bu::put(p_object.isWeakRef(), p_context);
		bu::put(p_object.getIndex(), p_context);
		break;
		
	case OT_INSTANCE:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < m_instances.size());
		bu::put(p_object.isWeakRef(), p_context);
		bu::put(p_object.getIndex(), p_context);
		break;
		
	case OT_TABLE:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < m_tables.size());
		bu::put(p_object.isWeakRef(), p_context);
		bu::put(p_object.getIndex(), p_context);
		break;
		
	case OT_CLASS:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < m_classes.size());
		bu::put(p_object.isWeakRef(), p_context);
		bu::put(p_object.getIndex(), p_context);
		break;
		
	case OT_CLOSURE:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < m_closures.size());
		bu::put(p_object.getIndex(), p_context);
		break;
		
	case OT_STRING:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < m_strings.size());
		bu::put(p_object.getIndex(), p_context);
		break;
		
	default:
		TT_PANIC("Unhandled type '%s'", tt::script::sqObjectTypeName(p_object.getType()));
		break;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

template <typename Type, typename Data>
ProcessedObject SQSerializer::addObject(HSQUIRRELVM p_vm, const HSQOBJECT& p_object, Type p_objectPtr,
                                        Registry<Type, Data>& p_registry, s32 p_reservedMembers)
{
#if !defined(TT_BUILD_FINAL)
	tt::script::SqTopRestorerHelper helper(p_vm, true);
#endif
	{
		// Check for early out (simple types and cached complex types)
		ProcessedObject result(getProcessedObject(p_object));
		if (result.isValid())
		{
			return result;
		}
	}
	
	// Push object on stack
	sq_pushobject(p_vm, p_object);
	SQObjectType type = sq_gettype(p_vm, -1);
	
	TT_ASSERT(type != OT_NATIVECLOSURE);
	
	Data data;
	s32 idx = p_registry.add(p_objectPtr);
	
	// Push object on stack
	if (type == OT_INSTANCE)
	{
		// Set user pointer
		SQUserPointer userPtr;
		sq_getinstanceup(p_vm, -1, &userPtr, 0);
		data.setUserPointer(userPtr);
		
		// Push class of this instance on stack
		sq_getclass(p_vm, -1);
		
		// Get classobject
		HSQOBJECT classObject;
		sq_getstackobj(p_vm, -1, &classObject);
		SQClass* classPtr = classObject._unVal.pClass;
		
		// Don't store ID to registry containing all classes, only store ID to registry containing all used classes
		const SQCache::ClassCache& classCache = m_cache->getClassCache();
		s32 classID = classCache.indexOf(classPtr);
		TT_ASSERT(classID >= 0);
		s32 usedID = m_classes.indexOf(classPtr);
		if (usedID < 0)
		{
			usedID = m_classes.add(classPtr, classCache[classID].name);
		}
		data.setClassID(usedID);
		
		// Set user type from instance (now at stackposition -2)
		SQUserPointer typeTag;
		sq_gettypetag(p_vm, -2, &typeTag);
		
		UserType::Type userType = UserType::Type_None;
		if (typeTag != 0)
		{
			userType = m_cache->getUserType(typeTag);
			
			TT_ASSERTMSG(userType != UserType::Type_Invalid,
				"Class '%s' cannot be found in the userType cache. Looking for typetag %p",
				classCache[classID].name.c_str(), typeTag);
		}
		data.setUserType(userType);
	}
	
	// Serialize object members
	using ScriptObjectPairs = std::vector<std::pair<HSQOBJECT, HSQOBJECT>> ;
	ScriptObjectPairs members; // Members are stored here and are added at the end. (When we no longer have anything on sq stack.)
	members.reserve(p_reservedMembers);
	
	sq_pushnull(p_vm);
	while(SQ_SUCCEEDED(sq_next_getweakrefs(p_vm, -2)))
	{
		if (type == OT_INSTANCE)
		{
			// Since we're iterating through the class, the class defaults are currently the value
			// replace with actual value of the instance
			
			// pop class value (default) as it is not used
			sq_poptop(p_vm);
			
			// In instances, the key (member name) should always be a string
			TT_ASSERT(sq_gettype(p_vm, -1) == OT_STRING);
			
			// Duplicate key, because sq_get will consume it, and the code below assumes that 
			// a key/value pair in on the stack
			sq_push(p_vm, -1);
			
			// Check for this instance if the member "is field"
			// If it's not a field then it's stored in the class not the instance. (e.g. static, methode, etc.)
			// We don't need to store those for per instance.
			if (sq_isfield(p_vm, -5))
			{
				// Push value from instance on stack
				SQRESULT result = sq_get_getweakrefs(p_vm, -5);
				
				TT_ASSERT(SQ_SUCCEEDED(result));
			}
			else
			{
				sq_pop(p_vm, 2); // Clean up so the iterator is on top and we can start the next iteration.
				continue;
			}
		}
		
		//here -1 is the value and -2 is the key
		SQObjectType valueType = sq_gettype(p_vm, -1);
		SQObjectType keyType   = sq_gettype(p_vm, -2);
		
		// Skip native closures
		if (valueType == OT_NATIVECLOSURE)
		{
			// Native closures should only occur in roottae
			TT_ASSERT(m_rootTableIndex < 0);
			// Pop key and val before the next iteration
			sq_pop(p_vm, 2);
			continue;
		}
		else if (valueType == OT_TABLE && keyType == OT_STRING)
		{
			const SQChar* str;
			if (SQ_FAILED(sq_getstring(p_vm, -2, &str)))
			{
				TT_PANIC("sq_getstring failed");
				str = "";
			}
			if (strncmp(str, "__", 2) == 0)
			{
				// Pop key and val before the next iteration
				sq_pop(p_vm, 2);
				continue;
			}
		}
		
		// Check if this member was already cached (and should therefore not be serialized)
		HSQOBJECT key;
		HSQOBJECT value;
		sq_getstackobj(p_vm, -2, &key);
		sq_getstackobj(p_vm, -1, &value);
		members.emplace_back(key, value);
		
		// Pop key and val
		sq_pop(p_vm, 2);
	}
	
	if (type == OT_INSTANCE)
	{
		// Pop null-iterator, class and object
		sq_pop(p_vm, 3);
	}
	else
	{
		// Pop null-iterator and object
		sq_pop(p_vm, 2);
	}
	
#if !defined(TT_BUILD_FINAL)
	helper.restoreTop(); // Make sure sq stack usage is zero for this function.
#endif
	
	TT_ASSERTMSG(static_cast<s32>(members.size()) <= p_reservedMembers, "Reserved members '%d' not sufficient for type '%s'",
	             p_reservedMembers, tt::script::sqObjectTypeName(type));
	
	for (ScriptObjectPairs::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		// Store key/value
		data.add(addObject(p_vm, it->first , p_reservedMembers), addObject(p_vm, it->second, p_reservedMembers));
	}
	
	// Store table data
	p_registry[idx] = data;
	
	// Return result
	return ProcessedObject::createIndexed(type, idx);
}


// Namespace end
}
}
}
