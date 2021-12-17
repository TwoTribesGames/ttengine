#ifndef _DLL_H_
#define _DLL_H_

#define EXPORT extern "C" __declspec (dllexport)


EXPORT int WINAPI HandleAssert(const char* p_file, int p_line, 
                               const char* p_function, const char* p_message);

EXPORT void WINAPI SetBuildInfo(int p_libRevision, int p_clientRevision);

EXPORT void WINAPI SetParentWindow(HWND p_window);

EXPORT int WINAPI IsAssertInIgnoreList(const char* p_file, int p_line);

#endif
