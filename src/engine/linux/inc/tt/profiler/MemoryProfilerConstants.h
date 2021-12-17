#if !defined(INC_TT_PROFILER_MEMORYPROFILERCONSTANTS_H)
#define INC_TT_PROFILER_MEMORYPROFILERCONSTANTS_H


namespace tt {
namespace profiler {

// Enable this to profile memory on this platform
//#define PROFILE_MEMORY

// Enable this to enable guard bytes checks in the profiler
#define MEMORY_PROFILER_ENABLE_GUARD_BYTES

// Change this if required to use a different output method
#ifdef PROFILE_MEMORY
	#define TT_MEMORY_PROFILER_PRINTF	TT_Printf
#endif


#if defined(PROFILE_MEMORY)

// Platform specific constants
enum Constants
{
	Constants_MaxMemBlocks  = 25000,
	Constants_NumberOfHeaps = 1
};

#endif // PROFILE_MEMORY

// Namespace end
}
}


#endif  // !defined(INC_TT_PROFILER_MEMORYPROFILERCONSTANTS_H)
