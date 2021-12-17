#include <tt/app/Application.h>
#include <tt/app/ComHelper.h>
#include <tt/app/WinApp.h>
#if defined(TT_STEAM_BUILD)
#	include <tt/app/SteamAppSystems.h>
	
#	if defined(TT_BUILD_FINAL)
		// Only send a steam minidump for steam final builds. (Don't send for internal builds.)
#		define USE_STEAM_MINIDUMP
#	endif
#endif
#include <tt/snd/OpenALSoundSystem.h>
#include <tt/snd/XAudio2SoundSystem.h>
#include <tt/input/MouseController.h>

#define SDL_THROUGH_APP_SYSTEMS 1

#if SDL_THROUGH_APP_SYSTEMS
#	include <tt/input/SDLJoyPadController.h>
#	include <tt/input/SDLMouseController.h>
#	include <tt/input/SDLKeyboardController.h>
#endif


#ifdef USE_STEAM_MINIDUMP
#if defined(TT_STEAM_BUILD)
#include <steam/steam_api.h>
#endif
#endif

#include <toki/unittest/unittest.h>
#include <toki/AppGlobal.h>
#include <toki/AppMain.h>

#include "../../windows/src/resource.h"
#include "audio/constants_win.h"

#ifndef TT_BUILD_FINAL
// For converting leveldata to text
#include <tt/fs/WindowsFileSystem.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/LevelData.h>

// For squirrel compile.
#include <toki/audio/AudioPlayer.h>
#include <toki/main/loadstate/LoadStateAudioPlayer.h>
#include <toki/main/loadstate/LoadStateScriptLists.h>
#include <toki/main/loadstate/LoadStateScriptMgr.h>
#include <toki/script/ScriptMgr.h>
#endif

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
		
#if defined(USE_OPENAL_BACKEND)
		return tt::snd::OpenALSoundSystem::instantiate(0, true);
#else
		return tt::snd::XAudio2SoundSystem::instantiate(0, true);
#endif
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


#if !defined(TT_BUILD_FINAL)
inline std::string getActualPathName(const std::string& p_path)
{
	// One way to get the actual on-disk casing of a path is to convert the path to a "short path" first,
	// then convert that short path back into a "long path". However, this does not always work.
	// (for example, it fails if the application does not have rights to list directory contents
	//  of one of the path components)
	
	// First turn the input path into a short path
	std::unique_ptr<char[]> shortPathBuffer;
	{
		DWORD shortPathLength = 0;
		
		// Get the buffer size needed
		shortPathLength = GetShortPathNameA(p_path.c_str(), 0, 0);
		if (shortPathLength == 0)
		{
			TT_PANIC("Retrieving short path length for path '%s' failed.", p_path.c_str());
			return p_path;
		}
		
		// Create required buffer and convert the path for real
		shortPathBuffer.reset(new char[shortPathLength]);
		shortPathLength = GetShortPathNameA(p_path.c_str(), shortPathBuffer.get(), shortPathLength);
		if (shortPathLength == 0)
		{
			TT_PANIC("Converting path '%s' to a short path failed.", p_path.c_str());
			return p_path;
		}
	}
	
	// Now convert the short path back into a long path
	DWORD longPathLength = GetLongPathNameA(shortPathBuffer.get(), 0, 0);
	if (longPathLength == 0)
	{
		TT_PANIC("Retrieving long path length for short path '%s' failed.\nOriginal path: '%s'",
		         shortPathBuffer.get(), p_path.c_str());
		return p_path;
	}
	
	std::unique_ptr<char[]> longPathBuffer(new char[longPathLength]);
	longPathLength = GetLongPathNameA(shortPathBuffer.get(), longPathBuffer.get(), longPathLength);
	if (longPathLength == 0)
	{
		TT_PANIC("Converting short path '%s' to a long path failed.\nOriginal path: '%s'",
		         shortPathBuffer.get(), p_path.c_str());
		return p_path;
	}
	
	return longPathBuffer.get();
}
#endif


int realMain()
{
#if !defined(TT_BUILD_FINAL)
	if (tt::app::getCmdLine().exists("unittest"))
	{
		return runUnitTests();
	}
	else if (tt::app::getCmdLine().exists("level_to_text"))
	{
		tt::fs::FileSystemPtr fs = tt::fs::WindowsFileSystem::instantiate(0, "Convert level to text");
		const std::string levelFilename(
			getActualPathName(tt::app::getCmdLine().getString("level_to_text")));
		toki::level::LevelDataPtr level = toki::level::LevelData::loadLevel(levelFilename);
		if (level == 0)
		{
			std::cerr << "Failed to open level " << levelFilename;
			return 1;
		}
		const std::string result = toki::game::editor::getLevelDataAsString(level);
		
		std::cout << result;
		return 0;
	}
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
	settings.version   = 1;
	settings.portrait  = false;
	settings.targetFPS = 60;
	settings.emulate   = tt::app::AppSettings::Emulate_None;
	
#if defined(TT_STEAM_BUILD) && defined(TT_BUILD_FINAL)
	settings.windowsDir    = "\\";
#else
	settings.windowsDir    = "\\win";
#endif
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
	
	if (toki::AppGlobal::shouldCompileSquirrel())
	{
		tt::fs::FileSystemPtr fs = tt::fs::WindowsFileSystem::instantiate(0, "Compile Squirrel");
		if (fs == 0)
		{
			return 1;
		}
		
		const std::string workDir(fs->getWorkingDir() + settings.windowsDir);
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
	
	// Use single-threaded COM (needed for tt::http::HttpConnectMgr::openUrlExternally)
	tt::app::ComHelper::setMultiThreaded(false);
	
	tt::app::WinApp app(new toki::AppMain, settings);
	
	// Disable DXUT hotkeys
	const bool allowAltEnterFullScreen = true;
	DXUTSetHotkeyHandling(allowAltEnterFullScreen, false, false);
	
	// Load the game cursor
	HCURSOR cursor = static_cast<HCURSOR>(::LoadImage(NULL, L"cursors/game_cursor.cur", IMAGE_CURSOR, 64, 64, LR_LOADFROMFILE));
	tt::input::MouseController::setDefaultCursor(cursor);
	
	return app.run();
}


//-----------------------------------------------------------------------------
// Generate a minidump on the user's computer in the user's Temp folder
//-----------------------------------------------------------------------------
void miniDumpGenerator(unsigned int p_exceptionCode, EXCEPTION_POINTERS* p_exception)
{
	(void)p_exceptionCode;
	
#ifdef USE_STEAM_MINIDUMP
	// You can build and set an arbitrary comment to embed in the minidump here,
	// maybe you want to put what level the user was playing, how many players on the server,
	// how much memory is free, etc...
	SteamAPI_SetMiniDumpComment("Minidump comment: no comment\n");
	
	SteamAPI_WriteMiniDump(p_exceptionCode, p_exception, TT_REVISION_NUMBER);
	
	MessageBoxA(
		0,
		"An unknown error occured, causing the application to shut down.\n"
		"If the problem persists, please contact the developer at http://www.twotribes.com/support",
		"Fatal Error",
		0);
#else
	
	#if defined(TT_BUILD_FINAL) && defined(TT_STEAM_BUILD)
		#error Steam minidump must be used for final Steam builds! (USE_STEAM_MINIDUMP should be defined!)
	#endif
	
	// Generate the minidump
	tt::app::WinApp::generateDump(p_exception);
	
	// TODO: Provide a way to send the dump
	// Inform the user
	MessageBoxA(0,
	            "A dump file has been generated in your temporary folder\n"
	            "Please contact the developer.",
	            "Unhandled Exception", 0);
#endif
}
