#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_RESOLUTIONCHANGERWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_RESOLUTIONCHANGERWRAPPER_H


#include <tt/engine/renderer/fwd.h>
#include <tt/script/helpers.h>

#include <toki/game/hud/fwd.h>
#include <toki/game/script/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'ResolutionChanger' in Squirrel.*/
class ResolutionChangerWrapper
{
public:
	static bool supportsScale(real p_scale);
	static void setScale(real p_scale);
	static real getScale();
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	typedef std::vector<tt::math::Point2> Point2s;
	static const s32 minimumVerticalResolution = 480;
	static Point2s ms_supportedResolutions;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_RESOLUTIONCHANGERWRAPPER_H)
