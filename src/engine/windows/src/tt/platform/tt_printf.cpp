#if !defined(TT_BUILD_FINAL)

#define NOMINMAX
#include <windows.h>

//#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <stdarg.h>
#include <stdio.h>


//namespace tt {

// Private helper to reduce code duplication
void TT_PrintfImpl(bool p_toErrorStream, const char* p_format, va_list p_valist)
{
	static const bool debuggerPresent = (IsDebuggerPresent() == TRUE);
	if (debuggerPresent)
	{
		enum { BufferSize = 8192 };
		static char __declspec(thread) buffer[BufferSize] = { 0 };
		
		vsnprintf(buffer, BufferSize, p_format, p_valist);
		buffer[BufferSize - 1] = 0;
		
		OutputDebugStringA(buffer);
	}
	else
	{
		vfprintf_s(p_toErrorStream ? stderr : stdout, p_format, p_valist);
	}
}


// Implementations of the public functions

void TT_Printf(const char* p_format, ...)
{
	va_list args;
	va_start(args, p_format);
	
	TT_PrintfImpl(false, p_format, args);
	
	va_end(args);
}


void TT_VPrintf(const char* p_format, va_list p_valist)
{
	TT_PrintfImpl(false, p_format, p_valist);
}


void TT_ErrPrintf(const char* p_format, ...)
{
	va_list args;
	va_start(args, p_format);
	
	TT_PrintfImpl(true, p_format, args);
	
	va_end(args);
}


void TT_ErrVPrintf(const char* p_format, va_list p_valist)
{
	TT_PrintfImpl(true, p_format, p_valist);
}

// Namespace end
//}

#endif // #if !defined(TT_BUILD_FINAL)
