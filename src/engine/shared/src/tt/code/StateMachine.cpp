#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/code/StateMachine.h>

namespace tt {
namespace code {

//------------------------------------------------------------------------------
// Public member functions

void StateMachine::update(real p_deltaTime)
{
	// Get the current state pointer
	TT_ASSERTMSG(m_currentState.isValid(), "Current state is invalid.");
	State* curState = m_states[m_currentState.getValue()];
	
	// Update the state
	TT_ASSERTMSG(curState != 0,
	             "Current state (%d) does not have a valid pointer.",
	             m_currentState.getValue());
	
	// Check if a state change is requested
	if (m_resetCurrentState)
	{
		// Exit and enter the current state
		m_resetCurrentState = false;
		changeCurrentState(m_currentState);
	}
	else if (m_desiredState.isValid() && m_desiredState != m_currentState)
	{
		// Get the next state in the path to the desired state
		StateID nextState(curState->getPathToState(m_desiredState));
		if (nextState != m_currentState)
		{
			// Check if another state must first be visited
			StateID prereq(StateID::invalid);
			if (m_states[nextState.getValue()]->getPrerequisite(prereq))
			{
				// Only switch to the prerequisite if that
				// isn't the state we're coming from
				if (prereq != m_currentState)
				{
					TT_Printf("StateMachine::update: Next state %d has state %d "
					          "as prerequisite.\n",
					          nextState.getValue(), prereq.getValue());
					nextState = prereq;
				}
			}
			
			changeCurrentState(nextState);
		}
	}
	
	// Get the current state pointer
	TT_ASSERTMSG(m_currentState.isValid(), "Current state is invalid.");
	curState = m_states[m_currentState.getValue()];
	
	// Update the state
	TT_ASSERTMSG(curState != 0,
	             "Current state (%d) does not have a valid pointer.",
	             m_currentState.getValue());
	
	curState->update(p_deltaTime);
}


void StateMachine::updateForRender(real p_deltaTime)
{
	TT_ASSERTMSG(m_currentState.isValid(), "Current state is invalid.");
	TT_ASSERTMSG(m_states[m_currentState.getValue()] != 0,
	             "Current state (%d) does not have a valid pointer.",
	             m_currentState.getValue());
	m_states[m_currentState.getValue()]->updateForRender(p_deltaTime);
}


void StateMachine::render()
{
	// Render the state
	TT_ASSERTMSG(m_currentState.isValid(), "Current state is invalid.");
	TT_ASSERTMSG(m_states[m_currentState.getValue()] != 0,
	             "Current state (%d) does not have a valid pointer.",
	             m_currentState.getValue());
	m_states[m_currentState.getValue()]->render();
}


void StateMachine::changeState(StateID p_newState, bool p_force)
{
	if (p_newState.isValid() == false)
	{
		TT_PANIC("Invalid state ID specified.");
		return;
	}
	
	if (m_currentState.isValid() == false)
	{
		TT_PANIC("No initial state was set. Cannot change to state ID %d.", p_newState.getValue());
		return;
	}
	
	if (p_force == false                 &&
	    m_desiredState.isValid()         &&
	    m_currentState != m_desiredState &&
	    p_newState != m_desiredState)
	{
		TT_PANIC("Cannot change states when already working on a state change. "
		         "Currently changing from state ID %d to %d. "
		         "Requesting change to state ID %d.",
		         m_currentState.getValue(), m_desiredState.getValue(),
		         p_newState.getValue());
		return;
	}
	
	m_desiredState = p_newState;
}


void StateMachine::resetState()
{
	m_resetCurrentState = true;
}


StateID StateMachine::getCurrentState() const
{
	return m_currentState;
}


//------------------------------------------------------------------------------
// Protected member functions

StateMachine::StateMachine()
:
m_states(0),
m_currentState(StateID::invalid),
m_desiredState(StateID::invalid),
m_resetCurrentState(false)
{
	// Lock state ID registration
	StateID::lock();
	
	// Create storage for the states
	m_states = new State*[StateID::getCount()];
	for (int i = 0; i < StateID::getCount(); ++i)
	{
		m_states[i] = 0;
	}
}


StateMachine::~StateMachine()
{
	// Allow the current state to exit
	if (m_currentState.isValid() && m_states[m_currentState.getValue()] != 0)
	{
		m_states[m_currentState.getValue()]->exit();
	}
	
	// Clean up all the states
	for (int i = 0; i < StateID::getCount(); ++i)
	{
		delete m_states[i];
	}
	delete[] m_states;
}


void StateMachine::setState(StateID p_stateID, State* p_state)
{
	if (p_stateID.isValid() == false)
	{
		TT_PANIC("Invalid state ID specified.");
		return;
	}
	
	if (p_state == 0)
	{
		TT_PANIC("Invalid state pointer specified.");
		return;
	}
	
	if (m_states[p_stateID.getValue()] != 0)
	{
		TT_PANIC("A state for state ID %d was already specified (0x%08X).",
		         p_stateID.getValue(), p_state);
		return;
	}
	
	m_states[p_stateID.getValue()] = p_state;
}


void StateMachine::setInitialState(StateID p_stateID)
{
	// Can only set initial state once
	if (m_currentState.isValid())
	{
		TT_PANIC("Cannot set initial state, because one "
		         "was already set (ID %d).", p_stateID.getValue());
		return;
	}
	
	// Initial state ID must be valid
	if (p_stateID.isValid() == false)
	{
		TT_PANIC("Initial state ID is not valid.");
		return;
	}
	
	// Make sure all state pointers have been set
	for (int i = 0; i < StateID::getCount(); ++i)
	{
		if (m_states[i] == 0)
		{
			TT_PANIC("Cannot set initial state, because not all of the %d state "
			         "pointers are valid (ID %d is invalid).", StateID::getCount(), i);
			return;
		}
	}
	
	// Check if the specified state has prerequisites
	StateID prereq(StateID::invalid);
	StateID target(p_stateID);
	while (m_states[target.getValue()]->getPrerequisite(prereq))
	{
		if (prereq.isValid() == false)
		{
			TT_PANIC("Invalid prerequisite state specified.");
			break;
		}
		target = prereq;
	}
	
	// Set the initial state and enter it
	m_currentState = target;
	m_desiredState = p_stateID;
	m_states[m_currentState.getValue()]->enter();
	
	// Update the state for the first time
	m_states[m_currentState.getValue()]->update(1 / 30.0f);
}


//------------------------------------------------------------------------------
// Private member functions

void StateMachine::changeCurrentState(StateID p_state)
{
	// Exit the current state
	m_states[m_currentState.getValue()]->exit();
	
	// Change the current state
	m_currentState = p_state;
	
	// Enter the new state
	if (m_states[m_currentState.getValue()] == 0)
	{
		TT_PANIC("New state pointer is invalid (state %d).",
		         m_currentState.getValue());
		return;
	}
	m_states[m_currentState.getValue()]->enter();
}

// Namespace end
}
}
