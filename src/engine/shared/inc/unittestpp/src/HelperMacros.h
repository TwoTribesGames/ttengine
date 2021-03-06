#ifndef UNITTEST_HELPERMACROS_H
#define UNITTEST_HELPERMACROS_H

#include "../config.h"

#define UNITTEST_MULTILINE_MACRO_BEGIN do {

#ifdef UNITTEST_WIN32
	#define UNITTEST_MULTILINE_MACRO_END \
		} __pragma(warning(push)) __pragma(warning(disable:4127)) while (0) __pragma(warning(pop))
#else
	#define UNITTEST_MULTILINE_MACRO_END } while(0)
#endif


#ifdef UNITTEST_WIN32_DLL
	#define UNITTEST_IMPORT __declspec(dllimport)
	#define UNITTEST_EXPORT	__declspec(dllexport)

	#ifdef UNITTEST_DLL_EXPORT
		#define UNITTEST_LINKAGE UNITTEST_EXPORT
		#define UNITTEST_IMPEXP_TEMPLATE
	#else
		#define UNITTEST_LINKAGE UNITTEST_IMPORT
		#define UNITTEST_IMPEXP_TEMPLATE extern
	#endif

	#define UNITTEST_STDVECTOR_LINKAGE(T) \
		__pragma(warning(push)) \
		__pragma(warning(disable:4231)) \
		UNITTEST_IMPEXP_TEMPLATE template class UNITTEST_LINKAGE std::allocator< T >; \
		UNITTEST_IMPEXP_TEMPLATE template class UNITTEST_LINKAGE std::vector< T >; \
		__pragma(warning(pop))
#else
	#define UNITTEST_IMPORT
	#define UNITTEST_EXPORT
	#define UNITTEST_LINKAGE
	#define UNITTEST_IMPEXP_TEMPLATE
	#define UNITTEST_STDVECTOR_LINKAGE(T) class tt_dummy_class_declaration_to_avoid_semicolon_causing_an_illegal_empty_declaration_error_on_twl
#endif

#ifdef UNITTEST_WIN32
	#define UNITTEST_JMPBUF jmp_buf
	#define UNITTEST_SETJMP setjmp
	#define UNITTEST_LONGJMP longjmp
#elif defined UNITTEST_POSIX
	#define UNITTEST_JMPBUF std::jmp_buf
	#define UNITTEST_SETJMP setjmp
	#define UNITTEST_LONGJMP std::longjmp
#else
#error Unknown platform. please add defines for JMP.
#endif

#endif
