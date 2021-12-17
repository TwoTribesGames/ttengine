#include <tt/menu/elements/ValueDecorator.h>
#include <tt/menu/MenuDebug.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

ValueDecorator::ValueDecorator(MenuElementInterface* p_targetElement,
                               const std::string&    p_value)
:
Decorator(p_targetElement),
m_value(p_value)
{
	MENU_CREATION_Printf("ValueDecorator::ValueDecorator: "
	                     "Decorating element '%s' with value '%s'.\n",
	                     getName().c_str(), m_value.c_str());
}


ValueDecorator::~ValueDecorator()
{
	MENU_CREATION_Printf("ValueDecorator::~ValueDecorator: "
	                     "Destructing value decorator for element '%s' "
	                     "(value '%s').\n",
	                     getName().c_str(), m_value.c_str());
}


std::string ValueDecorator::getValue() const
{
	return m_value;
}


void ValueDecorator::setValue(const std::string& p_value)
{
	m_value = p_value;
}


void ValueDecorator::dumpSelectionTree(int p_treeLevel) const
{
	{
		std::string indent(static_cast<std::string::size_type>(p_treeLevel), '-');
		TT_Printf("%s { ValueDecorator, value '%s' }\n",
		          indent.c_str(), m_value.c_str());
	}
	
	Decorator::dumpSelectionTree(p_treeLevel + 1);
}


ValueDecorator* ValueDecorator::clone() const
{
	return new ValueDecorator(*this);
}


//------------------------------------------------------------------------------
// Private member functions

ValueDecorator::ValueDecorator(const ValueDecorator& p_rhs)
:
Decorator(p_rhs),
m_value(p_rhs.m_value)
{
}

// Namespace end
}
}
}
