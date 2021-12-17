#include <sstream>

#ifdef TT_BUILD_DEV
#define D3D_DEBUG_INFO
#endif

#define NOMINMAX
#include <windows.h>

#pragma warning (push)
#pragma warning (disable:4091)
#include <dbghelp.h>
#pragma warning (pop)

#include <fcntl.h>
#include <intrin.h> // Needed for __cpuid. Remove!
#include <io.h>
#include <shellapi.h>
#include <shlobj.h>

#include <visualboy/VisualBoy.h>

#include <tt/app/ComHelper.h>
#include <tt/app/fatal_error.h>
#include <tt/app/WinApp.h>
#include <tt/app/Platform.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/DXUT/DXUT.h>
#include <tt/engine/renderer/DXUT/DXUTcamera.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/fs/MemoryFileSystem.h>
#include <tt/fs/WindowsFileSystem.h>
#include <tt/http/HttpConnectMgr.h>
#include <tt/input/ControllerType.h>
#if !defined(TT_BUILD_FINAL)
#include <tt/input/IphoneController.h>
#endif
#include <tt/input/KeyboardController.h>
#include <tt/input/MouseController.h>
#include <tt/input/SDLJoyPadController.h>
#include <tt/input/SDLMouseController.h>
#include <tt/input/SDLKeyboardController.h>
#include <tt/input/Xbox360Controller.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_error_win.h>
#include <tt/str/str.h>
#include <tt/system/CPUInfo.h>
#include <tt/system/Time.h>
#include <tt/thread/ThreadedWorkload.h>
#include <tt/version/Version.h>


// Defines
//#define DEBUG_VS   // Uncomment this line to debug vertex shaders
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders

#define USE_SDL_INPUT 0 // Change to 1 for SDL input support. (Will need to link to sdl lib, and have dll!)


////////////////////////////////////////////////////////////
// 
//  Callback declarations for the DXUT framework
//
static bool    CALLBACK isDeviceAcceptable(D3DCAPS9*,D3DFORMAT,D3DFORMAT,bool,void*);
static bool    CALLBACK modifyDeviceSettings(DXUTDeviceSettings*,void*);

static HRESULT CALLBACK onCreateDevice (IDirect3DDevice9*,const D3DSURFACE_DESC*,void*);
static void    CALLBACK onDestroyDevice(void*);

static LRESULT CALLBACK msgProc(HWND, UINT, WPARAM, LPARAM, bool*, void*);

static bool g_needToModifyDeviceSettingsWindowed   = true;
static bool g_needToModifyDeviceSettingsFullScreen = true;


