#if !defined(INC_TT_MENU_MENUDEBUG_H)
#define INC_TT_MENU_MENUDEBUG_H


#include <tt/platform/tt_printf.h>


// Comment this out if menu debug output should be suppressed altogether
//#define MENU_DEBUG_OUTPUT_ENABLED


// These macros control which menu debug output is displayed.
// Can be used to control debug output flooding.

#define MENU_TRACE_CREATION
//#define MENU_TRACE_KEY_INPUT
//#define MENU_TRACE_STYLUS_INPUT


#if defined(MENU_DEBUG_OUTPUT_ENABLED)
	#define MENU_Printf(...) TT_Printf(__VA_ARGS__)
#else
	#define MENU_Printf(...)
#endif


#if defined(MENU_TRACE_CREATION)
	#define MENU_CREATION_Printf(...) MENU_Printf(__VA_ARGS__)
#else
	#define MENU_CREATION_Printf(...)
#endif


#if defined(MENU_TRACE_KEY_INPUT)
	#define MENU_KEY_Printf(...) MENU_Printf(__VA_ARGS__)
#else
	#define MENU_KEY_Printf(...)
#endif


#if defined(MENU_TRACE_STYLUS_INPUT)
	#define MENU_STYLUS_Printf(...) MENU_Printf(__VA_ARGS__)
#else
	#define MENU_STYLUS_Printf(...)
#endif


#endif  // !defined(INC_TT_MENU_MENUDEBUG_H)
