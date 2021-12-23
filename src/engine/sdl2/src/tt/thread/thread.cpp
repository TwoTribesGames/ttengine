#include <SDL2/SDL.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/thread.h>
#include <cstring>

namespace tt {
namespace thread {

// Priority types (dummy for Linux: not yet supported)
const priority_type priority_idle         = SDL_THREAD_PRIORITY_LOW;
const priority_type priority_lowest       = SDL_THREAD_PRIORITY_LOW;
const priority_type priority_below_normal = SDL_THREAD_PRIORITY_LOW;
const priority_type priority_normal       = SDL_THREAD_PRIORITY_NORMAL;
const priority_type priority_above_normal = SDL_THREAD_PRIORITY_NORMAL;
const priority_type priority_highest      = SDL_THREAD_PRIORITY_HIGH;
const priority_type priority_realtime     = SDL_THREAD_PRIORITY_HIGH;


struct Thread
{
	SDL_Thread *handle;
	SDL_threadID threadID;
	
	
	explicit inline Thread(SDL_Thread* p_handle)
	:
	handle(p_handle),
	threadID(SDL_GetThreadID(p_handle))
	{ }
	
	inline ~Thread()
	{
		if (threadID != SDL_ThreadID())
		{
			/*
			DWORD exitcode = 0;
			GetExitCodeThread(handle, &exitcode);
			TT_WARNING(exitcode != STILL_ACTIVE,
			           "Deleting thread object (id %s) while thread still active.", id);
			CloseHandle(handle);
			*/
		}
	}
	
private:
	// No copying
	Thread(const Thread&);
	Thread& operator=(const Thread&);
};

struct ThreadAttribs {
	ThreadProc		proc;
	priority_type	priority;
	void*			param;
	
	ThreadAttribs(ThreadProc _proc, priority_type	_priority, void* _param)
	:
		proc(_proc),
		priority(_priority),
		param(_param)
	{}
};

static void destroyThread(Thread* p_thread)
{
	delete p_thread;
}

int sdl2_initThread(void* data)
{
	ThreadAttribs *attribs = static_cast<ThreadAttribs*>(data);

	SDL_SetThreadPriority(static_cast<SDL_ThreadPriority>(attribs->priority));
	
	return attribs->proc(attribs->param);
}

s32 getProcessorCount()
{
    return SDL_GetCPUCount();
}

handle create(ThreadProc    p_proc,
              void*         p_param,
              bool          p_suspended,
              size_type     p_stackSize,
              priority_type p_priority,
              Affinity      p_affinity,
              const char*   p_name)
{
	(void) p_affinity;
	
	TT_WARNING(p_suspended == false, "Thread suspension is not supported in SDL2.");
	
	
	
	// Set up attributes for the thread
	ThreadAttribs *attrib = new ThreadAttribs(p_proc, p_priority, p_param);
	
	if (p_name == 0) {
		p_name = "Thread";
	}
	SDL_Thread *threadHandle = SDL_CreateThread(sdl2_initThread, p_name, attrib);

	if (threadHandle == 0) {
		TT_PANIC("Creating SDL2 thread failed with error: %s", SDL_GetError());
		delete attrib;

		return handle();
	}

	return handle(new Thread(threadHandle), destroyThread);
}


void resume(const handle& /*p_thread*/)
{
	TT_WARN("Thread suspension is not supported in SDL2.");
}


void suspend(const handle& /*p_thread*/)
{
	TT_WARN("Thread suspension is not supported in SDL2.");
}


void sleep(int p_milliseconds)
{
	TT_ASSERT(p_milliseconds >= 0);
	SDL_Delay(p_milliseconds);
}


void yield()
{
	SDL_Delay(0);
}


void terminate(const handle& p_thread, int p_exitCode)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->threadID != SDL_threadID());
	
	(void)p_exitCode;
	SDL_DetachThread(p_thread->handle);
}


void exit(int p_exitCode)
{
	//TT_ASSERT(p_exitCode != STILL_ACTIVE);
	(void)p_exitCode;
}

priority_type getPriority(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	return priority_normal;
}


void setPriority(const handle& p_thread, priority_type p_priority)
{
	TT_ASSERT(p_thread != 0);
}


void wait(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->threadID != SDL_threadID());
	
	SDL_WaitThread(p_thread->handle, NULL);
}


bool hasEnded(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_WARN("Retrieving a thread's running status is not supported on SDL2.");
	(void)p_thread;
	return true;
}


void setName(const handle& p_thread, const char* /*p_name*/)
{
	TT_NULL_ASSERT(p_thread);
	// FIXME: Not implemented for SDL2
}


// Namespace end
}
}
