// AssertHandler.cpp: implementation of the AssertHandler class.
//
//////////////////////////////////////////////////////////////////////

#include <tt/assert/assert.h>
#include <tt/assert/AssertHandler.h>
#include <tt/assert/Exceptionhandler.h>
#include <tt/assert/resource.h>

#include <winbase.h>
#include <commctrl.h>
#include <richedit.h>
#include <tchar.h>

#if defined(TT_PLATFORM_WIN)
// FIXME: Pointer conversion to greater size
#pragma warning (disable:4312)
#endif

extern HINSTANCE ghInstance;
extern tt::assert::Assert*	gAssert;

namespace tt {
namespace assert {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AssertHandler::AssertHandler()
{
	m_type = type_none;
	m_line = 0;
	m_hwnd = NULL;
	m_expHandler = NULL;
	m_debuggerPresent = false;
	m_isMuteAssertsChecked = true;
}

AssertHandler::~AssertHandler()
{
	ClearAllIgnoredAsserts();
}


// Function name	: AssertHandler::AddAssertToIgnoreList
// Description	    : Adds an assert to the list of ones tob e ignores
// 
// Return type		: void 
// Argument         : const char* p_file
// Argument         : const int line
void AssertHandler::AddAssertToIgnoreList(const char* p_file, int p_line)
{

	if (p_file == NULL) return;

	AssertInfo* info = new AssertInfo;

	info->file = p_file;
	info->lineNumber = p_line;

	m_ignoredAsserts.push_back(info);
}


// Function name	: AssertHandler::IsAssertInIgnoreList
// Description	    : Checks to see if a provided assert has been set to ignore
// 
// Return type		: bool 
// Argument         : const char* p_file
// Argument         : const int line
bool AssertHandler::IsAssertInIgnoreList(const char* p_file, int p_line) const
{
	assertList::const_iterator it, end;

	for (it = m_ignoredAsserts.begin(), 
		 end = m_ignoredAsserts.end(); it != end; ++it)
	{
		const AssertInfo* info = *it;

		if (info->lineNumber == p_line &&
			info->file == p_file)
		{
			return true;
		}
	}

	return false;
}



// Function name	: AssertHandler::ClearAllIgnoredAsserts
// Description	    : removes all asserts in the list to be ignored
// 
// Return type		: void 
// Argument         : void
void AssertHandler::ClearAllIgnoredAsserts( void )
{
	assertList::iterator it, end;

	for (it = m_ignoredAsserts.begin(), 
			end = m_ignoredAsserts.end(); it != end; ++it)
	{
		AssertInfo* info = *it;

		delete info;
	}

	m_ignoredAsserts.clear();
}


// Function name	: AssertHandler::CreateAssertDialog
// Description	    : Create and run the assert dialog
// 
// Return type		: int 
// Argument         : void
int AssertHandler::CreateAssertDialog( void )
{
	INT_PTR nResult;
	
	ShowCursor(TRUE);
	nResult = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_ASSERT), NULL,
	                         (DLGPROC)sDialogProc, (LPARAM)this);
	
	if (nResult <= 0)
	{
		char buf[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 256, 0);
		MessageBoxA(0, buf, "Error in creating dialog", MB_ICONWARNING | MB_OK);
	}
	
	ShowCursor(FALSE);
	return (int)nResult;
}



// Function name	: AssertHandler::HandleAssert
// Description	    : Handles an assert, if this func is reached it is assumed the assert is not in the ignore list.
// 
// Return type		: int 
// Argument         : ExceptionHandler* pHandler
// Argument         : const char* pExpression
// Argument         : const char* p_file
// Argument         : int line
// Argument         : const char* pComment
int AssertHandler::HandleAssert(ExceptionHandler* p_handler, const char* p_file, 
								int p_line, const char* p_function, 
								const char* p_message)
{

	m_expHandler = p_handler;
	m_message = p_message;
	m_file = p_file;
	m_function = p_function;

	m_line = p_line;

	m_type = type_assert;


	//
	// GRRRRRRRRRRRRRRRRRR!
	// 
	// IsDebuggerPresent despite being documented doesn't seem to be either defined
	// or linked (if I define it myself). It does exist under NT/98 so load
	// it dynamically
	typedef BOOL (WINAPI * dllIsDebuggerPresent)(void);

	// get the proc from kernel32
	dllIsDebuggerPresent IsDebuggerPresent = (dllIsDebuggerPresent)GetProcAddress( GetModuleHandle("KERNEL32"), "IsDebuggerPresent");

	if (IsDebuggerPresent)
	{
		m_debuggerPresent = (IsDebuggerPresent()==TRUE);
	}

	// create the dialog and return its exit value
	return CreateAssertDialog();
}



