#ifndef INC_TT_PRINTF_H
#define INC_TT_PRINTF_H

#include <cstdio>
#include <cstdarg>
#include <cstring>

#if !defined(TT_BUILD_FINAL)

void TT_Printf(const char* p_format, ...);
void TT_VPrintf(const char* p_format, va_list p_valist);

// Prints to the error output stream
void TT_ErrPrintf(const char* p_format, ...);
void TT_ErrVPrintf(const char* p_format, va_list p_valist);

#else // #if !defined(TT_BUILD_FINAL)

#define TT_Printf(...)
#define TT_VPrintf(...)

// Prints to the error output stream
#define TT_ErrPrintf(...)
#define TT_ErrVPrintf(...)

#endif // #else // #if !defined(TT_BUILD_FINAL)

#endif  // !defined(INC_TT_PRINTF_H)
