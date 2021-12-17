#import "ClientObjCApp.h"

#if defined(TT_STEAM_BUILD)
#include <tt/app/SteamAppSystems.h>
#endif
#include <tt/snd/OpenALSoundSystem.h>


#define SDL_THROUGH_APP_SYSTEMS 1

#if SDL_THROUGH_APP_SYSTEMS
#	include <tt/input/SDLJoyPadController.h>
#	include <tt/input/SDLMouseController.h>
#	include <tt/input/SDLKeyboardController.h>
#endif

#include <toki/__revision_autogen.h>
#include <toki/AppGlobal.h>
#include <toki/AppMain.h>


// App systems instantiator
class GameAppSystems :
#if defined(TT_STEAM_BUILD)
		public tt::app::SteamAppSystems
#else
		public tt::app::AppSystems
#endif
{
public:
	GameAppSystems()
	:
#if defined(TT_STEAM_BUILD)
	tt::app::SteamAppSystems()
#else
	tt::app::AppSystems()
#endif
	{ }
	
	virtual tt::snd::SoundSystemPtr instantiateSoundSystem()
	{
#if !defined(TT_BUILD_FINAL)
		if (toki::AppGlobal::isAudioInSilentMode())
		{
			return tt::snd::SoundSystemPtr();
		}
#endif
		
		return tt::snd::OpenALSoundSystem::instantiate(0, true);
	}
	
#if SDL_THROUGH_APP_SYSTEMS
	virtual inline void sdl_init()
	{
		if (tt::input::SDLMouseController::initialize() == false)
		{
			TT_PANIC("SDLMouseController::initialize failed.");
		}
		if (tt::input::SDLKeyboardController::initialize() == false)
		{
			TT_PANIC("SDLKeyboardController::initialize failed.");
		}
		if (tt::input::SDLJoypadController::initialize() == false)
		{
			TT_PANIC("SDLJoypadController::initialize failed.");
		}
	}
	
	virtual inline void sdl_shutdown()
	{
		tt::input::SDLMouseController::deinitialize();
		tt::input::SDLKeyboardController::deinitialize();
		tt::input::SDLJoypadController::deinitialize();
	}
	
	virtual inline void sdl_update()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			tt::input::SDLMouseController::processEvent(event);
			tt::input::SDLKeyboardController::processEvent(event);
			tt::input::SDLJoypadController::processEvent(event);
		}
		
		tt::input::SDLMouseController::update();
		tt::input::SDLKeyboardController::update();
		tt::input::SDLJoypadController::update();
	}
#endif
};



@implementation ClientObjCApp

- (void)getAppSettings:(tt::app::AppSettings*)p_settings
{
	toki::AppGlobal::parseCommandLineFlags();
	
#if defined(TT_STEAM_BUILD)
	p_settings->platform   = tt::app::AppSettings::Platform_Steam;
	p_settings->useCloudFS = true;
#else
	p_settings->platform   = tt::app::AppSettings::Platform_StandAlone;
	p_settings->useCloudFS = false;
#endif
	p_settings->systems.reset(new GameAppSystems);
	//p_settings->useFixedDeltaTime = true;
	
#if TT_DEMO_BUILD == 0
	p_settings->name       = "Toki Tori 2+";
#else
	p_settings->name       = "Toki Tori 2+ Demo";
#endif
	if (toki::AppGlobal::isInLevelEditorMode())
	{
		p_settings->name += " Level Editor";
	}
	p_settings->version    = TT_REVISION_NUMBER;
	p_settings->portrait   = false;
	p_settings->targetFPS  = 60;
	p_settings->emulate    = tt::app::AppSettings::Emulate_None;
	p_settings->useFixedDeltaTime = false;
	
	p_settings->graphicsSettings.minimumSize.setValues (184, 162);
	p_settings->graphicsSettings.windowedSize.setValues(1280, 720);
	p_settings->graphicsSettings.startUpscaleSize.setValues(1920, 1200);
	p_settings->graphicsSettings.aspectRatioRange.setValues(1280.0f/1024.0f, 1366.0f/768.0f);
	p_settings->graphicsSettings.allowHotKeyFullScreenToggle = true;
	p_settings->graphicsSettings.allowResize = true;
}


- (tt::app::AppInterface*)createAppInterface:(tt::app::AppSettings*)p_settings
{
	(void)p_settings;
	return new toki::AppMain;
}

@end
