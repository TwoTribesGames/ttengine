
#include <tt/platform/tt_printf.h>

#include <SDL2/SDL_log.h>

#include <stdarg.h>
#include <stdio.h>

#if !defined(TT_BUILD_FINAL)

//namespace tt {

void TT_Printf(const char* p_format, ...)
{
	va_list args;
	va_start(args, p_format);
	
	TT_VPrintf(p_format, args);
	
	va_end(args);
}


void TT_VPrintf(const char* p_format, va_list p_valist)
{
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, p_format, p_valist);
}


void TT_ErrPrintf(const char* p_format, ...)
{
	va_list args;
	va_start(args, p_format);
	TT_ErrVPrintf(p_format, args);
	va_end(args);
}


void TT_ErrVPrintf(const char* p_format, va_list p_valist)
{
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, p_format, p_valist);
}


// Namespace end
//}

#endif // #if !defined(TT_BUILD_FINAL)