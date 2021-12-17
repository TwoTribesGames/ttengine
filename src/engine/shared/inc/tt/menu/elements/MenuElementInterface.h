#if !defined(INC_TT_MENU_ELEMENTS_MENUELEMENTINTERFACE_H)
#define INC_TT_MENU_ELEMENTS_MENUELEMENTINTERFACE_H


#include <string>

#include <tt/platform/tt_types.h>
#include <tt/math/Rect.h>
//#include <tt/memory/SafeAllocator.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/MenuLayout.h>
#include <tt/menu/elements/SelectionCursorType.h>


namespace tt {
namespace menu {

class MenuElementAction;
class MenuKeyboard;

namespace elements {

/*! \brief Interface class for menu elements. */
class MenuElementInterface
{
public:
	// Safe heap selection path
	//typedef std::vector<s32, memory::SafeAllocator<s32> > SelectionPath;
	typedef std::vector<s32> SelectionPath;
	
	
	MenuElementInterface() { }
	virtual ~MenuElementInterface() { }
	
	virtual void loadResources()   = 0;
	virtual void unloadResources() = 0;
	
	virtual std::string getName() const = 0;
	virtual void        doLayout(const math::PointRect& p_rect) = 0;
	virtual void        dumpLayout() const = 0;
	virtual void        render(const math::PointRect& p_rect, s32 p_z) = 0;
	virtual void        update() = 0;
	
	virtual void       addAction(const MenuAction& p_action) = 0;
	virtual int        getActionCount()       const = 0;
	virtual MenuAction getAction(int p_index) const = 0;
	virtual void       clearActions()               = 0;
	
	virtual s32 getMinimumWidth()    const = 0;
	virtual s32 getMinimumHeight()   const = 0;
	virtual s32 getRequestedWidth()  const = 0;
	virtual s32 getRequestedHeight() const = 0;
	
	virtual s32 getRequestedHorizontalPosition() const = 0;
	virtual s32 getRequestedVerticalPosition()   const = 0;
	
	virtual MenuLayout&       getLayout() = 0;
	virtual const MenuLayout& getLayout() const = 0;
	
	/*! \brief Indicates whether the element can receive input focus. */
	virtual bool canHaveFocus() const = 0;
	
	/*! \brief Indicates whether the element is selected. */
	virtual bool isSelected() const = 0;
	
	/*! \brief Indicates whether the element is selected by default. */
	virtual bool isDefaultSelected() const = 0;
	
	/*! \brief Indicates whether the element is enabled. */
	virtual bool isEnabled() const = 0;
	
	/*! \brief Indicates whether the element is visible. */
	virtual bool isVisible() const = 0;
	
	/*! \brief Whether the element wants a selection cursor to be shown when selected. */
	virtual SelectionCursorType wantCursor() const = 0;
	
	/*! \brief Whether the element responds to stylus input only. */
	virtual bool isStylusOnly() const = 0;
	
	
	/*! \brief Sets whether the element is selected. */
	virtual void setSelected(bool p_selected) = 0;
	
	/*! \brief Sets whether the element is selected by default. */
	virtual void setDefaultSelected(bool p_selected) = 0;
	
	/*! \brief Sets whether the element is enabled. */
	virtual void setEnabled(bool p_enabled) = 0;
	
	/*! \brief Sets whether the element is visible. */
	virtual void setVisible(bool p_visible) = 0;
	
	/*! \brief Sets whether a selection cursor should be displayed when selected. */
	virtual void setWantCursor(SelectionCursorType p_wantCursor) = 0;
	
	/*! \brief Sets whether the element should respond to stylus input only. */
	virtual void setStylusOnly(bool p_stylusOnly) = 0;
	
	/*! \brief Sets whether the container is allowed to loop by the user. */
	virtual void setUserLoopEnable(bool p_enabled) = 0;
	
	/*! \brief Sets whether the container is allowed to loop by its parent. */
	virtual void setContainerLoopEnable(bool p_enabled,
	                                    MenuLayout::OrderType p_parentOrder) = 0;
	
	/*! \brief Indicates whether the container is allowed to loop by the user. */
	virtual bool isUserLoopEnabled() const = 0;
	
	/*! \brief Indicates whether the container is allowed to loop by its parent. */
	virtual bool isContainerLoopEnabled() const = 0;
	
	
	/*! \brief Called when the stylus was depressed on this element.
	           Default implementation does nothing.
	    \param p_x The X position of the stylus relative to the element.
	    \param p_y The Y position of the stylus relative to the element.
	    \return Whether the input was handled by the element. */
	virtual bool onStylusPressed (s32 p_x, s32 p_y) = 0;
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside) = 0;
	virtual bool onStylusReleased(s32 p_x, s32 p_y) = 0;
	virtual bool onStylusRepeat(s32 p_x, s32 p_y) = 0;
	
	/*! \brief Called when one or more keys were pressed for this element.
	           Default implementation does nothing.
	    \param p_keys The keys that were depressed.
	    \return Whether the input was handled by the element. */
	virtual bool onKeyPressed(const MenuKeyboard& p_keys) = 0;
	
