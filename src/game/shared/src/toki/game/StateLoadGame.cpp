#include <tt/app/Application.h>
#include <tt/code/helpers.h>
#include <tt/code/StateMachine.h>
#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/platform/tt_printf.h>
#include <tt/thread/CriticalSection.h>
#include <tt/thread/thread.h>

#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/Game.h>
#include <toki/game/StartInfo.h>
#include <toki/game/StateLoadGame.h>
#include <toki/savedata/utils.h>
#include <toki/serialization/utils.h>
#include <toki/statelist/statelist.h>
#include <toki/utils/utils.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>
#include <toki/constants.h>

#if STATELOADGAME_THREADED_LOADING && defined(TT_PLATFORM_WIN)
#include <combaseapi.h>
#endif

namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

StateLoadGame::StateLoadGame(tt::code::StateMachine* p_stateMachine)
:
tt::code::State(p_stateMachine),
m_applicationIsExiting(false),
m_firstFrame(true),
m_haveBgAndLogo(false),
m_loadStep(LoadStep_Loading),
m_activityIndicatorNeedsFadeIn(false),
m_activityIndicatorFadeInTimeout(0.0f),
m_backgroundQuad(),
m_backgroundMusicVolume(1.0f)
#if STATELOADGAME_THREADED_LOADING
,
m_loadThread(),
m_mutex(0)
#endif
{
}


StateLoadGame::~StateLoadGame()
{
}


void StateLoadGame::enter()
{
	// Do not allow the HOME Button Menu while loading
	tt::app::getApplication()->setPlatformMenuEnabled(false);
	
	TT_ASSERTMSG(AppGlobal::hasGame() == false,
	             "Internal inconsistency: Game instance was not destroyed "
	             "before StateLoadGame::enter was called.");
	
	AppGlobal::setBusyLoading(true);
	
	// We don't need a controller to be connected during loads
	// FIXME CONTROLLER: Perhaps we should know at this point HOW MANY controllers should be connected based on number of players
	AppGlobal::getController(tt::input::ControllerIndex_One).setConnectionRequired(false);
	
	using tt::engine::renderer::QuadSprite;
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	m_haveBgAndLogo = gfx.hasLoadingBgAndLogo();
	
	if (m_haveBgAndLogo == false)
	{
		// Came from game (loading new level): ensure we black out the screen
		m_backgroundQuad = QuadSprite::createQuad(10000.0f, 10000.0f, tt::engine::renderer::ColorRGB::black);
		
		// Also only fade the activity indicator in after some time has passed
		m_activityIndicatorNeedsFadeIn   = true;
		m_activityIndicatorFadeInTimeout = 3.0f;
		
		for (s32 i = 0; i < Screen_Count; ++i)
		{
			gfx.getActivityIndicator(static_cast<Screen>(i))->resetFlag(QuadSprite::Flag_Visible);
		}
	}
	else
	{
		// Activity indicator is still visible from the app load state: don't fade it in
		m_activityIndicatorNeedsFadeIn   = false;
		m_activityIndicatorFadeInTimeout = 0.0f;
	}
	
	gfx.getFadeQuad()->setOpacity(0);
	gfx.getFadeQuad()->resetFlag(QuadSprite::Flag_Visible);
	
	m_applicationIsExiting = false;
	m_firstFrame           = true;
	m_loadStep             = LoadStep_Loading;
	
	m_backgroundMusicVolume = tt::math::TimedLinearInterpolation<real>(1.0f);
	
	gfx.updateAll();
	
#if STATELOADGAME_THREADED_LOADING
	TT_ASSERT(m_mutex == 0);
	m_mutex = new tt::thread::Mutex;
	
	TT_ASSERT(m_loadThread == 0);
	m_loadThread = tt::thread::create(staticGameLoadThread, this, false, 0,
			tt::thread::priority_below_normal, tt::thread::Affinity_None, "Game Load Thread");
#endif
}


