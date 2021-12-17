#if !defined(INC_TT_MENU_ELEMENTS_VALUEDECORATOR_H)
#define INC_TT_MENU_ELEMENTS_VALUEDECORATOR_H


#include <tt/menu/elements/Decorator.h>
#include <tt/menu/elements/element_traits.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Decorator that adds a string value to the target element. */
class ValueDecorator : public Decorator
{
public:
	ValueDecorator(MenuElementInterface* p_targetElement,
	               const std::string&    p_value);
	virtual ~ValueDecorator();
	
	virtual std::string getValue() const;
	virtual void        setValue(const std::string& p_value);
	
	virtual void dumpSelectionTree(int p_treeLevel) const;
	
	virtual ValueDecorator* clone() const;
	
private:
	std::string m_value;
	
	
	// Elements must provide a copy constructor for clone() to work
	ValueDecorator(const ValueDecorator& p_rhs);
	
	// No menu element may be assigned to
	const ValueDecorator& operator=(const ValueDecorator&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_VALUEDECORATOR_H)
