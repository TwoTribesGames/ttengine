#if !defined(INC_TOKI_MAIN_STATELOADAPP_H)
#define INC_TOKI_MAIN_STATELOADAPP_H

#include <atomic>
#include <list>

#include <tt/audio/player/fwd.h>
#include <tt/code/State.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>
#include <tt/pres/fwd.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/thread.h>

#include <toki/main/loadstate/LoadState.h>
#include <toki/constants.h>


namespace toki {
namespace main {

/*! \brief State responsible for loading all application-global resources
           (is the first state that gets executed). */
class StateLoadApp : public tt::code::State
{
public:
	explicit StateLoadApp(tt::code::StateMachine* p_stateMachine);
	virtual ~StateLoadApp();
	
	virtual void enter();
	virtual void exit();
	virtual void update(real p_deltaTime);
	virtual void updateForRender(real p_deltaTime);
	virtual void render();
	virtual void handleVBlankInterrupt();
	virtual tt::code::StateID getPathToState(tt::code::StateID p_targetState) const;
	
	virtual void onResetDevice();
	virtual void onAppPaused();
	virtual void onAppResumed();
	
private:
	typedef std::vector<tt::pres::PresentationObjectPtr> Presentations;
	typedef std::list<loadstate::LoadStatePtr> LoadStates;
	enum GraphicState
	{
		GraphicState_Loading,
		GraphicState_Fade,
		GraphicState_Cleanup
	};
	
	void setupLoadStates();
	void calculateLoadSteps();
	void createLoadingGraphics();
	void destroyLoadingGraphics();
	void positionLoadingGraphics();
	void startFadeOut();
	void onLoadComplete();
	void alignLoadingGraphics();
	
#if USE_THREADED_LOADING
	static int staticLoadThread(void* p_arg);
	int loadThread();
#endif
	
	
	LoadStates m_loadStates;
	s32        m_currentStep;
	s32        m_totalLoadSteps;
	
	u64 m_loadStartTimestamp;
	
	tt::code::StateID m_postLoadState;
	
	// Load graphics:
	tt::pres::PresentationMgrPtr    m_presentationMgr;
	Presentations                   m_presentations;
	bool                            m_isFadingIn;  // whether fading the load screen in from system startup
	GraphicState                    m_graphicState;
	
#if USE_THREADED_LOADING
	std::atomic<bool>  m_loadSuspended;
	std::atomic<bool>  m_stopLoading;
	tt::thread::handle m_loadThread;
	tt::thread::Mutex* m_mutex;
#endif
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_MAIN_STATELOADAPP_H)
