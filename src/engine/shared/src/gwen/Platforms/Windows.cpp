/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/

#ifdef _WIN32

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501
#else
#	if _WIN32_WINNT < 0x0501
#		error Unsupported platform
#	endif
#endif

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"
#include "Gwen/Input/Windows.h"

#include <windows.h>
#include <ShlObj.h>

#include <tt/input/MouseController.h>


using namespace Gwen;
using namespace Gwen::Platform;

static const size_t FileStringSize		= 256;
static const size_t FilterBufferSize	= 512;

static Gwen::Input::Windows GwenInput;

static LPCTSTR iCursorConversion[] =
{
	IDC_ARROW,
	IDC_IBEAM,
	IDC_SIZENS,
	IDC_SIZEWE,
	IDC_SIZENWSE,
	IDC_SIZENESW,
	IDC_SIZEALL,
	IDC_NO,
	IDC_WAIT,
	IDC_HAND
};

void Gwen::Platform::SetCursor( unsigned char iCursor )
{
	if (iCursor != 255)  // Two Tribes change: interpret 255 as "don't set a cursor"
	{
		// Todo.. Properly.
		// Two Tribes change: use MouseController to set the cursor,
		// so that the WM_SETCURSOR message is also handled properly
		tt::input::MouseController::setCustomCursor(LoadCursor(0, iCursorConversion[iCursor]));
		//::SetCursor( LoadCursor( NULL, iCursorConversion[iCursor] ) );
	}
}

void Gwen::Platform::GetCursorPos( Gwen::Point & po )
{
	POINT p;
	::GetCursorPos( &p );
	po.x = p.x;
	po.y = p.y;
}

void Gwen::Platform::GetDesktopSize( int & w, int & h )
{
	w = GetSystemMetrics( SM_CXFULLSCREEN );
	h = GetSystemMetrics( SM_CYFULLSCREEN );
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	if ( !OpenClipboard( NULL ) ) { return L""; }

	HANDLE hData = GetClipboardData( CF_UNICODETEXT );

	if ( hData == NULL )
	{
		CloseClipboard();
		return L"";
	}

	wchar_t* buffer = ( wchar_t* ) GlobalLock( hData );
	UnicodeString str = buffer;
	GlobalUnlock( hData );
	CloseClipboard();
	return str;
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString & str )
{
	if ( !OpenClipboard( NULL ) ) { return false; }

	EmptyClipboard();
	// Create a buffer to hold the string
	size_t iDataSize = ( str.length() + 1 ) * sizeof( wchar_t );
	HGLOBAL clipbuffer = GlobalAlloc( GMEM_DDESHARE, iDataSize );
	// Copy the string into the buffer
	wchar_t* buffer = ( wchar_t* ) GlobalLock( clipbuffer );
	wcscpy( buffer, str.c_str() );
	GlobalUnlock( clipbuffer );
	// Place it on the clipboard
	SetClipboardData( CF_UNICODETEXT, clipbuffer );
	CloseClipboard();
	return true;
}

double GetPerformanceFrequency()
{
	static double Frequency = 0.0f;

	if ( Frequency == 0.0f )
	{
		__int64 perfFreq;
		QueryPerformanceFrequency( ( LARGE_INTEGER* ) &perfFreq );
		Frequency = 1.0 / ( double ) perfFreq;
	}

	return Frequency;
}

float Gwen::Platform::GetTimeInSeconds()
{
	static float fCurrentTime = 0.0f;
	static __int64 iLastTime = 0;
	__int64 thistime;
	QueryPerformanceCounter( ( LARGE_INTEGER* ) &thistime );
	float fSecondsDifference = ( double )( thistime - iLastTime ) * GetPerformanceFrequency();

	if ( fSecondsDifference > 0.1f ) { fSecondsDifference = 0.1f; }

	fCurrentTime += fSecondsDifference;
	iLastTime = thistime;
	return fCurrentTime;
}



