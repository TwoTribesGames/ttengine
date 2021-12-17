#if !defined(INC_TT_ENGINE_EFFECT_SYSTEMWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_SYSTEMWRAPPER_H

#include <spark/SPK.h>
#include <squirrel/sqbind.h>

#include <tt/engine/effect/wrapper/GroupWrapper.h>
#include <tt/script/VirtualMachine.h>


namespace tt {
namespace engine {
namespace effect {

class Effect;

namespace wrapper {


class SystemWrapper
{
public:
	explicit SystemWrapper(SPK::System* p_system = 0)
	:
	m_system(p_system)
	{}

	~SystemWrapper() {}


	/*! \brief Adds a particle group to the system */
	inline void addGroup(const GroupWrapper& p_group)
	{
		TT_NULL_ASSERT(m_system);
		TT_NULL_ASSERT(p_group.getGroup());
		m_system->addGroup(p_group.getGroup());
	}


	/*! \brief Grow the particle system to the specified time with a certain time step */
	inline void grow(float p_time, float p_step)
	{
		TT_NULL_ASSERT(m_system);
		m_system->grow(p_time, p_step);
	}

	
	/*! \brief Define type in squirrel and bind methods */
	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<SystemWrapper>::init(p_vm->getVM(), _SC("ParticleSystem"));
		sqbind_method(p_vm->getVM(), "addGroup", &SystemWrapper::addGroup);
		sqbind_method(p_vm->getVM(), "grow",     &SystemWrapper::grow);
	}


private:
	/*! \brief Retrieve underlying system pointer */
	friend class tt::engine::effect::Effect;
	inline SPK::System* getSystem() const {return m_system;}

	SPK::System* m_system;
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_SYSTEMWRAPPER_H