namespace tt {
namespace app {

////////////////////////////////////////////////////////////////////////////////////////////
//
// Fatal Error reporting
//

void reportDXUTError(HRESULT p_hr)
{
	switch(p_hr)
	{
	case DXUTERR_NODIRECT3D            : reportFatalError("DXUTInit: Could not initialize Direct3D.");
	case DXUTERR_NOCOMPATIBLEDEVICES   : reportFatalError("DXUTInit: Could not find any compatible Direct3D devices.");
	case DXUTERR_MEDIANOTFOUND         : reportFatalError("DXUTInit: Could not find required media.");
	case DXUTERR_NONZEROREFCOUNT       : reportFatalError("DXUTInit: The Direct3D device has a non-zero reference count.");
	case DXUTERR_CREATINGDEVICE        : reportFatalError("DXUTInit: An error occurred when attempting to create a Direct3D device.");
	case DXUTERR_RESETTINGDEVICE       : reportFatalError("DXUTInit: An error occurred when attempting to reset a Direct3D device.");
	case DXUTERR_CREATINGDEVICEOBJECTS : reportFatalError("DXUTInit: An error occurred in the device create callback function.");
	case DXUTERR_RESETTINGDEVICEOBJECTS: reportFatalError("DXUTInit: An error occurred in the device reset callback function.");
	case DXUTERR_DEVICEREMOVED         : reportFatalError("DXUTInit: The Direct3D device was removed.");
	default                            : reportFatalError("DXUTInit: Unknown error occured while initializing DXUT.");
	}
}


#if !defined(TT_BUILD_FINAL)
// This function is registered with tt error to be called when a panic is triggered.
static void onPanic()
{
	// If we are running in fullscreen, first switch back to windowed
	if(DXUTIsWindowed() == FALSE)
	{
		DXUTToggleFullScreen();
	}
}
#endif //#if !defined(TT_BUILD_FINAL)


u32   FrameRateManager::ms_targetFPS    = 0;
float FrameRateManager::ms_averageFPS   = 0;
bool  FrameRateManager::ms_vsyncEnabled = false;
FrameRateManager::NativeRefreshRates FrameRateManager::ms_nativeRefreshRates;
FrameRateManager::FrameRates         FrameRateManager::ms_framerateHistory;


//--------------------------------------------------------------------------------------------------
// Public member functions

WinApp::WinApp(AppInterface* p_app, const AppSettings& p_settings)
:
m_startupState(p_settings.version, version::getLibRevisionNumber(), p_settings.name),
m_creationThreadId(GetCurrentThreadId()),
m_shouldPostQuitMessage(false),
m_postQuitMessageMutex(),
m_settings(p_settings),
m_app(p_app),
m_initialized(false),
m_frameTime(system::Time::getInstance()->getMicroSeconds()),
m_targetTimeSlice(16667),
m_fps30Mode(false),
m_active(true),
m_frameLimiterEnabled(true),
m_frameLimiterForced(false),
#if !defined(TT_BUILD_FINAL)
m_curWaitFrame(0),
m_totalWaitFrames(0),
m_frameStepMode(false),
m_shouldDisplayDebugInfo(false),
m_displayConsole(false),
#endif
m_cmdLine(args::CmdLine::getApplicationCmdLine()),
m_debugKeys(DebugKeys_All)
{
	// Check for memory leaks in debug builds
#if defined(TT_BUILD_DEV)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc();
#endif
	
	// FIXME: This check might be too late.
	tt::system::CPUInfo info = tt::system::getCPUInfo();
	if (info.cpuFeatures.checkFlag(tt::system::CpuFeature_SSE2) == false)
	{
		if (IsProcessorFeaturePresent( PF_XMMI64_INSTRUCTIONS_AVAILABLE ) == FALSE)
		{
			reportFatalError("This program needs a processor with SSE2 support in order to run.\n");
		}
		else
		{
			// False positive when using cpuinfo!?
			int buf[4];
			__cpuid(buf, 1); // When removing this line also remove #include <intrin.h>!
			
			int bufTwo[4];
			__cpuid(bufTwo, 0);
			
			std::string message("This program needs a processor with SSE2 support in order to run.\n"
			                    "CPU id: ");
			message += info.idString;
			const u32 features = info.cpuFeatures.getFlags();
			message += "\nF: " + tt::str::toStr(features);
			message += " R: " + tt::str::toStr(u32(buf   [3]));
			tt::system::CPUInfo::CpuFeatures testBM = tt::system::CPUInfo::CpuFeatures(buf[3]);
			const u32 bitmask = testBM.getFlags();
			message += "\nBM: " + tt::str::toStr(bitmask);
			message += " C: " + tt::str::toStr(u32(bufTwo[0])); // Max value for InfoType.
			MessageBoxA(DXUTGetHWND(), message.c_str(), "Error", MB_OK);
		}
	}
	
#ifndef TT_BUILD_FINAL
	if (m_cmdLine.exists("console"))
	{
		toggleConsole();
	}
#endif
	
	// Always initialize COM (since lots of Windows services need this)
	ComHelper::initCom();
	
	TT_NULL_ASSERT(m_app);
	registerPlatformCallbackInterface(m_app);
	
	m_startupState.setStartupStep(StartupStep_SystemInit);
	
	//
	// General Initialization
	//
	version::setClientVersionInfo(m_settings.version, m_settings.versionString.c_str());
	settings::setRegion(m_settings.region);
	settings::setApplicationName(str::widen(m_settings.name));
	
#ifdef TT_BUILD_FINAL
	// ---------------------------------------------------------------------------------------------
	// At start of the function the cmdline settings are always available (also in Final!)
	
	const bool showBuildLabel(m_cmdLine.exists("version"));
	
	str::Strings keepThese;
	
	keepThese.push_back("width");
	keepThese.push_back("height");
	keepThese.push_back("version");
	keepThese.push_back("no_aspect_ratio_restriction");
	keepThese.push_back("bfsw");
	keepThese.push_back("force_framelimiter");
	
	// Only keepThese command line options in final mode
	m_cmdLine.clear(keepThese);
	
	// ---------------------------------------------------------------------------------------------
	// Below here are the debug cmdline settings.
#endif
	
	m_settings.hideCursor = (m_settings.hideCursor || m_cmdLine.exists("nocursor"));
	
	// Frame limiter
	if (m_cmdLine.exists("fps"))
	{
		s32 fps(m_cmdLine.getInteger("fps"));
		if (fps > 0)
		{
			m_settings.targetFPS = fps;
		}
	}
	m_frameLimiterEnabled = (m_settings.targetFPS > 0);
	FrameRateManager::setTargetFramerate(m_settings.targetFPS);
	
	if (m_cmdLine.exists("no-framelimiter"))
	{
		m_frameLimiterEnabled = false;
		m_frameLimiterForced = false;
	}
	else
	{
		m_frameLimiterForced = true;
	}
	
	m_settings.graphicsSettings.useIOS2xMode = (m_settings.graphicsSettings.useIOS2xMode || m_cmdLine.exists("ios2xmode"));
	
	
	setPlatformEmulation(m_settings.emulate);
	
	// Create the renderer instance
	using tt::engine::renderer::Renderer;
	if (Renderer::hasInstance() == false &&
	    Renderer::createInstance(m_settings.graphicsSettings.useIOS2xMode) == false)
	{
		reportFatalError("Could not initialize render system");
	}
	Renderer* renderer(Renderer::getInstance());
	
	// Initialize memory filesystem
	if (m_settings.useMemoryFS)
	{
		m_memfs = fs::MemoryFileSystem::instantiate(0, 2);
		if(m_memfs == 0)
		{
			reportFatalError("Could not initialize memory filesystem.");
		}
	}
	
	// Initialize filesystem
	m_winfs = fs::WindowsFileSystem::instantiate(m_settings.useMemoryFS ? 2 : 0, m_settings.name);
	if(m_winfs == 0)
	{
		reportFatalError("Could not initialize filesystem.");
	}
	
	// Adjust working directory
	{
		// Get current path
		char path[MAX_PATH] = { 0 };
		
		std::string assetRootDirectory;
		str::Strings checkPaths;
		
		// 1) Current working directory
		GetCurrentDirectoryA(MAX_PATH, path);
		checkPaths.push_back(std::string(path));
		
		// Executable path
		GetModuleFileNameA(0, path, MAX_PATH);
		str::Strings subdirComponents(str::explode(std::string(path), "\\/"));
		
		// Pop the filename
		subdirComponents.pop_back();
		
		// 2) Executable directory
		checkPaths.push_back(str::implode(subdirComponents, "\\"));
		
		// 3) From packs folder
		while (subdirComponents.empty() == false && subdirComponents.back() != "packs")
		{
			subdirComponents.pop_back();
		}
		
		// Packs dir found
		if (subdirComponents.empty() == false)
		{
			subdirComponents.pop_back();
			subdirComponents.push_back("assets");
			subdirComponents.push_back("output");
			checkPaths.push_back(str::implode(subdirComponents, "\\"));
		}
		
		std::string assetRootDir;
		for (str::Strings::const_iterator it = checkPaths.begin(); it != checkPaths.end(); ++it)
		{
			std::string composedDir = composeAssetRootDir(*it);
			if (fs::dirExists(composedDir))
			{
				assetRootDir = composedDir;
				break;
			}
		}
		
		if (assetRootDir.empty())
		{
			std::string errorMsg = "Could not find data directory.\nThe following data directories were checked:\n";
			for (str::Strings::const_iterator it = checkPaths.begin(); it != checkPaths.end(); ++it)
			{
				errorMsg += (*it) + "\n";
			}
			
#ifdef TT_BUILD_FINAL
			errorMsg += "If you moved or removed this directory, please try to reinstall the game.";
#endif
			
			reportFatalError(errorMsg);
		}
		
		setAssetRootDir(assetRootDir);
	}
	
	// Set target timeslice if target FPS is set.
	if (m_settings.targetFPS > 0)
	{
		m_targetTimeSlice = static_cast<u32>(1000000.0f / m_settings.targetFPS);
	}
	
	// Create platform API
	if (m_settings.systems != 0)
	{
		m_platformApi = m_settings.systems->instantiatePlatformApi(this);
	}
	
	// Handle platform API initialization
	// FIXME: Properly handle these:
	//if(m_steamEnabled && m_cmdLine.exists("nosteam") == false)
	if (m_platformApi != 0)
	{
		m_platformApi->init();
	}
	else
	{
		// Setup cloud FS to use regular filesystem
		m_cloudfs = fs::WindowsFileSystem::instantiate(1, m_settings.name);
		if(m_cloudfs == 0)
		{
			reportFatalError("Could not initialize cloud filesystem.");
		}
	}
	
	// Set the DXUT callback functions
	DXUTSetCallbackD3D9DeviceAcceptable(::isDeviceAcceptable);
	DXUTSetCallbackDeviceChanging      (::modifyDeviceSettings, &m_settings);
	DXUTSetCallbackD3D9DeviceCreated   (::onCreateDevice);
	DXUTSetCallbackD3D9DeviceReset     (WinApp::onResetDevice, this);
	DXUTSetCallbackD3D9DeviceLost      (WinApp::onLostDevice,  this);
	DXUTSetCallbackD3D9DeviceDestroyed (::onDestroyDevice);
	DXUTSetCallbackMsgProc             (::msgProc, &m_settings.hideCursor);
	
	DXUTSetCallbackFrameMove           (WinApp::onFrameMove,   this);
	DXUTSetCallbackD3D9FrameRender     (WinApp::onFrameRender, this);
	
	// FIXME: How is this controlled?
	// Currently updates occur @ 60fps when no frame limiting is applied
	if(m_settings.useFixedDeltaTime)
	{
		DXUTSetConstantFrameTime(true, m_targetTimeSlice / 1000000.0f);
	}
	
	// Initialize DXUT and create the window and device for the application
	HRESULT hr = DXUTInit(false, m_cmdLine.exists("nodxutbox") == false);
	
	if (SUCCEEDED(hr) == false)
	{
		TT_PANIC("Failed to initialize DXUT");
		
		// This is pretty bad..
		reportDXUTError(hr);
		return;
	}
	DXUTSetCursorSettings(true, true);
	
#ifdef TT_BUILD_FINAL
	// Disable debug hotkeys in final mode
	DXUTSetHotkeyHandling(false, false, false);
	
	// Disable shortcut keys in fullscreen mode
	DXUTSetShortcutKeySettings(false, true);
#endif
	
	// Compose window title from app name & revision info
	std::wostringstream wndTitle;
	wndTitle << m_settings.name.c_str();
	
	
	
#ifndef TT_BUILD_FINAL
	wndTitle << " (REVISION: " << version::getClientRevisionNumber() <<
		"." << version::getLibRevisionNumber() << ")";
	
	Renderer::getInstance()->getDebug()->setBaseCaptureFilename(m_settings.name);
	
	if (isDualScreen())
	{
		wndTitle << " [DualScreen MODE]";
	}
	
	if (m_cmdLine.exists("window-title-suffix"))
	{
		wndTitle << " " << str::utf8ToUtf16(m_cmdLine.getString("window-title-suffix"));
	}
	
#else
	if(showBuildLabel)
	{
		wndTitle << " (Build " << version::getClientRevisionNumber() <<
		"." << version::getLibRevisionNumber() << ")";
	}
#endif
	
	if (m_cmdLine.exists("resizable"))
	{
		m_settings.graphicsSettings.allowResize = true;
	}
	bool isResizable = isDualScreen() ? false : m_settings.graphicsSettings.allowResize;
	bool fullscreenBorderless = m_cmdLine.exists("bfsw");
	
	hr = DXUTCreateWindow(
		wndTitle.str().c_str(), 0, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, isResizable, fullscreenBorderless);
	
	if (SUCCEEDED(hr) == false)
	{
		reportFatalError("Could not create main window.");
	}
	
	platform::error::setAppWindowHandle(DXUTGetHWND());
	
#if !defined(TT_BUILD_FINAL)
	platform::error::registerPanicCallback(onPanic);
	
	{
		// FIXME: Screenshot tool also uses this code, remove code duplication
		std::string desktopPath;
		{
			// Retrieve the path to the desktop
			char path[MAX_PATH] = { 0 };
			BOOL pathSuccess = SHGetSpecialFolderPathA(0, path, CSIDL_DESKTOPDIRECTORY, FALSE);
			TT_ASSERTMSG(pathSuccess, "Could not retrieve desktop directory path.");
			desktopPath = path;
			if (desktopPath.empty() == false && *desktopPath.rbegin() != '\\')
			{
				desktopPath += "\\";
			}
		}
		
		std::string fileName(m_settings.name);
		str::replace(fileName, " ", "_");
		str::replace(fileName, ":", "_");
		str::replace(fileName, "/", "_");
		str::replace(fileName, "\\", "_");
		str::replace(fileName, "*", "_");
		str::replace(fileName, "?", "_");
		str::replace(fileName, "\"", "_");
		str::replace(fileName, "<", "_");
		str::replace(fileName, ">", "_");
		str::replace(fileName, "|", "_");
		
		std::stringstream ss;
		ss << desktopPath << fileName << "_";
		ss << version::getClientRevisionNumber() << "." << version::getLibRevisionNumber() << "_";
		ss << "texture_cache.txt";
		
		m_textureCacheFileName = ss.str();
		
		// remove texture cache log file
		if (fs::fileExists(m_textureCacheFileName))
		{
			fs::destroyFile(m_textureCacheFileName);
		}
	}
#endif
	
	renderer->setClearColor(engine::renderer::ColorRGB::black);
	engine::file::FileUtils::getInstance()->generateNamespaceMapping();
	
	// This application instance is ready to be used by client code
	makeApplicationAvailable();
	
	///////////////////////////////////////
	// Create new D3D device
	{
		determineScreenSize(&m_settings, m_app, getDesktopSize(), m_cmdLine);
		
		renderer->getUpScaler()->setMaxSize(m_settings.graphicsSettings.startUpscaleSize);
		
		if (m_cmdLine.exists("no_aspect_ratio_restriction") == false)
		{
			renderer->getUpScaler()->setAspectRatioRange(m_settings.graphicsSettings.aspectRatioRange);
		}
		
		const bool windowed = m_settings.graphicsSettings.startWindowed;
		math::Point2 size(m_settings.graphicsSettings.getScreenSize(windowed));
		
		// Create device for correct emulation settings
		if(FAILED(hr = DXUTCreateDevice(windowed, size.x, size.y)))
		{
			TT_PANIC("Failed to create Direct3D Device (%d, %d)", size.x, size.y);
			reportDXUTError(hr);
			return;
		}
	}
	
	{
#if !defined(TT_BUILD_FINAL)
		tt::engine::renderer::FixedFunction::initialize(engine::EngineID("FixedFunctionMipMapDebug", "shaders"));
#else
		tt::engine::renderer::FixedFunction::initialize(engine::EngineID("FixedFunction", "shaders"));
#endif
	}
	
	// Initialize audio system
	if (m_settings.systems != 0)
	{
		m_soundSystem = m_settings.systems->instantiateSoundSystem();
	}
	
	// Initialize the mouse controller
	if (input::MouseController::initialize(DXUTGetHWND(), m_cmdLine.exists("multimouse")) == false)
	{
		TT_PANIC("Initializing the mouse controller failed.");
	}
	
	// Initialize the keyboard controller
	if (input::KeyboardController::initialize(DXUTGetHWND()) == false)
	{
		TT_PANIC("Initializing the keyboard controller failed.");
	}

#if !defined(TT_BUILD_FINAL)
	// Initialize the iphone controller (used for emulation)
	if (input::IPhoneController::initialize() == false)
	{
		TT_PANIC("Initializing the iphone controller failed.");
	}
#endif
	
#if USE_SDL_INPUT
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
#else
	if (m_settings.systems != 0)
	{
		m_settings.systems->sdl_init();
	}
#endif
	
	http::HttpConnectMgr::createInstance();
	
	// Create the threads for the threaded workload pool
	tt::thread::ThreadedWorkload::createThreads();

	//
	// Application Initialization (should be at end)
	//
	m_startupState.setStartupStep(StartupStep_ClientInit);
	m_initialized = m_app->init();
	
	if (m_initialized == false)
	{
		reportFatalError("Initializing the application failed.");
	}
	else
	{
		m_startupState.setStartupStep(StartupStep_Running);
	}
	
	//
	// VisualBoy emulator (should be done after app init; since it requires texture loads potentially from memory archive)
	//
	VisualBoy::initialize();
}


WinApp::~WinApp()
{
	m_startupState.setStartupStep(StartupStep_Shutdown);
	
	VisualBoy::deinitialize();
	
	// Application Destruction
	unregisterPlatformCallbackInterface(m_app);
	delete m_app;
	m_app = 0;
	
	//
	// General Cleanup
	//
	engine::renderer::FixedFunction::destroy();
	engine::renderer::TextureCache::clear();
	
	// Deinitialize the mouse controller
	input::MouseController::deinitialize();
	
	// Deinitialize the keyboard controller
	input::KeyboardController::deinitialize();
	
#if !defined(TT_BUILD_FINAL)
	// Deinitialize the iphone controller
	input::IPhoneController::deinitialize();
#endif
	
	http::HttpConnectMgr::destroyInstance();
	
	// Destroy the threads of the threaded workload pool
	tt::thread::ThreadedWorkload::destroyThreads();
	
#if USE_SDL_INPUT
	tt::input::SDLMouseController::deinitialize();
	tt::input::SDLKeyboardController::deinitialize();
	tt::input::SDLJoypadController::deinitialize();
#else
	if (m_settings.systems != 0)
	{
		m_settings.systems->sdl_shutdown();
	}
#endif
	
#if !defined(TT_BUILD_FINAL)
	// Unregister.
	platform::error::registerPanicCallback(0);
	
	// Free the debug console
	FreeConsole();
#endif
	
	// Shut down renderer
	engine::renderer::Renderer::destroyInstance();
	
	system::Time::destroyInstance();
	
	m_cloudfs.reset();
	
	// Shut down the platform API
	if (m_platformApi != 0)
	{
		m_platformApi->shutdown();
	}
	
	ComHelper::uninitCom();
}


void WinApp::update(real p_elapsedTime)
{
	// Handle application switching between (in)active
	if(m_active != DXUTIsActive())
	{
		if(m_active)
		{
			// Switch to inactive
			for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
			{
				(*it)->onAppInactive();
			}
			m_active = false;
		}
		else
		{
			// Switch to active
			for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
			{
				(*it)->onAppActive();
			}
			m_active = true;
		}
	}
	
	updateInputControllers();
	
	if (http::HttpConnectMgr::hasInstance())
	{
		http::HttpConnectMgr::getInstance()->processResponses();
	}
	
	// Update platform API
	if (m_platformApi != 0)
	{
		m_platformApi->update();
	}
	
	// framestep handling
#if !defined(TT_BUILD_FINAL)
	if (m_curWaitFrame < m_totalWaitFrames)
	{
		++m_curWaitFrame;
		return;
	}
	
	// FIXME: Move keyhandling to handleCommonInput
	if (m_frameStepMode && (input::KeyboardController::getButtonState('P').pressed == false))
	{
		return;
	}
	
	m_curWaitFrame = 0;
#endif
	
	// Update the renderer
	engine::renderer::Renderer::getInstance()->update(p_elapsedTime);
	
	// Update the application
	m_app->update(p_elapsedTime);
	
	VisualBoy::update();
	
#ifndef TT_BUILD_FINAL
	// Compute update time
	u64 now(system::Time::getInstance()->getMicroSeconds());
	m_updateTime = static_cast<s32>(now - m_frameTime);
#endif
	
	//FrameRateManager::monitorFramerate();
	m_fps30Mode = engine::renderer::Renderer::getInstance()->isLowPerformanceMode();
	
	// Check if another thread requested PostQuitMessage
	{
		thread::CriticalSection critSec(&m_postQuitMessageMutex);
		if (m_shouldPostQuitMessage)
		{
			m_shouldPostQuitMessage = false;
			PostQuitMessage(0);
		}
	}
}


void WinApp::render()
{
	using engine::renderer::Renderer;
	Renderer* renderer = Renderer::getInstance();
	
	// Render the frame
	if (renderer->beginFrame() == false)
	{
		return;
	}
	
	{
		using engine::renderer::ViewPort;
		using engine::renderer::ViewPortContainer;
		
		for(ViewPortContainer::iterator it = ViewPort::getViewPorts().begin();
			it != ViewPort::getViewPorts().end(); ++it)
		{
			renderer->beginViewPort(*it, it == ViewPort::getViewPorts().end() - 1);
			
			m_app->render();
			
			// Last viewport
			if(it == ViewPort::getViewPorts().end() - 1)
			{
				// Force an empty hud pass
				renderer->beginHud();
				renderer->endHud();
				
#if !defined(TT_BUILD_FINAL)
				if (m_shouldDisplayDebugInfo)
				{
					engine::debug::DebugRendererPtr debugPtr(renderer->getDebug());
					const s32 xpos = renderer->getScreenWidth() / 2;
					if (m_frameStepMode)
					{
						debugPtr->printf(xpos, 5, "FRAME STEP MODE");
					}
					else if (m_totalWaitFrames > 0)
					{
						debugPtr->printf(xpos, 5, "WAIT: %d", m_totalWaitFrames);
					}
					else if (m_totalWaitFrames < 0)
					{
						debugPtr->printf(xpos, 5, "FASTMODE x%d", -m_totalWaitFrames * FASTMODE_UPDATES);
					}
				}
#endif
			}
			
			renderer->endViewPort(*it);
		}
	}
	// Done rendering
	renderer->endFrame();
	engine::renderer::Renderer::clearDeathRow();
	
#ifndef TT_BUILD_FINAL
	// Compute update time
	u64 now(system::Time::getInstance()->getMicroSeconds());
	m_renderTime = static_cast<s32>(now - m_frameTime);
#endif
	
	// Framerate limiter
	if ((m_frameLimiterEnabled && FrameRateManager::isVsyncEnabled() == false) || m_frameLimiterForced)
	{
		u64 currentTime(system::Time::getInstance()->getMicroSeconds());
		u32 timePassed = static_cast<u32>(currentTime - m_frameTime);
		u32 targetTime = m_targetTimeSlice;
		 
		// Set timer resolution to 1 millisecond
		timeBeginPeriod(1);
		
		static const u32 leaveTimeForVsync           =  300; // 0.3 ms
		static const u32 oneMilliSecondInNanoSeconds = 1000; // 1.0 ms
		
		while (timePassed < (targetTime - leaveTimeForVsync))
		{
			if ((m_targetTimeSlice - timePassed) > (oneMilliSecondInNanoSeconds + leaveTimeForVsync))
			{
				Sleep(1);
			}
			
			currentTime = system::Time::getInstance()->getMicroSeconds();
			
			timePassed = static_cast<u32>(currentTime - m_frameTime);
		}
		
		timeEndPeriod(1);
	}

	m_frameTime = system::Time::getInstance()->getMicroSeconds();
	
	// Show frame
	renderer->present();
}


void WinApp::onPlatformMenuEnter()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuEnter();
	}
}


