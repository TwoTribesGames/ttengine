#define NOMINMAX
#include <windows.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace thread {

#ifdef USE_CRITICALSECTION

	struct Mutex::InternalData : CRITICAL_SECTION
	{
		
	};

#else // USE_CRITICALSECTION

	struct Mutex::InternalData
	{
		HANDLE handle;
	};

#endif // USE_CRITICALSECTION


Mutex::Mutex()
:
m_mutex(new InternalData)
{
#ifdef USE_CRITICALSECTION
	InitializeCriticalSection(m_mutex);
#else
	m_mutex->handle = CreateMutex(NULL, FALSE, NULL); // unnamed mutex
#endif
}


Mutex::~Mutex()
{
#ifdef USE_CRITICALSECTION
	DeleteCriticalSection(m_mutex);
#else
	CloseHandle(m_mutex->handle);
#endif
	delete m_mutex;
}


void Mutex::lock()
{
#ifdef USE_CRITICALSECTION
	EnterCriticalSection(m_mutex);
#else
	DWORD result = WaitForSingleObject(m_mutex->handle, INFINITE);
	
	if (result == WAIT_FAILED)
	{
		char reason[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, reason, 256, 0);
		TT_PANIC("WaitForSingleObject failed. Reason: '%s'", reason);
	}
#endif
}


bool Mutex::tryLock()
{
#ifdef USE_CRITICALSECTION
	return TryEnterCriticalSection(m_mutex) == TRUE;
#else
	DWORD result = WaitForSingleObject(m_mutex->handle, 0);
	TT_ASSERT(result != WAIT_FAILED);
	if (result == WAIT_TIMEOUT)
	{
		return false;
	}
	return true;
#endif
}


void Mutex::unlock()
{
#ifdef USE_CRITICALSECTION
	LeaveCriticalSection(m_mutex);
#else
	ReleaseMutex(m_mutex->handle);
#endif
}

// Namespace end
}
}
