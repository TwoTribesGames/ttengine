#ifndef INC_TT_COMPILER_H
#define INC_TT_COMPILER_H

//-------------------------------------
// Crossplatform macros

// If __PRETTY_FUNCTION__ doesn't exist on a platform,
// a platform specific tt_compiler.h should be created.
#if defined(_MSC_VER)
#define TT_FUNC_SIG __FUNCSIG__
#else
#define TT_FUNC_SIG __PRETTY_FUNCTION__
#endif

//-------------------------------------

#endif  // !defined(INC_TT_COMPILER_H)
