#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/script/helpers.h>
#include <tt/script/SqTopRestorerHelper.h>
#include <tt/script/utils.h>

#include <toki/script/serialization/SQCache.h>
#include <toki/script/serialization/SQData.h>
#include <toki/script/serialization/SQUnserializer.h>


namespace toki {
namespace script {
namespace serialization {

//--------------------------------------------------------------------------------------------------
// Public member functions

SQUnserializer::~SQUnserializer()
{
	removeSquirrelReferences(m_strings);
	removeSquirrelReferences(m_arrays);
	removeSquirrelReferences(m_tables);
	removeSquirrelReferences(m_instances);
}


void SQUnserializer::unserialize(tt::code::BufferReadContext* p_context)
{
	if (m_objectsCreated)
	{
		TT_PANIC("Objects already created, cannot unserialize again");
		return;
	}
	
	namespace bu = tt::code::bufferutils;
	tt::script::SqTopRestorerHelper helper(m_vm, true);
	
	HSQOBJECT object;
	
	// Unserialize strings
	{
		s32 size = bu::get<s32>(p_context);
		for (s32 i = 0; i < size; ++i)
		{
			const std::string value(bu::get<std::string>(p_context));
			sq_pushstring(m_vm, value.c_str(), -1);
			sq_getstackobj(m_vm, -1, &object);
			sq_addref(m_vm, &object);
			sq_poptop(m_vm);
			m_strings.emplace_back(object);
		}
	}
	
	// Unserialize used classes
	{
		s32 size = bu::get<s32>(p_context);
		SQCache::ObjectCache cache(m_cache->getInverseClassCache());
		for (s32 i = 0; i < size; ++i)
		{
			const std::string name(bu::get<std::string>(p_context));
			s32 idx = cache.indexOf(name);
			if (idx < 0)
			{
				TT_PANIC("Serialization data refers to Squirrel class '%s', but this class does not exist.",
					name.c_str());
				continue;
			}
			
			m_classes.emplace_back(cache[idx]);
		}
	}
	
	// Unserialize used closures
	{
		s32 size = bu::get<s32>(p_context);
		SQCache::ObjectCache cache(m_cache->getInverseClosureCache());
		for (s32 i = 0; i < size; ++i)
		{
			const std::string name(bu::get<std::string>(p_context));
			s32 idx = cache.indexOf(name);
			if (idx < 0)
			{
				TT_PANIC("Serialization data refers to Squirrel closure '%s', but this closure does not exist.",
					name.c_str());
				continue;
			}
			
			m_closures.emplace_back(cache[idx]);
		}
	}
	
	// Unserialize arrays
	{
		tt::script::SqTopRestorerHelper restorer(m_vm, true);
		
		s32 size = bu::get<s32>(p_context);
		for (s32 i = 0; i < size; ++i)
		{
			s32 numEntries = bu::get<s32>(p_context);
			TT_ASSERT(numEntries >= 0);
			sq_newarray(m_vm, numEntries);
			sq_getstackobj(m_vm, -1, &object);
			sq_addref(m_vm, &object);
			m_arrays.emplace_back(object);
			
			// Iterate through all entries
			for (s32 j = 0; j < numEntries; ++j)
			{
				ProcessedObject key(ProcessedObject::createInteger(j));
				ProcessedObject value(unserializeProcessedObject(p_context));
				
				if (value.isIndexedType())
				{
					m_references.emplace_back(object, key, value);
				}
				else
				{
					setSlotOfStackObject(key, value);
				}
			}
			sq_poptop(m_vm);
		}
	}
	
	// Unserialize tables
	{
		tt::script::SqTopRestorerHelper restorer(m_vm, true);
		
		s32 rootTableIndex = bu::get<s32>(p_context);
		
		s32 size = bu::get<s32>(p_context);
		TT_ASSERT(rootTableIndex >= 0 && rootTableIndex < size);
		
		for (s32 i = 0; i < size; ++i)
		{
			s32 numEntries = bu::get<s32>(p_context);
			TT_ASSERT(numEntries >= 0);
			
			if (i == rootTableIndex)
			{
				sq_pushroottable(m_vm);
			}
			else
			{
				sq_newtableex(m_vm, numEntries);
			}
			sq_getstackobj(m_vm, -1, &object);
			sq_addref(m_vm, &object);
			m_tables.emplace_back(object);
			
			// Iterate through all entries
			for (s32 j = 0; j < numEntries; ++j)
			{
				ProcessedObject key(unserializeProcessedObject(p_context));
				ProcessedObject value(unserializeProcessedObject(p_context));
				
				if (key.isIndexedType() || value.isIndexedType())
				{
					m_references.emplace_back(object, key, value);
				}
				else
				{
					setSlotOfStackObject(key, value);
				}
			}
			sq_poptop(m_vm);
		}
	}
	
	// Unserialize instances
	{
		tt::script::SqTopRestorerHelper restorer(m_vm, true);
		
		s32 size = bu::get<s32>(p_context);
		for (s32 i = 0; i < size; ++i)
		{
			UserType::Type userType = bu::getEnum<s32, UserType::Type>(p_context);
			
			if (userType != UserType::Type_None)
			{
				UserType::unserialize(m_vm, *this, userType, p_context);
			}
			else
			{
				s32 classID = bu::get<s32>(p_context);
				ProcessedObject processedClass(ProcessedObject::createIndexed(OT_CLASS, classID));
				pushOnStack(processedClass);
				sq_createinstance(m_vm, -1);
				
				// Remove class from stack
				sq_remove(m_vm, -2);
			}
			
			sq_getstackobj(m_vm, -1, &object);
			
			if (sq_isnull(object) == false)
			{
				sq_addref(m_vm, &object);
			}
			
			m_instances.emplace_back(object);
			
			// Iterate through all members
			s32 numMembers = bu::get<s32>(p_context);
			TT_ASSERT(numMembers >= 0);
			for (s32 j = 0; j < numMembers; ++j)
			{
				ProcessedObject key(unserializeProcessedObject(p_context));
				ProcessedObject value(unserializeProcessedObject(p_context));
				
				if (sq_isnull(object) == false)
				{
					if (key.isIndexedType() || value.isIndexedType())
					{
						m_references.emplace_back(object, key, value);
					}
					else
					{
						setSlotOfStackObject(key, value);
					}
				}
			}
			sq_poptop(m_vm);
		}
	}
	
	m_objectsCreated = true;
	
	// All objects are now created, set all references
	for (References::const_iterator it = m_references.begin(); it != m_references.end(); ++it)
	{
		(*it).createReference(*this);
	}
}


HSQOBJECT SQUnserializer::unserializeObject(tt::code::BufferReadContext* p_context) const
{
	ProcessedObject processedObject(unserializeProcessedObject(p_context));
	HSQOBJECT result;
	pushOnStack(processedObject);
	sq_getstackobj(m_vm, -1, &result);
	sq_poptop(m_vm);
	
	return result;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SQUnserializer::Reference::createReference(const SQUnserializer& p_unserializer) const
{
	HSQUIRRELVM v = p_unserializer.getVM();
	tt::script::SqTopRestorerHelper stackRestorer(v, true);
	sq_pushobject(v, m_object);
	p_unserializer.setSlotOfStackObject(m_key, m_value);
	sq_poptop(v);
}


ProcessedObject SQUnserializer::unserializeProcessedObject(tt::code::BufferReadContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	SQObjectType type = bu::getEnum<u32, SQObjectType>(p_context);
	
	switch (type)
	{
	case OT_BOOL:
		return ProcessedObject::createBool(bu::get<bool>(p_context));
		
	case OT_FLOAT:
		return ProcessedObject::createFloat(bu::get<SQFloat>(p_context));
		
	case OT_INTEGER:
		// [ER] For TT 32 vs 64 compat.
		return ProcessedObject::createInteger(bu::get<s32>(p_context));
		
	case OT_NULL:
		return ProcessedObject::createNull();
		
	case OT_ARRAY:
	case OT_INSTANCE:
	case OT_TABLE:
	case OT_CLASS:
		{
			bool isWeakRef = bu::get<bool>(p_context);
			s32 idx = bu::get<s32>(p_context);
			return isWeakRef ? ProcessedObject::createWeakRef(type, idx) :
			                   ProcessedObject::createIndexed(type, idx);
		}
		
	case OT_CLOSURE:
	case OT_NATIVECLOSURE:
	case OT_STRING:
		return ProcessedObject::createIndexed(type, bu::get<s32>(p_context));
		
	default:
		TT_PANIC("Unhandled type '%s'", tt::script::sqObjectTypeName(type));
		break;
	}
	
	return ProcessedObject();
}


void SQUnserializer::pushOnStack(const ProcessedObject& p_object) const
{
	switch (p_object.getType())
	{
	case OT_BOOL:
		sq_pushbool(m_vm, p_object.getBool());
		return;
		
	case OT_INTEGER:
		sq_pushinteger(m_vm, p_object.getInteger());
		return;
		
	case OT_NULL:
		sq_pushnull(m_vm);
		return;
		
	case OT_FLOAT:
		sq_pushfloat(m_vm, p_object.getFloat());
		return;
		
	case OT_CLOSURE:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < static_cast<s32>(m_closures.size()));
		sq_pushobject(m_vm, m_closures[p_object.getIndex()]);
		return;
		
	case OT_STRING:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < static_cast<s32>(m_strings.size()));
		sq_pushobject(m_vm, m_strings[p_object.getIndex()]);
		return;
		
	// Objects below can potentially be weak referenced; so don't return, but break to handle weakreferences
		
	case OT_ARRAY:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < static_cast<s32>(m_arrays.size()));
		sq_pushobject(m_vm, m_arrays[p_object.getIndex()]);
		break;
		
