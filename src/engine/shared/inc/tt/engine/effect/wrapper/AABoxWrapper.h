#if !defined(INC_TT_ENGINE_EFFECT_AABOXWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_AABOXWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class AABoxWrapper : public ZoneWrapper
{
public:
	AABoxWrapper()
	{
		m_zone = SPK::AABox::create();
	}

	~AABoxWrapper()
	{
		SPK_Destroy(m_zone);
	}

	inline void setDimension(float p_width, float p_height, float p_depth)
	{
		TT_NULL_ASSERT(m_zone);
		static_cast<SPK::AABox*>(m_zone)->setDimension(SPK::Vector3D(p_width, p_height, p_depth));
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<AABoxWrapper>::init(p_vm->getVM(), _SC("AABox"), _SC("Zone"));
		sqbind_method(p_vm->getVM(), "setDimension", &AABoxWrapper::setDimension);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_AABOXWRAPPER_H
