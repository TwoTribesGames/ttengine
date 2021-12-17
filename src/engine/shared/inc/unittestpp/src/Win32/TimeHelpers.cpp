#include "TimeHelpers.h"

#if defined(_XBOX)
#include <xtl.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


namespace UnitTest {

Timer::Timer()
:
m_threadHandle(GetCurrentThread()),
m_startTime(0)
{
#if defined(UNITTEST_WIN32) && (_MSC_VER == 1200) // VC6 doesn't have DWORD_PTR
	typedef unsigned long DWORD_PTR;
#endif

#if !defined(_XBOX)
	DWORD_PTR systemMask;
	::GetProcessAffinityMask(GetCurrentProcess(), &m_processAffinityMask, &systemMask);
	::SetThreadAffinityMask(m_threadHandle, 1);
#endif
	::QueryPerformanceFrequency(reinterpret_cast< LARGE_INTEGER* >(&m_frequency));
#if !defined(_XBOX)
	::SetThreadAffinityMask(m_threadHandle, m_processAffinityMask);
#endif
}

void Timer::Start()
{
	m_startTime = GetTime();
}

double Timer::GetTimeInMs() const
{
	__int64 const elapsedTime = GetTime() - m_startTime;
	double const seconds = double(elapsedTime) / double(m_frequency);
	return seconds * 1000.0;
}

__int64 Timer::GetTime() const
{
	LARGE_INTEGER curTime;
#if !defined(_XBOX)
	::SetThreadAffinityMask(m_threadHandle, 1);
#endif
	::QueryPerformanceCounter(&curTime);
#if !defined(_XBOX)
	::SetThreadAffinityMask(m_threadHandle, m_processAffinityMask);
#endif
	return curTime.QuadPart;
}

void TimeHelpers::SleepMs(int ms)
{
	::Sleep(ms);
}

}
