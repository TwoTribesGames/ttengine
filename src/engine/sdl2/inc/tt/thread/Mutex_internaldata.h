#ifndef INC_TT_THREAD_MUTEXINTERNALDATA_H
#define INC_TT_THREAD_MUTEXINTERNALDATA_H


#include <SDL2/SDL_thread.h>

#include <tt/thread/Mutex.h>


namespace tt {
namespace thread {

struct Mutex::InternalData
{
	SDL_mutex *handle;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_THREAD_MUTEXINTERNALDATA_H)