void StateLoadGame::exit()
{
	TT_Printf("StateLoadGame::exit: Exiting state.\n");
	
#if STATELOADGAME_THREADED_LOADING
	tt::thread::wait(m_loadThread);
	m_loadThread.reset();
	
	delete m_mutex;
	m_mutex = 0;
#endif

	if (AppGlobal::hasGame() && m_applicationIsExiting == false)
	{
		AppGlobal::getGame()->initOnRenderThread();
	}
	
	m_backgroundQuad.reset();
	
	AppGlobal::getSharedGraphics().stopBackgroundMusic();
	
	if (m_applicationIsExiting && AppGlobal::hasGame())
	{
		Game* g = AppGlobal::getGame();
		AppGlobal::setGame(0);
		delete g;
	}
	
	// No longer loading: re-enable HOME Button Menu
	tt::app::getApplication()->setPlatformMenuEnabled(true);
}


void StateLoadGame::update(real p_deltaTime)
{
	AppGlobal::setBusyLoading(true);
	
	{
#if STATELOADGAME_THREADED_LOADING
		tt::thread::CriticalSection critSec(m_mutex);
#else
		// For non-threaded loading: load the game after first rendering our load screen
		if (m_firstFrame == false && m_loadStep == LoadStep_Loading)
		{
			gameLoadThread();
		}
#endif
		
		SharedGraphics& gfx(AppGlobal::getSharedGraphics());
		using tt::engine::renderer::QuadSprite;
		tt::engine::renderer::QuadSpritePtr fadeQuad(gfx.getFadeQuad());
		
		switch (m_loadStep)
		{
		case LoadStep_Loading:
			break;

		case LoadStep_LoadComplete:
			// If we're rendering the controls image, wait until it has been displayed for long enough
			if (m_haveBgAndLogo             == false ||
			    gfx.willShowControlsImage() == false ||
			    gfx.isControlsImageShownLongEnough())
			{
				fadeQuad->fadeIn(0.25f);
				m_backgroundMusicVolume.startNewInterpolation(0.0f, 0.25f);
				m_loadStep = LoadStep_FadingOut;
				// No longer need to fade the indicator in: we're already fading the screen out
				m_activityIndicatorNeedsFadeIn = false;
			}
			break;
		
		case LoadStep_FadingOut:
			if (fadeQuad->checkFlag(QuadSprite::Flag_FadingIn)  == false &&
			    fadeQuad->checkFlag(QuadSprite::Flag_FadingOut) == false)
			{
				m_loadStep = LoadStep_FadeComplete;
			}
			break;
		
		case LoadStep_FadeComplete:
			changeState(statelist::StateID_Game);
			break;
		
		default:
			TT_PANIC("Unsupported load step: %d", m_loadStep);
			m_loadStep = LoadStep_FadeComplete;
			break;
		}
	}
	
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	gfx.updateAnimTime(p_deltaTime);
	
	if (m_activityIndicatorNeedsFadeIn)
	{
		m_activityIndicatorFadeInTimeout -= p_deltaTime;
		if (m_activityIndicatorFadeInTimeout <= 0.0f)
		{
			m_activityIndicatorNeedsFadeIn = false;
			for (s32 i = 0; i < Screen_Count; ++i)
			{
				// FIXME: How fast should the indicator fade in?
				gfx.getActivityIndicator(static_cast<Screen>(i))->fadeIn(0.25f);
			}
		}
	}
	
	
	if (m_backgroundMusicVolume.isDone() == false)
	{
		m_backgroundMusicVolume.update(p_deltaTime);
		gfx.setBackgroundMusicVolume(m_backgroundMusicVolume.getValue());
	}
	gfx.updateBackgroundMusic();
	m_firstFrame = false;
}


void StateLoadGame::updateForRender(real /*p_deltaTime*/)
{
	AppGlobal::getSharedGraphics().updateAll();
	
	if (m_backgroundQuad != 0)
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		m_backgroundQuad->setPosition(renderer->getScreenWidth()  * 0.5f,
		                              renderer->getScreenHeight() * 0.5f,
		                              0.0f);
		m_backgroundQuad->update();
	}
}


