#include <pthread.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/Mutex_internaldata.h>


namespace tt {
namespace thread {

//--------------------------------------------------------------------------------------------------
// Public member functions

Mutex::Mutex()
:
m_mutex(new InternalData)
{	
	int result = pthread_mutexattr_init(&(m_mutex->attr));
	TT_ASSERTMSG(result == 0, "pthread_mutexattr_init failed! (returned: %d)", result);
	
	result = pthread_mutexattr_settype(&(m_mutex->attr), PTHREAD_MUTEX_RECURSIVE);
	TT_ASSERTMSG(result == 0, "pthread_mutexattr_settype failed! (returned: %d)", result);
	
	result = pthread_mutex_init(&(m_mutex->handle), &(m_mutex->attr));
	TT_ASSERTMSG(result == 0, "pthread_mutex_init failed! (returned: %d)", result);
}


Mutex::~Mutex()
{
	int result = pthread_mutex_destroy(&(m_mutex->handle));
	TT_ASSERTMSG(result == 0, "pthread_mutex_destroy failed! (returned: %d)", result);
	result = pthread_mutexattr_destroy(&(m_mutex->attr));
	TT_ASSERTMSG(result == 0, "pthread_mutexattr_destroy failed! (returned: %d)", result);
	delete m_mutex;
}


void Mutex::lock()
{
	int result = pthread_mutex_lock(&(m_mutex->handle));
	TT_ASSERTMSG(result == 0, "pthread_mutex_lock failed! (returned: %d)", result);
}


bool Mutex::tryLock()
{
	return pthread_mutex_trylock(&(m_mutex->handle)) == 0;
}


void Mutex::unlock()
{
	int result = pthread_mutex_unlock(&(m_mutex->handle));
	TT_ASSERTMSG(result == 0, "pthread_mutex_unlock failed! (returned: %d)", result);
}

// Namespace end
}
}
