#import <Foundation/NSAutoreleasePool.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <pthread.h>
#include <time.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/thread.h>


namespace tt {
namespace thread {

// Priority types (dummy for OS X: not yet supported)
const priority_type priority_idle         = 0;
const priority_type priority_lowest       = 0;
const priority_type priority_below_normal = 0;
const priority_type priority_normal       = 0;
const priority_type priority_above_normal = 0;
const priority_type priority_highest      = 0;
const priority_type priority_realtime     = 0;


struct Thread
{
	pthread_t handle;
	
	
	explicit inline Thread(pthread_t p_handle)
	:
	handle(p_handle)
	{ }
	
	inline ~Thread()
	{
		if (handle != pthread_self())
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

static void destroyThread(Thread* p_thread)
{
	delete p_thread;
}


// Helper struct and function to wrap new threads in NSAutoreleasePool
// so that Objective C usage won't leak
struct WrapperData
{
	ThreadProc proc;
	void*      param;
	
	inline WrapperData(ThreadProc p_proc, void* p_param)
	:
	proc (p_proc),
	param(p_param)
	{ }
};

static void* threadWrapper(void* p_param)
{
	TT_NULL_ASSERT(p_param);
	WrapperData* data = reinterpret_cast<WrapperData*>(p_param);
	if (data == 0)
	{
		return 0;
	}
	
	// Copy the wrapper data and clean up wrapper memory before forwarding to actual thread function
	ThreadProc proc  = data->proc;
	void*      param = data->param;
	delete data;
	
	// Wrap the actual thread function in an NSAutoreleasePool
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	const int retval = proc(param);
	
	[pool release];
	
	return reinterpret_cast<void*>(retval);
}


s32 getProcessorCount()
{
	int    mib[2]   = { CTL_HW, HW_NCPU };
	int    cpuCount = 0;
	size_t len      = sizeof(cpuCount);
	
	if (sysctl(mib, 2, &cpuCount, &len, 0, 0) != 0)
	{
		// Getting system info failed: assume a single CPU
		return 1;
	}
	
	return static_cast<s32>(cpuCount);
}


handle create(ThreadProc    p_proc,
              void*         p_param,
              bool          p_suspended,
              size_type     p_stackSize,
              priority_type p_priority,
              Affinity      p_affinity,
              const char*   p_name)
{
	(void)p_affinity;
	
	TT_WARNING(p_suspended == false, "Thread suspension is not supported in POSIX.");
	
	// Set up attributes for the thread
	pthread_attr_t threadAttribs;
	pthread_attr_init(&threadAttribs);
	if (p_stackSize > 0)
	{
		pthread_attr_setstacksize(&threadAttribs, p_stackSize);
	}
	
	/*
	sched_param scheduleParams = { 0 };
	pthread_attr_getschedparam(&threadAttribs, &scheduleParams);
	scheduleParams.sched_priority = p_priority;
	pthread_attr_setschedparam(&threadAttribs, &scheduleParams);
	//*/ (void)p_priority;
	
	// Create a new thread
	typedef void* (*PosixThreadProc)(void*);
	pthread_t threadHandle;
	int result = pthread_create(&threadHandle, &threadAttribs,
	                            threadWrapper, new WrapperData(p_proc, p_param));
	if (result != 0)
	{
		TT_PANIC("Creating POSIX thread failed with error %d: %s", result, strerror(result));
		return handle();
	}
	
	if (p_name != 0)
	{
		// Not implemented for OSX
		//pthread_setname_np(threadHandle, p_name);
	}
	
	pthread_attr_destroy(&threadAttribs);
	
	return handle(new Thread(threadHandle), destroyThread);
}


void resume(const handle& /*p_thread*/)
{
	/*
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != GetCurrentThread());
	TT_ASSERT(p_thread->id     != GetCurrentThreadId());
	
	ResumeThread(p_thread->handle);
	*/
	TT_WARN("Thread suspension is not supported in POSIX.");
}


void suspend(const handle& /*p_thread*/)
{
	/*
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != pthread_self());
	
	SuspendThread(p_thread->handle);
	*/
	TT_WARN("Thread suspension is not supported in POSIX.");
}


void sleep(int p_milliseconds)
{
	TT_ASSERT(p_milliseconds >= 0);
	timespec ts = { 0 };
	
	ts.tv_sec  = static_cast<time_t>(p_milliseconds / 1000);
	p_milliseconds -= ts.tv_sec * 1000;
	ts.tv_nsec = static_cast<long>(p_milliseconds * 1000000L);
	
	nanosleep(&ts, 0);
}


void yield()
{
	sched_yield();
}


void terminate(const handle& p_thread, int p_exitCode)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != pthread_self());
	
	(void)p_exitCode;
	pthread_cancel(p_thread->handle);
}


void exit(int p_exitCode)
{
	//TT_ASSERT(p_exitCode != STILL_ACTIVE);
	intptr exitCode = static_cast<intptr>(p_exitCode);
	pthread_exit(reinterpret_cast<void*>(exitCode));
}


handle getCurrent()
{
	return handle(new Thread(pthread_self()), destroyThread);
}


priority_type getPriority(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	
	int schedPolicy = 0;
	sched_param schedParam = { 0 };
	pthread_getschedparam(p_thread->handle, &schedPolicy, &schedParam);
	return schedParam.sched_priority;
}


void setPriority(const handle& p_thread, priority_type p_priority)
{
	TT_ASSERT(p_thread != 0);
	
	int schedPolicy = 0;
	sched_param schedParam = { 0 };
	pthread_getschedparam(p_thread->handle, &schedPolicy, &schedParam);
	schedParam.sched_priority = p_priority;
	pthread_setschedparam(p_thread->handle, schedPolicy, &schedParam);
}


void wait(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != pthread_self());
	
	pthread_join(p_thread->handle, 0);
}


bool hasEnded(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_WARN("Retrieving a thread's running status is not supported on POSIX.");
	(void)p_thread;
	return true;
}


void setName(const handle& p_thread, const char* /*p_name*/)
{
	TT_NULL_ASSERT(p_thread);
	// FIXME: Not implemented for OS X
}

// Namespace end
}
}