void StateLoadGame::render()
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	
	const Screen screen = utils::getScreenFromViewPortID(renderer->getActiveViewPort());
	TT_ASSERT(isValidScreen(screen));
	
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	
	renderer->beginHud();
	if (m_backgroundQuad != 0)
	{
		m_backgroundQuad->render();
	}
	gfx.renderLoadingBackground(screen);
	if (gfx.hasLevelLoadQuad())
	{
		gfx.renderLevelLoadQuad(screen);
		gfx.renderLogo(screen);
	}
	gfx.renderActivityIndicator(screen);
	gfx.renderFadeQuad();
	
	renderer->endHud();
}


void StateLoadGame::handleVBlankInterrupt()
{
}


tt::code::StateID StateLoadGame::getPathToState(tt::code::StateID p_targetState) const
{
	// Always allow immediate switching to the app exit state
	if (p_targetState == statelist::StateID_ExitApp)
	{
		m_applicationIsExiting = true;
		return p_targetState;
	}
	
	TT_ASSERT(p_targetState == statelist::StateID_Game);
	{
#if STATELOADGAME_THREADED_LOADING
		tt::thread::CriticalSection critSec(m_mutex);
#endif
		// Not allowed to go to a different state if loading isn't done yet
		if (m_loadStep != LoadStep_FadeComplete)
		{
			return statelist::StateID_LoadGame;
		}
	}
	
	return p_targetState;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

int StateLoadGame::staticGameLoadThread(void* p_arg)
{
	StateLoadGame* state = reinterpret_cast<StateLoadGame*>(p_arg);
	state->gameLoadThread();
	return 0;
}


void StateLoadGame::gameLoadThread()
{
#if STATELOADGAME_THREADED_LOADING && defined(TT_PLATFORM_WIN)
	// COM needs to be initialized on each thread it is used
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	TT_ASSERT(SUCCEEDED(hr));
#endif
	
	Game* g = new Game;
	AppGlobal::setGame(g);
	
	
	// Do fail safe check
	StartInfo startInfo;
	ProgressType overrideProgressType = ProgressType_Invalid;
	
	if (AppGlobal::shouldStartupFailSafeLevel())
	{
		const std::string failSafeLevelName(cfg()->getStringDirect("toki.startup.restore_failure_level"));
		startInfo.setLevel(failSafeLevelName);
		AppGlobal::setShouldStartupFailSafeLevel(false);
		AppGlobal::getShutdownDataMgr().reset();
	}
	else
	{
		startInfo = AppGlobal::getGameStartInfo();
		
		// Shutdown restore data overrides normal start info (go to the right level)
		if (AppGlobal::getShutdownDataMgr().hasData())
		{
			bool loadOk = false;
			startInfo = Game::loadStartInfo(*AppGlobal::getShutdownDataMgr().getData(), loadOk);
			if (loadOk == false)
			{
				// Shutdown data is no longer valid: start info could not be loaded
				// (this can happen if the level it points to no longer exists, for example)
				AppGlobal::getShutdownDataMgr().reset();
				startInfo = AppGlobal::getGameStartInfo();
			}
		}
		else
		{
			overrideProgressType = AppGlobal::getNextLevelOverrideProgressType();
		}
	}
	
	AppGlobal::setNextLevelOverrideProgressType(ProgressType_Invalid);
	g->init(startInfo, overrideProgressType);
	
	{
#if STATELOADGAME_THREADED_LOADING
		tt::thread::CriticalSection critSec(m_mutex);
#endif
		TT_ASSERT(m_loadStep == LoadStep_Loading);
		m_loadStep = LoadStep_LoadComplete;
	}
	
#if STATELOADGAME_THREADED_LOADING && defined(TT_PLATFORM_WIN)
	CoUninitialize();
#endif
}

// Namespace end
}
}
