#if !defined(INC_TT_ENGINE_SCENE_CAMERATARGET_H)
#define INC_TT_ENGINE_SCENE_CAMERATARGET_H


#include <tt/platform/tt_types.h>
#include <tt/engine/scene/SceneObject.h>


namespace tt {
namespace engine {
namespace scene {

class CameraTarget : public SceneObject
{
public:
	CameraTarget();
	virtual ~CameraTarget();
	
protected:
	virtual bool load(const fs::FilePtr& p_file);
	virtual void renderObject(renderer::RenderContext& p_renderContext);
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_CAMERATARGET_H
