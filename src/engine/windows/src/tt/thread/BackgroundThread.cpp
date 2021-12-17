#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <tt/thread/BackgroundThread.h>


namespace tt {
namespace thread {

BackgroundThread::BackgroundThread(size_t p_stacksize)
:
m_active(false),
m_idle(true)
{
	TT_ASSERT(p_stacksize > 0);
	
	m_stacksize = p_stacksize;
}


BackgroundThread::~BackgroundThread()
{
	DWORD exitcode;
	if ( m_idle == false )
	{
		if ( GetExitCodeThread(m_thread, &exitcode) == TRUE )
		{
			TT_ASSERTMSG(exitcode != STILL_ACTIVE,
			             "Please stop BackgroundThread before destroying it!");
		}
	}
	
	// It is not possible to call stop from here,
	// because the virtual function table is already destroyed at this point
}


void BackgroundThread::startThread(void* p_arg)
{
	TT_ASSERTMSG(m_active == false,
	             "Thread already running");
	m_active = true;
	m_idle   = false;
	m_arg = p_arg;
	
	m_thread = CreateThread(0, m_stacksize, runThread, this, CREATE_SUSPENDED, 0);
	TT_ASSERTMSG(m_thread != 0, "Failed to create thread");
	SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);
	ResumeThread(m_thread);
}


int BackgroundThread::stopThread()
{
	stop();
	
	if ( m_active == false )
	{
		// thread already terminated
		return m_ret;
	}
	
	if ( WaitForSingleObject(m_thread, 10000) == WAIT_TIMEOUT )
	{
		TT_PANIC("Timeout");
	}
	m_active = false;
	
	return m_ret;
}


DWORD WINAPI BackgroundThread::runThread(void* arg)
{
	BackgroundThread* bgt = static_cast<BackgroundThread*>(arg);
	bgt->m_ret = bgt->run(bgt->m_arg);
	return 0;
}

// Namespace end
}
}


