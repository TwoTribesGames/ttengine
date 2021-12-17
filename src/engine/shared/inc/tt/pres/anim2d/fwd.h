#if !defined(INC_TT_PRES_ANIM2D_FWD_H)
#define INC_TT_PRES_ANIM2D_FWD_H

#include <set>
#include <tt/math/hash/NamedHash.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace pres {
namespace anim2d {

typedef math::hash::NamedHash<32> Tag;
typedef std::set<Tag> Tags;

class Animation2D;
class RotationAnimation2D;
class ScaleAnimation2D;

class ColorAnimation2D;
typedef tt_ptr<ColorAnimation2D>::shared ColorAnimation2DPtr;

class PositionAnimation2D;
typedef tt_ptr<PositionAnimation2D>::shared PositionAnimation2DPtr;
typedef tt_ptr<const PositionAnimation2D>::shared ConstPositionAnimation2DPtr;

class TranslationAnimation2D;
typedef tt_ptr<TranslationAnimation2D>::shared TranslationAnimation2DPtr;


class AnimationStack2D;
class ColorAnimationStack2D;

}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_FWD_H)
