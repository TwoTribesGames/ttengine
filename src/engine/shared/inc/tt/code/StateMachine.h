#if !defined(INC_TT_CODE_STATEMACHINE_H)
#define INC_TT_CODE_STATEMACHINE_H


#include <tt/code/State.h>


namespace tt {
namespace code {

/*! \brief A finite state machine. */
class StateMachine
{
public:
	/*! \brief Update the current state. */
	void update(real p_deltaTime);
	
	/*! \brief Update the current state in prepration for rendering. */
	void updateForRender(real p_deltaTime);
	
	/*! \brief Render the current state. */
	void render();
	
	/*! \brief Switches to a new state (new state may not be immediately set;
	           a path may have to be followed, dictated by the states).
	    \param p_newState The state to switch to.
	    \param p_force    Whether to force the switch to this new state
	                      (disables preventing state change if already in the process of changing states). */
	void changeState(StateID p_newState, bool p_force = false);
	
	/*! \brief Exits and re-enters the current state. */
	void resetState();
	
	/*! \brief Returns the ID of the current state. */
	StateID getCurrentState() const;
	
	/*! \brief Returns the ID of the desired state. */
	inline StateID getDesiredState() const { return m_desiredState; }
	
	/*! \return Pointer to the State object that is currently active, or null if no state active. */
	inline const State* getCurrentStatePtr() const
	{ return m_currentState.isValid() ? m_states[m_currentState.getValue()] : 0; }
	
	/*! \return Pointer to the State object that is currently active, or null if no state active. */
	inline State* getCurrentStatePtr()
	{ return m_currentState.isValid() ? m_states[m_currentState.getValue()] : 0; }
	
protected:
	StateMachine();
	virtual ~StateMachine();
	
	/*! \brief Assigns a state pointer to a specific state ID. */
	void setState(StateID p_stateID, State* p_state);
	
	/*! \brief Sets the state to start in. Can only be called once. */
	void setInitialState(StateID p_stateID);
	
private:
	/*! \brief Exit the current state and change to a new state. */
	void changeCurrentState(StateID p_stateID);
	
	// No copying
	StateMachine(const StateMachine&);
	StateMachine& operator=(const StateMachine&);
	
	
	State** m_states;
	StateID m_currentState;
	StateID m_desiredState;
	bool    m_resetCurrentState;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_STATEMACHINE_H)
