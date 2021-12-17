// ExceptionHandler.h: interface for the ExceptionHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(INC_TT_ASSERT_EXCEPTIONHANDLER_H)
#define INC_TT_ASSERT_EXCEPTIONHANDLER_H

#include <windows.h>

#if defined(TT_PLATFORM_WIN)
#pragma warning (push)
#pragma warning (disable:4091)
#endif

#include <imagehlp.h>

#if defined(TT_PLATFORM_WIN)
#pragma warning (pop)
#endif

#include <stdio.h>
#include <list>
#include <tt/assert/String.h>

namespace tt {
namespace assert {

class ExceptionHandler  
{

	// private typedefs for imghelp.dll procedures
	typedef BOOL	(__stdcall *dllSymInitializePrc)			(HANDLE,LPSTR,BOOL);
	typedef BOOL	(__stdcall *dllSymCleanupPrc)				(HANDLE);
	typedef BOOL	(__stdcall *dllSymSetOptionsPrc)			(DWORD);
	typedef BOOL	(__stdcall *dllStackWalkPrc)				(DWORD,HANDLE,HANDLE,LPSTACKFRAME,LPVOID,PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE,PGET_MODULE_BASE_ROUTINE,PTRANSLATE_ADDRESS_ROUTINE);
	typedef LPVOID	(__stdcall *dllSymFunctionTableAccessPrc)	(HANDLE,DWORD);
	typedef DWORD	(__stdcall *dllSymGetModuleBasePrc)			(HANDLE,DWORD);
	typedef BOOL	(__stdcall *dllSymGetSymFromAddrPrc)		(HANDLE,DWORD,PDWORD,PIMAGEHLP_SYMBOL);
	typedef BOOL	(__stdcall *dllSymGetLineFromAddrPrc)		(HANDLE,DWORD,PDWORD,PIMAGEHLP_LINE);
	typedef BOOL	(__stdcall *dllUnDecorateSymbolNamePrc)		(PCSTR,PSTR,DWORD,DWORD);
	typedef DWORD	(__stdcall *dllSymLoadModule)				(HANDLE, HANDLE,PSTR,PSTR,DWORD,DWORD);

public:		// public definitions

	class CallStackItem
	{
		public:
			
			String<char>	functionName;
			String<char>	fileName;
			int				lineNumber;

			CallStackItem()
			{
				lineNumber = 0;
			}
	};

	typedef std::list<CallStackItem*>::const_iterator stackIterator;


public:

	ExceptionHandler();
	virtual ~ExceptionHandler();

	DWORD DumpCrash(HANDLE hThread, void* pExceptionInfo);
	

	const char* GetLastException( void );
	const char* GetModuleName( void );


	ExceptionHandler::stackIterator GetCallStackBegin();
	ExceptionHandler::stackIterator GetCallStackEnd();


protected:

	void InitClass( void );
	bool InitDLL( void );

	const char* GetLastExceptionDesc(DWORD dwCode, ULONG_PTR dwInfo0, ULONG_PTR dwInfo1);

	void ClearExceptions( void );
	void AddToCallStack(const char* funcName, const char* fileName, const int line);

	HMODULE mHImageHlpDll;

	// dll function pointers
	dllSymInitializePrc				SymInitialize;
	dllSymCleanupPrc				SymCleanup;
	dllSymSetOptionsPrc				SymSetOptions;
	dllStackWalkPrc					StackWalk;
	dllSymFunctionTableAccessPrc	SymFunctionTableAccess;
	dllSymGetModuleBasePrc			SymGetModuleBase;
	dllSymGetSymFromAddrPrc			SymGetSymFromAddr;
	dllSymGetLineFromAddrPrc		SymGetLineFromAddr;
	dllUnDecorateSymbolNamePrc		UnDecorateSymbolName;
	dllSymLoadModule				SymLoadModule;

	String<char>					mSearchPath;							// search path for debug info
	String<char>					mModuleName;
	String<char>					mLastExceptionDesc;

	std::list<CallStackItem*>		mCallStackList;

};

// namespace
}
}

#endif // !defined(INC_TT_ASSERT_EXCEPTIONHANDLER_H)
