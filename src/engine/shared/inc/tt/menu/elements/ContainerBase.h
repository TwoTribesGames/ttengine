#if !defined(INC_TT_MENU_ELEMENTS_CONTAINERBASE_H)
#define INC_TT_MENU_ELEMENTS_CONTAINERBASE_H


#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/elements/element_traits.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Simple menu element that can contain other menu elements. */
template<typename ChildType = MenuElementInterface>
class ContainerBase : public MenuElement
{
public:
	typedef ChildType             value_type;
	typedef container_element_tag element_category;
	
	
	ContainerBase(const std::string& p_name,
	              const MenuLayout&  p_layout);
	virtual ~ContainerBase();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	virtual void update();
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void dumpLayout() const;
	
	virtual void addChild(value_type* p_child);
	virtual void removeChildren();
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual bool canHaveFocus() const;
	
	virtual void setUserLoopEnable(bool p_enabled);
	virtual void setContainerLoopEnable(bool p_enabled,
	                                    MenuLayout::OrderType p_parentOrder);
	
	virtual bool isUserLoopEnabled() const;
	virtual bool isContainerLoopEnabled() const;
	
	virtual void setSelected(bool p_selected);
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyHold(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	virtual bool onKeyRepeat(const MenuKeyboard& p_keys);
	
	virtual MenuElementInterface* getMenuElement(const std::string& p_name);
	
	virtual MenuElementInterface* getSelectedElement();
	virtual const MenuElementInterface* getSelectedElement() const;
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const;
	
	virtual void setInitialChildByName(const std::string& p_name);
	
	/*! \brief Selects a direct child by the element's name.
	           Asserts if the name does not specify a direct child of this container. */
	virtual void selectChildByName(const std::string& p_name);
	
	/*! \brief Selects the previous selectable child in the container.
	    \return True if the selection was changed, false if the selection was not changed. */
	virtual bool selectPreviousChild();
	
	/*! \brief Selects the next selectable child in the container.
	    \return True if the selection was changed, false if the selection was not changed. */
	virtual bool selectNextChild();
	
	virtual s32 getDepth() const;
	
	virtual void onLayoutDone();
	virtual void onMenuActivated();
	virtual void onMenuDeactivated();
	
	virtual void setSelectionPath(SelectionPath& p_path);
	virtual void getSelectionPath(SelectionPath& p_path) const;
	
	virtual bool getSelectionPathForElement(SelectionPath& p_path,
	                                        const std::string& p_name) const;
	
	virtual void dumpSelectionTree(int p_treeLevel) const;
	
	virtual void recalculateChildSelection();
	
	virtual ContainerBase* clone() const;
	
	virtual void selectChildByIndex(int p_index, bool p_forceSelected = false);
	virtual int  getSelectedChildIndex() const;
	
	virtual int getChildCount() const;
	
protected:
	// Container for child elements
	typedef std::vector<value_type*> ChildVector;
	
	
	//! The children in this container.
	ChildVector m_children;
	
	//! The child element that has the focus.
	value_type* m_focusChild;
	
	//! The child element that has stylus focus.
	value_type* m_stylusFocusChild;
	
	//! The index of the child element that has the focus.
	int m_focusChildIndex;
	
	std::string m_initialChildName;
	
	bool m_userLoopEnabled;
	bool m_containerLoopEnabled;
	
	
	virtual bool isSelectableElement(const value_type* p_element) const;
	
	virtual int getSelectableChildrenCount() const;
	virtual int getLastSelectableChildIndex() const;
	virtual int getFirstSelectableChildIndex() const;
	virtual int getNextSelectableChildIndex(int p_index, bool p_wrap) const;
	virtual int getPreviousSelectableChildIndex(int p_index, bool p_wrap) const;
	
	/*! \brief Determines the initial selection to set,
	           and returns if this was successful. */
	virtual bool setInitialSelection();
	
	
	//--------------------------------------------------------------------------
	// Child layout functions
	
	s32 getMinimumChildrenWidth   (const ChildVector& p_children) const;
	s32 getMinimumChildrenHeight  (const ChildVector& p_children) const;
	s32 getRequestedChildrenWidth (const ChildVector& p_children) const;
	s32 getRequestedChildrenHeight(const ChildVector& p_children) const;
	
	// Renders all children in the specified container
	void renderChildren(ChildVector&           p_children,
	                    const math::PointRect& p_rect,
	                    s32                    p_z,
	                    s32                    p_start      = 0,
	                    s32                    p_childCount = -1);
	
	// Copy constructor is required for clone()
	ContainerBase(const ContainerBase& p_rhs);
	
private:
	// Menu elements cannot be assigned to
	const ContainerBase& operator=(const ContainerBase& p_rhs);
};

// Namespace end
}
}
}


// Include the implementation of this class (templates...)
#include "ContainerBase.tpl"


#endif  // !defined(INC_TT_MENU_ELEMENTS_CONTAINERBASE_H)
