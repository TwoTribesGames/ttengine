#if !defined(INC_TT_ENGINE_SCENE_SCENEBLURMGR_H)
#define INC_TT_ENGINE_SCENE_SCENEBLURMGR_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/pp/fwd.h>

namespace tt {
namespace engine {
namespace scene {

enum BlurType
{
	BlurType_Triangle,
	BlurType_Box,
	BlurType_TwoPassConvolution
};


class SceneBlurMgr
{
public:
	static bool initialize(const renderer::pp::FilterPtr& p_filterBlurH,
	                       const renderer::pp::FilterPtr& p_filterBlurV,
	                       const renderer::pp::FilterPtr& p_filterUpscale,
	                       const renderer::pp::FilterPtr& p_filterBoxBlur,
	                       const renderer::pp::FilterPtr& p_filterTriBlur,
	                       real                           p_renderTargetRatio);
	
	static void changeRenderTargetRatio(real p_ratio);
	
	static void beginRender();
	
	static void doBlurPass(BlurType p_type);
	
	static void endRender();
	
	static void cleanup();
	
private:
	SceneBlurMgr() {}
	~SceneBlurMgr() {}

	static renderer::RenderTargetPtr ms_renderTarget[2];

	static renderer::pp::FilterPtr ms_filterBlurH;
	static renderer::pp::FilterPtr ms_filterBlurV;
	static renderer::pp::FilterPtr ms_filterUpscale;
	static renderer::pp::FilterPtr ms_filterBoxBlur;
	static renderer::pp::FilterPtr ms_filterTriBlur;

	static s32 ms_activeIndex;
};


// Namespace end
}
} 
}

#endif // INC_TT_ENGINE_SCENE_SCENEBLURMGR_H
