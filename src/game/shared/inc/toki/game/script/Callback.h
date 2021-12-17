#if !defined(INC_TOKI_GAME_SCRIPT_CALLBACK_H)
#define INC_TOKI_GAME_SCRIPT_CALLBACK_H

#include <vector>

#include <tt/code/fwd.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/script/EntityBase.h> // Make sure SqBind<EntityBase> is resolved correctly.
#include <toki/game/script/fwd.h>
#include <toki/script/serialization/fwd.h>

namespace toki {
namespace game {
namespace script {

class Callback
{
public:
	Callback(HSQUIRRELVM p_vm, const HSQOBJECT& p_state, const std::string& p_name);
	~Callback();
	
	template <typename Type>
	inline void addParameter(const Type& p_parameter)
	{
		HSQOBJECT param;
		SqBind<Type>::push(m_vm, p_parameter);
		sq_getstackobj(m_vm, -1, &param);
		sq_addref(m_vm, &param);
		m_parameters.push_back(param);
		sq_poptop(m_vm);
	}
	
	template <typename Type>
	inline void addParameter(const Type* p_parameter)
	{
		HSQOBJECT param;
		SqBind<Type>::push(m_vm, p_parameter);
		sq_getstackobj(m_vm, -1, &param);
		sq_addref(m_vm, &param);
		m_parameters.push_back(param);
		sq_poptop(m_vm);
	}
	
	bool execute(const HSQOBJECT& p_instance) const;
	
	void addObjectToSQSerializer(toki::script::serialization::SQSerializer& p_serializer) const;
	
	void serialize(const toki::script::serialization::SQSerializer& p_serializer,
	               tt::code::BufferWriteContext*                    p_context) const;
	
	static CallbackPtr unserialize(const toki::script::serialization::SQUnserializer& p_unserializer,
	                               tt::code::BufferReadContext*                       p_context);
	
private:
	HSQUIRRELVM m_vm;
	HSQOBJECT m_state;
	std::string m_name;
	
	typedef std::vector<HSQOBJECT> Parameters;
	Parameters m_parameters;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_CALLBACK_H)
