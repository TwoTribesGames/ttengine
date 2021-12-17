#if !defined(INC_TT_ENGINE_EFFECT_RANDOMEMITTERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_RANDOMEMITTERWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class RandomEmitterWrapper : public EmitterWrapper
{
public:
	RandomEmitterWrapper()
	{
		m_emitter = SPK::RandomEmitter::create();
		TT_NULL_ASSERT(m_emitter);
	}

	~RandomEmitterWrapper() 
	{
		SPK_Destroy(m_emitter);
	}

	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<RandomEmitterWrapper>::init(p_vm->getVM(), _SC("RandomEmitter"), _SC("Emitter"));
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_RANDOMEMITTERWRAPPER_H
