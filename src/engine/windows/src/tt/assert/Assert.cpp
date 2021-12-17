#include <tt/assert/Assert.h>

namespace tt {
namespace assert {

Assert::Assert()
: 
m_richEdit(0),
m_libRevision(0),
m_clientRevision(0),
m_parentWindow(0)
{
	INITCOMMONCONTROLSEX commonControls;
	commonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commonControls.dwICC  = ICC_STANDARD_CLASSES | 
		ICC_LINK_CLASS | ICC_WIN95_CLASSES;
	
	InitCommonControlsEx(&commonControls);
	
	m_richEdit = LoadLibraryA("Riched20.dll");
	if (m_richEdit == NULL)
	{
		char buf[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 256, 0);
		MessageBoxA(0, buf, "Error Loading Riched20.dll", MB_ICONWARNING | MB_OK);
	}
}


Assert::~Assert()
{
	FreeLibrary(m_richEdit);
	m_richEdit = 0;
}


bool Assert::isAssertInIgnoreList(const char* p_file, int p_line) const
{
	return m_assertHandler.IsAssertInIgnoreList(p_file, p_line);
}


int Assert::handleAssert(const char* p_file, int p_line,
                         const char* p_function, const char* p_message)
{
	if (m_assertHandler.IsAssertInIgnoreList(p_file, p_line))
	{
		return ID_ASSERT_IGNORE;			// nothing to do
	}
	
	int dlgReturn = ID_ASSERT_IGNORE;		// default return value
	
	__try
	{
		// generate an exception to catch the exception state
		RaiseException(123, 0, 0, NULL);
	}
	__except ( m_exceptionHandler.DumpCrash( GetCurrentThread(), GetExceptionInformation() ) )
	{
		// handle the assert
		dlgReturn = m_assertHandler.HandleAssert(
						&m_exceptionHandler, p_file, 
						p_line, p_function, p_message);
	}
	
	return dlgReturn;
}

// Namespace end
}
}