void WinApp::onPlatformMenuExit()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onPlatformMenuExit();
	}
}


void WinApp::onLostDevice()
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onLostDevice();
	}
}


void WinApp::onResetDevice() 
{
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onResetDevice();
	}
}


s32 WinApp::run()
{
	if (m_initialized == false)
	{
		return 1;
	}
	
	// Start the main loop
	DXUTMainLoop();
	
#if !defined(TT_BUILD_FINAL)
	// Unregister the panic callback so that panics during shutdown don't try to toggle full-screen mode
	platform::error::registerPanicCallback(0);
	platform::error::setAppWindowHandle(HWND_DESKTOP);
#endif
	
	// Do DXUT cleanup
	DXUTShutdown(0);
	
	return DXUTGetExitCode();
}


bool WinApp::generateDump(EXCEPTION_POINTERS* p_exceptionPointers)
{
	// Get Temp directory
	WCHAR szPath[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, szPath);
	
	// Construct path to dump file
	WCHAR fileName[MAX_PATH] = { 0 };
	StringCchPrintf(fileName, MAX_PATH, L"%s%s", szPath, settings::getApplicationName().c_str());
	CreateDirectory(fileName, 0);
	
	SYSTEMTIME stLocalTime;
	GetLocalTime(&stLocalTime);
	
	// Construct complete path including filename
	StringCchPrintf(fileName, MAX_PATH, L"%s%s\\v%04d.%04d-%04d%02d%02d-%02d%02d%02d-P%lu-T%lu.dmp", 
	                szPath,                                                      // Full Temp Path
	                settings::getApplicationName().c_str(),                      // Application Name
	                version::getClientRevisionNumber(),                          // Client revision
	                version::getLibRevisionNumber(),                             // Lib revision
	                stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,     // Date
	                stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, // Time
	                GetCurrentProcessId(),                                       // Process
	                GetCurrentThreadId());                                       // Thread
	
	// Create the file
	HANDLE dumpFile = CreateFile(fileName,
		GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	
	// Create extra parameters
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;
	ExpParam.ThreadId          = GetCurrentThreadId();
	ExpParam.ExceptionPointers = p_exceptionPointers;
	ExpParam.ClientPointers    = TRUE;
	
	// Write out the dump file
	return MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			dumpFile,
			static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory),
			&ExpParam,
			NULL,
			NULL) == TRUE;
}


