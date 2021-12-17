#ifndef INC_TT_PROFILER_PERFORMANCE_PROFILER_CONSTANTS_H

namespace tt {
namespace profiler {

// enable this to profile performance on this platform
//#define PERFORMANCE_PROFILER_ENABLED

// platform specific constants
#ifdef PERFORMANCE_PROFILER_ENABLED
	#define TT_PROFILER_PRINTF	TT_Printf
#else
	#define TT_PROFILER_PRINTF(...)
#endif

// namespace
}
}

#endif // INC_TT_PROFILER_PERFORMANCE_PROFILER_CONSTANTS_H
