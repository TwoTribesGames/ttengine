#include <tt/platform/tt_error.h>
#include <tt/thread/BackgroundThread.h>
#include <tt/thread/thread.h>


namespace tt {
namespace thread {

// FIXME: This code is shared code; test it on other platforms and move to shared?

//--------------------------------------------------------------------------------------------------
// Public member functions

BackgroundThread::BackgroundThread(size_t p_stackSize)
:
m_thread(),
m_active(false),
m_idle(true),
m_arg(0),
m_ret(0),
m_stackSize(p_stackSize),
m_affinity(Affinity_None)
{
	TT_ASSERT(m_stackSize > 0);
}


BackgroundThread::~BackgroundThread()
{
	TT_ASSERTMSG(m_idle || m_thread == 0,
	             "Please stop BackgroundThread before destroying it! Forcing termination of thread!");
	
	if (m_thread != 0)
	{
		terminate(m_thread, 0);
		m_thread.reset();
	}
	
	// It is not possible to call stop from here,
	// because the virtual function table is already destroyed at this point
}


void BackgroundThread::startThread(void* p_arg, const char* p_threadName)
{
	TT_ASSERT(m_thread == 0);
	
	TT_ASSERTMSG(m_active == false, "Thread already running");
	
	m_active = true;
	m_idle   = false;
	m_arg    = p_arg;
	
	// Create the thread
	m_thread = create(runThread, this, false, m_stackSize, priority_normal, m_affinity, p_threadName);
}


int BackgroundThread::stopThread()
{
	TT_ASSERTMSG((m_thread != 0) == m_active,
		"Thread active flag (%d) mismatches thread status (%d)", (m_thread != 0), m_active);
	
	if (m_active == false)
	{
		// thread already terminated
		return m_ret;
	}
	
	stop();
	m_active = false;
	
	if (m_thread != 0)
	{
		wait(m_thread);
		m_thread.reset();
	}
	
	return m_ret;
}


void BackgroundThread::setAffinity(Affinity p_affinity)
{
	TT_ASSERTMSG(m_active == false, "Cannot change affinity of active thread yet");
	
	m_affinity = p_affinity;
}


Affinity BackgroundThread::getAffinity()
{
	return m_affinity;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

int BackgroundThread::runThread(void* p_arg)
{
	TT_NULL_ASSERT(p_arg);
	
	BackgroundThread* bgt = static_cast<BackgroundThread*>(p_arg);
	int ret = bgt->run(bgt->m_arg);
	bgt->m_active = false;
	bgt->m_ret = ret;
	
	bgt->m_thread.reset();
	
	return ret;
}


// Namespace end
}
}
