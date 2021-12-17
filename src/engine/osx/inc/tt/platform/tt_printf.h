#ifndef INC_TT_PRINTF_H
#define INC_TT_PRINTF_H

//#include <cstdio>
#include <cstdarg>

//#define TT_Printf std::printf

//namespace tt {

void TT_FinalPrintf(const char* p_format, ...);
void TT_FinalVPrintf(const char* p_format, va_list p_valist);

#if !defined(TT_BUILD_FINAL)
#define TT_Printf TT_FinalPrintf
#define TT_VPrintf TT_FinalVPrintf
#else
#define TT_Printf(...)
#define TT_VPrintf(...)
#endif

//namespace
//}


#endif  // !defined(INC_TT_PRINTF_H)
