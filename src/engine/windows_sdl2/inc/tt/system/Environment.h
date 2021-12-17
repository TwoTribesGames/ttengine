#if !defined(INC_TT_SYSTEM_ENVIRONMENT_H)
#define INC_TT_SYSTEM_ENVIRONMENT_H

#include <string>


namespace tt {
namespace system {

class Environment
{
public:
	static std::string resolve(const std::string& p_str);
	
	static bool hasVariable(const std::string& p_variable);
	static std::string getVariable(const std::string& p_variable);
	static void setVariable(const std::string& p_variable,
	                        const std::string& p_value);
	static void removeVariable(const std::string& p_variable);
};

// Namespace end
}
}

#endif // !defined(INC_TT_SYSTEM_ENVIRONMENT_H)
