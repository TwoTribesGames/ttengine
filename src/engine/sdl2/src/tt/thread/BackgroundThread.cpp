#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/thread/BackgroundThread.h>
#include <cstring>

namespace tt {
namespace thread {

//--------------------------------------------------------------------------------------------------
// Public member functions

BackgroundThread::BackgroundThread(size_t p_stacksize)
:
m_thread(),
m_active(false),
m_idle(true),
m_arg(0),
m_ret(0),
m_stacksize(p_stacksize),
m_stack(0)
{
	TT_ASSERT(p_stacksize > 0);
}


BackgroundThread::~BackgroundThread()
{
	if (m_idle == false)
	{
		TT_ASSERTMSG(m_active == false,
		             "Please stop BackgroundThread before destroying it!");
	}
	
	// It is not possible to call stop from here,
	// because the virtual function table is already destroyed at this point
}


void BackgroundThread::startThread(void* p_arg)
{
	TT_ASSERTMSG(m_active == false, "Thread already running");
	m_active = true;
	m_idle   = false;
	m_arg    = p_arg;
	
	m_thread = SDL_CreateThread(runThread, "Background Thread", (void*)this);
	if (m_thread == 0)
	{
		TT_PANIC("Creating SDL2 thread failed with error: %s", SDL_GetError());
	}
}


int BackgroundThread::stopThread()
{
	stop();
	
	if (m_active == false)
	{
		// Thread already terminated
		return m_ret;
	}
	
	SDL_WaitThread(m_thread, NULL);
	m_active = false;
	
	return m_ret;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

int BackgroundThread::runThread(void* p_arg)
{
	BackgroundThread* bgt = static_cast<BackgroundThread*>(p_arg);
	int ret = bgt->run(bgt->m_arg);
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
	bgt->m_active = false;
	bgt->m_ret    = ret;
	
	return 0;
}

// Namespace end
}
}


