#if !defined(INC_TT_ENGINE_EFFECT_DESTROYERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_DESTROYERWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class DestroyerWrapper : public ModifierWrapper
{
public:
	DestroyerWrapper()
	{
		m_modifier = SPK::Destroyer::create();
	}

	~DestroyerWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<DestroyerWrapper>::init(p_vm->getVM(), _SC("Destroyer"), _SC("Modifier"));
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_DESTROYERWRAPPER_H
