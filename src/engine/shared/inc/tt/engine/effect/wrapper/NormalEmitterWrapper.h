#if !defined(INC_TT_ENGINE_EFFECT_NORMALEMITTERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_NORMALEMITTERWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class NormalEmitterWrapper : public EmitterWrapper
{
public:
	NormalEmitterWrapper()
	{
		m_emitter = SPK::NormalEmitter::create();
		TT_NULL_ASSERT(m_emitter);
	}

	~NormalEmitterWrapper() 
	{
		SPK_Destroy(m_emitter);
	}

	inline void setInverted(bool p_inverted)
	{
		TT_NULL_ASSERT(m_emitter);
		static_cast<SPK::NormalEmitter*>(m_emitter)->setInverted(p_inverted);
	}

	inline void setNormalZone(const ZoneWrapper& p_zone)
	{
		TT_NULL_ASSERT(m_emitter);
		static_cast<SPK::NormalEmitter*>(m_emitter)->setNormalZone(p_zone.getZone());
	}

	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<NormalEmitterWrapper>::init(p_vm->getVM(), _SC("NormalEmitter"), _SC("Emitter"));
		sqbind_method(p_vm->getVM(), "setInverted",   &NormalEmitterWrapper::setInverted);
		sqbind_method(p_vm->getVM(), "setNormalZone", &NormalEmitterWrapper::setNormalZone);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_NORMALEMITTERWRAPPER_H
