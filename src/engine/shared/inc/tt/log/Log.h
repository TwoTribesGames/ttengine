#if !defined(INC_TT_LOG_LOG_H)
#define INC_TT_LOG_LOG_H


#include <iostream>
#include <sstream>
#include <string>

#include <tt/platform/tt_printf.h>
#include <tt/system/Time.h>


namespace tt {
namespace log {

/// \brief Supported log levels
enum LogLevel
{
	LogLevel_ERROR   = 0,
	LogLevel_WARNING = 1,
	LogLevel_INFO    = 2,
	LogLevel_DEBUG   = 3,
	LogLevel_DEBUG1  = 4,
	LogLevel_DEBUG2  = 5,
	LogLevel_DEBUG3  = 6,
	LogLevel_DEBUG4  = 7,
	LogLevel_TEST    = 8,
	LogLevel_MAX
};



/// \brief Log main class definition
template <typename T>
class Log
{
public:
	Log();
	virtual ~Log();
	std::ostringstream& Get(LogLevel p_level = LogLevel_INFO);
	
	static std::string ToString(LogLevel p_level);
	
protected:
	std::ostringstream m_outstream;
	
private:
	Log(const Log&);
	Log& operator=(const Log&);
	
	LogLevel m_mylevel;
};


/// \brief Construct
template <typename T>
Log<T>::Log()
{
}


/// \brief Destruct, emit log entry
template <typename T>
Log<T>::~Log()
{
	m_outstream << std::endl;
	T::Output(m_outstream.str(), m_mylevel);
}


/// \return log stream object
template <typename T>
std::ostringstream& Log<T>::Get(LogLevel p_level)
{
	m_mylevel = p_level;
	
	if (p_level > LogLevel_WARNING)
	{
		m_outstream << "- " <<  tt::system::Time::getInstance()->getNowAsString() << " ";
	}
	
	m_outstream << ToString(p_level) << ": ";
	
	m_outstream <<
		std::string((p_level > LogLevel_DEBUG) ? ( ((int)p_level) - ((int)LogLevel_DEBUG) ) : 0, '\t');
	
	return m_outstream;
}


/// \return String representation of debug level
template <typename T>
std::string Log<T>::ToString(LogLevel p_level)
{
	static const char* const buffer[] =
	{
		"  ERROR",
		"WARNING",
		"   INFO",
		"  DEBUG",
		" DEBUG1",
		" DEBUG2",
		" DEBUG3",
		" DEBUG4",
		"   TEST"
	};
	return buffer[p_level];
}




/// \brief Simplistic log appender, using TT_Printf
class ConsoleAppender 
{
public:
	static void Output(const std::string& p_msg, tt::log::LogLevel p_level);
	
	static LogLevel m_logLevel;
};



/// \brief Required Output method
inline void ConsoleAppender::Output(const std::string& p_msg, tt::log::LogLevel p_level)
{
	(void)p_msg; // prevent unused variable warnings when TT_Printf evaluates to nothing
	if (p_level <= ConsoleAppender::m_logLevel)
	{
#if defined(TT_PLATFORM_WIN)  // TT_ErrPrintf is currently only available on Windows
		if (p_level == LogLevel_ERROR)
		{
			TT_ErrPrintf("%s", p_msg.c_str());
		}
		else
#endif
		{
			TT_Printf("%s", p_msg.c_str());
		}
	}
}

// Namespace end
}
}


#endif  // !defined(INC_TT_LOG_LOG_H)
