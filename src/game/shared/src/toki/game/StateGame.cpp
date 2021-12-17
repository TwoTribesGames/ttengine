#include <tt/code/helpers.h>
#include <tt/code/StateMachine.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/Game.h>
#include <toki/game/StartInfo.h>
#include <toki/game/StateGame.h>
#include <toki/savedata/utils.h>
#include <toki/script/ScriptMgr.h>
#include <toki/serialization/utils.h>
#include <toki/statelist/statelist.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

StateGame::StateGame(tt::code::StateMachine* p_stateMachine)
:
tt::code::State(p_stateMachine),
m_game(0),
m_gameIsLoaded(false),
m_firstFrame(true)
{
}


StateGame::~StateGame()
{
	if (m_game != 0)
	{
		TT_PANIC("StateGame is being destroyed before StateGame::exit was called.");
		exit();
	}
}


void StateGame::enter()
{
	TT_Printf("StateGame::enter: Entering StateGame.\n");
	// NOTE: StateLoadGame will have already created and loaded Game, so just get the pointer here
	m_game         = AppGlobal::getGame();
	m_gameIsLoaded = true;
	m_firstFrame   = true;
	
	// During gameplay, a controller should be connected
	// FIXME CONTROLLER: Perhaps we should know at this point HOW MANY controllers should be connected based on number of players
	AppGlobal::getController(tt::input::ControllerIndex_One).setConnectionRequired(true);
	
	m_game->onLoadScreenComplete();
	
	// Fade the game load quad out (because game is now loaded)
	AppGlobal::getSharedGraphics().getFadeQuad()->fadeOut(0.25f);
}


void StateGame::exit()
{
	TT_Printf("StateGame::exit: Exiting StateGame.\n");
	m_game->onPreDestroy();
	AppGlobal::setGame(0);
	tt::code::helpers::safeDelete(m_game);
	m_gameIsLoaded = false;
}


void StateGame::update(real p_deltaTime)
{
	TT_NULL_ASSERT(m_game);
	m_game->update(p_deltaTime);
	
	if (m_game->shouldForceReload())
	{
		AppGlobal::getSharedGraphics().getFadeQuad()->setColor(tt::engine::renderer::ColorRGB::black);
		AppGlobal::getSharedGraphics().getFadeQuad()->setOpacity(255);
		changeState(statelist::StateID_LoadGame);
	}
}


void StateGame::updateForRender(real p_deltaTime)
{
	TT_NULL_ASSERT(m_game);
	m_game->updateForRender(p_deltaTime);
	AppGlobal::getSharedGraphics().updateFadeQuad();
	
	if (m_firstFrame)
	{
		// Must do this here to make sure GPU and CPU are synced and resources are not in use anymore
		// Destroy the loading background and logo: only the first level load should display these graphics
		AppGlobal::getSharedGraphics().destroyLoadingBgAndLogo();
		
		m_firstFrame = false;
	}
}


void StateGame::render()
{
	// NOTE: Explicitly checking for null pointer: on CAT, if a file system error occurs
	//       during level reloads, render() will still be called for the Error Viewer
	//       (even though there could be no Game instance anymore)
	TT_WARNING(m_game != 0, "Rendering StateGame without a Game instance!");
	if (m_game != 0 && m_gameIsLoaded)
	{
		m_game->render();
	}
	AppGlobal::getSharedGraphics().getFadeQuad()->render();
}


void StateGame::handleVBlankInterrupt()
{
}


tt::code::StateID StateGame::getPathToState(tt::code::StateID p_targetState) const
{
	return p_targetState;
}


bool StateGame::getPrerequisite(tt::code::StateID& p_prerequisite) const
{
	p_prerequisite = statelist::StateID_LoadGame;
	return true;
}


void StateGame::onResetDevice()
{
	if (m_game != 0)
	{
		m_game->onResetDevice();
	}
}


void StateGame::onRequestReloadAssets()
{
	if (m_game != 0)
	{
		m_game->onRequestReloadAssets();
	}
}


void StateGame::onAppActive()
{
	// Make sure no accidental inputs are registered in the game when the app gets its focus back. First all input needs to be released.
	input::Controller::State& inputState(AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	inputState.resetAllAndBlockUntilReleased();
}


void StateGame::onAppEnteredBackground()
{
	// Save the current game state when going into the background
	if (m_game != 0)
	{
		// Call generic onAppEnteredBackground to signal to script that the game enters background
		{
			tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
			vmPtr->callSqFun("onAppEnteredBackground");
		}
		serialization::savePersistentDataAndShutdownState(true);
	}
}


void StateGame::onAppLeftBackground()
{
	// Call generic onAppLeftBackground to signal to script that the game left background
	{
		tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
		vmPtr->callSqFun("onAppLeftBackground");
	}
}

// Namespace end
}
}
