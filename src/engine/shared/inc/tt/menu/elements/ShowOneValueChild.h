#if !defined(INC_TT_MENU_ELEMENTS_SHOWONEVALUECHILD_H)
#define INC_TT_MENU_ELEMENTS_SHOWONEVALUECHILD_H


#include <tt/menu/elements/ContainerBase.h>
#include <tt/menu/elements/ValueDecorator.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief ShowOneValueChild element.
    This container will only show one child at a time.
    Next or Previous children can be selected.
    It is also possible to change the child with a index. */
class ShowOneValueChild : public ContainerBase<ValueDecorator>
{
public:
	typedef value_element_tag element_category;
	
	
	ShowOneValueChild(const std::string& p_name,
	                  const MenuLayout&  p_layout);
	virtual ~ShowOneValueChild();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	/*! \return The value of the shown child. */
	std::string getValueShowChild() const;
	
	/*! \brief Retrieves the value of the specified child. */
	virtual std::string getChildValue(int p_index) const;
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual bool canHaveFocus() const;
	
	virtual void setSelected(bool p_selected);
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyHold(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	/*! \brief Sets whether to refresh the selection each time the shown child is changed. */
	void setForceSelectionRefresh(bool p_force);
	
	virtual void setInitialChildByValue(const std::string& p_value);
	
	virtual void selectChildByIndex(int p_index, bool p_forceSelected = false);
	
	/*! \brief Selects a direct child by the element's value.
	           Asserts if no direct child with the specified value was found. */
	virtual void selectChildByValue(const std::string& p_value);
	
	virtual void setContainerLoopEnable(bool p_enabled,
	                                    MenuLayout::OrderType p_parentOrder);
	
	virtual ShowOneValueChild* clone() const;
	
protected:
	ShowOneValueChild(const ShowOneValueChild& p_rhs);
	
	virtual bool setInitialSelection();
	
	virtual bool isSelectableElement(const value_type* p_element) const;
	
private:
	/*! \brief Return the (const) shown child if any.
	    \return the shown child (const). */
	const value_type* getShowChild() const;
	
	/*! \brief Return the shown child if any.
	    \return the shown child. */
	value_type* getShowChild();
	
	void updateChildrenResources();
	
	// No assignment
	const ShowOneValueChild& operator=(const ShowOneValueChild&);
	
	
	std::string m_initialChildValue;
	bool        m_resourcesLoaded;
	bool        m_forceSelectionRefresh;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SHOWONEVALUECHILD_H)
