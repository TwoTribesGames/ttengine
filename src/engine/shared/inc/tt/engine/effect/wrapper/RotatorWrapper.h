#if !defined(INC_TT_ENGINE_EFFECT_ROTATORWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_ROTATORWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class RotatorWrapper : public ModifierWrapper
{
public:
	RotatorWrapper()
	{
		m_modifier = SPK::Rotator::create();
	}

	~RotatorWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<RotatorWrapper>::init(p_vm->getVM(), _SC("Rotator"), _SC("Modifier"));
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_ROTATORWRAPPER_H
