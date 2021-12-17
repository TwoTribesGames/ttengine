#if !defined(INC_TT_ENGINE_RENDERER_DEVICE_ENUMERATION_H)
#define INC_TT_ENGINE_RENDERER_DEVICE_ENUMERATION_H


#include <set>
#include <tt/math/Point2.h>


namespace tt {
namespace engine {
namespace renderer {


typedef std::set<tt::math::Point2, tt::math::Point2Less> Resolutions;
Resolutions getSupportedResolutions(bool p_keepDesktopAspectRatio = true);


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_DEVICE_ENUMERATION_H
