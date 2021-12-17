#ifndef INC_TT_PLATFORM_TT_ERROR_H
#define INC_TT_PLATFORM_TT_ERROR_H


#include <tt/platform/tt_compile_time_error.h>
#include <tt/platform/tt_compiler.h>

#if defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_SDL)
#include <tt/engine/opengl_headers.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// do some sanity checks

#if !defined(TT_BUILD_FINAL) && !defined(TT_BUILD_TEST) && !defined(TT_BUILD_DEV) 
#error No valid build configuration passed to buildsystem
#endif

#if defined(TT_BUILD_FINAL) && defined(TT_ASSERT_ON)
#error TT_ASSERT_ON defined in TT_BUILD_FINAL setting
#endif

#if defined(TT_FINAL_BUILD)
#error TT_FINAL_BUILD used instead of TT_BUILD_FINAL
#endif

#if defined(TT_TEST_BUILD)
#error TT_TEST_BUILD used instead of TT_BUILD_TEST
#endif

#if defined(TT_DEV_BUILD)
#error TT_DEV_BUILD used instead of TT_BUILD_DEV
#endif

#if defined(TT_DEBUG)
#error deprecated TT_DEBUG used
#endif

#if defined(TT_RELEASE)
#error deprecated TT_RELEASE used
#endif

////////////////////////////////////////////////////////////////////////////////

namespace tt {
namespace platform {
namespace error {

/*! \brief Callback for when a panic is triggered.
    \note Only available for Windows and OS X at the moment.
          Only one callback function can be registered at a time. */
typedef void (*PanicCallback)();

#if !defined(TT_BUILD_FINAL)

#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)
void registerPanicCallback(PanicCallback p_fun);
#else
inline void registerPanicCallback(PanicCallback /*p_fun*/) {}
#endif



/*! \brief Call this function to turn headless mode on.
           With headless mode on, if a TTPanic is triggered
           it will NOT display a popup but instead will exit(-1).
   
    \note This is only implemented for Windows and OS X!
          Calling this function on any other platform will cause a linker error. */
void turnHeadlessModeOn();

/*! \brief Call this function to surpress all asserts and simply continue execution
   
    \note This is only implemented for Windows!
          Calling this function on any other platform will cause a linker error. */
void supressAssertsAndWarnings();

/*! \brief Call this method to reset the assert mute mode
   
    \note This is only implemented for Windows!
          Calling this function on any other platform will cause a linker error. */
void resetAssertMuteMode();

void TTPanic(const char* p_file, int p_line, 
             const char* p_function, const char* p_fmt, ...)
#if defined(__clang__)
#if __has_feature(attribute_analyzer_noreturn)
/* this tells the clang analyzer that we are using a custom assert function
	and that the analyzer should assume the condition is true */
	__attribute__((analyzer_noreturn))
#endif
#endif
;

void TTWarning(const char* p_file, int p_line, 
               const char* p_function, const char* p_fmt, ...);

#else

// Dummy implementations for final builds
inline void registerPanicCallback(PanicCallback /*p_fun*/) {}
inline void turnHeadlessModeOn() { }
inline void supressAssertsAndWarnings() { }
inline void resetAssertMuteMode() { }

// FIXME: Should TTPanic and TTWarning also be available as stubs in final?
/*
inline void TTPanic(const char*, int, const char*, const char*, ...) { }
inline void TTWarning(const char*, int, const char*, const char*, ...) { }
*/

#endif // !defined(TT_BUILD_FINAL)

// Namespace end
}
}
}


////////////////////////////////////////////////////////////////////////////////
// Assert macros partially taken from http://powerof2games.com/node/10
//

// FIXME: Remove this warning
#if defined(_MSC_VER)
#pragma warning(disable: 4127)
#endif


#if !defined(TT_BUILD_FINAL)

//==================================================================================================
// Real (non-final) implementations of assert macros

