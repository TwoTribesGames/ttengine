#if !defined(INC_TT_ENGINE_EFFECT_LINETRAILRENDERERWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_LINETRAILRENDERERWRAPPER_H

#include <spark/RenderingAPIs/Shared/SPK_SharedLineTrailRenderer.h>
#include <squirrel/squirrel.h>

#include <tt/engine/effect/wrapper/RendererWrapper.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/cache/FileTextureCache.h>

#if SPARK_USE_DX9LINETRAIL

namespace tt {
namespace engine {
namespace effect {
namespace wrapper {


class LineTrailRendererWrapper : public RendererWrapper
{
public:
	LineTrailRendererWrapper()
	{
		//m_renderer = SPK::DX9::DX9LineTrailRenderer::create();
        //
		//// Default setup
		//static_cast<SPK::DX9::DX9LineTrailRenderer*>(m_renderer)->setBlendingFunctions(D3DBLEND_SRCALPHA, D3DBLEND_ONE);
		//static_cast<SPK::DX9::DX9LineTrailRenderer*>(m_renderer)->enableBlending(true);
		//
		//m_renderer->setBlending(SPK::BLENDING_ADD);
		//m_renderer->enableRenderingHint(SPK::DEPTH_WRITE, false);
		//m_renderer->enableRenderingHint(SPK::DEPTH_TEST, true);
	}

	~LineTrailRendererWrapper()
	{
		SPK_Destroy(m_renderer);
	}


	inline void setNbSamples(s32 p_samples)
	{
		(void) p_samples;
		//static_cast<SPK::DX9::DX9LineTrailRenderer*>(m_renderer)->
		//	setNbSamples(static_cast<std::size_t>(p_samples));
	}


	inline void setWidth(float p_width)
	{
		(void) p_width;
		//static_cast<SPK::DX9::DX9LineTrailRenderer*>(m_renderer)->setWidth(p_width);
	}


	inline void setDuration(float p_duration)
	{
		(void) p_duration;
		//static_cast<SPK::DX9::DX9LineTrailRenderer*>(m_renderer)->setDuration(p_duration);
	}


	inline void setDegeneratedLines(float p_r, float p_g, float p_b, float p_a)
	{
		(void) p_r;
		(void) p_g;
		(void) p_b;
		(void) p_a;
		//static_cast<SPK::DX9::DX9LineTrailRenderer*>(m_renderer)->
		//	setDegeneratedLines(p_r, p_g, p_b, p_a);
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<LineTrailRendererWrapper>::init(p_vm->getVM(), _SC("LineTrailRenderer"), _SC("Renderer"));
		sqbind_method(p_vm->getVM(), "setNbSamples",        &LineTrailRendererWrapper::setNbSamples);
		sqbind_method(p_vm->getVM(), "setWidth",            &LineTrailRendererWrapper::setWidth);
		sqbind_method(p_vm->getVM(), "setDuration",         &LineTrailRendererWrapper::setDuration);
		sqbind_method(p_vm->getVM(), "setDegeneratedLines", &LineTrailRendererWrapper::setDegeneratedLines);
	}
};

// Namespace end
}
}
}
}

#endif // SPARK_USE_DX9LINETRAIL


#endif // INC_TT_ENGINE_EFFECT_LINETRAILRENDERERWRAPPER_H
