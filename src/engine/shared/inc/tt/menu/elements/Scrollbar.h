#if !defined(INC_TT_MENU_ELEMENTS_SCROLLBAR_H)
#define INC_TT_MENU_ELEMENTS_SCROLLBAR_H


#include <map>

#include <tt/platform/tt_error.h>
#include <tt/engine/renderer/fwd.h>

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Scrollbar element. */
class Scrollbar : public MenuElement
{
public:
	Scrollbar(const std::string& p_name,
	          const MenuLayout&  p_layout);
	virtual ~Scrollbar();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual bool onStylusPressed (s32 p_x, s32 p_y);
	virtual bool onStylusDragging(s32 p_x, s32 p_y, bool p_isInside);
	virtual bool onStylusReleased(s32 p_x, s32 p_y);
	virtual bool onStylusRepeat(s32 p_x, s32 p_y);
	
	virtual Scrollbar* clone() const;
	
	inline void setStepSize(real p_stepsize) { m_stepSize = p_stepsize; }
	inline void setRange(real p_min, real p_max)
	{ TT_ASSERT(p_min < p_max); m_minValue = p_min; m_maxValue = p_max; }
	inline real getValue() const { return m_value; }
	inline void setValue(real p_value)
	{ TT_ASSERT(p_value >= m_minValue && p_value <= m_maxValue); m_value = p_value; }
	
protected:
	Scrollbar(const Scrollbar& p_rhs);
	
private:
	enum BarSection
	{
		BarSection_Top,
		BarSection_Middle,
		BarSection_Bottom,
		
		BarSection_Count
	};
	
	enum ArrowSection
	{
		ArrowSection_Up,
		ArrowSection_Down,
		
		ArrowSection_Count
	};
	
	enum
	{
		BORDER_SIZE = 2
	};
	
	
	void setArrowPressed(ArrowSection p_section, bool p_pressed);
	
	void setBarTexture(real p_x, real p_y, real p_w, real p_h,
	                   const engine::renderer::ColorRGBA& p_color);
	void setPickerTexture(real p_x, real p_y, real p_w, real p_h,
	                      const engine::renderer::ColorRGBA& p_color);
	void setArrowTexture(real p_x, real p_y, real p_w, real p_h,
	                     const engine::renderer::ColorRGBA& p_color);
	
	void setUVs(const engine::renderer::QuadSpritePtr& p_quad,
	            s32                  p_blockX,
	            s32                  p_blockY,
	            const math::Vector2& p_blockSize,
	            real                 p_topLeftX,
	            real                 p_topLeftY);
	
	// No assignment
	const Scrollbar& operator=(const Scrollbar&);
	
	
	engine::renderer::QuadSpritePtr m_barSegs[BarSection_Count];
	engine::renderer::QuadSpritePtr m_pickerQuad;
	engine::renderer::QuadSpritePtr m_arrowSegs[ArrowSection_Count];
	
	engine::renderer::TexturePtr m_barTexture;
	engine::renderer::TexturePtr m_blockTexture;
	engine::renderer::TexturePtr m_arrowTexture;
	engine::renderer::TexturePtr m_arrowPressedTexture;
	
	real m_value; // Range: m_minValue - m_maxValue
	real m_minValue;
	real m_maxValue;
	
	bool m_picked;
	bool m_pickUp;
	bool m_pickDown;
	bool m_pressUp;
	bool m_pressDown;
	real m_pickOffset;
	real m_stepSize;
	
	real m_pickerPos;
	
	math::Vector2 m_size;
	math::Vector2 m_pickerBlockSize;
	math::Vector2 m_barBlockSize;
	math::Vector2 m_arrowBlockSize;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SCROLLBAR_H)
