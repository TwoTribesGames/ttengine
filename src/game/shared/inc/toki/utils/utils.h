#if !defined(INC_TOKI_UTILS_UTILS_H)
#define INC_TOKI_UTILS_UTILS_H


#include <tt/engine/renderer/enums.h>
#include <tt/math/Point2.h>

#include <toki/constants.h>


namespace toki {
namespace utils {

tt::math::Point2 getScreenSize(Screen p_screen);

inline Screen getScreenFromViewPortID(tt::engine::renderer::ViewPortID p_viewPort)
{
	// FIXME: This translation assumes Screen and ViewPortID enum values are identical! (may not always be the case)
	return static_cast<Screen>(p_viewPort);
}


inline tt::engine::renderer::ViewPortID getViewPortIDFromScreen(Screen p_screen)
{
	// FIXME: This translation assumes Screen and ViewPortID enum values are identical! (may not always be the case)
	return static_cast<tt::engine::renderer::ViewPortID>(p_screen);
}

// Namespace end
}
}


#endif  // !defined(INC_TOKI_UTILS_UTILS_H)
