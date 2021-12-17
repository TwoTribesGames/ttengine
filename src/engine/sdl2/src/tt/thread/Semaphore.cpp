#include <tt/platform/tt_error.h>
#include <tt/str/str.h>
#include <tt/thread/Semaphore.h>

#include <SDL2/SDL.h>

namespace tt {
namespace thread {

// ------------------------------------------------------------------------------------------------


struct Semaphore::InternalData
{
	SDL_sem* sem;
	
	inline InternalData()
	:
	sem(0)
	{ }
};


Semaphore::Semaphore(s32 p_initialCount)
:
m_data(new InternalData)
{
	const s32 maxCount = 0x7FFFFFFF;
	TT_MINMAX_ASSERT(p_initialCount, 0, maxCount);

	const unsigned int initialValue = static_cast<unsigned int>(p_initialCount);

	m_data->sem = SDL_CreateSemaphore(initialValue);
}


Semaphore::~Semaphore()
{
	SDL_DestroySemaphore(m_data->sem);
	delete m_data;
}


void Semaphore::wait()
{
	int result = SDL_SemWait(m_data->sem);
	TT_ASSERTMSG(result == 0, "SDL_SemWait failed with error: '%s'",
				 SDL_GetError());
}


bool Semaphore::tryWait()
{
	int result = SDL_SemTryWait(m_data->sem);
	if (result == SDL_MUTEX_TIMEDOUT)
	{
		return false;
	}
	TT_ASSERTMSG(result == 0, "SDL_SemTryWait failed with error: '%s'",
				 SDL_GetError());
	return true;
}


void Semaphore::signal()
{
	int result = SDL_SemPost(m_data->sem);
	TT_ASSERTMSG(result == 0, "sem_post failed with error: '%s'",
				 SDL_GetError());
}


// Namespace end
}
}
