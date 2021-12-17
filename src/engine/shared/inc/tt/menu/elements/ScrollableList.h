#if !defined(INC_TT_MENU_ELEMENTS_SCROLLABLELIST_H)
#define INC_TT_MENU_ELEMENTS_SCROLLABLELIST_H


#include <tt/menu/elements/ContainerBase.h>


namespace tt {
namespace menu {
namespace elements {

// Forward declarations
class Scrollbar;


/*! \brief Simple menu element that can contain other menu elements. */
class ScrollableList : public ContainerBase<>
{
public:
	ScrollableList(const std::string& p_name,
	               const MenuLayout&  p_layout);
	virtual ~ScrollableList();
	
	virtual void loadResources();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void addChild(value_type* p_child);
	virtual void removeChildren();
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	
	virtual void setSelected(bool p_selected);
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const;
	
	virtual void selectChildByIndex(int p_index, bool p_forceSelected = false);
	
	virtual void setContainerLoopEnable(bool p_enabled,
	                                    MenuLayout::OrderType p_parentOrder);
	
	virtual ScrollableList* clone() const;
	
protected:
	ScrollableList(const ScrollableList& p_rhs);
	
	// Overridden to do nothing, so that a selection
	// can be set after some calculations have been done
	virtual bool setInitialSelection();
	
private:
	void updateChildrenResources();
	
	// No assignment
	const ScrollableList& operator=(const ScrollableList&);
	
	
	Scrollbar* m_scrollbar;
	bool       m_useScrollbar;     //!< Whether a scrollbar is needed.
	bool       m_scrollPick;
	s32        m_scrollX;
	s32        m_topChild;         //!< The index of the top-most showable child.
	s32        m_showableChildren; //!< The number of children that can be shown at one time.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SCROLLABLELIST_H)
