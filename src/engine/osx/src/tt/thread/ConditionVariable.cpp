#include <pthread.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/ConditionVariable.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/Mutex_internaldata.h>


namespace tt {
namespace thread {

struct ConditionVariable::InternalData
{
	pthread_cond_t handle;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

ConditionVariable::ConditionVariable()
:
m_variable(new InternalData)
{
	pthread_cond_init(&(m_variable->handle), 0);
}


ConditionVariable::~ConditionVariable()
{
	wake(); // wake all threads waiting for this variable
	pthread_cond_destroy(&(m_variable->handle));
	delete m_variable;
}


void ConditionVariable::sleep(Mutex* p_mutex)
{
	TT_ASSERTMSG(p_mutex != 0, "No mutex specified!");
	
	pthread_cond_wait(&(m_variable->handle), &(p_mutex->getData()->handle));
}


void ConditionVariable::wake()
{
	pthread_cond_broadcast(&(m_variable->handle));
}

// Namespace end
}
}
