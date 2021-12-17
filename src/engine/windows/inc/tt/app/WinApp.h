#if !defined(INC_TT_APP_WINAPP_H)
#define INC_TT_APP_WINAPP_H

#include <map>
#include <deque>

#include <tt/app/AppInterface.h>
#include <tt/app/Application.h>
#include <tt/app/AppSettings.h>
#include <tt/app/PlatformApi.h>
#include <tt/app/StartupStateWin.h>
#include <tt/args/CmdLine.h>
#include <tt/engine/renderer/DXUT/DXUT.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/settings/settings.h>
#include <tt/snd/types.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace app {

class FrameRateManager
{
public:
	static void setTargetFramerate(s32 p_fps);
	static bool isVsyncEnabled() {return ms_vsyncEnabled;}
	static void modifyDeviceSettings(DXUTDeviceSettings* p_settings);
	static void monitorFramerate();
	
private:
	typedef std::map<u32, u32> NativeRefreshRates;
	static NativeRefreshRates ms_nativeRefreshRates;
	
	typedef std::deque<float> FrameRates;
	static FrameRates ms_framerateHistory;
	
	static bool ms_vsyncEnabled;
	static u32  ms_targetFPS;
	static float ms_averageFPS;
};


class WinApp : public Application, public PlatformCallbackInterface
{
public:
	WinApp(AppInterface* p_app, const AppSettings& p_settings);
	~WinApp();
	
	void update(real p_elapsedTime);
	void render();
	
	virtual void onPlatformMenuEnter();
	virtual void onPlatformMenuExit();
	virtual void onLostDevice();
	virtual void onResetDevice();
	
	s32 run();
	
	// Debugging
	static bool generateDump(EXCEPTION_POINTERS* pExceptionPointers);
	
	// Application:
	virtual std::string getAssetRootDir() const;
	virtual fs::identifier getSaveFsID() const;
	virtual const args::CmdLine& getCmdLine() const;
	virtual const StartupState& getStartupState() const;
	virtual void setFullScreen(bool p_fullScreen);
	virtual bool isFullScreen() const;
	virtual void setFullScreenResolution(const math::Point2& p_resolution);
	virtual tt::math::Point2 getFullScreenResolution() const;
	virtual tt::math::Point2 getDesktopSize() const;
	
	virtual bool shouldDisplayDebugInfo() const;
	
	virtual void terminate(bool p_graceful);
	virtual void setPaused(bool p_paused);
	virtual inline bool isActive() const { return m_active; }
	
	virtual void setHandleDebugKeys(u32 p_keys) { m_debugKeys = p_keys; }
	
	virtual void setPlatformMenuEnabled(bool p_enabled);
	
	virtual void       setTargetFPS(u32 p_fps);
	virtual inline u32 getTargetFPS() const { return m_settings.targetFPS; }
	
	virtual void setPlayerCount(u32 p_playerCount);
	
	virtual void handleResolutionChanged();
	
private:
	// No copying or assignment
	WinApp(const WinApp&);
	WinApp& operator=(const WinApp&);
	
	void updateInputControllers();
	void handleCommonInput();
	void toggleConsole();
	
	// Working directory helpers
	std::string composeAssetRootDir(const std::string& p_basePath) const;
	void setAssetRootDir(const std::string& p_path);
	
	// DXUT Hooks
	static HRESULT CALLBACK onResetDevice (IDirect3DDevice9*, const D3DSURFACE_DESC*, void*);
	static void CALLBACK onLostDevice( void*);
	static void CALLBACK onFrameMove(double, float, void*);
	static void CALLBACK onFrameRender(IDirect3DDevice9*, double, float, void*);
	
	
	// NOTE: This is the first member variable so that it gets initialized first,
	//       before any other member initialization might interfere/crash
	StartupStateWin m_startupState;
	
	const DWORD   m_creationThreadId;  // ID of the thread on which WinApp was created
	bool          m_shouldPostQuitMessage;
	thread::Mutex m_postQuitMessageMutex;
	
	AppSettings m_settings;
	
	// The client application
	AppInterface* m_app;
	
	PlatformApiPtr m_platformApi;
	
	bool m_initialized;
	
	u64  m_frameTime;
	u32 m_targetTimeSlice; // in micro seconds.
	
	inline bool isDualScreen()
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
	
	bool m_frameLimiterEnabled;
	bool m_frameLimiterForced;
	
#if !defined(TT_BUILD_FINAL)
	// framestepper
	enum 
	{
		MAX_WAIT_FRAMES  = 64, // FIXME: Perhaps make this configurable?
		FASTMODE_UPDATES = 5
	};
	
	int m_curWaitFrame;
	int m_totalWaitFrames;
	bool m_frameStepMode;
	
	s32 m_updateTime;
	s32 m_renderTime;
	
	std::string m_textureCacheFileName;
	
	bool m_shouldDisplayDebugInfo;
	bool m_displayConsole;
#endif
	
	fs::FileSystemPtr   m_memfs;
	fs::FileSystemPtr   m_winfs;
	fs::FileSystemPtr   m_cloudfs;
	snd::SoundSystemPtr m_soundSystem;
	
	std::string   m_assetRootDir;
	args::CmdLine m_cmdLine; // the command line arguments that were passed (possibly modified for final builds)

	u32 m_debugKeys;
};

// Namespace end
}
}

#endif // !defined(INC_TT_APP_WINAPP_H)
