#ifndef INC_TT_COMPILER_H
#define INC_TT_COMPILER_H

//-------------------------------------
// Crossplatform macros

// Determine appropriate TT_FUNC_SIG 
#if defined(_MSC_VER)
	#define TT_FUNC_SIG __FUNCSIG__
#else
	#define TT_FUNC_SIG __PRETTY_FUNCTION__
#endif

//-------------------------------------

#endif  // !defined(INC_TT_COMPILER_H)
