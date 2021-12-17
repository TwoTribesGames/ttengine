#if !defined(INC_TT_ENGINE_SCENE2D_FWD_H)
#define INC_TT_ENGINE_SCENE2D_FWD_H

#include <set>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace scene2d {

class SceneInterface;
class Scene2D;
typedef tt_ptr<Scene2D>::shared Scene2DPtr;
typedef tt_ptr<Scene2D>::weak   Scene2DWeakPtr;

class PlaneScene;
typedef tt_ptr<PlaneScene>::shared PlaneScenePtr;
typedef tt_ptr<PlaneScene>::weak   PlaneSceneWeakPtr;

class WorldScene;
typedef tt_ptr<WorldScene>::shared WorldScenePtr;
typedef tt_ptr<WorldScene>::weak   WorldSceneWeakPtr;

typedef std::set<real> BlurLayers;
enum BlurQuality
{
	BlurQuality_NoBlur,
	BlurQuality_OnePassThreeSamples,
	BlurQuality_OnePassFourSamples,
	BlurQuality_TwoPassConvolution,
	
	BlurQuality_Count
};

inline bool isValidBlurQuality(BlurQuality p_quality) { return p_quality >= 0 && p_quality < BlurQuality_Count; }

}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE2D_FWD_H)
