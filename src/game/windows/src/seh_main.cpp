#define NOMINMAX
#include <windows.h>
#include <crtdbg.h>
#include <eh.h>

int realMain();
void miniDumpGenerator(unsigned int p_exceptionCode, EXCEPTION_POINTERS* p_exception);

// WinMain has been rigged to direct unhandled exceptions to the miniDumpGenerator
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
	
	//_CrtSetBreakAlloc(23059);
	
	if(IsDebuggerPresent())
	{
		// We don't want to mask exceptions when debugging
		return realMain();
	}
	
	// NOTE: We need to compile with /EHa for this to work
	
	_set_se_translator(miniDumpGenerator);
	try  // this try block allows the SE translator to work
	{
		return realMain();
	}
	catch( ... )
	{
		return -1;
	}
}
