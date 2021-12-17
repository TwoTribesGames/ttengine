#if !defined(INC_TT_ENGINE_EFFECT_RENDERERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_RENDERERWRAPPER_H

#include <spark/Core/SPK_Renderer.h>
#include <squirrel/squirrel.h>

#include <tt/script/VirtualMachine.h>


namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class RendererWrapper
{
public:
	RendererWrapper()
	:
	m_renderer(0)
	{}

	~RendererWrapper() {}

	inline void setActive(bool p_active)
	{
		TT_NULL_ASSERT(m_renderer);
		m_renderer->setActive(p_active);
	}


	inline void setBlending(SPK::BlendingMode p_blendMode)
	{
		TT_NULL_ASSERT(m_renderer);
		m_renderer->setBlending(p_blendMode);
	}


	inline void enableRenderingHint(SPK::RenderingHint p_hint, bool p_enable)
	{
		TT_NULL_ASSERT(m_renderer);
		m_renderer->enableRenderingHint(p_hint, p_enable);
	}


	inline void setAlphaTestThreshold(float p_threshold)
	{
		TT_NULL_ASSERT(m_renderer);
		m_renderer->setAlphaTestThreshold(p_threshold);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<RendererWrapper>::init(p_vm->getVM(), _SC("Renderer"), (HSQOBJECT*)0, false);
		sqbind_method(p_vm->getVM(), "setActive",             &RendererWrapper::setActive);
		sqbind_method(p_vm->getVM(), "setBlending",           &RendererWrapper::setBlending);
		sqbind_method(p_vm->getVM(), "enableRenderingHint",   &RendererWrapper::enableRenderingHint);
		sqbind_method(p_vm->getVM(), "setAlphaTestThreshold", &RendererWrapper::setAlphaTestThreshold);
	}

protected:
	SPK::Renderer* m_renderer;

private:
	friend class GroupWrapper;
	inline SPK::Renderer* getRenderer() const {return m_renderer;}
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_RENDERERWRAPPER_H
