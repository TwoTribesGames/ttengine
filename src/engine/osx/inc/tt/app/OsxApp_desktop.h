#if !defined(INC_TT_APP_OSXAPP_DESKTOP_H)
#define INC_TT_APP_OSXAPP_DESKTOP_H

#if defined(TT_PLATFORM_OSX_MAC)


#include <tt/app/AppInterface.h>
#include <tt/app/Application.h>
#include <tt/app/AppSettings.h>
#include <tt/app/PlatformApi.h>
#include <tt/app/StartupStateOsx.h>
#include <tt/args/CmdLine.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/settings/settings.h>
#include <tt/snd/types.h>


namespace tt {

namespace engine {
namespace renderer {
class OpenGLContextWrapper;
}
}
namespace app {


class OsxApp : public Application, public PlatformCallbackInterface
{
public:
	OsxApp(AppInterface* p_app, const AppSettings& p_settings,
	       StartupStateOsx& p_startupState);
	virtual ~OsxApp();
	
	void update(real p_elapsedTime);
	void render();
	
	// PlatformCallbackInterface:
	virtual void onPlatformMenuEnter();
	virtual void onPlatformMenuExit();
	
	// Called by the view:
	virtual void onAppActive();
	virtual void onAppInactive();
	virtual void onResetDevice();
	
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
	
	virtual void setTargetFPS(u32 p_fps);
	virtual u32  getTargetFPS() const { return m_settings.targetFPS; }
	
	virtual void handleResolutionChanged();
	
	inline bool isInitialized() const { return m_initialized; }
	inline bool isFps30Mode()   const { return m_fps30Mode;   }
	inline void* getAppView() const { return m_appView; }
	inline const AppSettings& getSettings() const { return m_settings; }
	
	inline void setVsyncEnabled(bool p_enable) { m_vsyncEnabled = p_enable; }
	inline bool isVsyncEnabled() const { return m_vsyncEnabled; }
	
private:
	void handleCommonInput();
	
	bool createMainWindow(AppSettings& p_settings, bool p_showBuildLabel, 
						  bool p_ios2xMode, const args::CmdLine& p_cmdLine, bool* p_startWindowed_OUT);
	
	// No copying or assignment
	OsxApp(const OsxApp&);
	OsxApp& operator=(const OsxApp&);
	
	
	// NOTE: Calling code (TTdevObjCOsxApp) creates and owns the actual object
	StartupStateOsx& m_startupState;
	
	AppSettings m_settings;
	
	// The client application
	AppInterface* m_app;
	
	PlatformApiPtr m_platformApi;
	
	bool m_initialized;
	
	u64  m_frameTime;
	s32  m_targetTimeSlice; // in micro seconds
	
	bool m_emulateNitro;
	bool m_steamEnabled;
	bool m_hideCursor;
	bool m_useFixedDeltaTime;
	bool m_fps30Mode;
	bool m_active;
	
	bool m_frameLimiterEnabled;
	bool m_vsyncEnabled;
	
	
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
	
	std::string m_textureCacheFileName;
	
	bool m_shouldDisplayDebugInfo;
#endif
	
	fs::FileSystemPtr   m_osxfs;
	fs::FileSystemPtr   m_cloudfs;
	snd::SoundSystemPtr m_soundSystem;
	
	std::string   m_assetRootDir;
	args::CmdLine m_cmdLine;
	void*         m_appView;  // points to a TTdevObjCAppView instance (Objective C class)
	
	u32 m_debugKeys;
	engine::renderer::OpenGLContextWrapper* m_contextWrapper;
};

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_MAC)

#endif  // !defined(INC_TT_APP_OSXAPP_DESKTOP_H)
