#if !defined(INC_TT_ENGINE_EFFECT_EMITTERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_EMITTERWRAPPER_H

#include <squirrel/squirrel.h>

#include <spark/Core/SPK_Emitter.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>

namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class EmitterWrapper
{
public:
	EmitterWrapper()
	:
	m_emitter(0)
	{}

	~EmitterWrapper() {}

	inline void setActive(bool p_active)
	{
		TT_NULL_ASSERT(m_emitter);
		m_emitter->setActive(p_active);
	}

	inline void setTank(int p_tank)
	{
		TT_NULL_ASSERT(m_emitter);
		m_emitter->setTank(p_tank);
	}

	inline void setFlow(float p_flow)
	{
		TT_NULL_ASSERT(m_emitter);
		m_emitter->setFlow(p_flow);
	}

	inline void setForce(float p_min, float p_max)
	{
		TT_NULL_ASSERT(m_emitter);
		m_emitter->setForce(p_min, p_max);
	}

	inline void setZone(const ZoneWrapper& p_zone, bool p_fullZone)
	{
		TT_NULL_ASSERT(m_emitter);
		m_emitter->setZone(p_zone.getZone(), p_fullZone);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<EmitterWrapper>::init(p_vm->getVM(), _SC("Emitter"), (HSQOBJECT*)0, false);
		sqbind_method<EmitterWrapper>(p_vm->getVM(), "setActive", &EmitterWrapper::setActive);
		sqbind_method<EmitterWrapper>(p_vm->getVM(), "setTank",   &EmitterWrapper::setTank);
		sqbind_method<EmitterWrapper>(p_vm->getVM(), "setFlow",   &EmitterWrapper::setFlow);
		sqbind_method<EmitterWrapper>(p_vm->getVM(), "setForce",  &EmitterWrapper::setForce);
		sqbind_method<EmitterWrapper>(p_vm->getVM(), "setZone",   &EmitterWrapper::setZone);
	}

protected:
	SPK::Emitter* m_emitter;

private:
	friend class GroupWrapper;
	inline SPK::Emitter* getEmitter() const {return m_emitter;}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_EMITTERWRAPPER_H
