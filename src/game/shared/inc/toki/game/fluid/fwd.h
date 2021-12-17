#if !defined(INC_TOKI_GAME_FLUID_FWD_H)
#define INC_TOKI_GAME_FLUID_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace fluid {

class FluidGraphicsMgr;

struct FluidSettings;
typedef tt_ptr<FluidSettings>::shared FluidSettingsPtr;

class FluidMgr;
typedef tt_ptr<FluidMgr>::shared FluidMgrPtr;

class FluidParticlesMgr;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_FWD_H)
