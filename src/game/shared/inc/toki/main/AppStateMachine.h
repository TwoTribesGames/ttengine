#if !defined(INC_TOKI_MAIN_APPSTATEMACHINE_H)
#define INC_TOKI_MAIN_APPSTATEMACHINE_H


#include <tt/code/StateMachine.h>


namespace toki {
namespace main {

/*! \brief The main application state machine. */
class AppStateMachine : public tt::code::StateMachine
{
public:
	AppStateMachine(tt::code::StateID p_startState);
	virtual ~AppStateMachine();
	
	//
	
private:
	//
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_MAIN_APPSTATEMACHINE_H)
