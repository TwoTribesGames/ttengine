#define NOMINMAX
#include <windows.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/Semaphore.h>


namespace tt {
namespace thread {


std::string getLastErrorMessage()
{
#if !defined(TT_BUILD_FINAL)
	char message[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, message, 256, 0);
	return message;
#else
	return std::string();
#endif
}


// ------------------------------------------------------------------------------------------------


struct Semaphore::InternalData
{
	HANDLE handle;
};


Semaphore::Semaphore(s32 p_initialCount)
:
m_data(new InternalData)
{
	const s32 maxCount = 0x7FFFFFFF;
	TT_MINMAX_ASSERT(p_initialCount, 0, maxCount);
	m_data->handle = CreateSemaphore(NULL, p_initialCount, maxCount, NULL);
	TT_ASSERTMSG(m_data->handle != NULL, "CreateSemaphore failed! reason: '%s'", getLastErrorMessage().c_str());
}


Semaphore::~Semaphore()
{
	BOOL result = CloseHandle(m_data->handle);
	TT_ASSERTMSG(result != 0, "CloseHandle failed! reason: '%s'", getLastErrorMessage().c_str());
	delete m_data;
}


void Semaphore::wait()
{
	DWORD result = WaitForSingleObject(m_data->handle, INFINITE);
	TT_ASSERTMSG(result != WAIT_FAILED, "WaitForSingleObject failed. Reason: '%s'", getLastErrorMessage().c_str());
}


bool Semaphore::tryWait()
{
	DWORD result = WaitForSingleObject(m_data->handle, 0);
	TT_ASSERTMSG(result != WAIT_FAILED, "WaitForSingleObject failed. Reason: '%s'", getLastErrorMessage().c_str());
	if (result == WAIT_TIMEOUT)
	{
		return false;
	}
	
	return true;
}


void Semaphore::signal()
{
	BOOL result = ReleaseSemaphore(m_data->handle, 1, 0);
	TT_ASSERTMSG(result != 0, "ReleaseSemaphore failed! reason: '%s'", getLastErrorMessage().c_str());
}


// Namespace end
}
}
