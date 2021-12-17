#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Environment.h>


namespace tt {
namespace system {

// Public functions

std::string Environment::resolve(const std::string& p_str)
{
	std::string str(p_str);
	std::string::size_type pos = str.find("$(");
	while ( pos != std::string::npos )
	{
		std::string::size_type end = str.find(')', pos);
		if ( end == std::string::npos )
		{
			TT_WARN("Missing ) in '%s'.", p_str.c_str());
			return p_str;
		}
		
		std::string var = str.substr(pos + 2, (end - pos) - 2);
		
		char* val = new char[32767]; // maximum size of env var
		DWORD result = GetEnvironmentVariableA(var.c_str(), val, 32767);
		if (result == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
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
		delete[] val;
	}
	return str;
}


bool Environment::hasVariable(const std::string& p_variable)
{
	char buffer[2];
	DWORD result = GetEnvironmentVariableA(p_variable.c_str(), buffer, 2);
	if (result == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
	{
		return false;
	}
	return true;
}


std::string Environment::getVariable(const std::string& p_variable)
{
	enum { BufferSize = 32767 }; // maximum size of env var
	
	char* val = new char[BufferSize];
	ZeroMemory(val, BufferSize);
	
	DWORD result = GetEnvironmentVariableA(p_variable.c_str(), val, BufferSize);
	if (result == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
	{
		TT_WARN("Environment variable '%s' does not exist.", p_variable.c_str());
	}
	
	std::string ret;
	if (result != 0)
	{
		ret = std::string(val);
	}
	delete[] val;
	
	return ret;
}


void Environment::setVariable(const std::string& p_variable,
                              const std::string& p_value)
{
	BOOL result = SetEnvironmentVariableA(p_variable.c_str(), p_value.c_str());
	if (result == 0)
	{
		TT_WARN("Unable to set env variable %s to %s (error %d).",
		        p_variable.c_str(), p_value.c_str(), GetLastError());
	}
}


void Environment::removeVariable(const std::string& p_variable)
{
	BOOL result = SetEnvironmentVariableA(p_variable.c_str(), "");
	if (result == 0)
	{
		TT_WARN("Unable to erase env variable %s (error %d).",
		        p_variable.c_str(), GetLastError());
	}
}

// Namespace end
}
}
