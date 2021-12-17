
#if defined(__GNUC__) && !defined(TT_PLATFORM_OSX)
	// Use GCC compiler settings when using that compiler. (But not on osx/iPhone.)

	// Should only be used by tools.
	#include <boost/config/compiler/gcc.hpp>
#else
	// Turn off exceptions for compilers other than gcc for tools
	#define BOOST_NO_EXCEPTIONS
	#define BOOST_SP_DISABLE_THREADS
#endif


// Use own own asset and not std assert
#define BOOST_ENABLE_ASSERT_HANDLER

// Disable assert on TT BUILD FINAL.
#if defined(TT_BUILD_FINAL)
	#define BOOST_DISABLE_ASSERTS
#endif
