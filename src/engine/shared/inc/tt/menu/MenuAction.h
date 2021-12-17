#if !defined(INC_TT_MENU_MENUACTION_H)
#define INC_TT_MENU_MENUACTION_H


#include <string>
#include <vector>


namespace tt {
namespace menu {

/*! \brief Stores a menu action command and its parameters. */
class MenuAction
{
public:
	explicit MenuAction(const std::string& p_command);
	
	inline void addParameter(const std::string& p_param)
	{ m_parameters.push_back(p_param); }
	
	inline std::string getCommand() const { return m_command; }
	inline int getParameterCount() const
	{ return static_cast<int>(m_parameters.size()); }
	std::string getParameter(int p_index) const;
	
private:
	typedef std::vector<std::string> Parameters;
	
	std::string m_command;
	Parameters  m_parameters;
};


typedef std::vector<MenuAction> MenuActions;

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUACTION_H)
