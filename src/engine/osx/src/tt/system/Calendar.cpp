#include <ctime>
#include <cstring>

#include <tt/platform/tt_error.h>
#include <tt/system/Calendar.h>


namespace tt {
namespace system {


Calendar Calendar::getCurrentDate()
{
	Calendar ret;
	std::time_t now;
	std::time(&now);
	std::tm* localnow = localtime(&now);
	std::memcpy(&ret.m_internal, localnow, sizeof(std::tm));
	
	return ret;
}

// Namespace end
}
}
