#if !defined(INC_TOKI_GAME_LIGHT_LIGHTRAYTRACER_H)
#define INC_TOKI_GAME_LIGHT_LIGHTRAYTRACER_H


#include <tt/code/TileRayTracer.h>

#include <toki/game/Game.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace light {


//--------------------------------------------------------------------------------------------------
// Hit test function


inline bool lightHitTest(const tt::math::Point2& p_location)
{
	Game* game = AppGlobal::getGame();
	const level::AttributeLayerPtr& layer = game->getAttributeLayer();
	
	if (layer->contains(p_location) == false)
	{
		// Don't block light when outside of level.
		return false;
	}
	const level::TileRegistrationMgr& tileRegMgr = game->getTileRegistrationMgr();
	return tileRegMgr.isLightBlocking(p_location);
}


// Namespace end
}
}
}

#endif // !defined(INC_TOKI_GAME_LIGHT_LIGHTRAYTRACER_H)
