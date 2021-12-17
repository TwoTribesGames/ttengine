#define NOMINMAX
#include <windows.h>

#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/thread/thread.h>


namespace tt {
namespace thread {

// Priority types
const priority_type priority_idle         = THREAD_PRIORITY_IDLE;
const priority_type priority_lowest       = THREAD_PRIORITY_LOWEST;
const priority_type priority_below_normal = THREAD_PRIORITY_BELOW_NORMAL;
const priority_type priority_normal       = THREAD_PRIORITY_NORMAL;
const priority_type priority_above_normal = THREAD_PRIORITY_ABOVE_NORMAL;
const priority_type priority_highest      = THREAD_PRIORITY_HIGHEST;
const priority_type priority_realtime     = THREAD_PRIORITY_TIME_CRITICAL;

struct Thread
{
	HANDLE handle;
	DWORD  id;
	
	Thread(HANDLE p_handle, DWORD p_id)
	:
	handle(p_handle),
	id(p_id)
	{}
	
	~Thread()
	{
		if (handle != GetCurrentThread())
		{
			DWORD exitcode = 0;
			GetExitCodeThread(handle, &exitcode);
			TT_WARNING(exitcode != STILL_ACTIVE,
			           "Deleting thread object (id %d) while thread still active.", id);
			CloseHandle(handle);
		}
	}
};


static void destroyThread(Thread* p_thread)
{
	delete p_thread;
}


static void setThreadName(DWORD p_threadID, LPCSTR p_name)
{
#if !defined(TT_BUILD_FINAL)
	// This code only works if the application is running from a debugger
	if (IsDebuggerPresent() == FALSE)
	{
		return;
	}
	
	// This scary code is copied from http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
	static const DWORD MS_VC_EXCEPTION = 0x406D1388;
	
#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD  dwType;     // Must be 0x1000.
		LPCSTR szName;     // Pointer to name (in user addr space).
		DWORD  dwThreadID; // Thread ID (-1 = caller thread).
		DWORD  dwFlags;    // Reserved for future use, must be zero.
	};
#pragma pack(pop)
	
	THREADNAME_INFO info;
	info.dwType     = 0x1000;
	info.szName     = p_name;
	info.dwThreadID = p_threadID;
	info.dwFlags    = 0;
	
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#else
	(void)p_threadID;
	(void)p_name;
#endif
}


s32 getProcessorCount()
{
	SYSTEM_INFO sysinfo = { 0 };
	GetSystemInfo(&sysinfo);
	return static_cast<s32>(sysinfo.dwNumberOfProcessors);
}


handle create(ThreadProc    p_proc,
              void*         p_param,
              bool          p_suspended,
              size_type     p_stackSize,
              priority_type p_priority,
              Affinity      p_affinity,
              const char*   p_name)
{
	DWORD threadID = 0;
	HANDLE threadhandle = CreateThread(0, static_cast<SIZE_T>(p_stackSize),
		reinterpret_cast<LPTHREAD_START_ROUTINE>(p_proc),
		p_param,
		CREATE_SUSPENDED,
		&threadID);
	
	if (threadhandle == 0)
	{
		return handle();
	}
	
	if (p_affinity != Affinity_None)
	{
		// Set a preferred processor based on affinity (convert from mask to integer)
		DWORD idealProcessor = tt::math::findPowerOfTwo(static_cast<u32>(p_affinity));
		DWORD prevIdealProcessor = SetThreadIdealProcessor(threadhandle, idealProcessor);
		if (prevIdealProcessor == -1)
		{
			TT_WARN("Failed to set ideal processor for thread '%s'", p_name);
		}
		else
		{
			TT_Printf("Successfully set ideal processor for thread '%s' from %d to %d\n",
			          p_name, prevIdealProcessor, idealProcessor);
		}
	}
	
	SetThreadPriority(threadhandle, p_priority);
	
	if (p_name != 0)
	{
		setThreadName(threadID, p_name);
	}
	
	if (p_suspended == false)
	{
		ResumeThread(threadhandle);
	}
	return handle(new Thread(threadhandle, threadID), destroyThread);
}


void resume(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != GetCurrentThread());
	TT_ASSERT(p_thread->id     != GetCurrentThreadId());
	
	ResumeThread(p_thread->handle);
}


void suspend(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != GetCurrentThread());
	TT_ASSERT(p_thread->id     != GetCurrentThreadId());
	
	SuspendThread(p_thread->handle);
}


void sleep(int p_milliseconds)
{
	TT_ASSERT(p_milliseconds >= 0);
	Sleep(static_cast<DWORD>(p_milliseconds));
}


void yield()
{
	SwitchToThread();
}


void terminate(const handle& p_thread, int p_exitCode)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != GetCurrentThread());
	TT_ASSERT(p_thread->id     != GetCurrentThreadId());
	TT_ASSERT(p_exitCode != STILL_ACTIVE);
	
	TerminateThread(p_thread->handle, static_cast<DWORD>(p_exitCode));
}


void exit(int p_exitCode)
{
	TT_ASSERT(p_exitCode != STILL_ACTIVE);
	
	ExitThread(static_cast<DWORD>(p_exitCode));
}


handle getCurrent()
{
	return handle(new Thread(GetCurrentThread(), GetCurrentThreadId()), destroyThread);
}


priority_type getPriority(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	
	return GetThreadPriority(p_thread->handle);
}


void setPriority(const handle& p_thread, priority_type p_priority)
{
	TT_ASSERT(p_thread != 0);
	
	SetThreadPriority(p_thread->handle, p_priority);
}


void wait(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	TT_ASSERT(p_thread->handle != GetCurrentThread());
	TT_ASSERT(p_thread->id     != GetCurrentThreadId());
	
	WaitForSingleObject(p_thread->handle, INFINITE);
}


bool hasEnded(const handle& p_thread)
{
	TT_ASSERT(p_thread != 0);
	
	DWORD exitcode = 0;
	GetExitCodeThread(p_thread->handle, &exitcode);
	return exitcode != STILL_ACTIVE;
}


void setName(const handle& p_thread, const char* p_name)
{
	TT_NULL_ASSERT(p_thread);
	if (p_thread == 0)
	{
		return;
	}
	
	setThreadName(p_thread->id, p_name);
}

// Namespace end
}
}
