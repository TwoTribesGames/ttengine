#if !defined(INC_TOKI_GAME_STATELOADGAME_H)
#define INC_TOKI_GAME_STATELOADGAME_H


#include <tt/code/State.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/TimedLinearInterpolation.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/types.h>

#include <toki/constants.h>  // needed for USE_THREADED_LOADING


namespace toki {
namespace game {


#define STATELOADGAME_THREADED_LOADING USE_THREADED_LOADING


class StateLoadGame : public tt::code::State
{
public:
	explicit StateLoadGame(tt::code::StateMachine* p_stateMachine);
	virtual ~StateLoadGame();
	
	virtual void enter();
	virtual void exit();
	virtual void update(real p_deltaTime);
	virtual void updateForRender(real p_deltaTime);
	virtual void render();
	virtual void handleVBlankInterrupt();
	virtual tt::code::StateID getPathToState(tt::code::StateID p_targetState) const;
	
private:
	enum LoadStep
	{
		LoadStep_Loading,
		LoadStep_LoadComplete,
		LoadStep_FadingOut,
		LoadStep_FadeComplete
	};
	
	
	static int staticGameLoadThread(void* p_arg);
	void gameLoadThread();
	
	
	mutable bool m_applicationIsExiting;
	bool         m_firstFrame;
	
	bool     m_haveBgAndLogo;
	LoadStep m_loadStep;
	
	bool m_activityIndicatorNeedsFadeIn;
	real m_activityIndicatorFadeInTimeout;
	
	// Simple background for when no background image is available
	tt::engine::renderer::QuadSpritePtr m_backgroundQuad;
	tt::math::TimedLinearInterpolation<real> m_backgroundMusicVolume;
	
#if STATELOADGAME_THREADED_LOADING
	tt::thread::handle m_loadThread;
	tt::thread::Mutex* m_mutex;
#endif
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_STATELOADGAME_H)
