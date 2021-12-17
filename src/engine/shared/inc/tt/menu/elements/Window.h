#if !defined(INC_TT_MENU_ELEMENTS_WINDOW_H)
#define INC_TT_MENU_ELEMENTS_WINDOW_H


#include <map>

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/menu/elements/ContainerBase.h>


namespace tt {
namespace menu {
namespace elements {

class DynamicLabel;


/*! \brief Window element. */
class Window : public ContainerBase<>
{
public:
	Window(const std::string& p_name,
	       const MenuLayout&  p_layout,
	       const std::string& p_caption,
	       bool               p_hasPoles);
	
	Window(const std::string&  p_name,
	       const MenuLayout&   p_layout,
	       const std::wstring& p_caption,
	       bool                p_hasPoles);
	
	virtual ~Window();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const;
	
	virtual Window* clone() const;
	
protected:
	Window(const Window& p_rhs);
	
private:
	enum WindowConstants
	{
		TITLE_BOTTOM_PADDING = 3
	};
	
	enum WindowSection
	{
		WindowSection_Center,
		WindowSection_EdgeTop,
		WindowSection_EdgeBottom,
		WindowSection_EdgeLeft,
		WindowSection_EdgeRight,
		WindowSection_CornerTopLeft,
		WindowSection_CornerTopRight,
		WindowSection_CornerBottomLeft,
		WindowSection_CornerBottomRight,
		
		WindowSection_Count
	};
	
	enum PoleSection
	{
		PoleSection_LeftBottom,
		PoleSection_LeftTop,
		PoleSection_RightBottom,
		PoleSection_RightTop,
		
		PoleSection_Count
	};
	
	enum Decal
	{
		Decal_Top,
		Decal_Bottom,
		Decal_Left,
		Decal_Right,
		
		Decal_Count
	};
	
	
	void setWindowTexture(real p_x, real p_y, real p_w, real p_h,
	                      const engine::renderer::ColorRGBA& p_color);
	
	/*! \param p_count Number of pole textures to pick from. */
	void setPoleTexture(real p_x, real p_y, real p_w, real p_h, u32 p_count,
	                    const engine::renderer::ColorRGBA& p_color);
	
	void addDecal(Decal                               p_decal,
	              const engine::renderer::TexturePtr& p_texture,
	              const math::VectorRect&             p_uvRect,
	              const engine::renderer::ColorRGBA&  p_color);
	
	void setUVs(const engine::renderer::QuadSpritePtr& p_quad,
	            s32                  p_blockX,
	            s32                  p_blockY,
	            const math::Vector2& p_blockSize,
	            real                 p_topLeftX,
	            real                 p_topLeftY);
	
	// No assignment
	const Window& operator=(const Window&);
	
	
	engine::renderer::TexturePtr    m_texture;
	engine::renderer::TexturePtr    m_poleTexture;
	engine::renderer::QuadSpritePtr m_windowSegs[WindowSection_Count];
	engine::renderer::QuadSpritePtr m_poleSegs[PoleSection_Count];
	bool                            m_hasPoles;
	
	math::Vector2 m_topLeft;
	math::Vector2 m_bottomRight;
	
	math::Vector2 m_size;
	math::Vector2 m_blockSize;
	math::Vector2 m_poleBlockSize;
	
	math::PointRect m_innerRect;
	s32             m_borderRemainder;
	DynamicLabel*   m_label;
	
	bool m_singlePole;
	bool m_drawLeftPole;
	real m_leftRot;
	real m_rightRot;
	
	engine::renderer::QuadSpritePtr m_decals[Decal_Count];
	real                            m_decalOffsets[Decal_Count];
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_WINDOW_H)
