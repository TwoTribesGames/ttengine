#if !defined(INC_TT_MENU_ELEMENTS_LINE_H)
#define INC_TT_MENU_ELEMENTS_LINE_H


#include <tt/menu/elements/ContainerBase.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Container that treats all children as a horizontal line,
           applying selection and input to all children. */
class Line : public ContainerBase<>
{
public:
	Line(const std::string& p_name,
	     const MenuLayout&  p_layout);
	virtual ~Line();
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void setSelected(bool p_selected);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyHold(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	
	virtual MenuElementInterface*       getSelectedElement();
	virtual const MenuElementInterface* getSelectedElement() const;
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const;
	
	virtual Line* clone() const;
	
	virtual void makeSelectable(bool p_selectable);
	
protected:
	Line(const Line& p_rhs);
	
private:
	// No assignment
	const Line& operator=(const Line&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_LINE_H)
