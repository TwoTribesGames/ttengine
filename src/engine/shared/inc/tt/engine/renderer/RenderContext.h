#if !defined(INC_TT_ENGINE_RENDERER_RENDERCONTEXT_H)
#define INC_TT_ENGINE_RENDERER_RENDERCONTEXT_H


#include <tt/engine/scene/fwd.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace renderer {


enum RenderPass
{
	RenderPass_Normal,
	RenderPass_Transparents,
	RenderPass_Shadows,
	RenderPass_ShadowVolumes
};

struct RenderContext
{
	scene::Scene*    scene;
	scene::Instance* instance;
	real             textureAnimationTime;
	RenderPass       pass;

	RenderContext(scene::Scene* p_scene = 0, real p_time = 0)
	:
	scene(p_scene),
	instance(0),
	textureAnimationTime(p_time),
	pass(RenderPass_Normal)
	{}
};


// Namespace end
}
} 
}

#endif // INC_TT_ENGINE_RENDERER_RENDERCONTEXT_H
