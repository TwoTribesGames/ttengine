#if !defined(INC_TT_ENGINE_EFFECT_COLLISIONWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_COLLISIONWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class CollisionWrapper : public ModifierWrapper
{
public:
	CollisionWrapper()
	{
		m_modifier = SPK::Collision::create();
	}

	~CollisionWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	inline void setScale(float p_scale)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Collision*>(m_modifier)->setScale(p_scale);
	}


	inline void setElasticity(float p_elasticity)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Collision*>(m_modifier)->setElasticity(p_elasticity);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<CollisionWrapper>::init(p_vm->getVM(), _SC("Collision"), _SC("Modifier"));
		sqbind_method(p_vm->getVM(), "setScale",      &CollisionWrapper::setScale);
		sqbind_method(p_vm->getVM(), "setElasticity", &CollisionWrapper::setElasticity);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_COLLISIONWRAPPER_H