std::string WinApp::getAssetRootDir() const
{
	return m_assetRootDir;
}


fs::identifier WinApp::getSaveFsID() const
{
	return (m_settings.useCloudFS) ? 1 : 0;
}


const args::CmdLine& WinApp::getCmdLine() const
{
	return m_cmdLine;
}


const StartupState& WinApp::getStartupState() const
{
	return m_startupState;
}


void WinApp::setFullScreen(bool p_fullScreen)
{
	if (p_fullScreen != isFullScreen())
	{
		DXUTPause(true, true);
		engine::renderer::checkD3DSucceeded(DXUTToggleFullScreen());
		DXUTPause(false, false);
	}
}


bool WinApp::isFullScreen() const
{
	return DXUTIsWindowed() == false;
}


void WinApp::setFullScreenResolution(const math::Point2& p_resolution)
{
	m_settings.graphicsSettings.fullscreenSize =
		m_settings.graphicsSettings.getCorrectedScreenSize(p_resolution);
	g_needToModifyDeviceSettingsFullScreen = true;
}


tt::math::Point2 WinApp::getFullScreenResolution() const
{
	return m_settings.graphicsSettings.fullscreenSize;
}


math::Point2 WinApp::getDesktopSize() const
{
	// Get the desktop resolution of the current monitor
	UINT width, height;
	DXUTGetDesktopResolution(DXUTGetDeviceSettings().d3d9.AdapterOrdinal, &width, &height);
	
	return math::Point2(static_cast<s32>(width), static_cast<s32>(height));
}