bool Gwen::Platform::FileOpen( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	char Filestring[FileStringSize];
	String returnstring;

	char FilterBuffer[FilterBufferSize];
	{
		memset( FilterBuffer, 0, sizeof( FilterBuffer ) );
		memcpy( FilterBuffer, Extension.c_str(), Gwen::Min( Extension.length(), sizeof( FilterBuffer ) ) );

		for ( int i = 0; i < FilterBufferSize; i++ )
		{
			if ( FilterBuffer[i] == '|' )
			{ FilterBuffer[i] = 0; }
		}
	}
	OPENFILENAMEA opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = FilterBuffer;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = FileStringSize;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle = 50;
	opf.lpstrInitialDir = StartPath.c_str();
	opf.lpstrTitle = Name.c_str();
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = "*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR ) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof( OPENFILENAME );

	if ( GetOpenFileNameA( &opf ) )
	{
		if ( pHandler && fnCallback )
		{
			Gwen::Event::Information info;
			info.Control		= NULL;
			info.ControlCaller	= NULL;
			info.String			= opf.lpstrFile;
			( pHandler->*fnCallback )( info );
		}
	}

	return true;
}

// An annoying function just to change the folder that we start in, in the folder browser.
static String g_InitialFolder;

INT CALLBACK FolderBrowseCallback( HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM /*pData*/ )
{
	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
			{
				WCHAR szDir[MAX_PATH];

				if ( g_InitialFolder.empty() && GetCurrentDirectoryW( MAX_PATH, szDir) )
				{
					SendMessageW( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) szDir );
					return 0;
				}

				SendMessageW( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) Gwen::Utility::StringToUnicode( g_InitialFolder ).c_str() );
				return 0;
			}

			break;
	}

	return 0;
}

bool Gwen::Platform::FolderOpen( const String& Name, const String& StartPath, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	g_InitialFolder = StartPath;

	BROWSEINFOA   bi; 
	ZeroMemory(&bi,   sizeof(bi)); 

	bi.lpszTitle        =   Name.c_str(); 
	bi.ulFlags          =   BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lpfn				=	&FolderBrowseCallback;

	LPITEMIDLIST pidl = SHBrowseForFolderA( &bi );

	if ( pidl == NULL )
		return true;

	char szPathName[MAX_PATH]; 
	if( !SHGetPathFromIDListA( pidl, szPathName ) )
		return true;

	if ( pHandler && fnCallback )
	{
		Gwen::Event::Information info;
		info.Control		= NULL;
		info.ControlCaller	= NULL;
		info.String			= szPathName;

		(pHandler->*fnCallback)( info );
	}

	return true;
}

bool Gwen::Platform::FileSave( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionWithInformation fnCallback )
{
	char Filestring[FileStringSize] = { 0 };
	
	// Two Tribes modification: parse StartPath as full path including filename.
	// Set filename part as initial filename in save dialog, use directory part as initial directory
	// Further tweaks: do not set the default file extension to "*.*" (it is an invalid extension anyway)
	String startDir(StartPath);
	{
		String::size_type dirPos = startDir.find_last_of("\\/");
		if (dirPos != std::string::npos)
		{
			startDir.erase(dirPos);
			++dirPos; // move past the last directory separator for the filename
			memcpy( Filestring, StartPath.c_str() + dirPos, Gwen::Min( StartPath.length() - dirPos, FileStringSize - 1 ) );
		}
	}

	char FilterBuffer[FilterBufferSize];
	{
		memset( FilterBuffer, 0, sizeof( FilterBuffer ) );
		memcpy( FilterBuffer, Extension.c_str(), Gwen::Min( Extension.size(), sizeof( FilterBuffer ) ) );

		for ( int i = 0; i < FilterBufferSize; i++ )
		{
			if ( FilterBuffer[i] == '|' )
			{ FilterBuffer[i] = 0; }
		}
	}
	OPENFILENAMEA opf;
	opf.hwndOwner = 0;
	opf.lpstrFilter = FilterBuffer;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
	opf.nMaxFile = FileStringSize;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle = 50;
	opf.lpstrInitialDir = startDir.c_str();
	opf.lpstrTitle = Name.c_str();
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = NULL;
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR ) & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof( OPENFILENAME );

	if ( GetSaveFileNameA( &opf ) )
	{
		if ( pHandler && fnCallback )
		{
			Gwen::Event::Information info;
			info.Control		= NULL;
			info.ControlCaller	= NULL;
			info.String			= opf.lpstrFile;
			( pHandler->*fnCallback )( info );
		}
	}

	return true;
}


