#include <SDL2/SDL_thread.h>

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
	m_mutex->handle = SDL_CreateMutex();
}


Mutex::~Mutex()
{
	SDL_DestroyMutex(m_mutex->handle);
	delete m_mutex;
}


void Mutex::lock()
{
	int result = SDL_LockMutex(m_mutex->handle);
	TT_ASSERTMSG(result == 0, "SDL_LockMutex failed! (returned: %d)", result);
}


bool Mutex::tryLock()
{
	return SDL_TryLockMutex(m_mutex->handle) == 0;
}


void Mutex::unlock()
{
	int result = SDL_UnlockMutex(m_mutex->handle);
	TT_ASSERTMSG(result == 0, "SDL_UnlockMutex failed! (returned: %d)", result);
}

// Namespace end
}
}