bool WinApp::shouldDisplayDebugInfo() const
{
#if !defined(TT_BUILD_FINAL)
	return m_shouldDisplayDebugInfo;
#else
	return false;
#endif
}


void WinApp::terminate(bool p_graceful)
{
	if (p_graceful)
	{
		if (m_creationThreadId == GetCurrentThreadId())
		{
			// Terminate was requested from the main thread: can simply call PostQuitMessage
			PostQuitMessage(0);
		}
		else
		{
			// Terminate was requested from a different thread than the main thread:
			// tell the main thread that it should post a quit message itself
			thread::CriticalSection critSec(&m_postQuitMessageMutex);
			m_shouldPostQuitMessage = true;
		}
	}
	else
	{
		std::exit(0);
	}
}


void WinApp::setPaused(bool /*p_paused*/)
{
	// Not implemented yet on Windows
}


void WinApp::setPlatformMenuEnabled(bool /*p_enabled*/)
{
}


void WinApp::setTargetFPS(u32 p_fps)
{
	m_settings.targetFPS = p_fps;
	m_fps30Mode = (p_fps == 30);
	
	if (m_cmdLine.exists("no-framelimiter") == false)
	{
		m_frameLimiterEnabled = (m_settings.targetFPS > 0);
	}
	
	if (m_settings.targetFPS > 0)
	{
		m_targetTimeSlice = static_cast<u32>(1000000.0f / m_settings.targetFPS);
	}
	FrameRateManager::setTargetFramerate(m_settings.targetFPS);
}


void WinApp::setPlayerCount(u32 p_playerCount)
{
	// FIXME: Pass to controller?
	
	for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin(); it != m_appListeners.end(); ++it)
	{
		(*it)->onSetPlayerCount(p_playerCount);
	}
}


void WinApp::handleResolutionChanged()
{
	// Emulate a device reset, without actually resetting the device
	onLostDevice(this);
	onResetDevice(0,0,this);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


HRESULT WinApp::onResetDevice (IDirect3DDevice9*, const D3DSURFACE_DESC*, void* p_userContext)
{
	if(engine::renderer::Renderer::getInstance()->handleResetDevice() == false)
	{
		return S_FALSE;
	}
	
	// Notify all registered resources
	engine::renderer::D3DResourceRegistry::onResetDevice();
	
	input::MouseController::updateWindowRects(DXUTGetHWND());
	
	// Notify application (resolution might have changed)
	if(p_userContext)
	{
		static_cast<WinApp*>(p_userContext)->onResetDevice();
	}
	
	return S_OK;
}


void WinApp::onLostDevice(void* p_userContext)
{
	if (engine::renderer::Renderer::hasInstance())
	{
		engine::renderer::Renderer::getInstance()->handleLostDevice();
	}
	
	// Notify application (resolution might have changed)
	if (p_userContext && app::hasApplication())
	{
		static_cast<WinApp*>(p_userContext)->onLostDevice();
	}
	
	// Notify all registered resources
	engine::renderer::D3DResourceRegistry::onLostDevice();
}


void WinApp::onFrameMove(double, float p_elapsedTime, void* p_userContext)
{
	static_cast<WinApp*>(p_userContext)->update(p_elapsedTime);
#if !defined (TT_BUILD_FINAL)
	const s32 waitFrames = static_cast<WinApp*>(p_userContext)->m_totalWaitFrames;
	if (waitFrames < 0)
	{
		// fast mode
		const s32 updates = (FASTMODE_UPDATES-1) * -waitFrames;
		for (s32 i = 0; i < updates; ++i)
		{
			static_cast<WinApp*>(p_userContext)->update(p_elapsedTime);
		}
	}
#endif 
}


void WinApp::onFrameRender(IDirect3DDevice9*, double, float p_elapsedTime, void* p_userContext)
{
	static_cast<WinApp*>(p_userContext)->render();
}


void WinApp::updateInputControllers()
{
	// Update the controllers
	input::Xbox360Controller::update();
	input::MouseController::update();
	input::KeyboardController::update();
#if !defined(TT_BUILD_FINAL)
	input::IPhoneController::update();
#endif
	
	// Update the controller type
	tt::input::updateCurrentControllerType();
	
#if USE_SDL_INPUT
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			input::SDLMouseController::processEvent(event);
			input::SDLKeyboardController::processEvent(event);
			input::SDLJoypadController::processEvent(event);
		}
		
		input::SDLMouseController::update();
		input::SDLKeyboardController::update();
		input::SDLJoypadController::update();
	}
#else
	if (m_settings.systems != 0)
	{
		m_settings.systems->sdl_update();
	}
#endif
	
	handleCommonInput();
}