	/*! \brief Called when one or more keys remained pressed for this element.
	           Default implementation does nothing.
	    \param p_keys The keys that are still pressed.
	    \return Whether the input was handled by the element. */
	virtual bool onKeyHold(const MenuKeyboard& p_keys) = 0;
	
	/*! \brief Called when one or more keys were released for this element.
	           Default implementation does nothing.
	    \param p_keys The keys that were released.
	    \return Whether the input was handled by the element. */
	virtual bool onKeyReleased(const MenuKeyboard& p_keys) = 0;
	
	/*! \brief Called when one or more keys were held for a given amount of time.
	           Default implementation does nothing.
	    \param p_keys The keys that were held.
	    \return Whether the input was handled by the element. */
	virtual bool onKeyRepeat(const MenuKeyboard& p_keys) = 0;
	
	/*! \brief Called when there is an action which needs to be performed by this element.
	    \param p_action The action which needs to be performed by the element.
	    \return Whether this action was handled by the element. */
	virtual bool doAction(const MenuElementAction& p_action) = 0;
	
	/*! \brief Returns the first element found with the specified name.
	           First checks own name, then calls this function on all the children.
	    \param p_name The name of the element which needs to be returned.
	    \return Pointer to MenuElement with the specified name, or null pointer if not found. */
	virtual MenuElementInterface* getMenuElement(const std::string& p_name) = 0;
	
	/*! \brief Returns the deepest selected element in the hierarchy.
	    \returns Pointer to the selected element,
	             or a null pointer if no element is selected. */
	virtual MenuElementInterface* getSelectedElement() = 0;
	
	/*! \brief Returns the deepest selected element in the hierarchy.
	    \returns Constant pointer to the selected element,
	             or a null pointer if no element is selected. */
	virtual const MenuElementInterface* getSelectedElement() const = 0;
	
	/*! \brief Retrieves the rectangle of the deepest selected element in the hierarchy, in absolute coordinates.
	    \param p_rect Reference to a rectangle to store the selected element rectangle in.
	    \returns False if no element is selected, true if rectangle was retrieved (p_rect is valid). */
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const = 0;
	
	/*! \brief Return the rectangle taken by this element, relative to its parent. */
	virtual const math::PointRect& getRectangle() const = 0;
	
	/*! \brief Sets the rectangle taken by this element, relative to its parent. */
	virtual void setRectangle(const math::PointRect& p_rect) = 0;
	
	/*! \brief Returns the number of Z levels taken by this element (the depth of the element). */
	virtual s32 getDepth() const = 0;
	
	/*! \brief Notifies the element that menu layout is complete.
	           Allows the element to perform initialization after layout is complete. */
	virtual void onLayoutDone() = 0;
	
	/*! \brief Notifies the element that the menu it belongs to is activated. */
	virtual void onMenuActivated() = 0;
	
	/*! \brief Notifies the element that the menu it belongs to is deactivated. */
	virtual void onMenuDeactivated() = 0;
	
	/*! \brief Sets the selection. */
	virtual void setSelectionPath(SelectionPath& p_path) = 0;
	
	/*! \brief Fills the given SelectionPath object with the selection path. */
	virtual void getSelectionPath(SelectionPath& p_path) const = 0;
	
	/*! \brief Fills the given SelectionPath with a selection path for the specified element. */
	virtual bool getSelectionPathForElement(SelectionPath&     p_path,
	                                        const std::string& p_name) const = 0;
	
	/*! \brief Outputs the selection tree starting at the current node. */
	virtual void dumpSelectionTree(int p_treeLevel) const = 0;
	
	/*! \brief Returns an identical copy of this menu element.
	           NOTE: Every element class MUST implement this for their own type! */
	virtual MenuElementInterface* clone() const = 0;
	
	/*! \brief Sets the parent for this menu element. */
	virtual void setParent(MenuElementInterface* p_parent) = 0;
	
	/*! \brief Returns a pointer to the parent of this menu element. */
	virtual MenuElementInterface* getParent() = 0;
	
	/*! \brief Returns a constant pointer to the parent of this menu element. */
	virtual const MenuElementInterface* getParent() const = 0;
	
	/*! \brief Returns the top-most element of the menu element tree this element belongs to. */
	virtual MenuElementInterface* getRoot() = 0;
	
	/*! \brief Returns the top-most element of the menu element tree this element belongs to. */
	virtual const MenuElementInterface* getRoot() const = 0;
	
	/*! \brief Forces menu elements containing children to validate
	           the currently selected child, and set a default
	           selection if nothing is selected. */
	virtual void recalculateChildSelection() = 0;
	
private:
	// Elements must provide a copy constructor for clone() to work
	MenuElementInterface(const MenuElementInterface&) { }
	
	// No menu element may be assigned to
	const MenuElementInterface& operator=(const MenuElementInterface&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_MENUELEMENTINTERFACE_H)