#define TT_ASSERT(x)                    do { if (!(x)) { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, "Failed assertion: " #x); } } while(0)
#define TT_ASSERTMSG(x, ...)            do { if (!(x)) { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, __VA_ARGS__); } } while(0)
#define TT_PANIC(...)                   do { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, __VA_ARGS__); } while(0)
#define TT_WARNING(x, ...)              do { if (!(x)) { tt::platform::error::TTWarning(__FILE__, __LINE__, TT_FUNC_SIG, __VA_ARGS__); } } while(0)
#define TT_WARN(...)                    do { tt::platform::error::TTWarning(__FILE__, __LINE__, TT_FUNC_SIG, __VA_ARGS__); } while(0)
#define TT_NULL_ASSERT(x)               do { if ((x == 0)) { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, "Pointer '"#x"' must not be 0"); } } while(0)
#define TT_MIN_ASSERT(x, min)           do { if (!((x) >= (min))) { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, #x" (%d) is out of bounds\n%d <= "#x" not satisfied.", x, min); } } while(0)
#define TT_MAX_ASSERT(x, max)           do { if (!((x) <= (max))) { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, #x " (%d) is out of bounds\n"#x" <= %d not satisfied.", x, max); } } while(0)
#define TT_MINMAX_ASSERT(x, min, max)   do { if (!( (x) >= (min) && (x) <= (max) )) { tt::platform::error::TTPanic(__FILE__, __LINE__, TT_FUNC_SIG, #x " (%d) is out of bounds\n%d <= "#x" <= %d not satisfied.", x, min, max); } } while(0)

#else  // defined(TT_BUILD_FINAL)

//==================================================================================================
// Dummy no-op (final) implementations of assert macros

#if (defined(_MSC_VER) && _MSC_VER >= 1600)

// Special-cased dummy implementations for Visual Studio 2010,
// where sizeof(x) does not actually use x (still triggers warnings)

#define TT_ASSERT(x)                    do { ((void)(true ? 0 : ((void)(x), void(), 0))); } while(0)
#define TT_ASSERTMSG(x, ...)            do { ((void)(true ? 0 : ((void)(x), void(), 0))); ((void)(true ? 0 : ((void)(__VA_ARGS__), void(), 0))); } while(0)
#define TT_PANIC(...)                   do { ((void)(true ? 0 : ((void)(__VA_ARGS__), void(), 0))); } while(0)
#define TT_WARNING(x, ...)              do { ((void)(true ? 0 : ((void)(x), void(), 0))); ((void)(true ? 0 : ((void)(__VA_ARGS__), void(), 0))); } while(0)
#define TT_WARN(...)                    do { ((void)(true ? 0 : ((void)(__VA_ARGS__), void(), 0))); } while(0)
#define TT_NULL_ASSERT(x)               do { ((void)(true ? 0 : ((void)(x), void(), 0))); } while(0)
#define TT_MIN_ASSERT(x, min)           do { ((void)(true ? 0 : ((void)(x), void(), 0))); ((void)(true ? 0 : ((void)(min), void(), 0))); } while(0)
#define TT_MAX_ASSERT(x, max)           do { ((void)(true ? 0 : ((void)(x), void(), 0))); ((void)(true ? 0 : ((void)(max), void(), 0))); } while(0)
#define TT_MINMAX_ASSERT(x, min, max)   do { ((void)(true ? 0 : ((void)(x), void(), 0))); ((void)(true ? 0 : ((void)(min), void(), 0))); ((void)(true ? 0 : ((void)(max), void(), 0))); } while(0)

#else

// Dummy implementations for all other compilers

// Dummy helper to get rid of __VA_ARGS__
// Note, this function's body is NEVER defined because it is never called.
namespace tt { namespace platform { namespace error {
	extern int DummyHelper(...);
} } }

#define TT_ASSERT(x)                    do { (void)sizeof(x); } while(0)
#define TT_ASSERTMSG(x, ...)            do { (void)sizeof(x); (void)sizeof(tt::platform::error::DummyHelper(__VA_ARGS__)); } while(0)
#define TT_PANIC(...)                   do { (void)sizeof(tt::platform::error::DummyHelper(__VA_ARGS__)); } while(0)
#define TT_WARNING(x, ...)              do { (void)sizeof(x); (void)sizeof(tt::platform::error::DummyHelper(__VA_ARGS__)); } while(0)
#define TT_WARN(...)                    do { (void)sizeof(tt::platform::error::DummyHelper(__VA_ARGS__)); } while(0)
#define TT_NULL_ASSERT(x)               do { (void)sizeof(x); } while(0)
#define TT_MIN_ASSERT(x, min)           do { (void)sizeof(x); (void)sizeof(min); } while(0)
#define TT_MAX_ASSERT(x, max)           do { (void)sizeof(x); (void)sizeof(max); } while(0)
#define TT_MINMAX_ASSERT(x, min, max)   do { (void)sizeof(x); (void)sizeof(min); (void)sizeof(max); } while(0)

#endif  // defined(_MSC_VER) && _MSC_VER >= 1600

#endif  // defined(TT_BUILD_FINAL)


#endif  // !defined(INC_TT_PLATFORM_TT_ERROR_H)
