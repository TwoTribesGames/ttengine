#include <tt/platform/tt_error.h>
#include <tt/menu/MenuAction.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

MenuAction::MenuAction(const std::string& p_command)
:
m_command(p_command)
{
}


std::string MenuAction::getParameter(int p_index) const
{
	TT_ASSERTMSG(p_index >= 0 && p_index < getParameterCount(),
	             "MenuAction parameter index %d out of range [0 - %d).",
	             p_index, getParameterCount());
	return m_parameters.at(static_cast<Parameters::size_type>(p_index));
}

// Namespace end
}
}
