#if !defined(INC_TT_MENU_ELEMENTS_SUNKENBORDERDECORATOR_H)
#define INC_TT_MENU_ELEMENTS_SUNKENBORDERDECORATOR_H


#include <map>

#include <tt/math/Vector2.h>
#include <tt/math/Rect.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/menu/elements/Decorator.h>
#include <tt/menu/elements/element_traits.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Decorator that adds a sunken borer to the target element. */
class SunkenBorderDecorator : public Decorator
{
public:
	SunkenBorderDecorator(MenuElementInterface* p_targetElement);
	virtual ~SunkenBorderDecorator();
	
	virtual void doLayout(const math::PointRect& p_rect);
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual const math::PointRect& getRectangle() const;
	virtual void setRectangle(const math::PointRect& p_rect);
	
	virtual bool getSelectedElementRect(math::PointRect& p_rect) const;
	
	virtual void dumpSelectionTree(int p_treeLevel) const;
	
	virtual s32 getDepth() const;
	
	virtual SunkenBorderDecorator* clone() const;
	
private:
	enum
	{
		BORDER_SIZE = 7
	};
	
	enum BorderSection
	{
		BorderSection_Center,
		BorderSection_EdgeTop,
		BorderSection_EdgeBottom,
		BorderSection_EdgeLeft,
		BorderSection_EdgeRight,
		BorderSection_CornerTopLeft,
		BorderSection_CornerTopRight,
		BorderSection_CornerBottomLeft,
		BorderSection_CornerBottomRight,
		
		BorderSection_Count
	};
	
	
	SunkenBorderDecorator(const SunkenBorderDecorator& p_rhs);
	
	void setBorderTexture(real p_x, real p_y, real p_w, real p_h,
	                      const engine::renderer::ColorRGBA& p_color);
	
	void setUVs(const engine::renderer::QuadSpritePtr& p_quad,
	            s32                                    p_blockX,
	            s32                                    p_blockY,
	            const math::Vector2&                   p_blockSize,
	            real                                   p_topLeftX,
	            real                                   p_topLeftY);
	
	
	engine::renderer::TexturePtr    m_texture;
	engine::renderer::QuadSpritePtr m_borderSegs[BorderSection_Count];
	
	math::Vector2   m_size;
	math::Vector2   m_blockSize;
	math::PointRect m_borderRect;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SUNKENBORDERDECORATOR_H)
