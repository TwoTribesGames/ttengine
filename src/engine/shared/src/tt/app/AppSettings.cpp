#include <tt/app/AppInterface.h>
#include <tt/args/CmdLine.h>
#include <tt/app/AppSettings.h>
#include <tt/app/Application.h>
#include <tt/engine/renderer/ScreenSettings.h>

namespace tt {
namespace app {



void determineScreenSize(AppSettings* p_settings, AppInterface* p_app, 
                         const tt::math::Point2& p_desktopSize, const args::CmdLine& p_cmdLine)
{
	TT_NULL_ASSERT(p_settings);
	
	// Start with screen size from settings.
	determineScreenSizeFromSettings(p_settings);
	
	TT_NULL_ASSERT(p_app);
	
	// Allow client code to override.
	p_app->overrideGraphicsSettings(&(p_settings->graphicsSettings), p_desktopSize);
	
	// Get the cmd line overrides.
	determineScreenSizeFromCmdLine(p_settings, p_cmdLine);
}


void determineScreenSizeFromSettings(AppSettings* p_settings)
{
	TT_NULL_ASSERT(p_settings);
	using namespace engine::renderer;
	
	tt::math::Point2 screenSize(0, 0);
	
	// Get window size based on emulation settings
	switch (p_settings->emulate)
	{
	case AppSettings::Emulate_None:
		{
			// Nothing todo.
		}
		break;
		
	default:
		TT_PANIC("Invalid emulation mode: %d", p_settings->emulate);
	}
	
	GraphicsSettings& graphicsSettings = p_settings->graphicsSettings;

	if (screenSize.x != 0)
	{
		graphicsSettings.windowedSize.x   = screenSize.x;
		graphicsSettings.fullscreenSize.x = screenSize.x;
	}
	
	if (screenSize.y != 0)
	{
		graphicsSettings.windowedSize.y   = screenSize.y;
		graphicsSettings.fullscreenSize.y = screenSize.y;
	}
	
	if (p_settings->emulate != AppSettings::Emulate_None)
	{
		// Emulation may override the screen limits.
		// Client code should not set these.
		graphicsSettings.minimumSize      = screenSize;
		
		// Use desktop size for fullscreen, but upscale to target res.
		graphicsSettings.fullscreenSize   = math::Point2::zero;
		graphicsSettings.startUpscaleSize = screenSize;
		
		TT_ASSERT(graphicsSettings.clampScreenTo16t9 == false);
		graphicsSettings.clampScreenTo16t9 = false;
		TT_ASSERT(graphicsSettings.clampScreenTo4t3  == false);
		graphicsSettings.clampScreenTo4t3  = false;
		
		// Make sure correction doesn't 'fix' anything.
		TT_ASSERT(graphicsSettings.getCorrectedScreenSize(screenSize) == screenSize);
	}
}


void determineScreenSizeFromCmdLine(AppSettings* p_settings, const args::CmdLine& p_cmdLine)
{
	TT_NULL_ASSERT(p_settings);
	GraphicsSettings& graphicsSettings = p_settings->graphicsSettings;
	//using namespace engine::renderer;
	
	const bool makeScreenWide(p_cmdLine.exists("wide"));
	
	if (p_cmdLine.exists("portrait") || p_cmdLine.exists("landscape"))
	{
		TT_PANIC("Found portrait or landscape commandline argument, "
			        "but current emulation mode %d doesn't support that!",
			        p_settings->emulate);
	}
	
	if (p_settings->emulate == AppSettings::Emulate_None)
	{
		tt::math::Point2 screenSize(0,0);
		
		// Get window resolution from command-line
		if (p_cmdLine.exists("width"))
		{
			screenSize.x = p_cmdLine.getInteger("width");
		}
		if (p_cmdLine.exists("height"))
		{
			screenSize.y = p_cmdLine.getInteger("height");
		}
		
		if (screenSize.x != 0 || screenSize.y != 0)
		{
			// Make sure the (custom) resolution is valid
			screenSize = p_settings->graphicsSettings.getCorrectedScreenSize(screenSize);
			
			if (screenSize.x != 0)
			{
				graphicsSettings.windowedSize.x   = screenSize.x;
				graphicsSettings.fullscreenSize.x = screenSize.x;
			}
			
			if (screenSize.y != 0)
			{
				graphicsSettings.windowedSize.y   = screenSize.y;
				graphicsSettings.fullscreenSize.y = screenSize.y;
			}
		}
		
		if (p_cmdLine.exists("windowed-width"))
		{
			graphicsSettings.windowedSize.x = p_cmdLine.getInteger("windowed-width");
		}
		if (p_cmdLine.exists("windowed-height"))
		{
			graphicsSettings.windowedSize.y = p_cmdLine.getInteger("windowed-height");
		}
	}
	
	if (makeScreenWide)
	{
		const real screenAspectRatio = graphicsSettings.windowedSize.x / 
		                               static_cast<real>(graphicsSettings.windowedSize.y);
		
		if (screenAspectRatio < engine::renderer::wideScreenAspectRatio) // Not wide
		{
			// Make wide
			if (p_cmdLine.exists("resize-height-for-widescreen"))
			{
				// Make it a 16:9 resolution by modifying the window height
				graphicsSettings.windowedSize.y = static_cast<s32>(
					(graphicsSettings.windowedSize.x / engine::renderer::wideScreenAspectRatio) + 0.5f);
			}
			else
			{
				// Make it a 16:9 resolution by modifying the window width (the default)
				graphicsSettings.windowedSize.x = (16 * graphicsSettings.windowedSize.y) / 9;
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Public member functions


tt::math::Point2 GraphicsSettings::getCorrectedScreenSize(const tt::math::Point2& p_size) const
{
	tt::math::Point2 screenSize(p_size);
	
	// Make sure the resolution is valid
	screenSize.x = std::max(screenSize.x, minimumSize.x);
	screenSize.y = std::max(screenSize.y, minimumSize.y);
	
	const real screenAspectRatio = screenSize.x / static_cast<real>(screenSize.y);
	if ((clampScreenTo16t9 && screenAspectRatio > engine::renderer::wideScreenAspectRatio)) // Too wide
	{
		// Clamp to nearest 16:9
		screenSize.x = (16 * screenSize.y) / 9;
	}
	
	if (clampScreenTo4t3 && screenAspectRatio < engine::renderer::standardScreenAspectRatio) // Too narrow
	{
		screenSize.x = (4 * screenSize.y) / 3;
	}
	
	return screenSize;
}


tt::math::Point2 GraphicsSettings::getScreenSize(bool p_windowed) const
{
	tt::math::Point2 screenSize( (p_windowed) ? windowedSize : fullscreenSize);
	
	if (p_windowed == false) // Full screen
	{
		tt::math::Point2 desktopSize(getApplication()->getDesktopSize());
		if (screenSize.x == 0)
		{
			screenSize.x = desktopSize.x;
		}
		
		if (screenSize.y == 0)
		{
			screenSize.y = desktopSize.y;
		}
	}
	
	return getCorrectedScreenSize(screenSize);
}



// Namespace end
}
}
