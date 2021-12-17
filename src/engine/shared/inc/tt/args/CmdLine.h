#if !defined(INC_TT_ARGS_CMDLINE_H)
#define INC_TT_ARGS_CMDLINE_H

#include <map>
#include <string>

#include <tt/platform/tt_types.h>
#include <tt/str/str_types.h>


namespace tt {
namespace args {

class CmdLine
{
public:
	CmdLine(s32 p_argCount, char** p_arguments, const std::string& p_delimiter = "--");
	explicit CmdLine(const char* p_cmdLine, const std::string& p_delimiter = "--");
	explicit CmdLine(const str::Strings& p_arguments, const std::string& p_delimiter = "--");
	
	/*! \brief Check if a certain argument exists */
	bool exists(const std::string& p_arg) const;
	
	/*! \brief Returns the value of a certain argument as a string */
	std::string getString(const std::string& p_arg) const;
	
	/*! \brief Returns the value of a certain argument as a real */
	real getReal(const std::string& p_arg) const;
	
	/*! \brief Returns the value of a certain argument as an integer */
	s32 getInteger(const std::string& p_arg) const;
	
	/*! \brief Returns the value of a certain argument as a boolean */
	bool getBoolean(const std::string& p_arg) const;
	
	/*! \brief Clears all command line parameters from this object 
	    \param p_keepThese list of arguments to keep after clear. */
	void clear(const str::Strings& p_keepThese = str::Strings());
	
	/*! \brief Returns true if there are no command line parameters.*/
	inline bool empty() const { return m_argMap.empty(); }
	
	/*! \return All command line arguments for the current application
	           (has platform-specific implementation). */
	static CmdLine getApplicationCmdLine(const std::string& p_delimiter = "--");
	
private:
	void init(const str::Strings& p_arguments, const std::string& p_delimiter);
	
	
	typedef std::map<std::string, std::string> ArgumentMap;
	ArgumentMap m_argMap;
};

// Namespace end
}
}


#endif // INC_TT_ARGS_CMDLINE_H
