#include <tt/code/State.h>
#include <tt/code/StateMachine.h>


namespace tt {
namespace code {

//------------------------------------------------------------------------------
// Public member functions

StateID State::getPathToState(StateID p_targetState) const
{
	return p_targetState;
}


bool State::getPrerequisite(StateID& /* p_prerequisite */) const
{
	return false;
}


//------------------------------------------------------------------------------
// Protected member functions

void State::changeState(StateID p_desiredState)
{
	m_stateMachine->changeState(p_desiredState);
}


StateMachine& State::getStateMachine()
{
	return *m_stateMachine;
}


const StateMachine& State::getStateMachine() const
{
	return *m_stateMachine;
}

// Namespace end
}
}
