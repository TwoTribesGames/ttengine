#if !defined(INC_TT_CODE_STATE_H)
#define INC_TT_CODE_STATE_H


#include <tt/app/PlatformCallbackInterface.h>
#include <tt/code/StateID.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

class StateMachine;


/*! \brief Abstract class for a state in a finite state machine. */
class State : public app::PlatformCallbackInterface
{
public:
	virtual ~State() { }
	
	/*! \brief Called when entering the state. */
	virtual void enter() = 0;
	
	/*! \brief Called when exiting the state. */
	virtual void exit() = 0;
	
	/*! \brief Allows the state to perform update logic. */
	virtual void update(real p_deltaTime) = 0;
	
	/*! \brief Allows the state to prepair for render. */
	virtual void updateForRender(real p_deltaTime) { (void) p_deltaTime; };
	
	/*! \brief Allows the state to perform render logic. */
	virtual void render() = 0;
	
	/*! \brief Allows the state to respond to vblank interrupts. */
	virtual void handleVBlankInterrupt() = 0;
	
	/*! \brief Allows states to specify other states that should
	           be visited to get to a specific target state. */
	virtual StateID getPathToState(StateID p_targetState) const;
	
	/*! \brief Allows states to specify another state that should
	           be visited before this state can be entered.
	    \return False if no other state is required, true if there is. */
	virtual bool getPrerequisite(StateID& p_prerequisite) const;
	
protected:
	explicit inline State(StateMachine* p_stateMachine)
	:
	m_stateMachine(p_stateMachine) { }
	
	void changeState(StateID p_desiredState);
	StateMachine& getStateMachine();
	const StateMachine& getStateMachine() const;
	
private:
	// No copying
	State(const State&);
	State& operator=(const State&);
	
	
	StateMachine* m_stateMachine;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_STATE_H)
