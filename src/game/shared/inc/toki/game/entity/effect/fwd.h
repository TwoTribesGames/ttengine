#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_FWD_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_FWD_H


#include <tt/code/Handle.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

class ColorGradingEffectMgr;
typedef tt_ptr<ColorGradingEffectMgr>::shared ColorGradingEffectMgrPtr;

class EffectMgr;

class EffectRect;
typedef tt::code::Handle<EffectRect> EffectRectHandle;

class EffectRectMgr;

class FogEffectMgr;
typedef tt_ptr<FogEffectMgr>::shared FogEffectMgrPtr;


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_FWD_H)
