#if !defined(INC_TT_MENU_ELEMENTS_MENUELEMENT_H)
#define INC_TT_MENU_ELEMENTS_MENUELEMENT_H


#include <vector>

#include <tt/math/Rect.h>
#include <tt/menu/MenuLayout.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/elements/MenuElementInterface.h>
#include <tt/menu/elements/element_traits.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Base class for menu elements. */
class MenuElement : public MenuElementInterface
{
public:
	typedef action_element_tag element_category;
	
	
	MenuElement(const std::string& p_name,
	            const MenuLayout&  p_layout);
	virtual ~MenuElement();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual std::string getName() const;
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void dumpLayout() const;
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	virtual void update();
	
	virtual void       addAction(const MenuAction& p_action);
	virtual int        getActionCount() const;
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
	
	virtual bool canHaveFocus() const;
	
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
	
	virtual const math::PointRect& getRectangle() const;
	virtual void setRectangle(const math::PointRect& p_rect);
	
	virtual s32 getDepth() const;
	
	virtual void onLayoutDone();
	virtual void onMenuActivated();
	virtual void onMenuDeactivated();
	
	virtual void setSelectionPath(SelectionPath& p_path);
	virtual void getSelectionPath(SelectionPath& p_path) const;
	
	virtual bool getSelectionPathForElement(SelectionPath& p_path,
	                                        const std::string& p_name) const;
	
	virtual void dumpSelectionTree(int p_treeLevel) const;
	
	virtual MenuElement* clone() const;
	
	virtual void setParent(MenuElementInterface* p_parent);
	virtual MenuElementInterface* getParent();
	virtual const MenuElementInterface* getParent() const;
	
	virtual MenuElementInterface* getRoot();
	virtual const MenuElementInterface* getRoot() const;
	
	virtual void recalculateChildSelection();
	
protected:
	typedef std::vector<MenuAction> Actions;
	
	
	// Elements must provide a copy constructor for clone() to work
	MenuElement(const MenuElement& p_rhs);
	
	void setMinimumWidth      (s32 p_minimumWidth);
	void setMinimumHeight     (s32 p_minimumHeight);
	void setRequestedWidth    (s32 p_requestedWidth);
	void setRequestedHeight   (s32 p_requestedHeight);
	void setRequestedPositionX(s32 p_requestedX);
	void setRequestedPositionY(s32 p_requestedY);
	
	
	/*! \brief Sets whether the element can receive input focus. */
	void setCanHaveFocus(bool p_canHaveFocus);
	
	/*! \brief Sets the Z depth of this element. */
	void setDepth(s32 p_depth);
	
	/*! \brief Executes the actions associated with this element. */
	void performActions();
	
	/*! \brief Returns whether input should be handled by this element. */
	virtual bool shouldHandleInput() const;
	
private:
	// No menu element may be assigned to
	const MenuElement& operator=(const MenuElement&);
	
	
	const std::string     m_name;
	MenuLayout            m_layout;
	bool                  m_canHaveFocus;
	bool                  m_isSelected;
	bool                  m_isDefaultSelected;
	bool                  m_enabled;
	bool                  m_visible;
	bool                  m_stylusOnly;
	Actions               m_actions;
	s32                   m_minimumWidth;
	s32                   m_minimumHeight;
	s32                   m_requestedWidth;
	s32                   m_requestedHeight;
	s32                   m_requestedX;
	s32                   m_requestedY;
	s32                   m_depth;
	SelectionCursorType   m_wantCursor;
	math::PointRect       m_rectangle;
	MenuElementInterface* m_parent;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_MENUELEMENT_H)
