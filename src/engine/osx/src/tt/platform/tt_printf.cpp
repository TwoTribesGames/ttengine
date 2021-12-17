//#include <windows.h>

#include <tt/platform/tt_printf.h>

#include <stdarg.h>
#include <stdio.h>


//namespace tt {


void TT_FinalPrintf(const char* p_format, ...)
{
	va_list args;
	va_start(args, p_format);
	TT_FinalVPrintf(p_format, args);
	va_end(args);
}


void TT_FinalVPrintf(const char* p_format, va_list p_valist)
{
	vprintf(p_format, p_valist);
	
	/*
	 enum { BufferSize = 1024 };
	 char buffer[BufferSize] = { 0 };
	 
	 va_list args;
	 va_start(args, p_format);
	 vsnprintf(buffer, BufferSize, p_format, args);
	 va_end(args);
	 
	 static bool debuggerPresent = (IsDebuggerPresent() == TRUE);
	 if (debuggerPresent)
	 {
	 OutputDebugStringA(buffer);
	 }
	 else
	 {
	 printf("%s", buffer);
	 }
	 */
}

// Namespace end
//}
