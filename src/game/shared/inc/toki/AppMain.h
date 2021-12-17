#if !defined(INC_TOKI_APPMAIN_H)
#define INC_TOKI_APPMAIN_H


#include <tt/app/AppInterface.h>
#include <tt/engine/renderer/pp/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/fs/types.h>

#if !defined(TT_BUILD_FINAL)
#define ENABLE_DEBUG_INFO 1
#else
#define ENABLE_DEBUG_INFO 0
#endif

namespace toki {

namespace main {
class AppStateMachine;
}

class AppMain : public tt::app::AppInterface
{
public:
	AppMain();
	virtual ~AppMain();
	
	virtual bool init();
	
	virtual void update(real p_elapsedTime);
	virtual void render();
	
	virtual void overrideGraphicsSettings(tt::app::GraphicsSettings* p_current_OUT,
	                                      const tt::math::Point2&    p_desktopSize);
	
	virtual void onRequestReloadAssets();
	
	// From tt::app::PlatformCallbackInterface:
	virtual void onPlatformMenuEnter();
	virtual void onPlatformMenuExit();
	virtual void onAppInactive();
	virtual void onAppActive();
	virtual void onAppPaused();
	virtual void onAppResumed();
	virtual void onAppEnteredBackground();
	virtual void onAppLeftBackground();
	virtual void onLostDevice();
	virtual void onResetDevice();
	virtual void onSetPlayerCount(u32 p_newCount);
	
private:
	typedef std::vector<tt::engine::renderer::pp::FilterPtr> CachedShaders;
	void renderDebugInfo(s32 p_fps, u64 p_renderTime);
	void initializePostProcessing();
	bool hasLoadedGame() const;
	void takeLevelScreenshot();
	
	
	main::AppStateMachine* m_stateMachine;
	
	tt::fs::MemoryArchivePtr m_archive;
	CachedShaders m_cachedShaders;
	
	real m_badPerfTime;
	s32  m_badPerfUpdateCount;     //  The total number of updates for this sample window. (These are used to detect bad performance.)
	
#if ENABLE_DEBUG_INFO
	u64 m_maxUpdateTime;
	u64 m_minUpdateTime;
	u64 m_averageUpdateTime;
	u32 m_updateCount;
#endif // #if ENABLE_DEBUG_INFO
	tt::engine::renderer::QuadSpritePtr m_fpsQuad;
	bool m_showFps;
	
#if !defined(TT_BUILD_FINAL)
	bool m_enableMemoryBudgetWarning;
#endif
};

// Namespace end
}


#endif  // !defined(INC_TOKI_APPMAIN_H)