void WinApp::handleCommonInput()
{
#if !defined(TT_BUILD_FINAL)
	if(m_debugKeys == DebugKeys_None) return;
	
	using engine::renderer::Renderer;
	Renderer* renderer = Renderer::getInstance();
	
	const input::KeyboardController& kbd(input::KeyboardController::getState(input::ControllerIndex_One));
	
	const bool ctrlDown        = kbd.keys[input::Key_Control].down;
	const bool altDown         = kbd.keys[input::Key_Alt    ].down;
	const bool shiftDown       = kbd.keys[input::Key_Shift  ].down;
	const bool anyModifierDown = ctrlDown || altDown || shiftDown;

	if((m_debugKeys & DebugKeys_Function) != 0)
	{
		// console window
		if (kbd.keys[input::Key_F2].pressed && anyModifierDown == false)
		{
			toggleConsole();
		}

		if (kbd.keys[input::Key_F3].pressed && ctrlDown == false && altDown == false)
		{
			using tt::engine::debug::DebugRenderer;
			if (kbd.keys[input::Key_Shift].down)
			{
				renderer->getDebug()->captureScreen(DebugRenderer::ScreenCaptureMode_ToClipboard);
			}
			else
			{
				renderer->getDebug()->captureScreen(DebugRenderer::ScreenCaptureMode_ToFile);
			}
		}

		if (kbd.keys[input::Key_F5].pressed && anyModifierDown == false)
		{
			// Send request to reload currently loaded assets
			for (PlatformCallbackInterfaces::iterator it = m_appListeners.begin();
				it != m_appListeners.end(); ++it)
			{
				(*it)->onRequestReloadAssets();
			}
		}

		if (kbd.keys[input::Key_F6].pressed && anyModifierDown == false)
		{
			// Reload all graphical assets
			engine::renderer::TextureCache::reload();
			engine::renderer::ShaderCache::reload();
			engine::renderer::FixedFunction::setOverdrawModeEnabled(false);
		}

		if (kbd.keys[input::Key_F8].pressed && anyModifierDown == false)
		{
			renderer->getDebug()->toggleWireFrame();
		}

		// Safe Frame rendering support
		if (kbd.keys[input::Key_F9].pressed && anyModifierDown == false)
		{
			renderer->getDebug()->toggleSafeFrame();
		}

		// Axis display toggle
		if (kbd.keys[input::Key_F10].pressed && anyModifierDown == false)
		{
			renderer->getDebug()->toggleAxis(1.0f);
		}

		if (kbd.keys[input::Key_F11].pressed && anyModifierDown == false)
		{
			static bool lighting = true;
			lighting = (lighting == false);

			renderer->setLighting(lighting);
		}
	}
	
	// Normal keys (P,O,T,F)
	if((m_debugKeys & DebugKeys_Normal) == 0) return;

	// Overdraw mode
	if (kbd.keys[input::Key_O].pressed && anyModifierDown == false)
	{
		engine::debug::DebugRendererPtr debug(renderer->getDebug());
		// Toggle debug overdraw.
		debug->setOverdrawDebugActive(debug->isOverdrawDebugActive() == false);
		TT_Printf("WinApp::handleCommonInput - 'O' was pressed, toggling overdraw debug: %s\n",
		          debug->isOverdrawDebugActive() ? "On" : "Off");
	}
	
	// Mipmap Visualization Mode
	if (kbd.keys[input::Key_M].pressed && altDown)
	{
		engine::debug::DebugRendererPtr debug(renderer->getDebug());
		// Toggle mipmap visualization
		debug->setMipmapVisualizerActive(debug->isMipmapVisualizerActive() == false);
		TT_Printf("WinApp::handleCommonInput - 'M' was pressed, toggling mipmap visualization: %s\n",
			debug->isMipmapVisualizerActive() ? "On" : "Off");
	}
	
	// framestep keys
	if (m_frameStepMode)
	{
		if (kbd.keys[input::Key_P].pressed &&
		    shiftDown                      &&
		    ctrlDown == false              &&
		    altDown  == false)
		{
			m_frameStepMode = false;
		}
	}
	else if (kbd.keys[input::Key_P].pressed && anyModifierDown == false)
	{
		m_frameStepMode = true;
	}
	
	if ((kbd.keys[VK_ADD     ].pressed ||
	     kbd.keys[VK_OEM_PLUS].pressed) &&
	    anyModifierDown == false)
	{
		if (m_totalWaitFrames <= 0)
		{
			--m_totalWaitFrames; // fast mode
			if (m_totalWaitFrames < -4) m_totalWaitFrames = -4;
		}
		else
		{
			m_totalWaitFrames /= 2;
		}
	}
	
	if ((kbd.keys[VK_SUBTRACT ].pressed ||
	     kbd.keys[VK_OEM_MINUS].pressed) &&
	    anyModifierDown == false)
	{
		if (m_totalWaitFrames <= 0)
		{
			++m_totalWaitFrames;
		}
		else
		{
			m_totalWaitFrames *= 2;
		}
		
		if (m_totalWaitFrames > MAX_WAIT_FRAMES)
		{
			m_totalWaitFrames = MAX_WAIT_FRAMES;
		}
	}
	
	if (ctrlDown && kbd.keys[input::Key_T].pressed &&
	    altDown == false && shiftDown == false)
	{
		engine::cache::FileTextureCache::dumpToFile(m_textureCacheFileName);
		engine::renderer::TextureCache::dumpToFile(m_textureCacheFileName);
		
		ShellExecuteA(NULL, "open", m_textureCacheFileName.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	else if (kbd.keys[input::Key_T].pressed && shiftDown && ctrlDown == false && altDown == false)
	{
		engine::cache::FileTextureCache::dump();
		engine::renderer::TextureCache::dump();
	}
	
	if (kbd.keys[input::Key_F].pressed && shiftDown && ctrlDown == false && altDown == false)
	{
		m_shouldDisplayDebugInfo = (m_shouldDisplayDebugInfo == false);
	}
	
#endif  // !defined(TT_BUILD_FINAL)
}


void WinApp::toggleConsole()
{
#if !defined(TT_BUILD_FINAL)
	m_displayConsole = m_displayConsole == false;
	if (IsDebuggerPresent() == FALSE)
	{
		if (m_displayConsole == false)
		{
			fclose(stdout);
			fclose(stdin);
			fclose(stderr);
			std::ios::sync_with_stdio();
			FreeConsole();
		}
		else
		{
			// allocate a console for this app
			BOOL success = AllocConsole();
			TT_ASSERTMSG(success != FALSE, "AllocConsole failed!");
			
			// redirect unbuffered STDOUT to the console
			FILE* fp = freopen("CONOUT$", "w", stdout);
			setvbuf( stdout, NULL, _IONBF, 0 );
			
			// redirect unbuffered STDIN to the console
			fp = freopen("CONIN$", "r", stdin);
			setvbuf( stdin, NULL, _IONBF, 0 );
			
			// redirect unbuffered STDERR to the console
			fp = freopen("CONOUT$", "w", stderr);
			setvbuf( stderr, NULL, _IONBF, 0 );
			
			// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
			// point to console as well
			std::ios::sync_with_stdio();
			
			SetConsoleTitleA((m_settings.name + " - Debug Console").c_str());
		}
	}
#endif
}


std::string WinApp::composeAssetRootDir(const std::string& p_basePath) const
{
	std::string currentDirStr(p_basePath);
	if (m_settings.emulationOverrideDir.empty() == false)
	{
		// Use client-specified directory
		currentDirStr += m_settings.emulationOverrideDir;
	}
	else
	{
		// Set directory based on platform
		switch (m_settings.emulate)
		{
		case AppSettings::Emulate_None:       currentDirStr += m_settings.windowsDir; break;
		default: TT_PANIC("Unsupported emulation mode: %d", m_settings.emulate); break;
		}
	}
	
	return currentDirStr;
}


void WinApp::setAssetRootDir(const std::string& p_path)
{
	DWORD result = SetCurrentDirectoryA(p_path.c_str());
	if(result == 0)
	{
		std::string errorMsg = "Could not find data directory.\n";
		errorMsg += "Trying to set data directory '" + p_path + "' failed.\n\n";
#ifdef TT_BUILD_FINAL
		errorMsg += "If you moved or removed this directory, please try to reinstall the game.";
#endif
		
		reportFatalError(errorMsg);
	}
	
	char currentDir[MAX_PATH] = { 0 };
	// Save the working directory as asset root path
	GetCurrentDirectoryA(MAX_PATH, currentDir);
	m_assetRootDir = currentDir;
	if (*m_assetRootDir.rbegin() != '\\')
	{
		m_assetRootDir += '\\';
	}
}


//////////////////////
// FrameRateManager

void FrameRateManager::setTargetFramerate(s32 p_fps)
{
	if(static_cast<u32>(p_fps) != ms_targetFPS)
	{
		// Trigger device reset
		ms_targetFPS = p_fps;
		
		if(DXUTGetD3D9Device() != 0)
		{
			if(DXUTIsWindowed())
			{
				DXUTChangeWindowSize(DXUTGetWindowWidth(), DXUTGetWindowHeight());
			}
			else
			{
				DXUTForceDeviceReset();
			}
		}
		
	}
}

void FrameRateManager::modifyDeviceSettings(DXUTDeviceSettings* p_settings)
{
	// Enable triple buffering
	p_settings->d3d9.pp.BackBufferCount = 2;
	
	// Get monitor ID
	const u32 monitorID = p_settings->d3d9.AdapterOrdinal;
	
	// Do we have this one already?
	if(ms_nativeRefreshRates.find(monitorID) == ms_nativeRefreshRates.end())
	{
		// Figure out the refresh rate of the current monitor
		D3DDISPLAYMODE mode;
		DXUTGetD3D9Object()->GetAdapterDisplayMode(monitorID, &mode);
		
		// Store it for future lookup
		ms_nativeRefreshRates.insert(NativeRefreshRates::value_type(monitorID, mode.RefreshRate));
	}
	
	if(ms_targetFPS == 0)
	{
		ms_vsyncEnabled = true;
		p_settings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		return;
	}
	
	// For windowed modes
	if(p_settings->d3d9.pp.Windowed)
	{
		u32 refreshRate = ms_nativeRefreshRates[monitorID];
		if(refreshRate == 59) ++refreshRate; // 59.94 Hz is reported as 59
		
		// Turn off vsync unless the desktop refresh rate matches
		if (refreshRate % ms_targetFPS == 0)
		{
			// Turn on vsync
			p_settings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			
			// Only report vsync as on if it is an exact match, because we still need frame limiting
			// for 30 fps mode
			ms_vsyncEnabled = (refreshRate == ms_targetFPS);
		}
		else
		{
			// Turn off vsync
			p_settings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			ms_vsyncEnabled = false;
		}
	}
	// For fullscreen modes
	else
	{
		// We want settings that match our target fps + vsync
		DXUTDeviceSettings desiredSettings = *p_settings;
		u32 refreshRate = ms_targetFPS;
		if (ms_targetFPS < 60 && (60 % ms_targetFPS) == 0)
		{
			refreshRate = 60;
		}
		
		desiredSettings.d3d9.pp.FullScreen_RefreshRateInHz = refreshRate;
		desiredSettings.d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		
		// Create Match Options -> only interested in refresh rate
		DXUTMatchOptions matchOptions;
		matchOptions.eAPIVersion       = DXUTMT_PRESERVE_INPUT;
		matchOptions.eAdapterOrdinal   = DXUTMT_PRESERVE_INPUT;
		matchOptions.eOutput           = DXUTMT_PRESERVE_INPUT;
		matchOptions.eDeviceType       = DXUTMT_PRESERVE_INPUT;
		matchOptions.eWindowed         = DXUTMT_PRESERVE_INPUT;
		matchOptions.eAdapterFormat    = DXUTMT_PRESERVE_INPUT;
		matchOptions.eVertexProcessing = DXUTMT_PRESERVE_INPUT;
		matchOptions.eResolution       = DXUTMT_PRESERVE_INPUT;
		matchOptions.eBackBufferFormat = DXUTMT_PRESERVE_INPUT;
		matchOptions.eBackBufferCount  = DXUTMT_PRESERVE_INPUT;
		matchOptions.eMultiSample      = DXUTMT_PRESERVE_INPUT;
		matchOptions.eSwapEffect       = DXUTMT_PRESERVE_INPUT;
		matchOptions.eDepthFormat      = DXUTMT_PRESERVE_INPUT;
		matchOptions.eStencilFormat    = DXUTMT_PRESERVE_INPUT;
		matchOptions.ePresentFlags     = DXUTMT_PRESERVE_INPUT;
		matchOptions.eRefreshRate      = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.ePresentInterval  = DXUTMT_CLOSEST_TO_INPUT;
		
		// Try to acquire the wanted settings
		if(FAILED(DXUTFindValidDeviceSettings(p_settings, &desiredSettings, &matchOptions)))
		{
			TT_PANIC("Failed to find matching settings");
		}
		
		// If succeeded to match framerate, use it
		if (p_settings->d3d9.pp.FullScreen_RefreshRateInHz == refreshRate)
		{
			// Turn on vsync
			p_settings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			
			// Only report vsync as on if it is an exact match, because we still need frame limiting
			// for 30 fps mode
			ms_vsyncEnabled = (refreshRate == ms_targetFPS);
		}
		else
		{
			// Using closest match -> Turn off vsync
			p_settings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			ms_vsyncEnabled = false;
		}
	}
}


void FrameRateManager::monitorFramerate()
{
	// Remove last element
	if(ms_framerateHistory.size() > 100) ms_framerateHistory.pop_back();
	
	// Add new element
	ms_framerateHistory.push_front(DXUTGetFPS());
	
	// Compute average
	ms_averageFPS = 0;
	for(FrameRates::iterator it = ms_framerateHistory.begin(); it != ms_framerateHistory.end(); ++it)
	{
		ms_averageFPS += *it;
	}
	ms_averageFPS = ms_averageFPS / ms_framerateHistory.size();

	TT_Printf("Vsync: %s FPS: %g (%g) Target: %u\n", ms_vsyncEnabled ? "ON" : "OFF", DXUTGetFPS(), ms_averageFPS, ms_targetFPS);
}

// Namespace end
}
}


