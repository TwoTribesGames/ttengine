// ExceptionHandler.cpp: implementation of the ExceptionHandler class.
//
//////////////////////////////////////////////////////////////////////

#include <tt/assert/ExceptionHandler.h>
#include <vector>

#if defined(TT_PLATFORM_WIN)
// FIXME: Pointer truncation
#pragma warning (disable:4311)
#pragma warning (disable:4302)
#endif

namespace tt {
namespace assert {

void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid );

// Have to do thois because of the code added underneath to deal with Win9x

typedef DWORD	(__stdcall *dllSymLoadModule)				(HANDLE, HANDLE,PSTR,PSTR,DWORD,DWORD);

static dllSymLoadModule sSymLoadModule;	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



// Function name	: ExceptionHandler::InitClass
// Description	    : initialise the class
// 
// Return type		: void 
// Argument         : void
void ExceptionHandler::InitClass( void )
{
	mHImageHlpDll = NULL;

	SymInitialize = NULL;
	SymCleanup = NULL;
	SymSetOptions = NULL;
	StackWalk = NULL;
	SymFunctionTableAccess = NULL;
	SymGetModuleBase = NULL;
	SymGetSymFromAddr = NULL;
	SymGetLineFromAddr = NULL;
	UnDecorateSymbolName = NULL;
	sSymLoadModule = NULL;
	
}




// Function name	: ExceptionHandler::InitDLL
// Description	    : Loads imagehelp.dll and retrieves pointers to the functions we need
// 
// Return type		: bool 
// Argument         : void
bool ExceptionHandler::InitDLL( void )
{
	// load imagehlp.dll
	mHImageHlpDll= LoadLibrary("dbghelp.dll");

	if (mHImageHlpDll == NULL) return false;			// could not load the dll


	// get procedures from dll
	SymInitialize = (dllSymInitializePrc)GetProcAddress(mHImageHlpDll,"SymInitialize");
	SymCleanup = (dllSymCleanupPrc)GetProcAddress(mHImageHlpDll,"SymCleanup");
	SymSetOptions = (dllSymSetOptionsPrc)GetProcAddress(mHImageHlpDll,"SymSetOptions");
	StackWalk = (dllStackWalkPrc)GetProcAddress(mHImageHlpDll,"StackWalk");
	SymFunctionTableAccess = (dllSymFunctionTableAccessPrc)GetProcAddress(mHImageHlpDll,"SymFunctionTableAccess");
	SymGetModuleBase = (dllSymGetModuleBasePrc)GetProcAddress(mHImageHlpDll,"SymGetModuleBase");
	SymGetSymFromAddr = (dllSymGetSymFromAddrPrc)GetProcAddress(mHImageHlpDll,"SymGetSymFromAddr");
	SymGetLineFromAddr = (dllSymGetLineFromAddrPrc)GetProcAddress(mHImageHlpDll,"SymGetLineFromAddr");
	UnDecorateSymbolName = (dllUnDecorateSymbolNamePrc)GetProcAddress(mHImageHlpDll,"UnDecorateSymbolName");

	sSymLoadModule = SymLoadModule = (dllSymLoadModule)GetProcAddress( mHImageHlpDll, "SymLoadModule" );


	return true;
}

ExceptionHandler::ExceptionHandler()
{

	InitClass();
	InitDLL();

	mSearchPath.allocate(2048);	

	// fist build the search path for symbols

	char *tt = 0, *p;

	const int TTBUFLEN = 0xFFFF;

	tt = new char[TTBUFLEN]; 

	mSearchPath = "";

	// current directory
	if ( GetCurrentDirectory( TTBUFLEN, tt ) )
	{
		mSearchPath += tt;
		mSearchPath +=  ";" ;
	}

	// dir with executable
	if ( GetModuleFileName( 0, tt, TTBUFLEN ) )
	{
		for ( p = tt + strlen( tt ) - 1; p >= tt; -- p )
		{
			// locate the rightmost path separator
			if ( *p == '\\' || *p == '/' || *p == ':' )
				break;
		}
		// if we found one, p is pointing at it; if not, tt only contains
		// an exe name (no path), and p points before its first byte
		if ( p != tt ) // path sep found?
		{
			if ( *p == ':' ) // we leave colons in place
				++ p;
			*p = '\0'; // eliminate the exe name and last path sep
			mSearchPath += tt;
			mSearchPath +=  ";" ;
		}
	}
	// environment variable _NT_SYMBOL_PATH
	if ( GetEnvironmentVariable( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
	{
		mSearchPath += tt;
		mSearchPath +=  ";" ;
	}
	
	// environment variable _NT_ALTERNATE_SYMBOL_PATH
	if ( GetEnvironmentVariable( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
	{
		mSearchPath += tt;
		mSearchPath +=  ";" ;
	}

	// environment variable SYSTEMROOT
	if ( GetEnvironmentVariable( "SYSTEMROOT", tt, TTBUFLEN ) )
	{
		mSearchPath += tt;
		mSearchPath +=  ";" ;
	}

	delete [] tt;
}

ExceptionHandler::~ExceptionHandler()
{
	// clear exception and call stack
	ClearExceptions();

	FreeLibrary(mHImageHlpDll);
}




// Function name	: ExceptionHandler::DumpCrash
// Description	    : retrieves information about the exception state of a thread
// 
// Return type		: DWORD 
// Argument         : HANDLE hThread
// Argument         : void* pExceptionInfo
DWORD ExceptionHandler::DumpCrash(HANDLE hThread, void* pExceptionInfo)
{

	// clear all previous info
	ClearExceptions();

	HANDLE hProcess = GetCurrentProcess();

	// extract exception information
	EXCEPTION_POINTERS* pExcpPtrs = (EXCEPTION_POINTERS*)pExceptionInfo;
	CONTEXT* pContext = pExcpPtrs->ContextRecord;
	EXCEPTION_RECORD* pExcpRecord = pExcpPtrs->ExceptionRecord;

	// store the name of the module
	mModuleName.allocate(MAX_PATH);

	::GetModuleFileName(0, mModuleName, MAX_PATH);

	// get the exception description
	GetLastExceptionDesc(pExcpRecord->ExceptionCode, 
							pExcpRecord->ExceptionInformation[0],
							pExcpRecord->ExceptionInformation[1]);



	// initialize imagehlp.dll
	SymInitialize(hProcess, mSearchPath, FALSE);

	//printf("Search path is %s\n",mSearchPath.data());

	// undecorate automatically, include line info
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE);

	// load symbols
	enumAndLoadModuleSymbols( hProcess, GetCurrentProcessId() );

	// symbol buffer
	static char cSymbolBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_PATH];
	IMAGEHLP_SYMBOL* pSymbol=(IMAGEHLP_SYMBOL*)(cSymbolBuffer);

	// stackframe for StackWalk() 
	STACKFRAME sfStackFrame;
	CONTEXT ctxContext;

	::ZeroMemory(&sfStackFrame,sizeof(sfStackFrame));
#if !defined(_WIN64)
	ctxContext.Esp = pContext->Esp;
	ctxContext.Ebp = pContext->Ebp;
	ctxContext.Eip = pContext->Eip;

	sfStackFrame.AddrPC.Offset = pContext->Eip;
	sfStackFrame.AddrPC.Mode = AddrModeFlat;
	sfStackFrame.AddrFrame.Offset = pContext->Ebp;
	sfStackFrame.AddrFrame.Mode = AddrModeFlat;
#else
	ctxContext.Rsp = pContext->Rsp;
	ctxContext.Rbp = pContext->Rbp;
	ctxContext.Rip = pContext->Rip;

	sfStackFrame.AddrPC.Offset = pContext->Rip;
	sfStackFrame.AddrPC.Mode = AddrModeFlat;
	sfStackFrame.AddrFrame.Offset = pContext->Rbp;
	sfStackFrame.AddrFrame.Mode = AddrModeFlat;
#endif
	int FrameId = 0;

	while(FrameId < 20)	// make sure we dont die in this loop
	{
		PFUNCTION_TABLE_ACCESS_ROUTINE r = reinterpret_cast<PFUNCTION_TABLE_ACCESS_ROUTINE>(SymFunctionTableAccess);
		PGET_MODULE_BASE_ROUTINE b = reinterpret_cast<PGET_MODULE_BASE_ROUTINE>(SymGetModuleBase);
		BOOL bResult = StackWalk(IMAGE_FILE_MACHINE_I386,hProcess,hThread,
		        &sfStackFrame,&ctxContext,
				0,r,b,0);
		
		if(!bResult)
		{
			break;
		}
		

		/*

		Get the current module for the callstack walk?

  		HMODULE hInstance=(HMODULE)SymGetModuleBase(hProcess, sfStackFrame.AddrPC.Offset);

		if(hInstance == NULL)
		{
			continue;
		}

		if(GetModuleFileName(hInstance, ModuleName, MAX_PATH) == 0)
		{
			continue;
		}

		// dump module name if it differs from previous level
		if(ModuleName != LastModuleName)
		{
			// printf("%s\n\n", ModuleName.data(), FrameId);
			LastModuleName = ModuleName;
		}*/

		// setup symbol buffer
		::ZeroMemory(pSymbol,sizeof(IMAGEHLP_SYMBOL)+MAX_PATH);
		pSymbol->MaxNameLength=MAX_PATH;

		DWORD dwSymOffset;

		// get symbol
		if(SymGetSymFromAddr(hProcess,(DWORD)sfStackFrame.AddrPC.Offset,&dwSymOffset,pSymbol))
		{
			// decode function name, strip irrelevant information
			/*String<char> strFuncName(MAX_PATH);

			UnDecorateSymbolName(pSymbol->Name, strFuncName,
						MAX_PATH,UNDNAME_COMPLETE);*/

			IMAGEHLP_LINE imgHelpLine = {sizeof(imgHelpLine)};
		
			// try to get line information

			if(SymGetLineFromAddr(hProcess, (DWORD)sfStackFrame.AddrPC.Offset, &dwSymOffset, &imgHelpLine))
			{
				AddToCallStack(pSymbol->Name, imgHelpLine.FileName, imgHelpLine.LineNumber);
			}
			else
			{
				// couldn't get a filenamee, probably a DLL
				AddToCallStack(pSymbol->Name, "", dwSymOffset);
			}
		}
		else
		{
			// no symbol for this address
			AddToCallStack("????", "", 0);
		}

		FrameId++;
	};

	SymCleanup(hProcess);

	return EXCEPTION_EXECUTE_HANDLER;
}




// Function name	: ExceptionHandler::GetLastExceptionDesc
// Description	    : Gets a tectual description of an exception
// 
// Return type		: const char* 
// Argument         : DWORD dwCode
// Argument         : DWORD dwInfo0
// Argument         : DWORD dwInfo1
const char* ExceptionHandler::GetLastExceptionDesc(DWORD dwCode, ULONG_PTR dwInfo0, ULONG_PTR dwInfo1)
{

	mLastExceptionDesc.sprintf("Exception:0x%08lx : ", dwCode);

	switch(dwCode)
	{
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			mLastExceptionDesc += "ARRAY_BOUNDS_EXCEEDED";
			break;

		case EXCEPTION_BREAKPOINT:
			mLastExceptionDesc += "BREAKPOINT";
			break;

		case EXCEPTION_DATATYPE_MISALIGNMENT:
			mLastExceptionDesc += "DATATYPE_MISALIGNMENT";
			break;

		case EXCEPTION_FLT_DENORMAL_OPERAND:
			mLastExceptionDesc += "FLT_DENORMAL_OPERAND";
			break;

		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			mLastExceptionDesc += "FLT_DIVIDE_BY_ZERO";
			break;

		case EXCEPTION_FLT_INEXACT_RESULT:
			mLastExceptionDesc += "FLT_INEXACT_RESULT";
			break;

		case EXCEPTION_FLT_INVALID_OPERATION:
			mLastExceptionDesc += "";
			break;

		case EXCEPTION_FLT_OVERFLOW:
			mLastExceptionDesc += "FLT_OVERFLOW";
			break;

		case EXCEPTION_FLT_STACK_CHECK:
			mLastExceptionDesc += "FLT_STACK_CHECK";
			break;

		case EXCEPTION_FLT_UNDERFLOW:
			mLastExceptionDesc += "FLT_UNDERFLOW";
			break;

		case EXCEPTION_ILLEGAL_INSTRUCTION:
			mLastExceptionDesc += "ILLEGAL_INSTRUCTION";
			break;

		case EXCEPTION_IN_PAGE_ERROR:
			mLastExceptionDesc += "IN_PAGE_ERROR";
			break;

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			mLastExceptionDesc += "INT_DIVIDE_BY_ZERO";
			break;

		case EXCEPTION_INT_OVERFLOW:
			mLastExceptionDesc += "INT_OVERFLOW";
			break;

		case EXCEPTION_INVALID_DISPOSITION:
			mLastExceptionDesc += "INVALID_DISPOSITION";
			break;

		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			mLastExceptionDesc += "NONCONTINUABLE_EXCEPTION";
			break;

		case EXCEPTION_PRIV_INSTRUCTION:
			mLastExceptionDesc += "PRIV_INSTRUCTION";
			break;

		case EXCEPTION_SINGLE_STEP:
			mLastExceptionDesc += "SINGLE_STEP";
			break;

		case EXCEPTION_STACK_OVERFLOW:
			mLastExceptionDesc += "STACK_OVERFLOW";
			break;

		case EXCEPTION_ACCESS_VIOLATION:
			{
				String<char> temp;

				if(dwInfo0!=0)
				{
					temp.sprintf("ACCESS_VIOLATION: Write access at 0x%08lx",dwInfo1);
				}
				else
				{
					temp.sprintf("ACCESS_VIOLATION: Read access at 0x%08lx",dwInfo1);
				}

				mLastExceptionDesc += temp;
			}
			break;

		default:
			break;
	}	

	return mLastExceptionDesc;
};




// Function name	: ExceptionHandler::GetLastException
// Description	    : Returns the last excepttion string
// 
// Return type		: const char* 
// Argument         : void
const char* ExceptionHandler::GetLastException( void )
{
	return mLastExceptionDesc;
}



// Function name	: ExceptionHandler::GetModuleName
// Description	    : returns the module the exception occured in
// 
// Return type		: const char* 
// Argument         : void
const char* ExceptionHandler::GetModuleName( void )
{
	return mModuleName;
}



// Function name	: ExceptionHandler::AddToCallStack
// Description	    : Adds an item to the calsltack list
// 
// Return type		: void 
// Argument         : const char* funcName
// Argument         : const char* fileName
// Argument         : const int line
void ExceptionHandler::AddToCallStack(const char* funcName, const char* fileName, const int line)
{
	CallStackItem* newItem = new CallStackItem;

	newItem->functionName = funcName;
	newItem->fileName = fileName;
	newItem->lineNumber = line;

	mCallStackList.push_back(newItem);
}


// Function name	: ExceptionHandler::ClearExceptions
// Description	    : Clears all exception info
// 
// Return type		: void 
// Argument         : void
void ExceptionHandler::ClearExceptions( void )
{
	// clear the string info
	mLastExceptionDesc = "";
	mModuleName = "";

	// remove all the call stack info
	stackIterator it = mCallStackList.begin();
	stackIterator end = mCallStackList.end();

	while (it != end)
	{
		delete *it;

		++it;
	}

	mCallStackList.clear();
}


// Function name	: ExceptionHandler::GetCallStackBegin
// Description	    : returns the begin iterator for the callstack list
// 
// Return type		: ExceptionHandler::stackIterator 
ExceptionHandler::stackIterator ExceptionHandler::GetCallStackBegin()
{
	return mCallStackList.begin();
}


// Function name	: ExceptionHandler::GetCallStackEnd
// Description	    : returns the end iterator to the callstack list
// 
// Return type		: ExceptionHandler::stackIterator 
ExceptionHandler::stackIterator ExceptionHandler::GetCallStackEnd()
{
	return mCallStackList.end();
}



//  
//  hacky stuff I found on a web to load modules manually rather than rely 
//  on SymInitialise which fails on Win9x,	author alleges it woks but not 
//  on my TV. No harm in using it anyway I guess.....
//

struct ModuleEntry
{
	std::string imageName;
	std::string moduleName;
	DWORD baseAddress;
	DWORD size;
};


typedef std::vector< ModuleEntry > ModuleList;
typedef ModuleList::iterator ModuleListIter;
bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess );
bool fillModuleListTH32( ModuleList& modules, DWORD pid );
bool fillModuleListPSAPI( ModuleList& modules, DWORD pid, HANDLE hProcess );


#define gle (GetLastError())
#define lenof(a) (sizeof(a) / sizeof((a)[0]))
#define MAXNAMELEN 1024 // max name length for found symbols
#define IMGSYMLEN ( sizeof IMAGEHLP_SYMBOL )
#define TTBUFLEN 65536 // for a temp buffer


void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid )
{
	ModuleList modules;
	ModuleListIter it;
	char *img, *mod;

	// fill in module list
	fillModuleList( modules, pid, hProcess );

	for ( it = modules.begin(); it != modules.end(); ++ it )
	{
		// unfortunately, SymLoadModule() wants writeable strings
		img = new char[(*it).imageName.size() + 1];
		strcpy( img, (*it).imageName.c_str() );
		mod = new char[(*it).moduleName.size() + 1];
		strcpy( mod, (*it).moduleName.c_str() );

		
		if ( sSymLoadModule( hProcess, 0, img, mod, (*it).baseAddress, (*it).size ) == 0 )
		{
			/*printf( "Error %lu loading symbols for \"%s\"\n",
				gle, (*it).moduleName.c_str() );*/
		}
		else
		{
			//printf( "Symbols loaded: \"%s\"\n", (*it).moduleName.c_str() );
		}
		

		delete [] img;
		delete [] mod;
	}
}



bool fillModuleList( ModuleList& modules, DWORD pid, HANDLE hProcess )
{
	// try toolhelp32 first
	if ( fillModuleListTH32( modules, pid ) )
		return true;
	// nope? try psapi, then
	return fillModuleListPSAPI( modules, pid, hProcess );
}



// miscellaneous toolhelp32 declarations; we cannot #include the header
// because not all systems may have it
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008
#pragma pack( push, 8 )
typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;
#pragma pack( pop )



bool fillModuleListTH32( ModuleList& modules, DWORD pid )
{
	// CreateToolhelp32Snapshot()
	typedef HANDLE (__stdcall *tCT32S)( DWORD dwFlags, DWORD th32ProcessID );
	// Module32First()
	typedef BOOL (__stdcall *tM32F)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
	// Module32Next()
	typedef BOOL (__stdcall *tM32N)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );

