#if !defined(INC_TT_ENGINE_EFFECT_SPHEREWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_SPHEREWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class SphereWrapper : public ZoneWrapper
{
public:
	SphereWrapper()
	{
		m_zone = SPK::Sphere::create();
	}

	~SphereWrapper()
	{
		SPK_Destroy(m_zone);
	}


	inline void setRadius(float p_radius)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::Sphere*>(m_zone)->setRadius(p_radius);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<SphereWrapper>::init(p_vm->getVM(), _SC("Sphere"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "setRadius", &SphereWrapper::setRadius);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_SPHEREWRAPPER_H
