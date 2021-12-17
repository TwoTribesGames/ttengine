#if !defined(INC_TT_ENGINE_ANIM2D_FWD_H)
#define INC_TT_ENGINE_ANIM2D_FWD_H

#include <set>
#include <tt/math/hash/Hash.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace anim2d {

typedef math::hash::Hash<32> Tag;
typedef std::set<Tag> Tags;

class Animation2D;
class RotationAnimation2D;
class ScaleAnimation2D;
class ParticleAnimation2D;

class ColorAnimation2D;
typedef tt_ptr<ColorAnimation2D>::shared ColorAnimation2DPtr;

class PositionAnimation2D;
typedef tt_ptr<PositionAnimation2D>::shared PositionAnimation2DPtr;
typedef tt_ptr<const PositionAnimation2D>::shared ConstPositionAnimation2DPtr;

class TranslationAnimation2D;
typedef tt_ptr<TranslationAnimation2D>::shared TranslationAnimation2DPtr;


class AnimationStack2D;
class ColorAnimationStack2D;
typedef tt_ptr<AnimationStack2D>::shared AnimationStack2DPtr;
typedef tt_ptr<const AnimationStack2D>::shared ConstAnimationStack2DPtr;
typedef tt_ptr<AnimationStack2D>::weak   AnimationStack2DWeakPtr;
typedef tt_ptr<ColorAnimationStack2D>::shared ColorAnimationStack2DPtr;
typedef tt_ptr<const ColorAnimationStack2D>::shared ConstColorAnimationStack2DPtr;

}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_FWD_H)
