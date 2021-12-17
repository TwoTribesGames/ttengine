#include <SDL2/SDL.h>
#include <tt/engine/renderer/device_enumeration.h>


namespace tt {
namespace engine {
namespace renderer {


Resolutions getSupportedResolutions(bool /*p_keepDesktopAspectRatio*/)
{
	Resolutions resolutions;

	// only return the desktop resolution size as best practices on both
	// Mac and Linux are to not chance the desktop resolution

	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(0, &mode);
	const math::Point2 desktopSize(mode.w, mode.h);

	resolutions.insert(desktopSize);
	
	return resolutions;
}


// Namespace end
}
}
}
