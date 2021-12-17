#include <tt/app/Application.h>
#include <tt/engine/renderer/device_enumeration.h>

#include <toki/game/script/wrappers/ResolutionChangerWrapper.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

ResolutionChangerWrapper::Point2s ResolutionChangerWrapper::ms_supportedResolutions;

//--------------------------------------------------------------------------------------------------
// Public member functions

bool ResolutionChangerWrapper::supportsScale(real p_scale)
{
	const tt::math::Point2 nativeResolution(tt::app::getApplication()->getDesktopSize());
	const s32 resolutionY = static_cast<s32>(nativeResolution.y * p_scale + 0.5f);
	return resolutionY >= minimumVerticalResolution;
}


void ResolutionChangerWrapper::setScale(real p_scale)
{
	if (ms_supportedResolutions.empty())
	{
		tt::engine::renderer::Resolutions supportedResolutions = tt::engine::renderer::getSupportedResolutions(true);
	
		for (tt::engine::renderer::Resolutions::const_iterator it = supportedResolutions.begin(); 
			 it != supportedResolutions.end(); ++it)
		{
			ms_supportedResolutions.push_back(*it);
		}
	}
	
	if (supportsScale(p_scale) == false)
	{
		TT_PANIC("Scale '%f' is not supported as resulting y-resolution would be smaller than %d",
		         p_scale, minimumVerticalResolution);
		return;
	}
	
	const tt::math::Point2 nativeResolution(tt::app::getApplication()->getDesktopSize());
	const tt::math::Point2 upscaleSize(
			static_cast<s32>(nativeResolution.x * p_scale + 0.5f),
			static_cast<s32>(nativeResolution.y * p_scale + 0.5f));
	
	/*
	// Find closest fullscreen resolution
	// MARTIJN: Don't change fullscreen resolution; always stick to desktop one
	tt::math::Point2 fullscreenSize(tt::app::getApplication()->getDesktopSize());
	
	for (Point2s::const_iterator it = ms_supportedResolutions.begin(); 
	     it != ms_supportedResolutions.end(); ++it)
	{
		if (it->x >= upscaleSize.x && it->x < fullscreenSize.x &&
			it->y >= upscaleSize.y && it->y < fullscreenSize.y)
		{
			fullscreenSize = *it;
		}
	}
	*/
	
	AppOptions::getInstance().setUpscaleSize(upscaleSize, nativeResolution);
	AppOptions::getInstance().saveIfDirty();
}


real ResolutionChangerWrapper::getScale()
{
	const tt::math::Point2 nativeResolution(tt::app::getApplication()->getDesktopSize());
	const tt::math::Point2 currentResolution(AppOptions::getInstance().upscaleSize);
	return static_cast<real>(currentResolution.y) / static_cast<real>(nativeResolution.y);
}


void ResolutionChangerWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(ResolutionChangerWrapper, "ResolutionChanger");
	TT_SQBIND_STATIC_METHOD(ResolutionChangerWrapper, supportsScale);
	TT_SQBIND_STATIC_METHOD(ResolutionChangerWrapper, setScale);
	TT_SQBIND_STATIC_METHOD(ResolutionChangerWrapper, getScale);
}


// Namespace end
}
}
}
}