	// I think the DLL is called tlhelp32.dll on Win9X, so we try both
	const char *dllname[] = { "kernel32.dll", "tlhelp32.dll" };
	HINSTANCE hToolhelp = 0;
	tCT32S pCT32S = 0;
	tM32F pM32F = 0;
	tM32N pM32N = 0;

	HANDLE hSnap;
	MODULEENTRY32 me = { sizeof me };
	bool keepGoing;
	ModuleEntry e;
	int i;

	for ( i = 0; i < lenof( dllname ); ++ i )
	{
		hToolhelp = LoadLibrary( dllname[i] );
		if ( hToolhelp == 0 )
			continue;
		pCT32S = (tCT32S) GetProcAddress( hToolhelp, "CreateToolhelp32Snapshot" );
		pM32F = (tM32F) GetProcAddress( hToolhelp, "Module32First" );
		pM32N = (tM32N) GetProcAddress( hToolhelp, "Module32Next" );
		if ( pCT32S != 0 && pM32F != 0 && pM32N != 0 )
			break; // found the functions!
		FreeLibrary( hToolhelp );
		hToolhelp = 0;
	}

	if ( hToolhelp == 0 ) // nothing found?
		return false;

	hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
	if ( hSnap == (HANDLE) -1 )
		return false;

	keepGoing = !!pM32F( hSnap, &me );
	while ( keepGoing )
	{
		// here, we have a filled-in MODULEENTRY32
		//printf( "%08lXh %6lu %-15.15s %s\n", me.modBaseAddr, me.modBaseSize, me.szModule, me.szExePath );
		e.imageName = me.szExePath;
		e.moduleName = me.szModule;
		e.baseAddress = (DWORD) me.modBaseAddr;
		e.size = me.modBaseSize;
		modules.push_back( e );
		keepGoing = !!pM32N( hSnap, &me );
	}

