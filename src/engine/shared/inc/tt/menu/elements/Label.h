#if !defined(INC_TT_MENU_ELEMENTS_LABEL_H)
#define INC_TT_MENU_ELEMENTS_LABEL_H


#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/elements/Scrollbar.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Label menu element. */
class Label : public MenuElement
{
public:
	/*! \brief Constructs a label and automatically localizes the specified caption. */
	Label(const std::string& p_name,
	      const MenuLayout&  p_layout,
	      const std::string& p_caption,
	      bool               p_multiLine        = false,
	      s32                p_bottomLineOffset = 0);
	
	/*! \brief Constructs a label and leaves the caption intact (no localization). */
	Label(const std::string&  p_name,
	      const MenuLayout&   p_layout,
	      const std::wstring& p_caption,
	      bool                p_multiLine        = false,
	      s32                 p_bottomLineOffset = 0);
	
	virtual ~Label();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void update();
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	
	virtual bool canHaveFocus() const;
	
	virtual void setEnabled(bool p_enabled);
	virtual void setSelected(bool p_selected);
	
	virtual Label* clone() const;
	
	engine::glyph::GlyphSet::Alignment getHorizontalAlign() const;
	engine::glyph::GlyphSet::Alignment getVerticalAlign()   const;
	
	engine::renderer::ColorRGBA getTextColor()     const;
	engine::renderer::ColorRGBA getShadowColor()   const;
	engine::renderer::ColorRGBA getDisabledColor() const;
	engine::renderer::ColorRGBA getSelectedColor() const;
	
	void setHorizontalAlign(engine::glyph::GlyphSet::Alignment p_align);
	void setVerticalAlign(engine::glyph::GlyphSet::Alignment p_align);
	
	void setTextColor(const engine::renderer::ColorRGBA& p_color);
	void setShadowColor(const engine::renderer::ColorRGBA& p_color);
	void setDisabledColor(const engine::renderer::ColorRGBA& p_color);
	void setSelectedColor(const engine::renderer::ColorRGBA& p_color);
	
	static engine::glyph::GlyphSet::Alignment getAlignmentFromString(
			const std::string& p_align);
	
protected:
	Label(const Label& p_rhs);
	
private:
	/*! \brief Centralized constructor initialization, to avoid code duplication. */
	void construct(bool p_multiLine, s32 p_bottomLineOffset);
	
	/*! \brief Updates the quad colors based on the current state. */
	void updateQuadColor();
	
	engine::renderer::ColorRGBA getBodyQuadColor() const;
	engine::renderer::ColorRGBA getShadowQuadColor() const;
	
	/*! \brief Renders the text onto the texture. */
	bool renderTexture();
	
	// Menu elements cannot be assigned to
	const Label& operator=(const Label&);
	
	
	std::wstring m_text;
	
	s32 m_captionWidth;
	s32 m_captionHeight;
	
	s32 m_textureWidth;
	s32 m_textureHeight;
	
	math::Vector3 m_shadowOff;
	
	engine::glyph::GlyphSet::Alignment m_halign;
	engine::glyph::GlyphSet::Alignment m_valign;
	
	engine::renderer::QuadSpritePtr m_body;
	engine::renderer::QuadSpritePtr m_shade;
	engine::renderer::TexturePtr    m_texture;
	
	bool m_shouldRender;
	
	engine::renderer::ColorRGBA m_textColor;     //!< Color of the text when label not disabled or selected (normal state).
	engine::renderer::ColorRGBA m_shadowColor;   //!< Color of the shadow behind the text.
	engine::renderer::ColorRGBA m_disabledColor; //!< Text color when label disabled.
	engine::renderer::ColorRGBA m_selectedColor; //!< Text color when label selected.
	
	bool m_multiLine;
	s32  m_bottomLineOffset;
	
	s32 m_linesPerPage;
	s32 m_lines;
	s32 m_topLine;
	
	s32  m_scrollOffset;
	bool m_scrollRight;
	s32  m_scrollDelay;
	
	Scrollbar* m_scrollbar;
	bool       m_scrollPick;
	s32        m_scrollX;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_LABEL_H)
