#include <exception>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_types.h>


namespace boost {

#if !defined(TT_BUILD_FINAL)
void assertion_failed(char const* p_expr, char const* p_function, char const* p_file, long p_line)
{
	tt::platform::error::TTPanic(p_file, static_cast<int>(p_line), p_function, p_expr);
}
#endif  // !defined(TT_BUILD_FINAL)


void throw_exception(const std::exception& p_exception)
{
	TT_PANIC("Boost exception: '%s'", p_exception.what());
	
	// Also output for non-assert builds
	TT_Printf("Boost exception: '%s'", p_exception.what());
}

// Namespace end
}
