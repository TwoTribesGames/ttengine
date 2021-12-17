#if !defined(INC_TT_ENGINE_EFFECT_SPHERICEMITTERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_SPHERICEMITTERWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class SphericEmitterWrapper : public EmitterWrapper
{
public:
	SphericEmitterWrapper()
	{
		m_emitter = SPK::SphericEmitter::create();
		TT_NULL_ASSERT(m_emitter);
	}

	~SphericEmitterWrapper() 
	{
		SPK_Destroy(m_emitter);
	}

	inline void setDirection(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_emitter);
		static_cast<SPK::SphericEmitter*>(m_emitter)->setDirection(SPK::Vector3D(p_x, p_y, p_z));
	}

	inline void setAngles(float p_a, float p_b)
	{
		TT_NULL_ASSERT(m_emitter);
		static_cast<SPK::SphericEmitter*>(m_emitter)->setAngles(p_a, p_b);
	}

	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<SphericEmitterWrapper>::init(p_vm->getVM(), _SC("SphericEmitter"), _SC("Emitter"));
		sqbind_method(p_vm->getVM(), "setDirection", &SphericEmitterWrapper::setDirection);
		sqbind_method(p_vm->getVM(), "setAngles",    &SphericEmitterWrapper::setAngles);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_SPHERICEMITTERWRAPPER_H
