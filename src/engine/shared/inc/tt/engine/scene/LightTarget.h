#if !defined(INC_TT_ENGINE_SCENE_LIGHTTARGET_H)
#define INC_TT_ENGINE_SCENE_LIGHTTARGET_H

// Include necessary files
#include <tt/platform/tt_types.h>
#include <tt/engine/scene/SceneObject.h>


namespace tt {
namespace engine {
namespace scene {


class LightTarget : public SceneObject
{
public:
	LightTarget();
	virtual ~LightTarget();
	
protected:
	virtual bool load(const fs::FilePtr& p_file);
	virtual void renderObject(renderer::RenderContext& p_renderContext);
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_LIGHTTARGET_H