//--------------------------------------------------------------------------------------------------
// Callback functions for the DXUT framework

bool CALLBACK isDeviceAcceptable(D3DCAPS9* p_caps,
                                 D3DFORMAT p_adapterFormat,
                                 D3DFORMAT p_backBufferFormat,
                                 bool      p_windowed,
                                 void*)
{
	// Skip back buffer formats that don't support alpha blending
	IDirect3D9* d3d = DXUTGetD3D9Object();
	
	if (FAILED(d3d->CheckDeviceFormat(p_caps->AdapterOrdinal,
	                                  p_caps->DeviceType,
	                                  p_adapterFormat,
	                                  D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
	                                  D3DRTYPE_TEXTURE,
	                                  p_backBufferFormat)))
	{
		return false;
	}
	
	return true;
}


bool CALLBACK modifyDeviceSettings(DXUTDeviceSettings* p_settings, void* p_userContext)
{
	// Modify vsync & refresh rate
	tt::app::FrameRateManager::modifyDeviceSettings(p_settings);
	
	// Handle Anti-Aliasing
	using namespace tt::engine::renderer;
	Renderer* renderer(Renderer::getInstance());
	
	const bool windowed = (p_settings->d3d9.pp.Windowed != FALSE);
	
	const bool needToModifyDeviceSettings = (windowed) ? g_needToModifyDeviceSettingsWindowed : 
	                                                     g_needToModifyDeviceSettingsFullScreen;
	
	// check if device was reset and new settings weren't set yet.
	if (needToModifyDeviceSettings)
	{
		DXUTDeviceSettings desiredSettings = (*p_settings);
		
		tt::app::AppSettings* appSettings = reinterpret_cast<tt::app::AppSettings*>(p_userContext);
		
		TT_ASSERTMSG(appSettings != 0, "Expected user context with AppSettings");
		if (appSettings != 0)
		{
			const tt::math::Point2 size(appSettings->graphicsSettings.getScreenSize(windowed));
			
			TT_Printf("modifyDeviceSettings - Will try to set screen size to %d, %d - windowed: %d\n", size.x, size.y, windowed);
			
			desiredSettings.d3d9.pp.BackBufferWidth  = size.x;
			desiredSettings.d3d9.pp.BackBufferHeight = size.y;
		}
		
		if (windowed)
		{
			g_needToModifyDeviceSettingsWindowed     = false;
		}
		else
		{
			g_needToModifyDeviceSettingsFullScreen   = false;
		}
		
		DXUTMatchOptions matchOptions;
		matchOptions.eAPIVersion       = DXUTMT_PRESERVE_INPUT;
		matchOptions.eAdapterOrdinal   = DXUTMT_PRESERVE_INPUT;
		matchOptions.eDeviceType       = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eWindowed         = DXUTMT_PRESERVE_INPUT;
		matchOptions.eAdapterFormat    = DXUTMT_IGNORE_INPUT;
		matchOptions.eVertexProcessing = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eBackBufferFormat = DXUTMT_IGNORE_INPUT;
		matchOptions.eBackBufferCount  = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eMultiSample      = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eSwapEffect       = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eDepthFormat      = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eStencilFormat    = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.ePresentFlags     = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eRefreshRate      = DXUTMT_IGNORE_INPUT;
		matchOptions.ePresentInterval  = DXUTMT_CLOSEST_TO_INPUT;
		matchOptions.eResolution       = DXUTMT_CLOSEST_TO_INPUT;
		
		DXUTDeviceSettings validSettings;
		
		// Try to acquire the wanted settings
		if(FAILED(DXUTFindValidDeviceSettings(&validSettings, &desiredSettings, &matchOptions)))
		{
			TT_PANIC("Failed to find matching settings");
			return false;
		}
		else
		{
			p_settings->d3d9.pp.BackBufferWidth  = validSettings.d3d9.pp.BackBufferWidth;
			p_settings->d3d9.pp.BackBufferHeight = validSettings.d3d9.pp.BackBufferHeight;
			
			TT_Printf("modifyDeviceSettings - Will set screen size to %d, %d - windowed: %d\n",
			          p_settings->d3d9.pp.BackBufferWidth,
			          p_settings->d3d9.pp.BackBufferHeight,
			          p_settings->d3d9.pp.Windowed);
		}
	}
	
	bool upScaleNeeded = false;
	{
		tt::app::AppSettings* appSettings = reinterpret_cast<tt::app::AppSettings*>(p_userContext);
		
		TT_ASSERTMSG(appSettings != 0, "Expected user context with AppSettings");
		if (appSettings != 0)
		{
			if (appSettings->graphicsSettings.startUpscaleSize.x > 0)
			{
				upScaleNeeded = upScaleNeeded || 
					p_settings->d3d9.pp.BackBufferWidth  > static_cast<UINT>(appSettings->graphicsSettings.startUpscaleSize.x);
			}
			if (appSettings->graphicsSettings.startUpscaleSize.y > 0)
			{
				upScaleNeeded = upScaleNeeded || 
					p_settings->d3d9.pp.BackBufferHeight > static_cast<UINT>(appSettings->graphicsSettings.startUpscaleSize.y);
			}
		}
	}
	
	// Make sure stencil format is supported.
	TT_ASSERT(p_settings->d3d9.pp.EnableAutoDepthStencil == TRUE);
	p_settings->d3d9.pp.AutoDepthStencilFormat = D3DFMT_D24S8;
	
	// Also get alpha channel in back buffer. (Default is D3DFMT_X8R8G8B8.)
	p_settings->d3d9.pp.BackBufferFormat = D3DFMT_A8R8G8B8;
	
	// Only create a backbuffer with AA if this is not handled by post-processing or upscaling
	if (renderer->isAAEnabled() &&
	    renderer->getPP()->isActive() == false &&
	    upScaleNeeded == false)
	{
		TT_Printf("modifyDeviceSettings - [AA] Setting Frame Buffer Anti Aliasing!\n");
		p_settings->d3d9.pp.SwapEffect      = D3DSWAPEFFECT_DISCARD;
		p_settings->d3d9.pp.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(renderer->getAASamples());
		
		// The D3DPRESENTFLAG_LOCKABLE_BACKBUFFER is not allowed when doing multi sample.
		if ((p_settings->d3d9.pp.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER) == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
		{
			// Turn off flag.
			p_settings->d3d9.pp.Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		}
	}
	else
	{
		TT_Printf("modifyDeviceSettings - Setting Frame Buffer Anti Aliasing to none!\n");
		p_settings->d3d9.pp.SwapEffect      = D3DSWAPEFFECT_DISCARD;
		p_settings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	}
	
#ifdef DEBUG_VS
    if (p_settings->d3d9.DeviceType != D3DDEVTYPE_REF)
    {
        p_settings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        p_settings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        p_settings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    p_settings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
	
	return true;
}


HRESULT CALLBACK onCreateDevice(IDirect3DDevice9* p_device, const D3DSURFACE_DESC*, void*)
{
	tt::engine::renderer::Renderer::getInstance()->handleCreateDevice();
	
	g_needToModifyDeviceSettingsFullScreen = true;
	
	// Re-create all registered resources
	tt::engine::renderer::D3DResourceRegistry::onCreateDevice();
	
	return S_OK;
}


LRESULT CALLBACK msgProc(HWND   p_wnd,
                         UINT   p_msg,
                         WPARAM p_wparam,
                         LPARAM p_lparam,
                         bool*  p_noFurtherProcessing,
                         void*  p_userContext)
{
	switch (p_msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		
	case WM_SETCURSOR:
		bool* hideCursor = static_cast<bool*>(p_userContext);
		
		if (*hideCursor)
		{
			// We don't want a cursor in the client area of the window
			if (LOWORD(p_lparam) == HTCLIENT)
			{
				SetCursor(0);
				*p_noFurtherProcessing = true;
			}
		}
		break;
	}
	
	if (*p_noFurtherProcessing) return 0;
	
#ifndef TT_BUILD_FINAL
	// Let camera handle messages
	//tt::engine::renderer::Renderer::getInstance()->getDebug()->getDebugCamera()->HandleMessages(
	//	p_wnd, p_msg, p_wparam, p_lparam);
#endif
	
	return 0;
}


void CALLBACK onDestroyDevice(void*)
{
	if (tt::engine::renderer::Renderer::hasInstance())
	{
		tt::engine::renderer::Renderer::getInstance()->handleDestroyDevice();
	}
	tt::engine::renderer::Renderer::clearDeathRow();
	
	// Release all registered resources
	tt::engine::renderer::D3DResourceRegistry::onDestroyDevice();
}
