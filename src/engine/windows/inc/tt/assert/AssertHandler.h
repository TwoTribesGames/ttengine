// AssertHandler.h: interface for the AssertHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(INC_TT_ASSERT_ASSERTHANDLER_H)
#define INC_TT_ASSERT_ASSERTHANDLER_H


#include <windows.h>
#include <list>
#include <tt/assert/String.h>

namespace tt {
namespace assert {

class ExceptionHandler;


class AssertHandler  
{

	struct AssertInfo
	{
		String<char>		file;
		int					lineNumber;

		AssertInfo()
		{
			lineNumber = 0;
		}
	};
	
	typedef std::list<AssertInfo*> assertList;


	enum type
	{
		type_none,
		type_assert,
		type_excepton,
	};


public:
	AssertHandler();
	virtual ~AssertHandler();
	
	bool IsAssertInIgnoreList(const char* p_file, int p_line) const;

	void ClearAllIgnoredAsserts( void );

	int HandleAssert(ExceptionHandler* p_handler, const char* p_file, 
					 int p_line, const char* p_function, 
					 const char* p_message);

	static int WINAPI sDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


private:

	void OnInitDialog( HWND hWnd );
	void OnCommand(WPARAM wParam, LPARAM lParam);
	int CreateAssertDialog( void );
	void AddAssertToIgnoreList(const char* p_file, int p_line);


	assertList			m_ignoredAsserts;					// list of ignored asserts

	String<char>		m_message;							// expression/message of the current assert
	String<char>		m_file;								// file the current assert is in
	String<char>		m_function;							// function for the current assert
	int					m_line;						// line no of the current assert
	bool				m_debuggerPresent;					// is a debugger present
	
	HWND				m_hwnd;								// handle to the assert dialog
	ExceptionHandler*	m_expHandler;						// exception handler
	type				m_type;								// type, excpetion or assert
	bool                m_isMuteAssertsChecked;
		

};

// namespace
}
}

#endif // !defined(INC_TT_ASSERT_ASSERTHANDLER_H)
