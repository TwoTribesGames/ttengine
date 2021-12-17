#include <tt/system/Time.h>

#include "TimeHelpers.h"


namespace UnitTest {

Timer::Timer()
{
	// Create Time.
	tt::system::Time::getInstance();
}

void Timer::Start()
{
	tt::system::Time* time = tt::system::Time::getInstance();
	m_startTime = time->getMilliSeconds();
}

double Timer::GetTimeInMs() const
{
	tt::system::Time* time = tt::system::Time::getInstance();
	return static_cast<double>(time->getMilliSeconds() - m_startTime);
}


void TimeHelpers::SleepMs(int ms)
{
	//::Sleep(ms);
	(void) ms;
	TT_PANIC("Sleep is unsupported for this platform.");
}

}
