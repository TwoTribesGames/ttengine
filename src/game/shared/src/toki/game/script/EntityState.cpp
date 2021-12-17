#include <tt/str/str.h>

#include <toki/game/script/EntityState.h>


namespace toki {
namespace game {
namespace script {


//--------------------------------------------------------------------------------------------------
// Public member functions

EntityState::EntityState()
{
	reset();
}


EntityState::EntityState(const std::string& p_name, const HSQOBJECT& p_sqState)
:
m_name(p_name),
m_sqState(p_sqState)
{
}

//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
