#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/thread/BackgroundThread.h>


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
	
	// Set up attributes for the thread
	pthread_attr_t threadAttribs;
	pthread_attr_init(&threadAttribs);
	pthread_attr_setstacksize(&threadAttribs, m_stacksize);
	
	sched_param scheduleParams = { 0 };
	pthread_attr_getschedparam(&threadAttribs, &scheduleParams);
	scheduleParams.sched_priority -= 5;
	pthread_attr_setschedparam(&threadAttribs, &scheduleParams);
	
	// Create a new thread
	int result = pthread_create(&m_thread, &threadAttribs, runThread, this);
	if (result != 0)
	{
		TT_PANIC("Creating POSIX thread failed with error %d: %s", result, strerror(result));
	}
	
	pthread_attr_destroy(&threadAttribs);
}


int BackgroundThread::stopThread()
{
	stop();
	
	if (m_active == false)
	{
		// Thread already terminated
		return m_ret;
	}
	
	pthread_join(m_thread, 0);
	m_active = false;
	
	return m_ret;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void* BackgroundThread::runThread(void* p_arg)
{
	BackgroundThread* bgt = static_cast<BackgroundThread*>(p_arg);
	int ret = bgt->run(bgt->m_arg);
	bgt->m_active = false;
	bgt->m_ret    = ret;
	
	return 0;
}

// Namespace end
}
}