void* Gwen::Platform::CreatePlatformWindow( int x, int y, int w, int h, const Gwen::String & strWindowTitle )
{
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	WNDCLASSA	wc;
	ZeroMemory( &wc, sizeof( wc ) );
	wc.style			= CS_OWNDC | CS_DROPSHADOW;
	wc.lpfnWndProc		= DefWindowProc;
	wc.hInstance		= GetModuleHandle( NULL );
	wc.lpszClassName	= "GWEN_Window_Class";
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
	RegisterClassA( &wc );
	HWND hWindow = CreateWindowExA( WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, wc.lpszClassName, strWindowTitle.c_str(), WS_POPUP | WS_VISIBLE, x, y, w, h, NULL, NULL, GetModuleHandle( NULL ), NULL );
	ShowWindow( hWindow, SW_SHOW );
	SetForegroundWindow( hWindow );
	SetFocus( hWindow );
	// Curve the corners
	{
		HRGN rgn = CreateRoundRectRgn( 0, 0, w + 1, h + 1, 4, 4 );
		SetWindowRgn( hWindow, rgn, false );
	}
	return ( void* ) hWindow;
}

void Gwen::Platform::DestroyPlatformWindow( void* pPtr )
{
	DestroyWindow( ( HWND ) pPtr );
	CoUninitialize();
}

void Gwen::Platform::MessagePump( void* pWindow, Gwen::Controls::Canvas* ptarget )
{
	GwenInput.Initialize( ptarget );
	MSG msg;

	while ( PeekMessage( &msg, ( HWND ) pWindow, 0, 0, PM_REMOVE ) )
	{
		if ( GwenInput.ProcessMessage( msg ) )
		{ continue; }

		if ( msg.message == WM_PAINT )
		{
			ptarget->Redraw();
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	// If the active window has changed then force a redraw of our canvas
	// since we might paint ourselves a different colour if we're inactive etc
	{
		static HWND g_LastFocus = NULL;

		if ( GetActiveWindow()  != g_LastFocus )
		{
			g_LastFocus = GetActiveWindow();
			ptarget->Redraw();
		}
	}
}

void Gwen::Platform::SetBoundsPlatformWindow( void* pPtr, int x, int y, int w, int h )
{
	SetWindowPos( ( HWND ) pPtr, HWND_NOTOPMOST, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING );
	// Curve the corners
	{
		HRGN rgn = CreateRoundRectRgn( 0, 0, w + 1, h + 1, 4, 4 );
		SetWindowRgn( ( HWND ) pPtr, rgn, false );
	}
}

void Gwen::Platform::SetWindowMaximized( void* pPtr, bool bMax, Gwen::Point & pNewPos, Gwen::Point & pNewSize )
{
	if ( bMax )
	{
		ShowWindow( ( HWND ) pPtr, SW_SHOWMAXIMIZED );
		RECT rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, 0 );  // size excluding task bar
		SetWindowPos( ( HWND ) pPtr, HWND_NOTOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING );
		// Remove the corner curves
		{
			SetWindowRgn( ( HWND ) pPtr, NULL, false );
		}
	}
	else
	{
		ShowWindow( ( HWND ) pPtr, SW_RESTORE );
		// Curve the corners
		{
			RECT r;
			GetWindowRect( ( HWND ) pPtr, &r );
			HRGN rgn = CreateRoundRectRgn( 0, 0, ( r.right - r.left ) + 1, ( r.bottom - r.top ) + 1, 4, 4 );
			SetWindowRgn( ( HWND ) pPtr, rgn, false );
		}
	}

	RECT r;
	GetWindowRect( ( HWND ) pPtr, &r );
	pNewSize.x = r.right - r.left;
	pNewSize.y = r.bottom - r.top ;
	pNewPos.x = r.left;
	pNewPos.y = r.top;
}

void Gwen::Platform::SetWindowMinimized( void* pPtr, bool bMinimized )
{
	if ( bMinimized )
	{
		ShowWindow( ( HWND ) pPtr, SW_SHOWMINIMIZED );
	}
	else
	{
		ShowWindow( ( HWND ) pPtr, SW_RESTORE );
	}
}

bool Gwen::Platform::HasFocusPlatformWindow( void* pPtr )
{
	return GetActiveWindow() == ( HWND ) pPtr;
}

void Gwen::Platform::Sleep( unsigned int iMS )
{
	::Sleep( iMS );
}

#endif // WIN32
