#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ViewPort.h>

#include <toki/utils/utils.h>


namespace toki {
namespace utils {

tt::math::Point2 getScreenSize(Screen p_screen)
{
	if (isValidScreen(p_screen) == false)
	{
		TT_PANIC("Invalid screen: %d", p_screen);
		return tt::math::Point2::zero;
	}
	
	// Single screen: always return the single screen size
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		return tt::math::Point2(renderer->getScreenWidth(), renderer->getScreenHeight());
	}
}

// Namespace end
}
}
