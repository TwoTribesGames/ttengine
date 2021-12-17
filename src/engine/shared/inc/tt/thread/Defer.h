#if !defined(INC_TT_THREAD_DEFER_H)
#define INC_TT_THREAD_DEFER_H

#include <vector>

#include <tt/platform/tt_types.h>


namespace tt {
namespace thread {


class Defer
{
public:
	typedef void (*function)(void*);
	
	/*! \brief Defers the call of a function by a specified time.
	    \param p_delay Number of microseconds until the the function should be called.
	    \param p_function Function to call.
	    \param p_data Data to pass to function.*/
	static void defer(u64 p_delay, function p_function, void* p_data = 0);
	
	/*! \brief Updates deferred calls.
	    \param p_delta Time passed since last update in microseconds.*/
	static void update(u64 p_delta);
	
private:
	Defer(u64 p_time, function p_function, void* p_data);
	~Defer();
	Defer(const Defer&);                  // no copying allowed
	const Defer& operator=(const Defer&); // no assignment allowed
	
	void trigger();
	
	static bool predicate(const Defer* p_lhs, const Defer* p_rhs);
	
	typedef std::vector<Defer*> Defers;
	
	static Defers ms_defers; //!< Deferred calls.
	static u64    ms_time;   //!< Current time.
	
	u64      m_time;     //!< Time at which the function should be called.
	function m_function; //!< Function to be called.
	void*    m_data;     //!< Data for the function.
};

// namespace end
}
}


#endif // !defined(INC_TT_THREAD_DEFER_H)