// Function name	: AssertHandler::OnInitDialog
// Description	    : Dialog initialisation
// 
// Return type		: void 
// Argument         : void
void AssertHandler::OnInitDialog( HWND hWnd )
{
	m_hwnd = hWnd;

	String<char> assertText(1024);

	// build the text
	assertText += "Assert failed in: ";
	assertText += m_expHandler->GetModuleName();

	// set the title of the dialog
	if (m_type == type_assert)
	{
		SetWindowText(hWnd, assertText);
	}
	else
	{
		SetWindowText(hWnd, "Exception failed!");
	}

	String<char> messageText(1024);

	// show the expression
	messageText += m_message;
	messageText += "\n";

	// show the line
	String<char> lineText(1024);
	String<char> strTemp;
	lineText += "File: ";
	lineText += m_file;
	strTemp.sprintf("\nLine: %d", m_line);
	lineText += strTemp;
	lineText += "\n\n";

	// show the comment text
	String<char> functionText(1024);

	// show the comment
	functionText += "Function:\n";
	functionText += m_function;
	functionText += "\n\n";

	// write to richedit control

	HWND hWndRich = GetDlgItem(hWnd, IDC_RICHTEXT);

	String<char> buildText(1024);
	strTemp.sprintf("Revision:\r\n%d.%d",
		gAssert->getClientRevision(),
		gAssert->getLibRevision());

	buildText += strTemp;
	buildText += "\r\n\r\n";

	SetDlgItemText(m_hwnd, IDC_BUILDINFO, buildText);

	// richedit example: http://www.winehq.org/pipermail/wine-devel/2004-August/029154.html
	CHARFORMAT cf;


//	SendMessage(hWndRich, EM_SETWORDWRAPMODE,0, (LPARAM)(WBF_WORDWRAP | WBF_WORDBREAK));

	// set font to Courier
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_FACE | CFM_SIZE;
	strcpy(cf.szFaceName, "Courier New");
	cf.yHeight = 200;
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cf);

	// add bold text
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_BOLD;
	cf.dwEffects = CFE_BOLD;
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)messageText);

	// line
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)lineText);

	// function
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)functionText);

	// build
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)buildText);

	// additional info
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)"*** Additional Info ***\n\n");

	// module
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)assertText);

	// call stack
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM)(LPCSTR)"\n\nCall stack:\n");

	/*
	// add strikeout text
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_UNDERLINE | CFM_STRIKEOUT;
	cf.dwEffects = CFE_STRIKEOUT;
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "strikeout");

	// add normal text
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) " normal\n");

	// add red text
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = RGB(255,0,0);
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "red ");

	// add green text
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = RGB(0,255,0);
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "green ");

	// add blue text
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = RGB(0,0,255);
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "blue\n");

	// try some fonts
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_FACE | CFM_SIZE;
	cf.yHeight = 300;
	strcpy(cf.szFaceName, "Tahoma");
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "tahoma ");

	// try some fonts
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_FACE;
	strcpy(cf.szFaceName, "Helvetica");
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "Helvetica ");

	// try some fonts
	memset( &cf, 0, sizeof cf );
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_FACE;
	strcpy(cf.szFaceName, "Courier");
	SendMessage( hWndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
	SendMessage( hWndRich, EM_REPLACESEL, FALSE, (LPARAM) "Courier ");
	*/

	// handle list
	HWND hWndList = GetDlgItem(hWnd, IDC_CALLSTACK);

	// add the call stack the the listbox
	ExceptionHandler::stackIterator it = m_expHandler->GetCallStackBegin();
	ExceptionHandler::stackIterator end = m_expHandler->GetCallStackEnd();

	int count = 0;

	while (it != end)
	{
		ExceptionHandler::CallStackItem* pItem = *it;

		count++;

		if (count > 3)
		{
			LRESULT index = SNDMSG(hWndList, LB_ADDSTRING, 0, (LPARAM)pItem->functionName.data());

			if (index != LB_ERR)
			{
				// store the item in it's data
				SNDMSG(hWndList, LB_SETITEMDATA, index, (LPARAM)pItem);

				// and send it to the richedit textbox
				String<char> str;
				if (pItem->fileName.length() > 0)
				{
					str.sprintf("%d> %s (%s, %d)\n", count-3, pItem->functionName.data(), 
						pItem->fileName.data(), pItem->lineNumber);
				}
				else
				{
					str.sprintf("%d> %s\n", count-3, pItem->functionName.data());
				}

				SendMessage( hWndRich, EM_REPLACESEL, FALSE, 
					(LPARAM)(LPCSTR)str);
			}
		}

		++it;
	}
	
	if (m_isMuteAssertsChecked)
	{
		SNDMSG(GetDlgItem(m_hwnd, IDC_MUTEASSERTS), BM_SETCHECK, BST_CHECKED, 0);
	}
	else
	{
		SNDMSG(GetDlgItem(m_hwnd, IDC_MUTEASSERTS), BM_SETCHECK, BST_UNCHECKED, 0);
	}
	
	// select the first line in the listbox
	SNDMSG(hWndList, LB_SETCURSEL, 0, 0);

	// fake a command message so it'll set the file display up
	SNDMSG(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_CALLSTACK, LBN_SELCHANGE), (LPARAM)hWndList);


	// is their a debugger present? Less options if not
	if (m_debuggerPresent == false)
	{
		// can't debug
		EnableWindow(GetDlgItem(m_hwnd, ID_DEBUG), FALSE);
	}

	SetFocus(m_hwnd);
	SetFocus(GetDlgItem(m_hwnd, ID_IGNORE));
}



