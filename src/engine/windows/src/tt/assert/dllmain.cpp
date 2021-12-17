#include <windows.h>

#include <tt/assert/assert.h>
#include <tt/assert/exceptionhandler.h>
#include <tt/assert/asserthandler.h>

#include <tt/assert/dll.h>

// globals we use
HINSTANCE			ghInstance = 0;
tt::assert::Assert*	gAssert = 0;


int WINAPI DllMain( HANDLE hModule, DWORD dwReason, void* /*pReserved*/ )
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			// process has attached, create handler functons and save their pointers
			if ( ghInstance == NULL )
			{
				ghInstance = (HINSTANCE)hModule;
			}
			
			//
			if (gAssert == 0)
			{
				gAssert = new tt::assert::Assert;
			}
			break;
			
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
			
		case DLL_PROCESS_DETACH:
			// free memory
			delete gAssert;
			gAssert = 0;
			break;
	}
	
	return TRUE;
}



// Function name	: WINAPI HandleAssert
// Description	    : Externally callable function to deal with an assert. If the assert fails an exception is
//						raised to allow the handlers to pull out debug information
// 
// Return type		: int 
// Argument         : const char* pExpression			- expression evaluated
// Argument         : const char* pFile					- name of the file the assert occured in
// Argument         : const int line					- line number
// Argument         : const char* pComment				- user comment
EXPORT int WINAPI HandleAssert(const char* p_file, int p_line, 
                               const char* p_function, const char* p_message)
{
	return gAssert->handleAssert(p_file, p_line, p_function, p_message);
}


EXPORT void WINAPI SetBuildInfo(int p_libRevision, int p_clientRevision)
{
	gAssert->setLibRevision(p_libRevision);
	gAssert->setClientRevision(p_clientRevision);
}


EXPORT void WINAPI SetParentWindow(HWND p_window)
{
	gAssert->setParentWindow(p_window);
}


EXPORT int WINAPI IsAssertInIgnoreList(const char* p_file, int p_line)
{
	return gAssert->isAssertInIgnoreList(p_file, p_line) ? 1 : 0;
}