	case OT_TABLE:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < static_cast<s32>(m_tables.size()));
		sq_pushobject(m_vm, m_tables[p_object.getIndex()]);
		break;
		
	case OT_INSTANCE:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < static_cast<s32>(m_instances.size()));
		sq_pushobject(m_vm, m_instances[p_object.getIndex()]);
		break;
		
	case OT_CLASS:
		TT_ASSERT(p_object.getIndex() >= 0 && p_object.getIndex() < static_cast<s32>(m_classes.size()));
		sq_pushobject(m_vm, m_classes[p_object.getIndex()]);
		break;
		
	default:
		TT_PANIC("Unhandled type '%s'", tt::script::sqObjectTypeName(p_object.getType()));
		break;
	}
	
	if (p_object.isWeakRef() == false || sq_gettype(m_vm, -1) == OT_NULL)
	{
		return;
	}
	
	// Weak ref on top stack. Get rid of original object
	sq_weakref(m_vm, -1);
	TT_ASSERTMSG(sq_gettype(m_vm, -1) == OT_WEAKREF, 
		"pushOnStack: Got type '%s' Expected type OT_WEAKREF. Base type is '%s'",
		tt::script::sqObjectTypeName(sq_gettype(m_vm, -1)),
		tt::script::sqObjectTypeName(p_object.getType()));
	
	sq_remove(m_vm, -2);
}

