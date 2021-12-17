#if !defined(INC_TT_ENGINE_ANIMATION_FWD_H)
#define INC_TT_ENGINE_ANIMATION_FWD_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace animation {

class Animation;
typedef tt_ptr<Animation>::shared AnimationPtr;
typedef tt_ptr<Animation>::weak   AnimationWeakPtr;

class AnimationControl;
typedef tt_ptr<AnimationControl>::shared AnimationControlPtr;
typedef tt_ptr<AnimationControl>::weak   AnimationControlWeakPtr;

class HermiteFloatController;
typedef tt_ptr<HermiteFloatController>::shared HermiteFloatControllerPtr;
typedef tt_ptr<HermiteFloatController>::weak   HermiteFloatControllerWeakPtr;

class StepFloatController;
typedef tt_ptr<StepFloatController>::shared StepFloatControllerPtr;
typedef tt_ptr<StepFloatController>::weak   StepFloatControllerWeakPtr;

class TransformController;
typedef tt_ptr<TransformController>::shared TransformControllerPtr;
typedef tt_ptr<TransformController>::weak   TransformControllerWeakPtr;

class TexMatrixController;
typedef tt_ptr<TexMatrixController>::shared TexMatrixControllerPtr;
typedef tt_ptr<TexMatrixController>::weak   TexMatrixControllerWeakPtr;

}
}
}

#endif // !defined(INC_TT_ENGINE_ANIMATION_FWD_H)
