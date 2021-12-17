#if !defined(INC_TOKI_MAIN_STATEEXITAPP_H)
#define INC_TOKI_MAIN_STATEEXITAPP_H


#include <tt/code/State.h>


namespace toki {
namespace main {

class StateExitApp : public tt::code::State
{
public:
	StateExitApp(tt::code::StateMachine* p_stateMachine);
	virtual ~StateExitApp();
	
	virtual void enter();
	virtual void exit();
	virtual void update(real p_deltaTime);
	virtual void render();
	virtual void handleVBlankInterrupt();
	virtual tt::code::StateID getPathToState(tt::code::StateID p_targetState) const;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_MAIN_STATEEXITAPP_H)
