#if !defined(INC_TT_ENGINE_SCENE_FWD_H)
#define INC_TT_ENGINE_SCENE_FWD_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace scene {

class Camera;
typedef tt_ptr<      Camera>::shared CameraPtr;
typedef tt_ptr<      Camera>::weak   CameraWeakPtr;
typedef tt_ptr<const Camera>::shared ConstCameraPtr;

class Instance;
typedef tt_ptr<Instance>::shared InstancePtr;
typedef tt_ptr<Instance>::weak   InstanceWeakPtr;

class Layer;
typedef tt_ptr<Layer>::shared LayerPtr;
typedef tt_ptr<Layer>::weak   LayerWeakPtr;

class Model;
typedef tt_ptr<Model>::shared ModelPtr;
typedef tt_ptr<Model>::weak   ModelWeakPtr;

class Scene;
typedef tt_ptr<Scene>::shared ScenePtr;
typedef tt_ptr<Scene>::weak   SceneWeakPtr;

class SceneObject;
typedef tt_ptr<SceneObject>::shared SceneObjectPtr;
typedef tt_ptr<SceneObject>::weak   SceneObjectWeakPtr;

class Light;
typedef tt_ptr<Light>::shared LightPtr;
typedef tt_ptr<Light>::weak   LightWeakPtr;

class Fog;
typedef tt_ptr<Fog>::shared FogPtr;
typedef tt_ptr<Fog>::weak   FofWeakPtr;

}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE_FWD_H)
