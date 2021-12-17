#if !defined(INC_TT_ENGINE_EFFECT_LINERENDERERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_LINERENDERERWRAPPER_H

#include <spark/RenderingAPIs/Shared/SPK_SharedLineRenderer.h>
#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/RendererWrapper.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/cache/FileTextureCache.h>

#if SPARK_USE_DX9LINE

namespace tt {
namespace engine {
namespace effect {
namespace wrapper {


class LineRendererWrapper : public RendererWrapper
{
public:
	LineRendererWrapper()
	{
#if defined(TT_PLATFORM_WIN)
		m_renderer = SPK::Shared::DX9LineRenderer::create();
#endif
		// Default setup
		static_cast<SPK::Shared::DX9LineRenderer*>(m_renderer)->setBlendingFunctions(D3DBLEND_SRCALPHA, D3DBLEND_ONE);
		
		m_renderer->enableRenderingHint(SPK::DEPTH_TEST, true);
		m_renderer->setBlending(SPK::BLENDING_ADD);
		m_renderer->enableRenderingHint(SPK::DEPTH_WRITE, false);
	}
	
	
	~LineRendererWrapper()
	{
		SPK_Destroy(m_renderer);
	}
	
	
	inline void setLength(float p_length)
	{
		static_cast<SPK::Shared::DX9LineRenderer*>(m_renderer)->setLength(p_length);
	}
	
	
	inline void setWidth(float p_width)
	{
		static_cast<SPK::Shared::DX9LineRenderer*>(m_renderer)->setWidth(p_width);
	}
	
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<LineRendererWrapper>::init(p_vm->getVM(), _SC("LineRenderer"), _SC("Renderer"));
		sqbind_method(p_vm->getVM(), "setLength", &LineRendererWrapper::setLength);
		sqbind_method(p_vm->getVM(), "setWidth",  &LineRendererWrapper::setWidth);
	}
};

// Namespace end
}
}
}
}

#endif // SPARK_USE_DX9LINETRAIL

#endif // INC_TT_ENGINE_EFFECT_LINERENDERERWRAPPER_H
