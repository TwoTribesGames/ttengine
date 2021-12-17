#ifndef INC_TT_MEM_MEM_TRACER_H
#define INC_TT_MEM_MEM_TRACER_H

namespace tt {
namespace mem {

// FIXME: Move to shared

/**
 * Memory Tracer Dummy implementation for Windows
 */
class MemTracer
{
public:
	MemTracer(const char* /* p_function */) {}
	~MemTracer() {}

private:
	MemTracer(const MemTracer& p_rhs);
	MemTracer& operator=(const MemTracer& p_rhs);
};

#define TRACE_MEM() 
#define TRACE_MEM_MESSAGE(...)

}
}

#endif	// INC_TT_MEM_MEM_TRACER_H
