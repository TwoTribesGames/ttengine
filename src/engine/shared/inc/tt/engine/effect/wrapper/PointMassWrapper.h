#if !defined(INC_TT_ENGINE_EFFECT_POINTMASSWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_POINTMASSWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class PointMassWrapper : public ModifierWrapper
{
public:
	PointMassWrapper()
	{
		m_modifier = SPK::PointMass::create();
	}

	~PointMassWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	inline void setMass(float p_mass)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::PointMass*>(m_modifier)->setMass(p_mass);
	}


	inline void setMinDistance(float p_minDistance)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::PointMass*>(m_modifier)->setMinDistance(p_minDistance);
	}

	inline void setPosition(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::PointMass*>(m_modifier)->setPosition(SPK::Vector3D(p_x, p_y, p_z));
	}

	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<PointMassWrapper>::init(p_vm->getVM(), _SC("PointMass"), _SC("Modifier"));
		sqbind_method(p_vm->getVM(), "setMass",        &PointMassWrapper::setMass);
		sqbind_method(p_vm->getVM(), "setMinDistance", &PointMassWrapper::setMinDistance);
		sqbind_method(p_vm->getVM(), "setPosition",    &PointMassWrapper::setPosition);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_POINTMASSWRAPPER_H
