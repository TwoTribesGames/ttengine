#if !defined(INC_TT_ENGINE_EFFECT_OBSTACLEWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_OBSTACLEWRAPPER_H

#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class ObstacleWrapper : public ModifierWrapper
{
public:
	ObstacleWrapper()
	{
		m_modifier = SPK::Obstacle::create();
	}

	~ObstacleWrapper()
	{
		SPK_Destroy(m_modifier);
	}


	inline void setBouncingRatio(float p_bounceRatio)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Obstacle*>(m_modifier)->setBouncingRatio(p_bounceRatio);
	}


	inline void setFriction(float p_friction)
	{
		TT_NULL_ASSERT(m_modifier);
		static_cast<SPK::Obstacle*>(m_modifier)->setFriction(p_friction);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ObstacleWrapper>::init(p_vm->getVM(), _SC("Obstacle"), _SC("Modifier"));
		sqbind_method(p_vm->getVM(), "setBouncingRatio", &ObstacleWrapper::setBouncingRatio);
		sqbind_method(p_vm->getVM(), "setFriction",      &ObstacleWrapper::setFriction);
	}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_OBSTACLEWRAPPER_H
