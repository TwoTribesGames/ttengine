#if !defined(INC_TT_MENU_ELEMENTS_DYNAMICLABEL_H)
#define INC_TT_MENU_ELEMENTS_DYNAMICLABEL_H


#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Label with dynamic content. */
class DynamicLabel : public MenuElement
{
public:
	/*! \brief Constructs a label and automatically localizes the specified caption. */
	DynamicLabel(const std::string& p_name,
	             const MenuLayout&  p_layout,
	             const std::string& p_caption);
	
	/*! \brief Constructs a dynamic label with a given caption. */
	DynamicLabel(const std::string&  p_name,
	             const MenuLayout&   p_layout,
	             const std::wstring& p_caption);
	
	virtual ~DynamicLabel();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void update();
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void setEnabled(bool p_enabled);
	virtual void setSelected(bool p_selected);
	
	engine::glyph::GlyphSet::Alignment getHorizontalAlign() const;
	engine::glyph::GlyphSet::Alignment getVerticalAlign()   const;
	
	engine::renderer::ColorRGBA getTextColor()   const;
	engine::renderer::ColorRGBA getShadowColor() const;
	engine::renderer::ColorRGBA getDisabledColor() const;
	engine::renderer::ColorRGBA getSelectedColor() const;
	
	void setHorizontalAlign(engine::glyph::GlyphSet::Alignment p_align);
	void setVerticalAlign(engine::glyph::GlyphSet::Alignment p_align);
	
	void setTextColor(const engine::renderer::ColorRGBA& p_color);
	void setShadowColor(const engine::renderer::ColorRGBA& p_color);
	void setDisabledColor(const engine::renderer::ColorRGBA& p_color);
	void setSelectedColor(const engine::renderer::ColorRGBA& p_color);
	
	/*! \brief Sets a literal caption for the label. */
	void setCaption(const std::wstring& p_caption);
	
	/*! \brief Sets a caption using a localization ID (will be translated). */
	void setCaption(const std::string& p_caption);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual DynamicLabel* clone() const;
	
	inline s32 getCaptionWidth() const { return m_captionWidth; }
	
protected:
	DynamicLabel(const DynamicLabel& p_rhs);
	
private:
	/*! \brief Centralized constructor initialization, to avoid code duplication. */
	void construct();
	
	void createTexture();
	
	/*! \brief Updates the quad colors based on the current state. */
	void updateQuadColor();
	
	engine::renderer::ColorRGBA getBodyQuadColor() const;
	engine::renderer::ColorRGBA getShadowQuadColor() const;
	
	/*! \brief Renders the text onto the texture. */
	bool renderTexture();
	
	// No assignment
	const DynamicLabel& operator=(const DynamicLabel&);
	
	
	std::wstring m_text;
	
	s32 m_captionWidth;
	s32 m_captionHeight;
	
	s32 m_textureWidth;
	s32 m_textureHeight;
	s32 m_minTextureWidth;
	
	math::Vector3 m_shadowOffset;
	
	engine::glyph::GlyphSet::Alignment m_halign;
	engine::glyph::GlyphSet::Alignment m_valign;
	
	engine::renderer::QuadSpritePtr m_body;
	engine::renderer::QuadSpritePtr m_shade;
	engine::renderer::TexturePtr    m_texture;
	
	bool m_shouldRender;
	
	engine::renderer::ColorRGBA m_textColor;
	engine::renderer::ColorRGBA m_shadowColor;
	engine::renderer::ColorRGBA m_disabledColor;
	engine::renderer::ColorRGBA m_selectedColor;
	
	s32  m_scrollOffset;
	bool m_scrollRight;
	s32  m_scrollDelay;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_DYNAMICLABEL_H)
