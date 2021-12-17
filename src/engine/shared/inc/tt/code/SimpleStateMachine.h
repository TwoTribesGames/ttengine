#ifndef INC_TT_CODE_SIMPLESTATEMACHINE_H
#define INC_TT_CODE_SIMPLESTATEMACHINE_H

#include <tt/platform/tt_types.h>
#include <tt/code/Uncopyable.h>


namespace tt {
namespace code {

/**
 * Very simplistic statemachine - good for letting small objects subclass
 */
class SimpleStateMachine : public Uncopyable
{
public:
	SimpleStateMachine(s32 p_initstate = 0);
	virtual ~SimpleStateMachine();

	virtual void setState(s32 p_newstate);
	virtual s32 getState();
	virtual bool isInState(s32 p_stateid);
	virtual u64 getStateLife();
	virtual s32 getPreviousState();

protected:
	/** Number of milliseconds since last state change */
	u64 m_last_change;

	/** The current numeric state */
	s32 m_currentstate;

	/** Previous state */
	s32 m_previous_state;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_SIMPLESTATEMACHINE_H)
