#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/audio/player/TTIMPlayer.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_printf.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/system/Time.h>
#include <tt/thread/CriticalSection.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/types.h>
#include <toki/input/Recorder.h>
#include <toki/main/loadstate/LoadStateAudioPlayer.h>
#include <toki/main/loadstate/LoadStateEntityLibrary.h>
#include <toki/main/loadstate/LoadStateGame.h>
#include <toki/main/loadstate/LoadStateInitSaveSystem.h>
#include <toki/main/loadstate/LoadStateGenerateMetaData.h>
#include <toki/main/loadstate/LoadStatePrecache.h>
#include <toki/main/loadstate/LoadStateScriptLists.h>
#include <toki/main/loadstate/LoadStateScriptMgr.h>
#include <toki/main/loadstate/LoadStateShoeboxPrecache.h>
#include <toki/main/loadstate/LoadStateSkinConfigs.h>
#include <toki/main/StateLoadApp.h>
#include <toki/pres/TriggerFactory.h>
#include <toki/statelist/statelist.h>
#include <toki/utils/GlyphSetMgr.h>
#include <toki/utils/utils.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>

#if defined(TT_PLATFORM_WIN)
#include <combaseapi.h>
#endif

namespace toki {
namespace main {



//--------------------------------------------------------------------------------------------------
// Public member functions

StateLoadApp::StateLoadApp(tt::code::StateMachine* p_stateMachine)
:
tt::code::State(p_stateMachine),
m_loadStartTimestamp(0),
m_postLoadState(statelist::StateID_Game),
m_presentationMgr(),
m_presentations(),
m_isFadingIn(true),
m_graphicState(GraphicState_Loading)
#if USE_THREADED_LOADING
,
m_loadSuspended(false),
m_stopLoading(false),
m_loadThread(),
m_mutex(0)
#endif
{
}


StateLoadApp::~StateLoadApp()
{
}


void StateLoadApp::enter()
{
	// Do not allow the HOME Button Menu while loading
	tt::app::getApplication()->setPlatformMenuEnabled(false);
	
	m_loadStartTimestamp = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("StateLoadApp::enter: Starting application load.\n");
	
	// We don't need a controller to be connected during loads
	// FIXME CONTROLLER: Perhaps we should know at this point HOW MANY controllers should be connected based on number of players
	AppGlobal::getController(tt::input::ControllerIndex_One).setConnectionRequired(false);
	
	m_postLoadState = statelist::StateID_Game;
	m_graphicState  = GraphicState_Loading;
	
#if !defined(TT_BUILD_FINAL)
	const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
	
	if (cmdLine.exists("state"))
	{
		const std::string stateName(cmdLine.getString("state"));
		tt::code::StateID stateFromCmdLine(statelist::getStateFromName(stateName));
		if (stateFromCmdLine.isValid() == false)
		{
			TT_PANIC("Invalid state specified on command line: '%s'", stateName.c_str());
		}
		else if (stateFromCmdLine == statelist::StateID_LoadApp ||
		         stateFromCmdLine == statelist::StateID_ExitApp)
		{
			TT_PANIC("Cannot start in state '%s' (specified on command line).", stateName.c_str());
		}
		else
		{
			m_postLoadState = stateFromCmdLine;
		}
	}
	
	if (AppGlobal::shouldCompileSquirrel())
	{
		m_postLoadState = statelist::StateID_ExitApp;
	}
#endif
	
	// HACK: Load glyph set here (should actually be in a separate load state)
	utils::GlyphSetMgr::loadAll();
	
	setupLoadStates();
	
	m_presentationMgr = tt::pres::PresentationMgr::create(
		tt::pres::Tags(),
		tt::pres::TriggerFactoryInterfacePtr(new toki::pres::TriggerFactory));
	
	// Create some graphics to display while loading
	createLoadingGraphics();
	
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	// NOTE: Loading music is now started by LoadStateInitSaveSystem
	// (so that volume settings are available before music starts playing)
	//gfx.startBackgroundMusic();
	
	AppGlobal::setBusyLoading(true);
	tt::engine::renderer::Renderer::getInstance()->setZBufferEnabled(false);
	
	m_isFadingIn = true;
	gfx.getFadeQuad()->fadeOut(0.25f);
	
#if USE_THREADED_LOADING
	m_loadSuspended = false;
	m_stopLoading   = false;
	TT_ASSERT(m_mutex == 0);
	m_mutex = new tt::thread::Mutex;
	
	TT_ASSERT(m_loadThread == 0);
	m_loadThread = tt::thread::create(staticLoadThread, this, false, 0, tt::thread::priority_below_normal, 
			tt::thread::Affinity_None, "Application Load Thread");
#endif
}


void StateLoadApp::exit()
{
#if USE_THREADED_LOADING
	TT_NULL_ASSERT(m_mutex);
	m_mutex->lock();
	m_loadSuspended = false;
	m_stopLoading   = true;
#endif
	
	m_loadStates.clear();
	destroyLoadingGraphics();
	
#if USE_THREADED_LOADING
	m_mutex->unlock();
	
	tt::thread::wait(m_loadThread);
	m_loadThread.reset();
	
	delete m_mutex;
	m_mutex = 0;
#endif
	
	if (AppGlobal::isInLevelEditorMode() == false)
	{
		// In case our internal builds didn't get the chance to fade the logo in yet,
		// make sure it is visible at the level load screen
		AppGlobal::getSharedGraphics().ensureLogoVisible();
	}
	
	if (m_postLoadState == statelist::StateID_PresentationViewer)
	{
		AppGlobal::getSharedGraphics().stopBackgroundMusic();
	}
	
	// No longer loading: re-enable HOME Button Menu
	tt::app::getApplication()->setPlatformMenuEnabled(true);
}


void StateLoadApp::update(real p_deltaTime)
{
#if USE_THREADED_LOADING
	tt::thread::CriticalSection critSec(m_mutex);
#endif
	
	if (m_presentationMgr != 0)
	{
		m_presentationMgr->update(p_deltaTime);
	}
	
	if (tt::engine::particles::ParticleMgr::hasInstance())
	{
		tt::engine::particles::ParticleMgr::getInstance()->update(p_deltaTime);
	}
	
	// Check for failsafe level startup button combination
	if (AppGlobal::getController(tt::input::ControllerIndex_One).cur.startupFailSafeLevel.pressed)
	{
		AppGlobal::setShouldStartupFailSafeLevel(true);
		TT_Printf("*** STARTING UP FAILSAFE LEVEL ***\n");
	}
	
	AppGlobal::setBusyLoading(true);
	
	// Check if we should create the audio player on the main thread
	if (audio::AudioPlayer::needsCreateOnMainThread() &&
	    audio::AudioPlayer::hasInstance() == false    &&
	    audio::AudioPlayer::shouldCreateOnMainThreadNow())
	{
		//TT_Printf("StateLoadApp::update: Creating AudioPlayer on main thread.\n");
		audio::AudioPlayer::createInstance();
	}
	
	using tt::engine::renderer::QuadSprite;
	
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	gfx.updateAnimTime(p_deltaTime);
	gfx.updateBackgroundMusic();
	
	if (m_isFadingIn                                                      &&
	    gfx.getFadeQuad()->checkFlag(QuadSprite::Flag_FadingIn)  == false &&
	    gfx.getFadeQuad()->checkFlag(QuadSprite::Flag_FadingOut) == false)
	{
		m_isFadingIn = false;
		
		// Fade the activity indicator in after the startup fade is gone
		for (s32 i = 0; i < Screen_Count; ++i)
		{
			gfx.getActivityIndicator(static_cast<Screen>(i))->fadeIn(0.5f);
		}
	}
	
	// Perform another load step
#if USE_THREADED_LOADING == 0
	if (m_loadStates.empty() == false)
	{
		const loadstate::LoadStatePtr& state(m_loadStates.front());
		state->doLoadStep();
		// FIXME: Wrong... should update this step counter some other way (doLoadStep could perform more than one step per call)
		++m_currentStep;
		
		if (state->isDone())
		{
			m_loadStates.pop_front();
		}
	}
	else
#endif  // USE_THREADED_LOADING == 0
	if (m_graphicState == GraphicState_Fade &&
	    gfx.getFadeQuad()->checkFlag(QuadSprite::Flag_FadingIn) == false)
	{
		{
			// And clean up the non-fade rendering resources
			destroyLoadingGraphics();
			m_graphicState = GraphicState_Cleanup;
		}
	}
	else if (m_graphicState == GraphicState_Cleanup)
	{
		// Switch to the new app state only after the screen is fully faded out
		// (and fully faded out screen has been rendered)
		onLoadComplete();
	}
	
	// Only move on from the loading state if loading is complete
	// (however, do not wait for the loading graphics to complete in "no precache" mode: we want fast startup in that case)
	if (m_graphicState == GraphicState_Loading &&
	    m_loadStates.empty())
	{
		startFadeOut();
	}
}


void StateLoadApp::updateForRender(real p_deltaTime)
{
#if USE_THREADED_LOADING
	tt::thread::CriticalSection critSec(m_mutex);
#endif
	
	alignLoadingGraphics();
	
	if (m_presentationMgr != 0)
	{
		m_presentationMgr->updateForRender(p_deltaTime);
	}
}


void StateLoadApp::render()
{
#if USE_THREADED_LOADING
	tt::thread::CriticalSection critSec(m_mutex);
#endif
	
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	
	const Screen screen = utils::getScreenFromViewPortID(renderer->getActiveViewPort());
	TT_ASSERT(isValidScreen(screen));
	
	renderer->beginHud();
	
	if (m_presentationMgr != 0)
	{
		m_presentationMgr->render();
	}
	
	if (tt::engine::particles::ParticleMgr::hasInstance())
	{
		tt::engine::particles::ParticleMgr::getInstance()->renderAllGroups();
	}
	
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	gfx.renderActivityIndicator(screen);
	
/*
	gfx.renderLoadingBackground(screen);
	
	if (m_currentGraphicIndex < m_loadGraphics.size())
	{
		const LoadGraphic& graphic(m_loadGraphics[m_currentGraphicIndex]);
		if (graphic.quad[screen] != 0)
		{
			graphic.quad[screen]->render();
		}
	}
	else if (gfx.hasLevelLoadQuad())
	{
		gfx.renderLevelLoadQuad(screen);
		gfx.renderLogo(screen);
	}
*/
	gfx.renderActivityIndicator(screen);
	
#if !defined(TT_BUILD_FINAL)
	if (screen == Screen_TV)
	{
		const std::string loadText(m_loadStates.empty() ? "Starting game..." : m_loadStates.front()->getName());
		s32 textOffset = static_cast<s32>(loadText.length() * 3);
		tt::math::Point2 loadTextPos((renderer->getScreenWidth() / 2) - textOffset, 20);
		
		static const tt::engine::renderer::ColorRGBA shadowColor(0,   0,   0, 255);
		static const tt::engine::renderer::ColorRGBA textColor  (255, 238, 8, 255);
		
		renderer->getDebug()->renderText(loadText, loadTextPos.x + 1, loadTextPos.y + 1, shadowColor);
		renderer->getDebug()->renderText(loadText, loadTextPos.x,     loadTextPos.y,     textColor);
	}
#endif
	
	gfx.getFadeQuad()->render();
	
	if (m_graphicState != GraphicState_Loading)
	{
		// When app is faded in, render the indicator in front of the fade
		// (so that it stays visible when switching to the game load state, making the transition seamless)
		gfx.renderActivityIndicator(screen);
	}
	
	renderer->endHud();
}


void StateLoadApp::handleVBlankInterrupt()
{
}


tt::code::StateID StateLoadApp::getPathToState(tt::code::StateID p_targetState) const
{
	return p_targetState;
}


void StateLoadApp::onResetDevice()
{
	positionLoadingGraphics();
}


void StateLoadApp::onAppPaused()
{
#if USE_THREADED_LOADING
	//TT_Printf("StateLoadApp::onAppPaused: Suspending load thread.\n");
	//tt::thread::CriticalSection critSec(m_mutex);
	m_loadSuspended = true;
#endif
}


void StateLoadApp::onAppResumed()
{
#if USE_THREADED_LOADING
	//TT_Printf("StateLoadApp::onAppResumed: Resuming load thread.\n");
	//tt::thread::CriticalSection critSec(m_mutex);
	m_loadSuspended = false;
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void StateLoadApp::setupLoadStates()
{
	// Special case: if entering a state that does not require the complete game to be loaded
	// (such as the presentation viewer), add just a minimal set of load states, to keep load time down
	const bool performFullLoad = (m_postLoadState != statelist::StateID_PresentationViewer);
	
#if !defined(TT_BUILD_FINAL)
	if (AppGlobal::shouldCompileSquirrel())
	{
		m_loadStates.push_back(loadstate::LoadStateScriptLists::create());
		m_loadStates.push_back(loadstate::LoadStateAudioPlayer::create());
		m_loadStates.push_back(loadstate::LoadStateScriptMgr::create());
		
		calculateLoadSteps();
		return;
	}
#endif
	
	// Prepare all load steps here
	m_loadStates.clear();
	m_loadStates.push_back(loadstate::LoadStateInitSaveSystem::create());  // We want to get save ready asap.
	if (performFullLoad)
	{
		m_loadStates.push_back(loadstate::LoadStateScriptLists::create()); // Level list needed to validated shutdown level (next state)
		m_loadStates.push_back(loadstate::LoadStateGame::create());        // Decide which level we should start in. (Done early so we can show a level specific load screen.) FIXME: Should be renamed to LoadStateStartInfo.
		m_loadStates.push_back(loadstate::LoadStateSkinConfigs::create());
		if (AppGlobal::shouldDoPrecache())
		{
			m_loadStates.push_back(loadstate::LoadStateShoeboxPrecache::create());
			m_loadStates.push_back(loadstate::LoadStatePrecache::create());
		}
	}
	m_loadStates.push_back(loadstate::LoadStateAudioPlayer::create());
	if (performFullLoad)
	{
		m_loadStates.push_back(loadstate::LoadStateScriptMgr::create());
		m_loadStates.push_back(loadstate::LoadStateEntityLibrary::create());
		
#if !defined(TT_BUILD_FINAL)
		if (tt::app::getCmdLine().exists("generate_meta_data"))
		{
			m_loadStates.push_back(loadstate::LoadStateGenerateMetaData::create());
		}
#endif
	}
	
	calculateLoadSteps();
}


void StateLoadApp::calculateLoadSteps()
{
	// Calculate how many load steps to expect in total
	m_currentStep    = 0;
	m_totalLoadSteps = 0;
	for (LoadStates::iterator it = m_loadStates.begin(); it != m_loadStates.end(); ++it)
	{
		const s32 stepCount = (*it)->getEstimatedStepCount();
		//TT_Printf("StateLoadApp::setupLoadStates: State '%s' has %d steps.\n",
		//          (*it)->getName().c_str(), stepCount);
		m_totalLoadSteps += stepCount;
	}
}


void StateLoadApp::createLoadingGraphics()
{
	// Set up the loading presentations
	const std::string& dir(cfg()->getStringDirect("toki.loading.presentations"));
	tt::str::StringSet filenames(tt::fs::utils::getFilesInDir(dir, "*"));
	
	for (tt::str::StringSet::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
	{
		const std::string filename(dir + "/" + (*it));
		tt::pres::PresentationObjectPtr pres(m_presentationMgr->createPresentationObject(filename));
		if (pres != 0)
		{
			pres->setInScreenSpace(true);
			m_presentations.push_back(pres);
		}
	}
	
	// Set up the shared graphics
	using tt::engine::renderer::QuadSprite;
	SharedGraphics& gfx(AppGlobal::getSharedGraphics());
	gfx.getFadeQuad()->setFlag(QuadSprite::Flag_Visible);
	gfx.getFadeQuad()->setColor(tt::engine::renderer::ColorRGB::white);
	
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		gfx.getActivityIndicator(static_cast<Screen>(i))->resetFlag(QuadSprite::Flag_Visible);
	}
	
	alignLoadingGraphics();
	positionLoadingGraphics();
}


void StateLoadApp::destroyLoadingGraphics()
{
	m_presentations.clear();
}


void StateLoadApp::positionLoadingGraphics()
{
	tt::engine::renderer::Renderer* r = tt::engine::renderer::Renderer::getInstance();
	const real height = static_cast<real>(r->getScreenHeight());
	const real width = static_cast<real>(r->getScreenWidth());
	const real aspectRatio = width / height;
	
	for (Presentations::const_iterator it = m_presentations.begin(); it != m_presentations.end(); ++it)
	{
		(*it)->setScale(tt::math::Vector3(height, height, 1.0f));
		(*it)->addCustomPresentationValue("aspectRatio", aspectRatio);
		(*it)->setPosition(tt::math::Vector3(width * 0.5f, -height * 0.5f, 0.0f));
		(*it)->start(false);
	}
}


void StateLoadApp::startFadeOut()
{
#if !defined(TT_BUILD_FINAL)
	const u64 loadEnd      = tt::system::Time::getInstance()->getMilliSeconds();
	const u32 totalTime    = u32(loadEnd - m_loadStartTimestamp);
	const u32 totalMinutes = (totalTime / 60000);
	const u32 totalSeconds = (totalTime % 60000) / 1000;
	TT_Printf("StateLoadApp::startFadeOut: Application load completed in %u ms (%u minutes %u seconds).\n",
	          totalTime, totalMinutes, totalSeconds);
	
	AppGlobal::setLoadTimeApp(totalTime);
#endif
	
	AppGlobal::getSharedGraphics().getFadeQuad()->setColor(tt::engine::renderer::ColorRGB::black);
	AppGlobal::getSharedGraphics().getFadeQuad()->fadeIn(0.1f);
	m_graphicState = GraphicState_Fade;
}


void StateLoadApp::onLoadComplete()
{
	/* DEBUG: To control when to move from the load state to a new state, only switch when the Stomp button is pressed.
	if (AppGlobal::getInput().cur.stomp.pressed == false)
	{
		return;
	}
	// */
	
	TT_Printf("StateLoadApp::onLoadComplete: Loading complete; continuing to state '%s' (level '%s'; file: '%s').\n",
	          statelist::getStateName(m_postLoadState),
	          AppGlobal::getGameStartInfo().getLevelName().c_str(),
	          AppGlobal::getGameStartInfo().getLevelFilePath().c_str());
	
	changeState(m_postLoadState);
}


void StateLoadApp::alignLoadingGraphics()
{
	AppGlobal::getSharedGraphics().updateAll();
}


#if USE_THREADED_LOADING

int StateLoadApp::staticLoadThread(void* p_arg)
{
	return reinterpret_cast<StateLoadApp*>(p_arg)->loadThread();
}


int StateLoadApp::loadThread()
{
#if defined(TT_PLATFORM_WIN)
	// COM needs to be initialized on each thread it is used
	// We need it for Xact initialization
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	TT_ASSERT(SUCCEEDED(hr));
#endif
	
	// Make sure the game has had time to render at least one frame before starting load
	// (for LOT, we should initialize the save library after rendering at least one frame,
	//  so that any possible errors can be displayed properly)
	tt::thread::sleep(50);
	
	for ( ;; )
	{
		// Get a new state to load
		m_mutex->lock();
		
		if (m_loadSuspended)
		{
			// We're not allowed to do anything at this time: wait for a bit and try again
			m_mutex->unlock();
			tt::thread::sleep(50);
			continue;
		}
		
		if (m_stopLoading)
		{
			m_mutex->unlock();
			return 0;
		}
		
		if (m_loadStates.empty())
		{
			m_mutex->unlock();
			break;
		}
		
		loadstate::LoadStatePtr state(m_loadStates.front());
		//TT_Printf("StateLoadApp::loadThread: Step %d of %d (state '%s')...\n",
		//          m_currentStep, m_totalLoadSteps, state->getName().c_str());
		
		m_mutex->unlock();
		
		// Load
		state->doLoadStep();
		
		// Move on to the next step
		m_mutex->lock();
		
		++m_currentStep;
		if (state->isDone() && m_loadStates.empty() == false)
		{
			m_loadStates.pop_front();
		}
		
		m_mutex->unlock();
	}
	
	// Done loading: simply exit the loading thread. The main thread will take care of the rest.
	
#if defined(TT_PLATFORM_WIN)
	CoUninitialize();
#endif
	
	return 0;
}

#endif  // USE_THREADED_LOADING

// Namespace end
}
}
