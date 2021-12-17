#include <cstdlib>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Environment.h>


namespace tt {
namespace system {

//--------------------------------------------------------------------------------------------------
// Public member functions

std::string Environment::resolve(const std::string& p_str)
{
	std::string str(p_str);
	std::string::size_type pos = str.find("$(");
	while (pos != std::string::npos)
	{
		std::string::size_type end = str.find(')', pos);
		if (end == std::string::npos)
		{
			TT_WARN("Missing ) in '%s'.", p_str.c_str());
			return p_str;
		}
		
		std::string var(str.substr(pos + 2, (end - pos) - 2));
		
		char* val = std::getenv(var.c_str());
		if (val == 0)
		{
			TT_Printf("Unknown variable '%s', skipping.\n", var.c_str());
			pos = str.find("$(", end);
		}
		else
		{
			str.erase(pos, (end - pos) + 1);
			str.insert(pos, val);
			pos = str.find("$(", pos);
		}
	}
	
	return str;
}


bool Environment::hasVariable(const std::string& p_variable)
{
	return std::getenv(p_variable.c_str()) != 0;
}


std::string Environment::getVariable(const std::string& p_variable)
{
	char* val = std::getenv(p_variable.c_str());
	TT_WARNING(val != 0, "Environment variable '%s' does not exist.", p_variable.c_str());
	return val;
}


void Environment::setVariable(const std::string& p_variable,
                              const std::string& p_value)
{
	int result = setenv(p_variable.c_str(), p_value.c_str(), 1);
	if (result != 0)
	{
		TT_WARN("Unable to set env variable '%s' to '%s' (error %d: '%s').",
		        p_variable.c_str(), p_value.c_str(), result, strerror(result));
	}
}


void Environment::removeVariable(const std::string& p_variable)
{
	int result = setenv(p_variable.c_str(), "", 1);
	if (result != 0)
	{
		TT_WARN("Unable to erase env variable '%s' (error %d: '%s').",
		        p_variable.c_str(), result, strerror(result));
	}
}

// Namespace end
}
}
