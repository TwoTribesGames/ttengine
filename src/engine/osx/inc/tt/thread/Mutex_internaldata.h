#ifndef INC_TT_THREAD_MUTEXINTERNALDATA_H
#define INC_TT_THREAD_MUTEXINTERNALDATA_H


#include <pthread.h>

#include <tt/thread/Mutex.h>


namespace tt {
namespace thread {

struct Mutex::InternalData
{
	pthread_mutex_t handle;
	pthread_mutexattr_t attr;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_THREAD_MUTEXINTERNALDATA_H)
