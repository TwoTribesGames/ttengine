#if !defined(INC_TT_APP_APPORIENTATION_H)
#define INC_TT_APP_APPORIENTATION_H


#include <string>


namespace tt {
namespace app {

enum AppOrientation
{
	AppOrientation_Portrait,
	AppOrientation_PortraitUpsideDown,
	AppOrientation_LandscapeLeft,
	AppOrientation_LandscapeRight,
	
	AppOrientation_Count,
	AppOrientation_Invalid
};


inline bool isAppOrientationValid(AppOrientation p_orientation)
{ return p_orientation >= 0 && p_orientation < AppOrientation_Count; }

const char*    getAppOrientationName(AppOrientation p_orientation, bool p_allowInvalid = false);
AppOrientation getAppOrientationFromName(const std::string& p_name);

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_APPORIENTATION_H)
