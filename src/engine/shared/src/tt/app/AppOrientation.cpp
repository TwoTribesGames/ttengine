#include <tt/app/AppOrientation.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace app {

const char* getAppOrientationName(AppOrientation p_orientation, bool p_allowInvalid)
{
	switch (p_orientation)
	{
	case AppOrientation_Portrait:           return "portrait";
	case AppOrientation_PortraitUpsideDown: return "portrait_upside_down";
	case AppOrientation_LandscapeLeft:      return "landscape_left";
	case AppOrientation_LandscapeRight:     return "landscape_right";
		
	default:
		if (p_allowInvalid)
		{
			return "invalid";
		}
		TT_PANIC("Unsupported AppOrientation: %d", p_orientation);
		break;
	}
	return "";
}


AppOrientation getAppOrientationFromName(const std::string& p_name)
{
	for (s32 i = 0; i < AppOrientation_Count; ++i)
	{
		const AppOrientation orientation = static_cast<AppOrientation>(i);
		if (p_name == getAppOrientationName(orientation))
		{
			return orientation;
		}
	}
	
	return AppOrientation_Invalid;
}

// Namespace end
}
}
