#include <tt/menu/elements/Container.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

Container::Container(const std::string& p_name,
                     const MenuLayout&  p_layout)
:
ContainerBase<>(p_name, p_layout)
{
	MENU_CREATION_Printf("Container::Container: Element '%s': New Container.\n",
	                     getName().c_str());
}


Container::~Container()
{
	MENU_CREATION_Printf("Container::~Container: Element '%s': "
	                     "Destructor.\n", getName().c_str());
}


Container* Container::clone() const
{
	return new Container(*this);
}


//------------------------------------------------------------------------------
// Public member functions

Container::Container(const Container& p_rhs)
:
ContainerBase<>(p_rhs)
{
}

// Namespace end
}
}
}
