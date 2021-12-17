#if !defined(INC_TT_ASSERT_ASSERT_H)
#define INC_TT_ASSERT_ASSERT_H


// these are the four return values possible from the assert dialog
#define ID_ASSERT_DEBUG			1
#define ID_ASSERT_IGNORE		2
#define ID_ASSERT_IGNORE_MUTE	3
#define ID_ASSERT_EXIT			4

#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>

#include <tt/assert/ExceptionHandler.h>
#include <tt/assert/AssertHandler.h>

namespace tt {
namespace assert {

class Assert
{
public:
	Assert();
	~Assert();
	
	void setLibRevision(int p_revision)
	{
		m_libRevision = p_revision;
	}
	
	void setClientRevision(int p_revision)
	{
		m_clientRevision = p_revision;
	}
	
	int getLibRevision()
	{
		return m_libRevision;
	}
	
	int getClientRevision()
	{
		return m_clientRevision;
	}
	
	inline void setParentWindow(HWND p_window) { m_parentWindow = p_window; }
	inline HWND getParentWindow() const        { return m_parentWindow;     }
	
	bool isAssertInIgnoreList(const char* p_file, int p_line) const;
	
	int handleAssert(const char* p_file, int p_line,
	                 const char* p_function, const char* p_message);
	
private:
	ExceptionHandler m_exceptionHandler;
	AssertHandler m_assertHandler;
	
	HMODULE m_richEdit;
	int m_libRevision;
	int m_clientRevision;
	HWND m_parentWindow;
};
/*
// only defined in debug versions
#ifdef _DEBUG


	int StdAssert(const char* pExpression, const char* pFile, const int line, const char* pComment);


	#define ASSERT_MACRO(x, str) if (!(x)) 	\
				if (StdAssert(#x, __FILE__, __LINE__, str) == ID_ASSERT_DEBUG) \
						__asm int 3; \

	// seperate debug levels to allow different levels of asserts to be discarded
	#ifdef _DEBUGLEVEL1
		#define ASSERT1(x) ASSERT_MACRO(x, "")
		#define ASSERTS1(x, str) ASSERT_MACRO(x, str)
	#else
		#define ASSERT1(x) ((void)0)
		#define ASSERTS1(x, str) ((void)0)
	#endif	// _DEBUGLEVEL1


	#ifdef _DEBUGLEVEL2
		#define ASSERT2(x) ASSERT_MACRO(x, "")
		#define ASSERTS2(x, str) ASSERT_MACRO(x, str)
	#else
		#define ASSERT2(x) ((void)0)
		#define ASSERTS2(x, str) ((void)0)
	#endif	// _DEBUGLEVEL2

	#ifdef _DEBUGLEVEL3
		#define ASSERT3(x) ASSERT_MACRO(x, "")
		#define ASSERTS3(x, str) ASSERT_MACRO(x, str)
	#else
		#define ASSERT3(x) ((void)0)
		#define ASSERTS3(x, str) ((void)0)
	#endif	// _DEBUGLEVEL3


	#define ASSERT(x) ASSERT_MACRO(x, "")
	#define ASSERTS(x, str) ASSERT_MACRO(x, str)

#else

	#define ASSERT1(x) ((void)0)
	#define ASSERTS1(x, str) ((void)0)

	#define ASSERT2(x) ((void)0)
	#define ASSERTS2(x, str) ((void)0)

	#define ASSERT3(x) ((void)0)
	#define ASSERTS3(x, str) ((void)0)

	#define ASSERT(x) ((void)0)
	#define ASSERTS(x, str) ((void)0)


#endif	// _DEBUG
*/

// namespace
}
}

#endif // !defined(INC_TT_ASSERT_ASSERT_H)
