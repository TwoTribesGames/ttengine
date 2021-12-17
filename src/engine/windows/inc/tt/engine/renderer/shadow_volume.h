#if !defined(INC_TT_ENGINE_RENDERER_SHADOWVOLUME_H)
#define INC_TT_ENGINE_RENDERER_SHADOWVOLUME_H


#include <tt/engine/renderer/fwd.h>


namespace tt {
namespace engine {
namespace renderer {

void initShadowVolumePass();
void beginShadowPass();
void endShadowPass();
bool isStencilAvailable();

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_SHADOWVOLUME_H)
