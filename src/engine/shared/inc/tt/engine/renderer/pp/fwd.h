#if !defined(INC_TT_ENGINE_RENDERER_PP_FWD_H)
#define INC_TT_ENGINE_RENDERER_PP_FWD_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace renderer {
namespace pp {

class Effect;
typedef tt_ptr<Effect>::shared EffectPtr;
typedef tt_ptr<Effect>::weak   EffectWeakPtr;

class Filter;
typedef tt_ptr<Filter>::shared FilterPtr;
typedef tt_ptr<Filter>::weak   FilterWeakPtr;

class PostProcessor;
typedef tt_ptr<PostProcessor>::shared PostProcessorPtr;
typedef tt_ptr<PostProcessor>::weak   PostProcessorWeakPtr;


}
}
}
}

#endif // INC_TT_ENGINE_RENDERER_PP_FWD_H
