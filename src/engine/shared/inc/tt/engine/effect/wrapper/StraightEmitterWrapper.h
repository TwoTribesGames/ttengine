#if !defined(INC_TT_ENGINE_EFFECT_STRAIGHTEMITTERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_STRAIGHTEMITTERWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class StraightEmitterWrapper : public EmitterWrapper
{
public:
	StraightEmitterWrapper()
	{
		m_emitter = SPK::StraightEmitter::create();
	}

	~StraightEmitterWrapper()
	{
		SPK_Destroy(m_emitter);
	}

	inline void setDirection(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_emitter);
		static_cast<SPK::StraightEmitter*>(m_emitter)->setDirection(SPK::Vector3D(p_x, p_y, p_z));
	}

	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<StraightEmitterWrapper>::init(p_vm->getVM(), _SC("StraightEmitter"), _SC("Emitter"));
		sqbind_method(p_vm->getVM(), "setDirection", &StraightEmitterWrapper::setDirection);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_STRAIGHTEMITTERWRAPPER_H
