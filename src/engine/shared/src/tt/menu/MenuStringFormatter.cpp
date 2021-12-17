#include <tt/menu/MenuStringFormatter.h>
#include <tt/menu/MenuSystem.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

MenuStringFormatter::MenuStringFormatter(const std::string& p_localizationID,
                                         bool               p_numberedParams)
:
str::StringFormatter(
	MenuSystem::getInstance()->translateString(p_localizationID),
	p_numberedParams)
{
}


MenuStringFormatter::~MenuStringFormatter()
{
}

// Namespace end
}
}