// Function name	: AssertHandler::OnNotify
// Description	    : Process notification messages for this dialog, at present the only one we
//						are interested in is selection of items in the lsitbox
// 
// Return type		: void 
// Argument         : WPARAM wParam
// Argument         : LPARAM lParam
void AssertHandler::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// only interested in notifications from the listbox
	switch (LOWORD(wParam))
	{
		case IDC_CALLSTACK:

			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
		
				// type of notification?
				HWND hwndList = (HWND)lParam; 

				// selection has changed, get the data for the 
				LRESULT index = SNDMSG(hwndList, LB_GETCURSEL, 0, 0);

				// assume blank
				SetDlgItemText(m_hwnd, IDC_LINE, "");

				if (index != LB_ERR)
				{
					ExceptionHandler::CallStackItem *pItem;

					// get the pointer we shoved in here
					LPARAM param = SNDMSG(hwndList, LB_GETITEMDATA, index, 0);

					if (param != LB_ERR)
					{
						pItem = (ExceptionHandler::CallStackItem*)param;

						if (pItem && pItem->fileName.length())
						{
							String<char> str;

							str.sprintf("%s, %d", pItem->fileName.data(), pItem->lineNumber);

							// set the text on the line number
							SetDlgItemText(m_hwnd, IDC_LINE, str);
						}
					}
				}
			}
			break;

		//
		// pass the return value back to the callee as one of the known return values
		//
		
		case ID_DEBUG:
			m_isMuteAssertsChecked = IsDlgButtonChecked(m_hwnd, IDC_MUTEASSERTS) == BST_CHECKED;
			EndDialog(m_hwnd, ID_ASSERT_DEBUG);
			break;

		case ID_IGNORE:
			m_isMuteAssertsChecked = IsDlgButtonChecked(m_hwnd, IDC_MUTEASSERTS) == BST_CHECKED;
			EndDialog(m_hwnd, m_isMuteAssertsChecked ? 
					ID_ASSERT_IGNORE_MUTE : ID_ASSERT_IGNORE);
			break;

		case ID_EXIT:
			m_isMuteAssertsChecked = IsDlgButtonChecked(m_hwnd, IDC_MUTEASSERTS) == BST_CHECKED;
			EndDialog(m_hwnd, ID_ASSERT_EXIT);
			break;

	}
}


// Function name	: AssertHandler::DialogProc
// Description	    : Message processor for the assert dialog
// 
// Return type		: int 
// Argument         : HWND hWnd
// Argument         : UINT uMsg
// Argument         : WPARAM wParam
// Argument         : LPARAM lParam
int AssertHandler::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			OnInitDialog(hWnd);
			break;
			
		case WM_COMMAND:
			OnCommand(wParam, lParam);
			break;
			
		case WM_SYSCOMMAND:
			// if the sys close (the ickle X) is pressed then ignore
			if (wParam == SC_CLOSE)
			{
				EndDialog(hWnd, IsDlgButtonChecked(m_hwnd, IDC_MUTEASSERTS) ? 
						ID_ASSERT_IGNORE_MUTE : ID_ASSERT_IGNORE);
			}
			break;
			
		case WM_DESTROY:
			// do we ignore this in future?
			if (IsDlgButtonChecked(hWnd, IDC_IGNOREALWAYS))
			{
				AddAssertToIgnoreList(m_file, m_line);
			}
			break;
	}


	return 0;
}



// Function name	: AssertHandler::sDialogProc
// Description	    : static function to handle callbacks from the assert dialog
// 
// Return type		: int WINAPI 
// Argument         : HWND hWnd
// Argument         : UINT uMsg
// Argument         : WPARAM wParam
// Argument         : LPARAM lParam
int WINAPI AssertHandler::sDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	int ret = 0;

	if (uMsg == WM_INITDIALOG)
	{
		AssertHandler* pHandler = (AssertHandler*)lParam;

		// store a pointer to the handler in the user data
		SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

		pHandler->DialogProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		AssertHandler* pHandler = (AssertHandler*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		if (pHandler)
		{
			// allow the class to handle this message
			ret = pHandler->DialogProc(hWnd, uMsg, wParam, lParam);

			// if destroy, remove the pointer so no more messages will be passed
			if (uMsg == WM_DESTROY)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			}
		}
	}


	return ret;
}

// namespace
}
}
