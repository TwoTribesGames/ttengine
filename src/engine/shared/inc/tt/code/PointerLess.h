#if !defined(INC_TT_CODE_POINTERLESS_H)
#define INC_TT_CODE_POINTERLESS_H


#include <functional>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

template<typename T>
struct PointerLess
{
	// Functor for operator<
	inline bool operator()(const T* p_lhs, const T* p_rhs) const
	{
		TT_ASSERTMSG(p_lhs != 0 && p_rhs != 0, "Arguments must not be 0.");
		
		// Apply operator< to operands
		return (*p_lhs < *p_rhs);
	}
};



template<typename T>
struct SmartPointerLess
{
	// Functor for operator<
	inline bool operator()(const typename tt_ptr<const T>::shared& p_lhs,
	                       const typename tt_ptr<const T>::shared& p_rhs) const
	{
		TT_ASSERTMSG(p_lhs != 0 && p_rhs != 0, "Arguments must not be 0.");
		
		// Apply operator< to operands
		return (*p_lhs < *p_rhs);
	}
};


template<typename T>
struct PointerEqual
{
	inline bool operator()(const T* p_lhs, const T* p_rhs) const
	{
		TT_ASSERTMSG(p_lhs != 0 && p_rhs != 0, "Arguments must not be 0.");
		
		return (*p_lhs == *p_rhs);
	}
};


// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_POINTERLESS_H)
