#if !defined(INC_TT_ENGINE_EFFECT_RINGWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_RINGWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class RingWrapper : public ZoneWrapper
{
public:
	RingWrapper()
	{
		m_zone = SPK::Ring::create();
	}

	~RingWrapper()
	{
		SPK_Destroy(m_zone);
	}

	inline void setNormal(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::Ring*>(m_zone)->setNormal(SPK::Vector3D(p_x, p_y, p_z));
	}


	inline void setRadius(float p_min, float p_max)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::Ring*>(m_zone)->setRadius(p_min, p_max);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<RingWrapper>::init(p_vm->getVM(), _SC("Ring"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "setNormal", &RingWrapper::setNormal);
		sqbind_method(p_vm->getVM(), "setRadius", &RingWrapper::setRadius);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_RINGWRAPPER_H
