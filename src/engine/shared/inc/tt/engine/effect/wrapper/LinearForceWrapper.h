#if !defined(INC_TT_ENGINE_EFFECT_LINEARFORCEWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_LINEARFORCEWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {


class LinearForceWrapper : public ModifierWrapper
{
public:
	LinearForceWrapper()
	{
		m_modifier = SPK::LinearForce::create();
	}

	~LinearForceWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	inline void setForce(float p_x, float p_y, float p_z)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::LinearForce*>(m_modifier)->setForce(SPK::Vector3D(p_x, p_y, p_z));
	}


	inline void setFactor(SPK::ForceFactor p_type, SPK::ModelParam p_param)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::LinearForce*>(m_modifier)->setFactor(p_type, p_param);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<LinearForceWrapper>::init(p_vm->getVM(), _SC("LinearForce"), _SC("Modifier"));
		sqbind_method(p_vm->getVM(), "setForce",  &LinearForceWrapper::setForce);
		sqbind_method(p_vm->getVM(), "setFactor", &LinearForceWrapper::setFactor);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_LINEARFORCEWRAPPER_H
