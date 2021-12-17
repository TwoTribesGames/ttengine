#include <tt/args/CmdLine.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>


namespace tt {
namespace args {

//--------------------------------------------------------------------------------------------------
// Public member functions

CmdLine::CmdLine(s32 p_argCount, char **p_arguments, const std::string& p_delimiter)
:
m_argMap()
{
	str::Strings args;
	args.reserve(static_cast<str::Strings::size_type>(p_argCount));
	for (s32 i = 0; i < p_argCount; ++i)
	{
		args.push_back(p_arguments[i]);
	}
	
	init(args, p_delimiter);
}


CmdLine::CmdLine(const char* p_cmdLine, const std::string& p_delimiter)
:
m_argMap()
{
	// Get command line as vector of arguments
	init(str::explode(p_cmdLine, " "), p_delimiter);
}


CmdLine::CmdLine(const str::Strings& p_arguments, const std::string& p_delimiter)
:
m_argMap()
{
	init(p_arguments, p_delimiter);
}


bool CmdLine::exists(const std::string& p_arg) const
{
	return m_argMap.find(p_arg) != m_argMap.end();
}


std::string CmdLine::getString(const std::string& p_arg) const
{
	ArgumentMap::const_iterator it = m_argMap.find(p_arg);
	
	if (it == m_argMap.end())
	{
		return std::string();
	}
	return it->second;
}


real CmdLine::getReal(const std::string& p_arg) const
{
	return str::parseReal(getString(p_arg), 0);
}


s32 CmdLine::getInteger(const std::string& p_arg) const
{
	return str::parseS32(getString(p_arg), 0);
}


bool CmdLine::getBoolean(const std::string& p_arg) const
{
	return str::parseBool(getString(p_arg), 0);
}


void CmdLine::clear(const str::Strings& p_keepThese)
{
	ArgumentMap copy;
	for (str::Strings::const_iterator strIt = p_keepThese.begin(); strIt != p_keepThese.end(); ++strIt)
	{
		ArgumentMap::iterator argIt = m_argMap.find(*strIt);
		if (argIt != m_argMap.end())
		{
			copy.insert(*argIt);
		}
	}
	m_argMap = copy;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void CmdLine::init(const str::Strings& p_arguments, const std::string& p_delimiter)
{
	// NOTE: Starting at argument 1, since the first argument is assumed to be the executable name
	for (str::Strings::size_type i = 1; i < p_arguments.size(); ++i)
	{
		// Get current command
		std::string command(p_arguments[i]);
		
		// Compare with delimiter
		if (str::startsWith(command, p_delimiter) == false)
		{
			// Not supported --> Ignore
			TT_WARN("Ignoring unrecognized option: '%s'", command.c_str());
		}
		else
		{
			// Strip off delimiter
			command = command.substr(p_delimiter.length());
			
			// Check if there is an argument
			std::string argument;
			
			if (p_arguments.size() > (i + 1))
			{
				argument = p_arguments[i + 1];
				
				if (str::startsWith(argument, p_delimiter) == false)
				{
					++i;
				}
				else
				{
					argument.clear();
				}
			}
			
			// Check for double commands
			if (exists(command))
			{
				//TT_WARN("Option specified more than once: '%s'", command.c_str());
			}
			
			m_argMap[command] = argument;
		}
	}
}

// Namespace end
}
}
