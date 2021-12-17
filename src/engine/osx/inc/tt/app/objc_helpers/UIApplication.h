#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iPhone builds only

#import <UIKit/UIKit.h>

#include <tt/app/AppOrientation.h>
#include <tt/platform/tt_error.h>

namespace tt {
namespace app {


// Translates iOS SDK 'UIInterfaceOrientation' to TT AppOrientation
inline AppOrientation getAppOrientation(UIInterfaceOrientation p_iosOrientation)
{
	switch (p_iosOrientation)
	{
	case UIInterfaceOrientationPortrait:           return AppOrientation_Portrait;
	case UIInterfaceOrientationPortraitUpsideDown: return AppOrientation_PortraitUpsideDown;
	case UIInterfaceOrientationLandscapeLeft:      return AppOrientation_LandscapeLeft;
	case UIInterfaceOrientationLandscapeRight:     return AppOrientation_LandscapeRight;
	
	default:
		TT_PANIC("Unsupported UIInterfaceOrientation: %d", p_iosOrientation);
		return AppOrientation_Invalid;
	}
}


inline UIInterfaceOrientation getUIInterfaceOrientation(AppOrientation p_appOrientation)
{
	switch (p_appOrientation)
	{
	case AppOrientation_Portrait:           return UIInterfaceOrientationPortrait; 
	case AppOrientation_PortraitUpsideDown: return UIInterfaceOrientationPortraitUpsideDown;
	case AppOrientation_LandscapeLeft:      return UIInterfaceOrientationLandscapeLeft;
	case AppOrientation_LandscapeRight:     return UIInterfaceOrientationLandscapeRight;
	
	default:
		TT_PANIC("Unsupported AppOrientation: %d", p_appOrientation);
		return UIInterfaceOrientationLandscapeRight;
	}
}
	

inline AppOrientation getAppOrientation(UIDeviceOrientation p_deviceOrientation)
{
	switch (p_deviceOrientation)
	{
		case UIDeviceOrientationPortrait:           return AppOrientation_Portrait;
		case UIDeviceOrientationPortraitUpsideDown: return AppOrientation_PortraitUpsideDown;
		case UIDeviceOrientationLandscapeRight:     return AppOrientation_LandscapeLeft;
		case UIDeviceOrientationLandscapeLeft:      return AppOrientation_LandscapeRight;
		
		case UIDeviceOrientationUnknown:            return AppOrientation_Invalid;
		case UIDeviceOrientationFaceUp:             return AppOrientation_Invalid;
		case UIDeviceOrientationFaceDown:           return AppOrientation_Invalid;
			
		default:
			TT_PANIC("Unsupported UIDeviceOrientation: %d", p_deviceOrientation);
			return AppOrientation_Invalid;
	}
}

// End namespace
}
}

#endif  // defined(TT_PLATFORM_OSX_IPHONE)
