#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>

#include <tt/code/SimpleStateMachine.h>


namespace tt {
namespace code {



/**
 * Constructor
 *
 * @param p_initstate numeric state to start in
 */
SimpleStateMachine::SimpleStateMachine(s32 p_initstate)
:
m_last_change(tt::system::Time::getInstance()->getMilliSeconds()),
m_currentstate(p_initstate),
m_previous_state(p_initstate)
{
}


SimpleStateMachine::~SimpleStateMachine()
{
}


/**
 * Set internal numeric state
 *
 * @param p_state new state
 */
void SimpleStateMachine::setState(s32 p_state)
{
	using tt::system::Time;
	m_last_change    = Time::getInstance()->getMilliSeconds();
	m_previous_state = m_currentstate;
	m_currentstate   = p_state;
}


/**
 *
 * @return the current state number
 */
s32 SimpleStateMachine::getState()
{
	return m_currentstate;
}


/**
 * Predicate
 *
 * @param p_stateid state identifier to ask for
 *
 * @return true if the statemachine is in the state passed as parameter
 */
bool SimpleStateMachine::isInState(s32 p_stateid)
{
	return (m_currentstate == p_stateid);
}


/**
 *
 * @return number of milliseconds spent in the current state
 */
u64 SimpleStateMachine::getStateLife()
{
	using tt::system::Time;
	return (Time::getInstance()->getMilliSeconds() - m_last_change);
}


/**
 *
 * @return the previous state
 */
s32 SimpleStateMachine::getPreviousState()
{
	return m_previous_state;
}

// Namespace end
}
}

