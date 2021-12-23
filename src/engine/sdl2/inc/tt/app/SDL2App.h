#if !defined(INC_TT_APP_SDL2APP_DESKTOP_H)
#define INC_TT_APP_SDL2APP_DESKTOP_H

#include <tt/app/AppInterface.h>
#include <tt/app/Application.h>
#include <tt/app/AppSettings.h>
#include <tt/app/PlatformApi.h>
#include <tt/app/StartupStateSDL2.h>
#include <tt/args/CmdLine.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/settings/settings.h>
#include <tt/snd/types.h>

#include <queue>

struct SDL_Window;

namespace tt {

namespace engine {
namespace renderer {
class OpenGLContextWrapper;
}
}
namespace app {

class FrameRateManager
{
public:
	static void setTargetFramerate(s32 p_fps);
	static void modifyDeviceSettings(SDL_Window* window);
	static bool isVsyncEnabled() {return ms_vsyncEnabled;}
	static void monitorFramerate();
	
private:
	typedef std::deque<float> FrameRates;
	static FrameRates ms_framerateHistory;
	
	static bool ms_vsyncEnabled;
	static u32  ms_targetFPS;
	static float ms_averageFPS;
};

class SDL2App : public Application, public PlatformCallbackInterface
{
public:
	SDL2App(AppInterface* p_app, const AppSettings& p_settings, const std::string& p_appIcon = "");
	virtual ~SDL2App();
	
	void update(real p_elapsedTime);
	void render();
	
	// PlatformCallbackInterface:
	virtual void onPlatformMenuEnter();
	virtual void onPlatformMenuExit();
	
	s32 run();

	// Application:
	virtual std::string getAssetRootDir() const;
	virtual fs::identifier getSaveFsID() const;
	virtual const args::CmdLine& getCmdLine() const;
	virtual const StartupState& getStartupState() const;
	virtual void setFullScreen(bool p_fullScreen);
	virtual bool isFullScreen() const;
	virtual void setFullScreenResolution(const math::Point2& p_resolution);
	virtual math::Point2 getFullScreenResolution() const;
	virtual math::Point2 getDesktopSize() const;

	virtual bool shouldDisplayDebugInfo() const;

	virtual void terminate(bool p_graceful);
	virtual void setPaused(bool p_paused);
	virtual inline bool isActive() const { return m_active; }

	virtual void setHandleDebugKeys(u32 p_keys) { m_debugKeys = p_keys; }

	virtual void setPlatformMenuEnabled(bool p_enabled);

	virtual void setTargetFPS(u32 p_fps);
	virtual u32  getTargetFPS() const { return m_settings.targetFPS; }
	
	virtual void handleResolutionChanged();
private:
	// No copying or assignment
	SDL2App(const SDL2App&);
	SDL2App& operator=(const SDL2App&);

	void updateInputControllers();
	void handleCommonInput();

	// Working directory helpers
	std::string composeAssetRootDir(const std::string& p_basePath) const;
	void setAssetRootDir(const std::string& p_path);

	// Called by the view:
	virtual void onAppActive();
	virtual void onAppInactive();

	bool setVideoMode(const bool p_windowed, const math::Point2& p_size, const std::string& title = "");

	bool createMainWindow(bool p_showBuildLabel, const args::CmdLine& p_cmdLine, const std::string& p_appIcon);
	
	// NOTE: This is the first member variable so that it gets initialized first,
	//       before any other member initialization might interfere/crash
	StartupStateSDL2 m_startupState;
	
	AppSettings m_settings;

	// The client application
	AppInterface* m_app;
	
	PlatformApiPtr m_platformApi;
	
	bool m_initialized;
	
	u64  m_frameTime;
	s32  m_targetTimeSlice;
	
	inline bool isDualScreen() // was bool m_emulateNitro
	{
		return false;
	}
	inline bool isSteamEnabled() // was bool m_steamEnabled;
	{
		return m_settings.emulate  == AppSettings::Emulate_None &&
		       m_settings.platform == AppSettings::Platform_Steam;
	}

	bool m_fps30Mode;
	bool m_active;
	bool m_done;
	
	bool m_frameLimiterEnabled;
	bool m_frameLimiterForced;

	u64 m_prevFrameTimestamp;


#if !defined(TT_BUILD_FINAL)
	// Frame stepper
	enum
	{
		MAX_WAIT_FRAMES = 64   // FIXME: Perhaps make this configurable?
	};
	
	s32 m_curWaitFrame;
	s32 m_totalWaitFrames;
	bool m_frameStepMode;
	
	s32 m_updateTime;
	s32 m_renderTime;
	
	bool m_shouldDisplayDebugInfo;
#endif

	fs::FileSystemPtr   m_memfs;
	fs::FileSystemPtr   m_hostfs;
	fs::FileSystemPtr   m_cloudfs;
	snd::SoundSystemPtr m_soundSystem;
	
	std::string   m_assetRootDir;
	args::CmdLine m_cmdLine;
    
    math::Point2 m_desktopSize;
	SDL_Window* m_screen;
	math::Point2 m_last;
	u32 m_debugKeys;
	engine::renderer::OpenGLContextWrapper* m_contextWrapper;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_APP_SDL2APP_DESKTOP_H)
