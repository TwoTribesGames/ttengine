#if !defined(INC_TT_ENGINE_EFFECT_PLANEWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_PLANEWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class PlaneWrapper : public ZoneWrapper
{
public:
	PlaneWrapper()
	{
		m_zone = SPK::Plane::create();
	}

	~PlaneWrapper()
	{
		SPK_Destroy(m_zone);
	}

	inline void setNormal(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::Plane*>(m_zone)->setNormal(SPK::Vector3D(p_x, p_y, p_z));
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<PlaneWrapper>::init(p_vm->getVM(), _SC("Plane"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "setNormal", &PlaneWrapper::setNormal);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_PLANEWRAPPER_H
