#if !defined(INC_TT_ENGINE_EFFECT_POINTWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_POINTWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class PointWrapper : public ZoneWrapper
{
public:
	PointWrapper()
	{
		m_zone = SPK::Point::create();
	}

	~PointWrapper()
	{
		SPK_Destroy(m_zone);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<PointWrapper>::init(p_vm->getVM(), _SC("Point"), _SC("Zone"));
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_POINTWRAPPER_H
