#if !defined(INC_TT_MENU_ELEMENTS_COLORITEM_H)
#define INC_TT_MENU_ELEMENTS_COLORITEM_H


#include <tt/engine/renderer/fwd.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Image Editor menu element. */
class ColorItem : public MenuElement
{
public:
	ColorItem(const std::string& p_name,
	          const MenuLayout&  p_layout,
	          u32                p_color);
	virtual ~ColorItem();
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void addChild(MenuElement* p_child);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual s32 getRequestedHorizontalPosition() const;
	virtual s32 getRequestedVerticalPosition()   const;
	
	virtual bool onStylusPressed(s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	
	virtual ColorItem* clone() const;
	
protected:
	ColorItem(const ColorItem& p_rhs);
	
private:
	const ColorItem& operator=(const ColorItem&);
	
	
	u32                             m_color;
	engine::renderer::QuadSpritePtr m_quad;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_COLORITEM_H)
