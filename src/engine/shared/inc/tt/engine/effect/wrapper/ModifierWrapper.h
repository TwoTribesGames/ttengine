#if !defined(INC_TT_ENGINE_EFFECT_MODIFIERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_MODIFIERWRAPPER_H

#include <squirrel/squirrel.h>

#include <spark/Core/SPK_Modifier.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class ModifierWrapper
{
public:
	ModifierWrapper()
	:
	m_modifier(0)
	{}

	~ModifierWrapper() {}


	inline void setActive(bool p_active)
	{
		TT_NULL_ASSERT(m_modifier);
		m_modifier->setActive(p_active);
	}


	inline void setZone(const ZoneWrapper& p_zone)
	{
		TT_NULL_ASSERT(m_modifier);
		m_modifier->setZone(p_zone.getZone());
	}


	// FIXME: Create enum inside script
	inline void setTrigger(SPK::ModifierTrigger p_trigger)
	{
		TT_NULL_ASSERT(m_modifier);
		m_modifier->setTrigger(p_trigger);
	}


	inline void setLocalToSystem(bool p_enable)
	{
		TT_NULL_ASSERT(m_modifier);
		m_modifier->setLocalToSystem(p_enable);
	}


	inline bool isLocalToSystem() const
	{
		TT_NULL_ASSERT(m_modifier);
		return m_modifier->isLocalToSystem();
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ModifierWrapper>::init(p_vm->getVM(), _SC("Modifier"), (HSQOBJECT*)0, false);
		sqbind_method(p_vm->getVM(), "setActive",        &ModifierWrapper::setActive);
		sqbind_method(p_vm->getVM(), "setZone",          &ModifierWrapper::setZone);
		sqbind_method(p_vm->getVM(), "setTrigger",       &ModifierWrapper::setTrigger);
		sqbind_method(p_vm->getVM(), "setLocalToSystem", &ModifierWrapper::setLocalToSystem);
		sqbind_method(p_vm->getVM(), "isLocalToSystem",  &ModifierWrapper::isLocalToSystem);
	}

	inline SPK::Modifier* getModifier() const {return m_modifier;}

protected:
	SPK::Modifier* m_modifier;
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_MODIFIERWRAPPER_H
