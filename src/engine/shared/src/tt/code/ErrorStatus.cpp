#include <limits>
#include <algorithm>

#include <tt/platform/tt_error.h>

#include <tt/code/ErrorStatus.h>


namespace tt {
namespace code {

#if !defined(TT_BUILD_FINAL)
thread::Mutex      ErrorStatus::ms_mutex;
std::ostringstream ErrorStatus::ms_nullStream;
#endif

//--------------------------------------------------------------------------------------------------
// Public member functions

ErrorStatus::ErrorStatus(const std::string& p_locationStr)
:
m_parent(0),
m_hasError(false),
m_ignoreErrors(false),
#ifndef TT_BUILD_FINAL
m_locInfo(),
m_locationStr(),
m_errorInfo(),
m_errorMsg(),
m_warnings()
#else
m_warnings(0)
#endif
{
#ifndef TT_BUILD_FINAL
	m_locationStr << p_locationStr;
#else
	(void)p_locationStr;
#endif
}


ErrorStatus::ErrorStatus(ErrorStatus* p_parent, const std::string& p_locationStr)
:
m_parent(p_parent),
m_hasError(false),
m_ignoreErrors(p_parent == 0),
#ifndef TT_BUILD_FINAL
m_errorMsg()
#else
m_warnings(0)
#endif
{
#ifndef TT_BUILD_FINAL
	m_locationStr << p_locationStr;
#else
	(void)p_locationStr;
#endif
	
	if (p_parent != 0 && p_parent->hasError())
	{
		m_hasError = true;
#ifndef TT_BUILD_FINAL
		m_errorMsg << "Parent has error";
#endif
	}
}


ErrorStatus::~ErrorStatus()
{
	if (m_parent != 0)
	{
		if (m_parent->hasError() == false)
		{
			if (m_hasError)
			{
				// At this point the client code had a change to stream the error message.
				// Add the message to the error info.
#ifndef TT_BUILD_FINAL
				m_errorInfo.setMsg(m_errorMsg.str());
				m_parent->childHasError(m_errorInfo);
#else
				m_parent->childHasError(SrcPosNested());
#endif
			}
			if (hasWarnings())
			{
				m_parent->childHasWarnings(m_warnings);
			}
		}
		else
		{
			// If the parent is in an error state then this child may not have had an error or warning.
			// The check for error is done when triggering an error. (Check for double error trigger.)
			// The check for warning is done here.
			TT_ASSERT(hasWarnings() == false);
		}
	}
}


std::string ErrorStatus::getErrorMessage(ReportLevel p_reportLevel) const
{
#ifndef TT_BUILD_FINAL
	// This old code has the error message twice.
	//std::ostringstream oss;
	//oss << m_errorMsg.str() << "\n";
	//oss << m_errorInfo.getFullMessage(p_withSourceInfo);
	//return oss.str();
	
	return m_errorInfo.getFullMessage(p_reportLevel);
#else
	(void)p_reportLevel;
	return std::string();
#endif
}


void ErrorStatus::resetError()
{
	if (m_hasError)
	{
#ifndef TT_BUILD_FINAL
		// Clear error information.
		m_errorInfo.reset();
#endif
		
		// Reset error flag.
		m_hasError = false;
	}
}


void ErrorStatus::demoteToWarning()
{
	if (hasError())
	{
#ifndef TT_BUILD_FINAL
		m_errorInfo.setMsg(m_errorMsg.str());
		
		// Get the full location path from parents. (Might change at some later time.)
		if (m_parent != 0)
		{
			m_parent->addFullLocTo(&m_errorInfo);
		}
		m_warnings.push_back(m_errorInfo);
#else
		++m_warnings;
#endif
		resetError();
	}
}


const SrcPosNested ErrorStatus::getWarning(s32 p_index) const
{
#ifdef TT_BUILD_FINAL
	(void)p_index;
	return SrcPosNested();
#else
	SrcPosNestedContainer::const_iterator it = m_warnings.begin();
	if (p_index < 0 || 
		p_index >= static_cast<s32>(
				std::min(m_warnings.size(), 
				static_cast<SrcPosNestedContainer::size_type>(std::numeric_limits<s32>::max()))
		                           )
		)
	{
		return SrcPosNested();
	}
	
	std::advance(it, p_index);
	// TODO: Test this!. What happens if index is too big. (or < 0).
	if (it != m_warnings.end())
	{
		return (*it);
	}
	
	return SrcPosNested();
#endif // #ifdef TT_BUILD_FINAL
}

#ifndef TT_BUILD_FINAL
std::ostream& ErrorStatus::USE_MACRO_error(const char* p_file, int p_line, const char* p_function)
{
	if (m_ignoreErrors)
	{
		// Clear old junk.
		ms_nullStream.str("");
		// Ignore
		return ms_nullStream;
	}
	
	if (m_hasError)
	{
		// Clear old junk.
		ms_nullStream.str("");
		TT_PANIC("Already in error state!\n"
		         "New error located in file: %s, line: %d, fun: %s\n"
		         "But Already have the error:\n%s\n",
		         p_file, p_line, p_function,
		         getErrorMessage(ReportLevel_ErrorWithSourceLocation).c_str());
		return ms_nullStream;
	}
	m_hasError = true;
	
	// Add the current loc info + message to error info.
	m_locInfo.setMsg(m_locationStr.str());
	m_errorInfo.pushParent(m_locInfo);
	m_errorInfo.set(p_file, p_line, p_function, "");
	
	// Return stream object so the error message can be streamed to it.
	// Will add error message in get destructor or getErrorMessage, which ever is called first.
	return m_errorMsg;
}


std::ostream& ErrorStatus::USE_MACRO_setLocation(const char* p_file, int p_line, const char* p_function)
{
	if (m_ignoreErrors)
	{
		// Clear old junk.
		ms_nullStream.str("");
		// Ignore
		return ms_nullStream;
	}
	
	if (m_hasError)
	{
		// Clear old junk.
		ms_nullStream.str("");
		// Already in error state, so ignore the new location.
		return ms_nullStream;
	}
	
	m_locInfo.set(p_file, p_line, p_function);
	
	// Clear old location.
	m_locationStr.str("");
	
	return m_locationStr;
}


std::ostream& ErrorStatus::USE_MACRO_addLocation(const char* p_file, int p_line, const char* p_function)
{
	if (m_ignoreErrors)
	{
		// Clear old junk.
		ms_nullStream.str("");
		// Ignore
		return ms_nullStream;
	}
	
	if (m_hasError)
	{
		// Clear old junk.
		ms_nullStream.str("");
		// Already in error state, so ignore the new location.
		return ms_nullStream;
	}
	
	m_locInfo.set(p_file, p_line, p_function);
	
	return m_locationStr;
}
#endif // #ifndef TT_BUILD_FINAL


//--------------------------------------------------------------------------------------------------
// Private member functions

void ErrorStatus::childHasError(const SrcPosNested& p_errorInfo)
/*
                                const char* p_file, int p_line, const char* p_function,
                                const std::string& p_errorMsg,
                                const std::string& p_locationStr)
*/
{
	(void)p_errorInfo;
	
	if (m_ignoreErrors)
	{
		return;
	}
	
	if (m_hasError)
	{
		TT_PANIC("Already in error state");
		return;
	}
	
#ifndef TT_BUILD_FINAL	
	// Copy error info from child
	m_errorInfo = p_errorInfo;
	
	// Add own location info.
	m_locInfo.setMsg(m_locationStr.str());
	m_errorInfo.pushParent(m_locInfo);
	
	// Clear error message stream just to be sure.
	m_errorMsg.str("");
	m_errorMsg << p_errorInfo.getMsg();
#endif
	
	// Set error flag.
	m_hasError  = true;
}


#ifndef TT_BUILD_FINAL
void ErrorStatus::addFullLocTo(SrcPosNested* p_srcPos)
{
	if (p_srcPos != 0)
	{
		// Add own loc info.
		{
			m_locInfo.setMsg(m_locationStr.str());
			p_srcPos->pushParent(m_locInfo);
			m_locInfo.setMsg("");
		}
		
		// If not the root, add loc info from parent.
		if (m_parent != 0)
		{
			m_parent->addFullLocTo(p_srcPos);
		}
	}
}


void ErrorStatus::childHasWarnings(const SrcPosNestedContainer& p_warnings)
{
	for (SrcPosNestedContainer::const_iterator it = p_warnings.begin(); it != p_warnings.end(); ++it)
	{
		m_warnings.push_back(*it);
	}
}

#else // #ifndef TT_BUILD_FINAL

void ErrorStatus::childHasWarnings(s32 p_warnings)
{
	m_warnings += p_warnings;
}

#endif



// Namespace end
}
}
