#if !defined(INC_TT_MENU_ELEMENTS_CONTAINER_H)
#define INC_TT_MENU_ELEMENTS_CONTAINER_H


#include <tt/menu/elements/ContainerBase.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Simple menu element that can contain other menu elements. */
class Container : public ContainerBase<>
{
public:
	Container(const std::string& p_name,
	          const MenuLayout&  p_layout);
	virtual ~Container();
	
	virtual Container* clone() const;
	
protected:
	// Copy constructor required for clone()
	Container(const Container& p_rhs);
	
	// Cannot assign to menu elements
	const Container& operator=(const Container&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_CONTAINER_H)
