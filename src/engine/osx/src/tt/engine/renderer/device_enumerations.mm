#import <Cocoa/Cocoa.h>
#include <tt/engine/renderer/device_enumeration.h>


namespace tt {
namespace engine {
namespace renderer {


Resolutions getSupportedResolutions(bool /*p_keepDesktopAspectRatio*/)
{
	Resolutions resolutions;
	
	NSRect frame = [[NSScreen mainScreen] frame];
	
	const math::Point2 desktopSize(frame.size.width, frame.size.height);
	resolutions.insert(desktopSize);
	
	// NOTE: OSX does not recommend changing the desktop resolution so only return
	//       the native resolution here and let application handle upscaling
	
	/*
	const real desktopAspectRatio = desktopSize.x / static_cast<real>(desktopSize.y);
	
	// Find out which aspect ratio we are closests to
	const real distTo4_3   = math::fabs(desktopAspectRatio - ( 4.0f/ 3.0f));
	const real distTo16_10 = math::fabs(desktopAspectRatio - (16.0f/10.0f));
	const real distTo16_9  = math::fabs(desktopAspectRatio - (16.0f/ 9.0f));
	
	real aspectRatioToUse(0);
	if      (distTo4_3   < distTo16_10 && distTo4_3   < distTo16_9) aspectRatioToUse =  4.0f/ 3.0f;
	else if (distTo16_10 < distTo4_3   && distTo16_10 < distTo16_9) aspectRatioToUse = 16.0f/10.0f;
	else                                                            aspectRatioToUse = 16.0f/ 9.0f;
	
	static const s32 numberOfResolutions = 5;
	static const s32 targetWidth[numberOfResolutions] = {640, 800, 1024, 1280, 1600};
	
	for (s32 i = 0; i < numberOfResolutions; ++i)
	{
		// Only add resolutions that are lower than the desktop resolution
		if (targetWidth[i] >= desktopSize.x) break;
		
		resolutions.insert(
		   math::Point2(targetWidth[i], static_cast<s32>((targetWidth[i] / aspectRatioToUse) + 0.5f)));
	}
	//*/
	
	return resolutions;
}


// Namespace end
}
}
}
