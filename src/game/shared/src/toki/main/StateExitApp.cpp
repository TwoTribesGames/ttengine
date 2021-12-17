#include <tt/app/Application.h>
#include <tt/platform/tt_printf.h>

#include <toki/main/StateExitApp.h>
#include <toki/statelist/statelist.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {

//--------------------------------------------------------------------------------------------------
// Public member functions

StateExitApp::StateExitApp(tt::code::StateMachine* p_stateMachine)
:
tt::code::State(p_stateMachine)
{
}


StateExitApp::~StateExitApp()
{
}


void StateExitApp::enter()
{
	TT_Printf("StateExitApp::enter: Exiting application.\n");
	
#if !defined(TT_BUILD_FINAL)
	if (AppGlobal::shouldCompileSquirrel())
	{
		tt::app::getApplication()->terminate(true);
	}
#endif
}


void StateExitApp::exit()
{
}


void StateExitApp::update(real /*p_deltaTime*/)
{
}


void StateExitApp::render()
{
}


void StateExitApp::handleVBlankInterrupt()
{
}


tt::code::StateID StateExitApp::getPathToState(tt::code::StateID /*p_targetState*/) const
{
	// May never leave this state
	return statelist::StateID_ExitApp;
}

// Namespace end
}
}
