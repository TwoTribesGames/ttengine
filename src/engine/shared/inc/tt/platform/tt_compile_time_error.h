#if !defined(INC_TT_PLATFORM_COMPILE_TIME_ERROR_H)
#define INC_TT_PLATFORM_COMPILE_TIME_ERROR_H

#if defined(_MSC_VER) && (_MSC_VER < 1900)
// VS2015 doesn't support static asserts without message yet (C++17 feature)
#include <boost/static_assert.hpp>
#define TT_STATIC_ASSERT BOOST_STATIC_ASSERT
#else
#define TT_STATIC_ASSERT static_assert
#endif

#endif // !defined(INC_TT_PLATFORM_COMPILE_TIME_ERROR_H)
