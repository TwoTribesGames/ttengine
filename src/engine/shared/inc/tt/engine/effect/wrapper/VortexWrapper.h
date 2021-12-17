#if !defined(INC_TT_ENGINE_EFFECT_VORTEXWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_VORTEXWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {


class VortexWrapper : public ModifierWrapper
{
public:
	VortexWrapper()
	{
		m_modifier = SPK::Vortex::create();
	}

	~VortexWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	inline void setPosition(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Vortex*>(m_modifier)->setPosition(SPK::Vector3D(p_x, p_y, p_z));
	}


	inline void setDirection(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Vortex*>(m_modifier)->setDirection(SPK::Vector3D(p_x, p_y, p_z));
	}


	inline void setRotationSpeed(float p_speed, bool p_angular)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Vortex*>(m_modifier)->setRotationSpeed(p_speed, p_angular);
	}


	inline void setAttractionSpeed(float p_speed, bool p_linear)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Vortex*>(m_modifier)->setAttractionSpeed(p_speed, p_linear);
	}


	inline void setEyeRadius(float p_radius)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Vortex*>(m_modifier)->setEyeRadius(p_radius);
	}


	inline void enableParticleKilling(bool p_enable)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Vortex*>(m_modifier)->enableParticleKilling(p_enable);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<VortexWrapper>::init(p_vm->getVM(), _SC("Vortex"), _SC("Modifier"));
		sqbind_method(p_vm->getVM(), "setPosition",           &VortexWrapper::setPosition);
		sqbind_method(p_vm->getVM(), "setDirection",          &VortexWrapper::setDirection);
		sqbind_method(p_vm->getVM(), "setRotationSpeed",      &VortexWrapper::setRotationSpeed);
		sqbind_method(p_vm->getVM(), "setAttractionSpeed",    &VortexWrapper::setAttractionSpeed);
		sqbind_method(p_vm->getVM(), "setEyeRadius",          &VortexWrapper::setEyeRadius);
		sqbind_method(p_vm->getVM(), "enableParticleKilling", &VortexWrapper::enableParticleKilling);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_VORTEXWRAPPER_H
