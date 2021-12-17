#if !defined(INC_TT_MEM_MEMTRACER_H)
#define INC_TT_MEM_MEMTRACER_H


namespace tt {
namespace mem {

// FIXME: Move to shared

/*! \brief Memory Tracer Dummy implementation for OS X. */
class MemTracer
{
public:
	MemTracer(const char* /* p_function */) { }
	~MemTracer() { }
	
private:
	MemTracer(const MemTracer& p_rhs);
	const MemTracer& operator=(const MemTracer& p_rhs);
};

#define TRACE_MEM() 
#define TRACE_MEM_MESSAGE(...)

// Namespace end
}
}


#endif	// !defined(INC_TT_MEM_MEMTRACER_H)
