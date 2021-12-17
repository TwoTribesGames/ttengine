#include <windows.h>
#include <sstream>
#include <iomanip>

#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>


namespace tt {
namespace system {

// Nullify static
Time* Time::m_instance = 0;

static LARGE_INTEGER g_performanceFrequency;


Time::Time()
{
	// Save the performance frequency for this system
	QueryPerformanceFrequency(&g_performanceFrequency);
}


Time::~Time()
{
}


/*! \brief Read time ticker.
    \return timer in microseconds */
u64 Time::getMicroSeconds() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return static_cast<u64>((now.QuadPart * 1000000) / g_performanceFrequency.QuadPart);
}


/*! \brief Read time ticker.
    \return timer in milliseconds */
u64 Time::getMilliSeconds() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return static_cast<u64>((now.QuadPart * 1000) / g_performanceFrequency.QuadPart);
}


/*! \brief Read time ticker.
    \return timer in seconds */
u64 Time::getSeconds() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return static_cast<u64>(now.QuadPart / g_performanceFrequency.QuadPart);
}


/*! \return Current time as a HH:MM:SS.TT string. */
std::string Time::getNowAsString() const
{
	time_t rawtime;
	time( &rawtime );
	struct tm* timeinfo = localtime(&rawtime);
	
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << (timeinfo->tm_hour) << ":"
	    << std::setw(2) << std::setfill('0') << (timeinfo->tm_min) << ":"
	    << std::setw(2) << std::setfill('0') << (timeinfo->tm_sec);
	
	return oss.str();
}

// Namespace end
}
}
