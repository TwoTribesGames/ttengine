#include <SDL2/SDL.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/ConditionVariable.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/Mutex_internaldata.h>


namespace tt {
namespace thread {

struct ConditionVariable::InternalData
{
	SDL_cond *handle;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

ConditionVariable::ConditionVariable()
:
m_variable(new InternalData)
{
	m_variable->handle = SDL_CreateCond();
}


ConditionVariable::~ConditionVariable()
{
	// wake all threads waiting for this variable
	SDL_CondBroadcast(m_variable->handle);
	SDL_DestroyCond(m_variable->handle);
	delete m_variable;
}


void ConditionVariable::sleep(Mutex* p_mutex)
{
	TT_ASSERTMSG(p_mutex != 0, "No mutex specified!");
	
	SDL_CondWait(m_variable->handle, p_mutex->getData()->handle);
}


void ConditionVariable::wake()
{
	SDL_CondSignal(m_variable->handle);
}

// Namespace end
}
}
