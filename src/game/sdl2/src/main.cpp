#include <tt/app/Application.h>
#ifdef TT_PLATFORM_WIN
#include <tt/app/ComHelper.h>
#endif
#include <tt/app/SDL2App.h>
#include <tt/args/CmdLine.h>
#include <tt/args/CmdLineSDL2.h>
#if defined(TT_STEAM_BUILD)
#	include <tt/app/SteamAppSystems.h>
#endif
#include <tt/snd/OpenALSoundSystem.h>
#include <tt/input/MouseController.h>

#include <toki/__revision_autogen.h>
#include <toki/unittest/unittest.h>
#include <toki/AppGlobal.h>
#include <toki/AppMain.h>

#ifndef TT_BUILD_FINAL
#if defined(TT_PLATFORM_WIN)
#include <tt/fs/WindowsFileSystem.h>
typedef tt::fs::WindowsFileSystem  SDL2FileSystem;
#else
#include <tt/fs/PosixFileSystem.h>
typedef tt::fs::PosixFileSystem  SDL2FileSystem;
#endif
// For converting leveldata to text
#include <toki/game/editor/helpers.h>
#include <toki/level/LevelData.h>

// For squirrel compile.
#include <toki/audio/AudioPlayer.h>
#include <toki/main/loadstate/LoadStateAudioPlayer.h>
#include <toki/main/loadstate/LoadStateScriptLists.h>
#include <toki/main/loadstate/LoadStateScriptMgr.h>
#include <toki/script/ScriptMgr.h>
#endif

#include <SDL2/SDL_main.h>

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
};


int main(int p_argc, char** p_argv)
{
	tt::args::setArgcArgv(p_argc, p_argv);
	
#if !defined(TT_BUILD_FINAL)
	const tt::args::CmdLine cmdLine(p_argc, p_argv);
	
	/*
	if (cmdLine.exists("unittest"))
	{
		return runUnitTests();
	}
	*/
#endif // !defined(TT_BUILD_FINAL)
	
	toki::AppGlobal::parseCommandLineFlags();
	
	// Create a new application with default settings
	tt::app::AppSettings settings;

	settings.region = tt::settings::Region_WW;
	
#if defined(TT_STEAM_BUILD)
	settings.useMemoryFS = true;
	settings.platform = tt::app::AppSettings::Platform_Steam;
	settings.useCloudFS = true;
#else
	settings.useMemoryFS = true;
	settings.platform = tt::app::AppSettings::Platform_StandAlone;
	settings.useCloudFS = false;
#endif
	settings.systems.reset(new GameAppSystems);
	
#if TT_DEMO_BUILD == 0
#if !defined(TT_BUILD_FINAL) && defined(TT_STEAM_BUILD)
	settings.name      = "RIVE Steam";
#else
	settings.name      = "RIVE";
#endif
#else
	settings.name      = "RIVE Demo";
#endif // TT_DEMO_BUILD
	if (toki::AppGlobal::isInLevelEditorMode())
	{
		settings.name += " Level Editor";
	}
	settings.version   = TT_REVISION_NUMBER;
	settings.portrait  = false;
	settings.targetFPS = 60;
	settings.emulate   = tt::app::AppSettings::Emulate_None;
	
	settings.useFixedDeltaTime = false;
	//settings.useFixedDeltaTime = true;
	
#if defined(TT_BUILD_FINAL)
	settings.graphicsSettings.startWindowed = false;
#endif
	settings.graphicsSettings.minimumSize.setValues(184, 162);
	settings.graphicsSettings.windowedSize.setValues(1280, 720);
	settings.graphicsSettings.startUpscaleSize.setValues(1920, 1200); // Full HD 16:10
	settings.graphicsSettings.allowHotKeyFullScreenToggle = true;
	settings.graphicsSettings.aspectRatioRange.setValues(16.0f/11.0f, 16.0f/8.65f);
	settings.graphicsSettings.allowResize = true;

	#if !defined(TT_BUILD_FINAL)
	if (tt::app::getCmdLine().exists("supress_asserts"))
	{
		tt::platform::error::supressAssertsAndWarnings();
	}
	
	if (tt::app::getCmdLine().exists("emulate_cat"))
	{
		settings.emulate = tt::app::AppSettings::Emulate_Cat;
		settings.graphicsSettings.clampScreenTo4t3  = false;
		settings.graphicsSettings.clampScreenTo16t9 = false;
	}
	
	if (toki::AppGlobal::shouldCompileSquirrel())
	{
		tt::fs::FileSystemPtr fs = SDL2FileSystem::instantiate(0, "Compile Squirrel");
		if (fs == 0)
		{
			return 1;
		}
		
		const std::string workDir(fs->getWorkingDir());
		if (fs->setWorkingDir(workDir) == false)
		{
			return 1;
		}
		
		// By pass WinApp creation (specificly Renderer creation) so this also works on the build server.
		
		typedef std::list<toki::main::loadstate::LoadStatePtr> LoadStates;
		LoadStates loadStates;
		loadStates.push_back(toki::main::loadstate::LoadStateScriptLists::create());
		loadStates.push_back(toki::main::loadstate::LoadStateAudioPlayer::create());
		loadStates.push_back(toki::main::loadstate::LoadStateScriptMgr::create());
		
		while (loadStates.empty() == false)
		{
			const toki::main::loadstate::LoadStatePtr& state(loadStates.front());
			state->doLoadStep();
			if (state->isDone())
			{
				loadStates.pop_front();
			}
			
			// AudioPlayer creation logic duplicated from StateLoadApp:
			// Check if we should create the audio player on the main thread
			if (toki::audio::AudioPlayer::needsCreateOnMainThread() &&
			    toki::audio::AudioPlayer::hasInstance() == false    &&
			    toki::audio::AudioPlayer::shouldCreateOnMainThreadNow())
			{
				//TT_Printf("StateLoadApp::update: Creating AudioPlayer on main thread.\n");
				toki::audio::AudioPlayer::createInstance();
			}
		}
		
		// Clean up static resources.
		toki::script::ScriptMgr::deinit();
		
		return 0;
	}
#endif

#ifdef TT_PLATFORM_WIN
	// Use single-threaded COM (needed for tt::http::HttpConnectMgr::openUrlExternally)
	tt::app::ComHelper::setMultiThreaded(false);
#endif

	tt::app::SDL2App app(new toki::AppMain, settings, "app_icon.png");

	auto cursor = tt::input::SDLMouseCursor::create("cursors/game_cursor.cur");
	tt::input::SDLMouseController::setDefaultCursor(cursor);

	return app.run();
}