void SQUnserializer::setSlotOfStackObject(const ProcessedObject& p_key, const ProcessedObject& p_value) const
{
	tt::script::SqTopRestorerHelper tmp(m_vm, true);
	
	// Assumes that object is at -1
	SQObjectType type = sq_gettype(m_vm, -1);
	
	if (type == OT_TABLE || type == OT_INSTANCE || type == OT_ARRAY)
	{
		pushOnStack(p_key);
		pushOnStack(p_value);
		
		if (type == OT_TABLE)
		{
			const SQRESULT result = sq_newslot(m_vm, -3, SQFalse);
			TT_ASSERT(SQ_SUCCEEDED(result));
		}
		else
		{
			const SQRESULT result = sq_set(m_vm, -3);
			TT_ASSERT(SQ_SUCCEEDED(result));
		}
	}
	else
	{
		TT_PANIC("setSlotOfStackObject only works on tables, arrays and instances. Type is '%s'",
			tt::script::sqObjectTypeName(type));
	}
}


void SQUnserializer::removeSquirrelReferences(HSQOBJECTS& p_objects)
{
	for (HSQOBJECTS::iterator it = p_objects.begin(); it != p_objects.end(); ++it)
	{
		HSQOBJECT& objectRef = (*it);
		if (sq_isnull(objectRef) == false)
		{
			sq_release(m_vm, &objectRef);
		}
	}
}


// Namespace end
}
}
}
