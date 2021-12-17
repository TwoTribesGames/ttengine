#if !defined(INC_TT_CODE_NODELETER_H)
#define INC_TT_CODE_NODELETER_H


namespace tt {
namespace code {

/*! \brief Custom deleter for smart pointers which does nothing.
    \note Use only for situations where a smart pointer is absolutely required
          (for example if an external class or function requires one),
          but memory should not be managed by it! */
template<typename T>
struct NoDeleter
{
	inline void operator()(T*) { }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_NODELETER_H)
