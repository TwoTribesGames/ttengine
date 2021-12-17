#if !defined(INC_TT_MENU_ELEMENTS_SELECTIONCURSOR_H)
#define INC_TT_MENU_ELEMENTS_SELECTIONCURSOR_H


#include <tt/engine/renderer/fwd.h>

#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

class TranslationAnimationInterface;
class PositionAnimationInterface;


/*! \brief Used to indicate the active selection by displaying a cursor. */
class SelectionCursor : public MenuElement
{
public:
	SelectionCursor(const std::string& p_name,
	                const MenuLayout&  p_layout);
	virtual ~SelectionCursor();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual SelectionCursor* clone() const;
	
	/*! \brief Sets the rectangle that this cursor should be a selection around. */
	void setSelectionRect(const math::PointRect& p_rect);
	
	/*! \brief Sets the type of rendering to be used. */
	inline void setSelectionType(SelectionCursorType p_type)
	{ m_selectionType = p_type; }
	
	/*! \brief Forces the rectangle that this cursor should be a selection around. */
	void forceSelectionRect(const math::PointRect& p_rect);
	
	/*! \brief Returns the rectangle that this cursor is a selection around. */
	inline math::PointRect getSelectionRect() const { return getRectangle(); }
	
	/*! \brief Returns which type of rendering is used. */
	inline SelectionCursorType getSelectionType() const
	{ return m_selectionType; }
	
protected:
	SelectionCursor(const SelectionCursor& p_rhs);
	
private:
	void createCursorQuads();
	
	// No assignment
	const SelectionCursor& operator=(const SelectionCursor&);
	
	
	engine::renderer::TexturePtr    m_texture;
	engine::renderer::QuadSpritePtr m_leftCursor;
	engine::renderer::QuadSpritePtr m_rightCursor;
	math::PointRect                 m_selectionRect;
	SelectionCursorType             m_selectionType;
	
	TranslationAnimationInterface* m_leftTranslation;
	TranslationAnimationInterface* m_rightTranslation;
	
	PositionAnimationInterface* m_leftPosition;
	PositionAnimationInterface* m_rightPosition;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SELECTIONCURSOR_H)