	CloseHandle( hSnap );

	FreeLibrary( hToolhelp );

	return modules.size() != 0;
}



// miscellaneous psapi declarations; we cannot #include the header
// because not all systems may have it
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;



bool fillModuleListPSAPI( ModuleList& modules, DWORD /*pid*/, HANDLE hProcess )
{
	// EnumProcessModules()
	typedef BOOL (__stdcall *tEPM)( HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
	// GetModuleFileNameEx()
	typedef DWORD (__stdcall *tGMFNE)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
	// GetModuleBaseName() -- redundant, as GMFNE() has the same prototype, but who cares?
	typedef DWORD (__stdcall *tGMBN)( HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
	// GetModuleInformation()
	typedef BOOL (__stdcall *tGMI)( HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

	HINSTANCE hPsapi;
	tEPM pEPM;
	tGMFNE pGMFNE;
	tGMBN pGMBN;
	tGMI pGMI;

	ModuleEntry e;
	DWORD cbNeeded;
	MODULEINFO mi;
	HMODULE *hMods = 0;
	char *tt = 0;

	hPsapi = LoadLibrary( "psapi.dll" );
	if ( hPsapi == 0 )
		return false;

	modules.clear();

	pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
	pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
	pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
	pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
	if ( pEPM == 0 || pGMFNE == 0 || pGMBN == 0 || pGMI == 0 )
	{
		// yuck. Some API is missing.
		FreeLibrary( hPsapi );
		return false;
	}

	hMods = new HMODULE[TTBUFLEN / sizeof HMODULE];
	tt = new char[TTBUFLEN];
	// not that this is a sample. Which means I can get away with
	// not checking for errors, but you cannot. :)

	if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
	{
		//printf( "EPM failed, gle = %lu\n", gle );
		goto cleanup;
	}

	if ( cbNeeded > TTBUFLEN )
	{
		//printf( "More than %lu module handles. Huh?\n", lenof( hMods ) );
		goto cleanup;
	}

	for ( unsigned int i = 0; i < cbNeeded / sizeof hMods[0]; ++ i )
	{
		// for each module, get:
		// base address, size
		pGMI( hProcess, hMods[i], &mi, sizeof mi );
		e.baseAddress = (DWORD) mi.lpBaseOfDll;
		e.size = mi.SizeOfImage;
		// image file name
		tt[0] = '\0';
		pGMFNE( hProcess, hMods[i], tt, TTBUFLEN );
		e.imageName = tt;
		// module name
		tt[0] = '\0';
		pGMBN( hProcess, hMods[i], tt, TTBUFLEN );
		e.moduleName = tt;
		/*printf( "%08lXh %6lu %-15.15s %s\n", e.baseAddress,
			e.size, e.moduleName.c_str(), e.imageName.c_str() );*/

		modules.push_back( e );
	}

cleanup:
	if ( hPsapi )
		FreeLibrary( hPsapi );
	delete [] tt;
	delete [] hMods;

	return modules.size() != 0;
}

// namespace
}
}
