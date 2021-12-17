#if !defined(INC_TT_APP_OSXAPP_IPHONE_H)
#define INC_TT_APP_OSXAPP_IPHONE_H

#if defined(TT_PLATFORM_OSX_IPHONE)


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
namespace app {

class OsxApp : public Application, public PlatformCallbackInterface
{
public:
	/*! \param p_objCApp Objective C application (delegate) class (type TTdevObjCOsxApp). */
	OsxApp(AppInterface* p_app, const AppSettings& p_settings, void* p_objCApp,
	       StartupStateOsx& p_startupState);
	virtual ~OsxApp();
	
	void update(real p_elapsedTime);
	void render();
	
	// PlatformCallbackInterface:
	virtual void onPlatformMenuEnter();
	virtual void onPlatformMenuExit();
	
	// Called by the Objective C application delegate:
	virtual void onAppActive();
	virtual void onAppInactive();
	virtual void onAppEnteredBackground();
	virtual void onAppLeftBackground();
	virtual bool onShouldAutoRotateToOrientation(AppOrientation p_orientation) const;
	virtual void onWillRotateToOrientation(AppOrientation p_orientation, real p_duration);
	virtual void onDidRotateFromOrientation(AppOrientation p_fromOrientation);
	
	// Application:
	virtual std::string getAssetRootDir() const;
	virtual fs::identifier getSaveFsID() const;
	virtual const args::CmdLine& getCmdLine() const;
	virtual const StartupState& getStartupState() const;
	virtual void setFullScreen(bool p_fullScreen);
	virtual bool isFullScreen() const;
	virtual bool shouldDisplayDebugInfo() const;
	
	virtual void terminate(bool p_graceful);
	virtual void setPaused(bool p_paused);
	virtual inline bool isActive() const { return m_active; }
	
	virtual void setHandleDebugKeys(u32 /*p_keys*/) { }
	
	
	inline bool isInitialized() const { return m_initialized; }
	inline bool isFps30Mode()   const { return m_fps30Mode;   }
	bool isSplashGone() const;
	
	/*! \return Application window (type UIWindow). */
	inline void* getAppWindow() const { return m_appWindow; }
	
	/*! \return Application view (type UIView). */
	inline void* getAppView() const { return m_appView; }
	
	/*! \return Objective C application (delegate) class (type TTdevObjCOsxApp). */
	inline void* getObjCApp() const { return m_objCApp; }
	
	/*! \return Application view controller (type UIViewController). */
	void* getAppViewController() const;
	
	/*! \brief Not for client code, needed by Obj C Osx App. */
	void handleTimerIsActive(bool p_active) { m_timerIsActive = p_active; }
	
private:
	void handleCommonInput();
	
	bool createMainWindow(const AppSettings& p_settings, bool p_showBuildLabel, 
						  bool p_ios2xMode, const args::CmdLine& p_cmdLine);
	
	// No copying or assignment
	OsxApp(const OsxApp&);
	OsxApp& operator=(const OsxApp&);
	
	
	// NOTE: Calling code (TTdevObjCOsxApp) creates and owns the actual object
	StartupStateOsx& m_startupState;
	
	// The client application
	AppInterface* m_app;
	
	PlatformApiPtr m_platformApi;
	
	bool m_initialized;
	
	u64  m_frameTime;
	s32  m_targetTimeSlice;
	
	bool m_emulateNitro;
	bool m_steamEnabled;
	bool m_hideCursor;
	bool m_useFixedDeltaTime;
	bool m_fps30Mode;
	bool m_active;
	bool m_appPaused;
	
	bool m_frameLimiterEnabled;
	
	// Frame stepper
#if !defined(TT_BUILD_FINAL)
	enum
	{
		MAX_WAIT_FRAMES = 64   // FIXME: Perhaps make this configurable?
	};
	
	s32 m_curWaitFrame;
	s32 m_totalWaitFrames;
	bool m_frameStepMode;
	
	s32 m_updateTime;
	s32 m_renderTime;
#endif
	bool m_timerIsActive; // Flag which can be used to force ignore update/render.
	                      // (Needed to handle tt panics with CADisplayLink. Will double trigger.)
	
	fs::FileSystemPtr   m_osxfs;
	fs::FileSystemPtr   m_buffs;
	fs::FileSystemPtr   m_cloudfs;
	snd::SoundSystemPtr m_soundSystem;
	
	std::string   m_assetRootDir;
	args::CmdLine m_cmdLine;
	bool          m_firstUpdate; // Whether this is the first update call of the application
	void*         m_appView;     // points to a TTdevObjCAppView instance (Objective C class)
	void*         m_appWindow;   // points to a UIWindow instance (Objective C class)
	void*         m_objCApp;     // points to our 'parent' Objective C application class
};

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)

#endif  // !defined(INC_TT_APP_OSXAPP_IPHONE_H)
