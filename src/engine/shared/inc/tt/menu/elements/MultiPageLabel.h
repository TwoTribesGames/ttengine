#if !defined(INC_TT_MENU_ELEMENTS_MULTIPAGELABEL_H)
#define INC_TT_MENU_ELEMENTS_MULTIPAGELABEL_H


#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Multi-page label menu element. */
class MultiPageLabel : public MenuElement
{
public:
	/*! \brief Constructs a label and automatically localizes the specified caption. */
	MultiPageLabel(const std::string& p_name,
	               const MenuLayout&  p_layout,
	               const std::string& p_caption,
	               const std::string& p_prev,
	               const std::string& p_next,
	               bool               p_prevHasOther,
	               bool               p_nextHasOther,
	               const std::string& p_buttonContainer);
	
	/*! \brief Constructs a label and leaves the caption intact (no localization). */
	MultiPageLabel(const std::string&  p_name,
	               const MenuLayout&   p_layout,
	               const std::wstring& p_caption,
	               const std::string&  p_prev,
	               const std::string&  p_next,
	               bool                p_prevHasOther,
	               bool                p_nextHasOther,
	               const std::string&  p_buttonContainer);
	
	virtual ~MultiPageLabel();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void update();
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void onMenuActivated();
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	void setEnabled(bool p_enabled);
	void setSelected(bool p_selected);
	
	virtual MultiPageLabel* clone() const;
	
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
	
	void selectPrev();
	void selectNext();
	
protected:
	MultiPageLabel(const MultiPageLabel& p_rhs);
	
private:
	typedef std::vector<std::wstring> StringVector;
	
	
	/*! \brief Centralized constructor initialization, to avoid code duplication. */
	void construct();
	
	/*! \brief Updates the quad colors based on the current state. */
	void updateQuadColor();
	
	engine::renderer::ColorRGBA getBodyQuadColor() const;
	engine::renderer::ColorRGBA getShadowQuadColor() const;
	
	/*! \brief Renders the text onto the texture. */
	bool renderTexture();
	
	// Menu elements cannot be assigned to
	const MultiPageLabel& operator=(const MultiPageLabel&);
	
	
	std::wstring m_text;
	std::string  m_prev;
	std::string  m_next;
	
	bool m_prevHasOther;
	bool m_nextHasOther;
	
	std::string m_buttonContainer;
	
	StringVector m_pages;
	
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
	
	s32 m_page;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_MULTIPAGELABEL_H)
