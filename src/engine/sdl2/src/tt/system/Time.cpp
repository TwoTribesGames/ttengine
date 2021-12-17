#include <sstream>
#include <iomanip>
#include <time.h>

#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>

#include <SDL2/SDL.h>

namespace tt {
namespace system {

Time* Time::m_instance = 0;


Time::Time()
{
}


Time::~Time()
{
}

#define SEC_PER_SEC 1
#define SEC_PER_NSEC 1E-9

#define MSEC_PER_SEC 1E3
#define MSEC_PER_NSEC 1E-6

#define USEC_PER_SEC 1E6
#define USEC_PER_NSEC 1E-3


u64 Time::getMicroSeconds() const
{
	u64 count = SDL_GetPerformanceCounter();
	u64 freq = SDL_GetPerformanceFrequency();

	return count * USEC_PER_SEC / freq;
}


u64 Time::getMilliSeconds() const
{
	u64 count = SDL_GetPerformanceCounter();
	u64 freq = SDL_GetPerformanceFrequency();

	return count * MSEC_PER_SEC / freq;
}


u64 Time::getSeconds() const
{
	u64 count = SDL_GetPerformanceCounter();
	u64 freq = SDL_GetPerformanceFrequency();

	return count / freq;
}


//! \return current time as a HH:MM:SS.TT string
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
