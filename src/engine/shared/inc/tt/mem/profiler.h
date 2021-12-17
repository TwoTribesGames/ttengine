#ifndef INC_TT_MEM_PROFILER_H
#define INC_TT_MEM_PROFILER_H

#include <tt/mem/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace mem {
namespace profiler {
	
	/*! \brief Initialize the memory profiler,
	           call this prior to initializing the allocation system.
	    \param p_leak Whether leak detection is enabled.
	    \param p_guard Whether guard bytes are enabled.
	    \param p_clear Whether to clear the memory before alloc and after free.*/
	void init(bool p_leak, bool p_guard, bool p_clear);
	
	/*! \brief Starts profiling.*/
	void start();
	
	/*! \brief Stops profiling.
	    \param p_assert Assert on leak/corruption.*/
	void stop(bool p_assert = false);
	
	/*! \brief Sets.a breakpoint on a specified allocation.
	    \param p_alloc Allocation number.
	    \param p_run 0 = absolute allocation number, > 0 = relative to p_run start.*/
	void setBreakAlloc(size_type p_alloc, size_type p_run = 0);
	
// namespace end
}
}
}

#endif // INC_TT_MEM_PROFILER_H
