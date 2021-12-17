#if !defined(INC_TT_MENU_ELEMENTS_BUTTON_H)
#define INC_TT_MENU_ELEMENTS_BUTTON_H


#include <map>

#include <tt/engine/renderer/fwd.h>

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuSound.h>


namespace tt {
namespace menu {
namespace elements {

// Forward declarations
class Label;
class Image;


/*! \brief Button menu element. */
class Button : public MenuElement
{
public:
	Button(const std::string&    p_name,
	       const MenuLayout&     p_layout,
	       const std::string&    p_caption,
	       const std::string&    p_image            = "",
	       const std::string&    p_pressed          = "",
	       const std::string&    p_selected         = "",
	       const std::string&    p_disabled         = "",
	       real                  p_u                = 0.0f,
	       real                  p_v                = 0.0f,
	       s32                   p_imageWidth       = 0,
	       s32                   p_imageHeight      = 0,
	       MenuKeyboard::MenuKey p_actionTriggerKey = MenuKeyboard::MENU_ACCEPT,
	       bool                  p_handleRepeat     = false);
	
	Button(const std::string&    p_name,
	       const MenuLayout&     p_layout,
	       const std::wstring&   p_caption,
	       const std::string&    p_image            = "",
	       const std::string&    p_pressed          = "",
	       const std::string&    p_selected         = "",
	       const std::string&    p_disabled         = "",
	       real                  p_u                = 0.0f,
	       real                  p_v                = 0.0f,
	       s32                   p_imageWidth       = 0,
	       s32                   p_imageHeight      = 0,
	       MenuKeyboard::MenuKey p_actionTriggerKey = MenuKeyboard::MENU_ACCEPT,
	       bool                  p_handleRepeat     = false);
	
	virtual ~Button();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual void setSelected(bool p_selected);
	virtual void setEnabled(bool p_enabled);
	
	virtual bool onStylusPressed(s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	virtual bool onKeyRepeat(const MenuKeyboard& p_keys);
	virtual bool onKeyReleased(const MenuKeyboard& p_keys);
	
	virtual Button* clone() const;
	
	void setSoundEffect(MenuSound p_effect);
	
	engine::renderer::ColorRGBA getTextColor()     const;
	engine::renderer::ColorRGBA getShadowColor()   const;
	engine::renderer::ColorRGBA getDisabledColor() const;
	engine::renderer::ColorRGBA getSelectedColor() const;
	
	void setTextColor(const engine::renderer::ColorRGBA& p_color);
	void setShadowColor(const engine::renderer::ColorRGBA& p_color);
	void setDisabledColor(const engine::renderer::ColorRGBA& p_color);
	void setSelectedColor(const engine::renderer::ColorRGBA& p_color);
	
protected:
	Button(const Button& p_rhs);
	
private:
	enum
	{
		BORDER_WIDTH             = 6,
		BORDER_MARGIN_HEIGHT     = 0,
		BORDER_IMAGE_WIDTH       = 18,
		BORDER_IMAGE_HEIGHT      = 20,
		BORDER_OPACITY_DEPRESSED = 23,
		BORDER_OPACITY_SELECTED  = 15,
		BORDER_OPACITY_NORMAL    = 9,
		BORDER_OPACITY_DISABLED  = 4
	};
	
	enum BorderSection
	{
		BorderSection_Center,
		BorderSection_LeftEdge,
		BorderSection_RightEdge,
		
		BorderSection_Count
	};
	
	
	void createBorder();
	void setBorderUVs(const engine::renderer::QuadSpritePtr& p_quad,
	                  s32 p_blockX,
	                  s32 p_blockY);
	
	void buttonClicked();
	void setBorderOpacity(u8 p_opacity);
	void updateButtonState(bool p_depressed);
	
	// Menu elements cannot be assigned to
	const Button& operator=(const Button&);
	
	
	Label*          m_label;
	Image*          m_image;
	std::string     m_imageName;
	std::string     m_pressedName;
	std::string     m_selectedName;
	std::string     m_disabledName;
	bool            m_buttonDepressed;
	engine::renderer::TexturePtr m_borderTexture;
	engine::renderer::QuadSpritePtr m_borderSegs[BorderSection_Count];
	bool            m_handleRepeat;
	
	math::Point2    m_topLeft;
	math::Point2    m_botRight;
	math::Vector2   m_size;
	math::Vector2   m_blockSize;
	math::PointRect m_innerRect;
	
	MenuKeyboard::MenuKey m_actionTriggerKey;
	MenuSound             m_soundEffect;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_BUTTON_H)
