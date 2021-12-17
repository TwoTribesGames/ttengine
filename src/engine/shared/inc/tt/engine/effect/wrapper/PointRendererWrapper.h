#if !defined(INC_TT_ENGINE_EFFECT_POINTRENDERERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_POINTRENDERERWRAPPER_H

#include <spark/RenderingAPIs/Shared/SPK_SharedPointRenderer.h>
#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/RendererWrapper.h>
#include <tt/engine/renderer/Texture.h>

#if SPARK_USE_DX9POINT

namespace tt {
namespace engine {
namespace effect {
namespace wrapper {

class PointRendererWrapper : public RendererWrapper
{
public:
	PointRendererWrapper()
	{
	/*
		m_renderer = SPK::DX9::DX9PointRenderer::create();

		// Default setup
		static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->enableWorldSize(true);
		//static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->setTextureBlending(D3DTOP_MODULATE);
		//static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->enableBlending(true);
		static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->setBlendingFunctions(D3DBLEND_SRCALPHA, D3DBLEND_ONE);
		m_renderer->setBlending(SPK::BLENDING_ADD);
		m_renderer->enableRenderingHint(SPK::DEPTH_WRITE, false);
		m_renderer->enableRenderingHint(SPK::DEPTH_TEST, true);
		*/
	}

	~PointRendererWrapper()
	{
		SPK_Destroy(m_renderer);
	}


	inline void setSize(float /*p_size*/)
	{
		//static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->setSize(p_size);
	}

	inline void setTexture(const std::string& /*p_name*/, const std::string& /*p_namespace*/)
	{
		//static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->
		//	setTexture(renderer::TextureCache::get(p_name, p_namespace, true));
	}


	// NOTE: For projects that cannot use the Asset Tool
	inline void setFileTexture(const std::string& /*p_filename*/)
	{
		//static_cast<SPK::DX9::DX9PointRenderer*>(m_renderer)->
		//	setTexture(cache::FileTextureCache::get(p_filename));
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<PointRendererWrapper>::init(p_vm->getVM(), _SC("PointRenderer"), _SC("Renderer"));
		sqbind_method(p_vm->getVM(), "setSize",        &PointRendererWrapper::setSize);
		sqbind_method(p_vm->getVM(), "setTexture",     &PointRendererWrapper::setTexture);
		sqbind_method(p_vm->getVM(), "setFileTexture", &PointRendererWrapper::setFileTexture);
	}
};

// Namespace end
}
}
}
}

#endif

#endif // INC_TT_ENGINE_EFFECT_POINTRENDERERWRAPPER_H
