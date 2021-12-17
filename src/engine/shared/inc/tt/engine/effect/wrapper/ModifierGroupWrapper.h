#if !defined(INC_TT_ENGINE_EFFECT_MODIFIERGROUPWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_MODIFIERGROUPWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class ModifierGroupWrapper : public ModifierWrapper
{
public:
	ModifierGroupWrapper()
	{
		m_modifier = SPK::ModifierGroup::create();
	}

	~ModifierGroupWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	inline void addModifier(const ModifierWrapper& p_modifier)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::ModifierGroup*>(m_modifier)->addModifier(p_modifier.getModifier());
	}

	inline void removeModifier(const ModifierWrapper& p_modifier)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::ModifierGroup*>(m_modifier)->removeModifier(p_modifier.getModifier());
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ModifierGroupWrapper>::init(p_vm->getVM(), _SC("ModifierGroup"), _SC("Modifier"));
		sqbind_method(p_vm->getVM(), "addModifier",    &ModifierGroupWrapper::addModifier);
		sqbind_method(p_vm->getVM(), "removeModifier", &ModifierGroupWrapper::removeModifier);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_MODIFIERGROUPWRAPPER_H
