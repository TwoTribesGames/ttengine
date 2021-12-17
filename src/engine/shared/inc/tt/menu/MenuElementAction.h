#if !defined(INC_TT_MENU_MENUELEMENTACTION_H)
#define INC_TT_MENU_MENUELEMENTACTION_H


#include <string>
#include <vector>


namespace tt {
namespace menu {

class MenuAction;


/*! \brief Stores a menu element action command and its parameters. */
class MenuElementAction
{
public:
	/*! \brief Constructs a menu element action based on a menu action. */
	explicit MenuElementAction(const MenuAction& p_action);
	
	/*! \brief Constructs a menu element action for the specified element. */
	MenuElementAction(const std::string& p_targetElement,
	                  const std::string& p_command);
	
	inline std::string getTargetElement() const { return m_targetElement; }
	inline std::string getCommand() const { return m_command; }
	
	void addParameter(const std::string& p_param)
	{ m_parameters.push_back(p_param); }
	
	int getParameterCount() const
	{ return static_cast<int>(m_parameters.size()); }
	
	std::string getParameter(int p_index) const;
	
private:
	typedef std::vector<std::string> Parameters;
	
	std::string m_targetElement;
	std::string m_command;
	Parameters  m_parameters;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUELEMENTACTION_H)
