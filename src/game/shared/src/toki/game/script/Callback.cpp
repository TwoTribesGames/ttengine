#include <tt/code/bufferutils.h>

#include <toki/game/script/EntityBase.h> // Include EntityBase.h before Callback.h
                                         // (This will cause Callback.h to be fully included first.
                                         //  Because, EntityBase.h will include Callback.h first.)
#include <toki/game/script/Callback.h>
#include <toki/script/serialization/SQSerializer.h>
#include <toki/script/serialization/SQUnserializer.h>
#include <toki/script/serialization/ProcessedObject.h>


namespace toki {
namespace game {
namespace script {


//--------------------------------------------------------------------------------------------------
// Public member functions

Callback::Callback(HSQUIRRELVM p_vm, const HSQOBJECT& p_state, const std::string& p_name)
:
m_vm(p_vm),
m_state(p_state),
m_name(p_name)
{
}


Callback::~Callback()
{
	for (Parameters::iterator it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		sq_release(m_vm, &(*it));
	}
}


bool Callback::execute(const HSQOBJECT& p_instance) const
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	sq_pushobject(m_vm, m_state); // Push state class here.
	sq_pushstring(m_vm, m_name.c_str(), -1);
	sq_get(m_vm, -2); //get the function from the class
	
	const SQObjectType type(sq_gettype(m_vm, sq_gettop(m_vm)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return false;
	}
	
	// Push the calling instance (first parameter)
	sq_pushobject(m_vm, p_instance);
	
	// Push parameters
	for (Parameters::const_iterator it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		sq_pushobject(m_vm, (*it));
	}
	
	if (SQ_FAILED(sq_call(m_vm, static_cast<SQInteger>(m_parameters.size() + 1), SQFalse, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_WARN("Calling squirrel function '%s' failed", m_name.c_str());
		return false;
	}
	return true;
}


void Callback::addObjectToSQSerializer(toki::script::serialization::SQSerializer& p_serializer) const
{
	p_serializer.addObject(m_vm, m_state);
	
	for (Parameters::const_iterator it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		p_serializer.addObject(m_vm, (*it));
	}
	
}


void Callback::serialize(const toki::script::serialization::SQSerializer& p_serializer,
                         tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	// Store function name
	bu::put(m_name, p_context);
	
	// Store state
	typedef toki::script::serialization::ProcessedObject ProcessedObject;
	ProcessedObject processedObject(p_serializer.getProcessedObject(m_state));
	TT_ASSERT(processedObject.isValid()); // Should be found because previous addObjectToSQSerializer call should have added it.
	p_serializer.serializeProcessedObject(processedObject, p_context);
	
	// Store parameters
	const u32 parameterCount = static_cast<u32>(m_parameters.size());
	bu::put(parameterCount, p_context);
	
	for (Parameters::const_iterator it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		processedObject = p_serializer.getProcessedObject((*it));
		TT_ASSERT(processedObject.isValid()); // Should be found because previous addObjectToSQSerializer call should have added it.
		p_serializer.serializeProcessedObject(processedObject, p_context);
	}
}


CallbackPtr Callback::unserialize(const toki::script::serialization::SQUnserializer& p_unserializer,
                                  tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	std::string name  = bu::get<std::string>(p_context);
	HSQOBJECT   state = p_unserializer.unserializeObject(p_context);
	
	HSQUIRRELVM v = p_unserializer.getVM();
	
	CallbackPtr callback(new Callback(v, state, name));
	
	const u32 parameterCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < parameterCount; ++i)
	{
		HSQOBJECT param = p_unserializer.unserializeObject(p_context);
		sq_addref(v, &param);
		callback->m_parameters.push_back(param);
	}
	
	return callback;
}

//--------------------------------------------------------------------------------------------------
// Private member functions

// Namespace end
}
}
}
