#if !defined(INC_TT_MENU_ELEMENTS_DECORATOR_H)
#define INC_TT_MENU_ELEMENTS_DECORATOR_H


#include <tt/menu/elements/MenuElementInterface.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Base class for menu element decorators. */
class Decorator : public MenuElementInterface
{
public:
	/*! \brief Constructs a decorator for the specified target element.
	    \param p_targetElement The element to decorate. Must not be null.
	                           Ownership is transferred to decorator. */
	Decorator(MenuElementInterface* p_targetElement);
	virtual ~Decorator();
	
	// All functions are forwarded to the target element by default
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual std::string getName() const;
	virtual void        doLayout(const math::PointRect& p_rect);
	virtual void        dumpLayout() const;
	virtual void        render(const math::PointRect& p_rect, s32 p_z);
	virtual void        update();
	
	virtual void       addAction(const MenuAction& p_action);
	virtual int        getActionCount()       const;
	virtual MenuAction getAction(int p_index) const;
	virtual void       clearActions();
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual s32 getRequestedHorizontalPosition() const;
	virtual s32 getRequestedVerticalPosition()   const;
	
	virtual MenuLayout&       getLayout();
	virtual const MenuLayout& getLayout() const;
	
	virtual bool                canHaveFocus() const;
	virtual bool                isSelected() const;
	virtual bool                isDefaultSelected() const;
	virtual bool                isEnabled() const;
	virtual bool                isVisible() const;
	virtual SelectionCursorType wantCursor() const;
	virtual bool                isStylusOnly() const;
	
	virtual void setSelected(bool p_selected);
	virtual void setDefaultSelected(bool p_selected);
	virtual void setEnabled(bool p_enabled);
	virtual void setVisible(bool p_visible);
	virtual void setWantCursor(SelectionCursorType p_wantCursor);
	virtual void setStylusOnly(bool p_stylusOnly);
	
	virtual void setUserLoopEnable(bool p_enabled);
	virtual void setContainerLoopEnable(bool p_enabled,
	                                    MenuLayout::OrderType p_parentOrder);
	
	virtual bool isUserLoopEnabled() const;
	virtual bool isContainerLoopEnabled() const;
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyHold(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	virtual bool onKeyRepeat(const MenuKeyboard& p_keys);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual MenuElementInterface* getMenuElement(const std::string& p_name);
	
	virtual MenuElementInterface*       getSelectedElement();
	virtual const MenuElementInterface* getSelectedElement() const;
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const;
	
	virtual void dumpSelectionTree(int p_treeLevel) const;
	
	virtual const math::PointRect& getRectangle() const;
	virtual void setRectangle(const math::PointRect& p_rect);
	
	virtual s32 getDepth() const;
	
	virtual void onLayoutDone();
	virtual void onMenuActivated();
	virtual void onMenuDeactivated();
	
	virtual void setSelectionPath(SelectionPath& p_path);
	virtual void getSelectionPath(SelectionPath& p_path) const;
	
	virtual bool getSelectionPathForElement(SelectionPath&     p_path,
	                                        const std::string& p_name) const;
	
	virtual Decorator* clone() const;
	
	virtual void setParent(MenuElementInterface* p_parent);
	virtual MenuElementInterface* getParent();
	virtual const MenuElementInterface* getParent() const;
	
	virtual MenuElementInterface* getRoot();
	virtual const MenuElementInterface* getRoot() const;
	
	virtual void recalculateChildSelection();
	
protected:
	// Elements must provide a copy constructor for clone() to work
	Decorator(const Decorator& p_rhs);
	
private:
	MenuElementInterface* m_decoratedElement;
	
	
	// No menu element may be assigned to
	const Decorator& operator=(const Decorator&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_DECORATOR_H)
