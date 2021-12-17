#if !defined(INC_TT_APP_APPSETTINGS_H)
#define INC_TT_APP_APPSETTINGS_H


#include <string>

#include <tt/app/AppSystems.h>
#include <tt/platform/tt_types.h>
#include <tt/math/Point2.h>
#include <tt/settings/settings.h>

// Forward declaration.
namespace tt {
	namespace app {
		class AppInterface;
	}
	namespace args {
		class CmdLine;
	}
}


namespace tt {
namespace app {

struct AppSettings; // forward declaration

void determineScreenSize(AppSettings* p_settings, AppInterface* p_app, 
                         const tt::math::Point2& p_desktopSize, const args::CmdLine& p_cmdLine);
void determineScreenSizeFromSettings(AppSettings* p_settings);
void determineScreenSizeFromCmdLine(AppSettings* p_settings, const args::CmdLine& p_cmdLine);

struct SaveDataSettings
{
	void* data;
	size_t size;
	
	SaveDataSettings()
	:
	data(nullptr),
	size(0)
	{}
};


struct GraphicsSettings
{
	bool         startWindowed;     //!< If app should start windowed. (Set to false for full screen.)
	bool         allowResize;       //!< If set to true the app window is resizable
	math::Point2 windowedSize;      //!< Try to match this size when windowed.
	math::Point2 fullscreenSize;    //!< Try to match this size when full screen. (0,0 is desktop size.)
	math::Point2 minimumSize;       //!< The minimum size. Will make sure window size does not get below this.
	math::Point2 startUpscaleSize;  //!< The maximum size. Will upscale when window gets larger. 0 is unlimited.
	math::Vector2 aspectRatioRange; //!< Valid range for aspect ratio, bars are shown if outside range
	bool         clampScreenTo16t9; //!< When set to true the aspect ratio may not exceed 16:9
	bool         clampScreenTo4t3;  //!< When set to true the aspect ratio may not drop below 4:3
	bool         allowHotKeyFullScreenToggle; //!< Whether toggling full screen mode using a hotkey/shortcut is allowed
	s32          colorBufferBits;     //!< Bit depth of color buffer
	s32          depthBufferBits;     //!< Bit depth of depth buffer, use 0 for no depth buffer
	s32          stencilBufferBits;   //!< Bit depth of stencil buffer, use 0 for no stencil buffer
	s32          antiAliasingSamples; //!< Number of AA samples requested for backbuffer
	bool         useIOS2xMode;        //!< Whether to enable 2x ("Retina") mode if available.
	
	GraphicsSettings()
	:
	startWindowed(true),
	allowResize(false),
	windowedSize(1280, 720),
	fullscreenSize(0, 0),
	minimumSize(800, 600),
	startUpscaleSize(0, 0),
	aspectRatioRange(0.0f, 9999.0f),
	clampScreenTo16t9(false),
	clampScreenTo4t3(false),
	allowHotKeyFullScreenToggle(false),
	colorBufferBits(32),
	depthBufferBits(0),
	stencilBufferBits(0),
	antiAliasingSamples(0),
	useIOS2xMode(false)
	{}
	
	tt::math::Point2 getCorrectedScreenSize(const tt::math::Point2& p_point) const;
	tt::math::Point2 getScreenSize(bool p_windowed) const;
};


struct AppSettings
{
	enum Emulation
	{
		Emulate_None,       // No emulation. Normal Windows.
	};

	enum Platform
	{
		Platform_Steam,
		Platform_StandAlone // Use this if no specific API integration is needed
	};
	
	settings::Region region;
	std::string      name;
	s32              version;
	std::string      versionString;
	
	/*! \brief The target FPS used for frame limiting,
	           or the target for fixed delta time.
	    \note  60 is used if target is 0. */
	s32              targetFPS;
	
	/*! \brief Flag to indicate if updates should use a fixed delta time.
	    \note  Uses targetFPS for most platforms.*/
	bool             useFixedDeltaTime;
	bool             useCloudFS;       //!< Use cloud FS if available
	bool             useMemoryFS;      //!< Use memory FS
	std::string      windowsDir;    //!< Custom directory for windows build
	std::string      emulationOverrideDir; //!< If non-empty, use this directory for assets instead of the default emulation directory.
	bool             hideCursor; //!< Hide (windows mouse) cursor. (Used for e.g. custom cursor rendering.)
	AppSystemsPtr    systems;
	Emulation        emulate;  //!< Which platform to emulate
	bool             portrait; //!< Screen orientation
	Platform         platform; //!< Targeted platform/service
	GraphicsSettings graphicsSettings;
	SaveDataSettings saveDataSettings;
	
	/*! \brief Constructor with default values.
	    \param p_systems Subsystems instantiator.*/
	AppSettings(const AppSystemsPtr& p_systems = AppSystemsPtr())
	:
	region(settings::Region_WW),
	name("Two Tribes Application"),
	version(0),
	versionString("undef"),
	targetFPS(0),
	useFixedDeltaTime(false),
	useCloudFS(false),
	useMemoryFS(false),
	windowsDir("/win"),
	emulationOverrideDir(),
	hideCursor(false),
	systems(p_systems),
	emulate(Emulate_None),
	portrait(false),
	platform(Platform_StandAlone),
	graphicsSettings()
	{ }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_APPSETTINGS_H)
