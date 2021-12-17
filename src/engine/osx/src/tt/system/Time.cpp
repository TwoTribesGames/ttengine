#include <sstream>
#include <iomanip>
#include <mach/mach_time.h>

#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>


namespace tt {
namespace system {

Time* Time::m_instance = 0;

static mach_timebase_info_data_t ms_info;
static real64 ms_microFactor;
static real64 ms_milliFactor;
static real64 ms_secondFactor;

Time::Time()
{
	// Save the performance frequency for this system
	mach_timebase_info(&ms_info);
	ms_microFactor = ms_info.numer / (ms_info.denom * 1000.0f);
	ms_milliFactor = ms_info.numer / (ms_info.denom * 1000000.0f);
	ms_secondFactor = ms_info.numer / (ms_info.denom * 1000000000.0f);
}


Time::~Time()
{
}


u64 Time::getMicroSeconds() const
{
	return mach_absolute_time() * ms_microFactor;
}


u64 Time::getMilliSeconds() const
{
	return mach_absolute_time() * ms_milliFactor;
}


u64 Time::getSeconds() const
{
	return mach_absolute_time() * ms_secondFactor;
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
