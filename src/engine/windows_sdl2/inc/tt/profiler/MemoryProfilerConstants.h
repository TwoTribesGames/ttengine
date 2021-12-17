#ifndef INC_TT_PROFILER_MEMORY_PROFILER_CONSTANTS_H

namespace tt {
namespace profiler {

// enable this to profile memory on this platform
#ifdef TT_BUILD_DEV
//	#define PROFILE_MEMORY
#endif

// enable this to enable guard bytes checks in the profiler
#define MEMORY_PROFILER_ENABLE_GUARD_BYTES

// change this if required to use a different output method
#ifdef PROFILE_MEMORY
	#define TT_MEMORY_PROFILER_PRINTF	TT_Printf
#endif

#ifdef PROFILE_MEMORY

// platform specific constants
enum Constants
{
	Constants_MaxMemBlocks = 25000,
	Constants_NumberOfHeaps = 1
};

#endif // PROFILE_MEMORY

// namespace
}
}

#endif // INC_TT_PROFILER_MEMORY_PROFILER_CONSTANTS_H
