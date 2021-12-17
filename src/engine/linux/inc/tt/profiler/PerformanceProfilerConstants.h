#if !defined(INC_TT_PROFILER_PERFORMANCEPROFILERCONSTANTS_H)
#define INC_TT_PROFILER_PERFORMANCEPROFILERCONSTANTS_H


#include <tt/platform/tt_printf.h>


namespace tt {
namespace profiler {

// Enable this to profile performance on this platform
#if !defined(TT_BUILD_FINAL)
//#define PERFORMANCE_PROFILER_ENABLED
#endif

// Platform specific constants
#ifdef PERFORMANCE_PROFILER_ENABLED
	#define TT_PROFILER_PRINTF	TT_Printf
#else
	#define TT_PROFILER_PRINTF(...)
#endif

// Namespace end
}
}


#endif  // !defined(INC_TT_PROFILER_PERFORMANCEPROFILERCONSTANTS_H)
