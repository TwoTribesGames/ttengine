#if !defined(INC_TOKI_SCREENSHOTSETTINGS_H)
#define INC_TOKI_SCREENSHOTSETTINGS_H


#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>

namespace toki {

enum ScreenshotType
{
	ScreenshotType_None,         // don't take a screenshot
	ScreenshotType_FullLevel,    // save an image with all of the level visible
	ScreenshotType_LevelPreview  // for Steam Workshop preview images
};


struct ScreenshotSettings
{
	ScreenshotType     type;
	// NOTE: The members below are currently only used with the ScreenshotType_LevelPreview type
	s32                width;
	s32                height;
	std::string        filename;
	tt::fs::identifier fileSystem;
	std::string        failureFallbackImage;  // image to save/copy to 'filename' if taking screenshot fails
	
	inline ScreenshotSettings()
	:
	type(ScreenshotType_None),
	width(0),
	height(0),
	filename(),
	fileSystem(0),
	failureFallbackImage()
	{ }
};

// Namespace end
}


#endif  // !defined(INC_TOKI_SCREENSHOTSETTINGS_H)
