#include <toki/game/StateGame.h>
#include <toki/game/StateLoadGame.h>
#include <toki/main/AppStateMachine.h>
#include <toki/main/StateExitApp.h>
#include <toki/main/StateLoadApp.h>
#include <toki/statelist/statelist.h>
#include <toki/viewer/StatePresentationViewer.h>


namespace toki {
namespace main {

//--------------------------------------------------------------------------------------------------
// Public member functions

AppStateMachine::AppStateMachine(tt::code::StateID p_startState)
{
	// Create all the application states
	setState(statelist::StateID_ExitApp, new StateExitApp(this));
	setState(statelist::StateID_LoadApp, new StateLoadApp(this));
	
	setState(statelist::StateID_LoadGame, new game::StateLoadGame(this));
	setState(statelist::StateID_Game,     new game::StateGame(this));
	
	setState(statelist::StateID_PresentationViewer, new viewer::StatePresentationViewer(this));
	
	// Set the initial state
	setInitialState(p_startState);
}


AppStateMachine::~AppStateMachine()
{
}

// Namespace end
}
}
