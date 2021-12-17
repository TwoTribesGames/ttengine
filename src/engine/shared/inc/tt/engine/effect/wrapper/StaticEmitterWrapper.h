#if !defined(INC_TT_ENGINE_EFFECT_STATICEMITTERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_STATICEMITTERWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class StaticEmitterWrapper : public EmitterWrapper
{
public:
	StaticEmitterWrapper()
	{
		m_emitter = SPK::StaticEmitter::create();
		TT_NULL_ASSERT(m_emitter);
	}

	~StaticEmitterWrapper() 
	{
		SPK_Destroy(m_emitter);
	}

	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<StaticEmitterWrapper>::init(p_vm->getVM(), _SC("StaticEmitter"), _SC("Emitter"));
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_STATICEMITTERWRAPPER_H
