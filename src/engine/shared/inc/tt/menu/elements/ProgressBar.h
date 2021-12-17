#if !defined(INC_TT_MENU_ELEMENTS_PROGRESSBAR_H)
#define INC_TT_MENU_ELEMENTS_PROGRESSBAR_H


#include <tt/engine/renderer/fwd.h>

#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Element that displays a progress bar. */
class ProgressBar : public MenuElement
{
public:
	ProgressBar(const std::string& p_name,
	            const MenuLayout&  p_layout);
	virtual ~ProgressBar();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual ProgressBar* clone() const;
	
	void setValue(real p_value);
	inline real getValue() const { return m_value; }
	
	void setMaxValue(real p_maxValue);
	inline real getMaxValue() const { return m_maxValue; }
	
protected:
	ProgressBar(const ProgressBar& p_rhs);
	
private:
	enum BarSegment
	{
		BarSegment_Center = 0,
		BarSegment_LeftEdge,
		BarSegment_RightEdge,
		
		BarSegment_Count
	};
	
	
	void createBars();
	void setBarUVs(const engine::renderer::QuadSpritePtr& p_quad,
	               const math::VectorRect& p_uvRect,
	               const math::Vector2&    p_blockSize,
	               s32                     p_blockX,
	               s32                     p_blockY);
	void updateOverlayBar();
	
	
	bool m_shouldUpdate;
	real m_overlayWidth;
	
	real m_value;
	real m_maxValue;
	
	engine::renderer::TexturePtr    m_backgroundTexture;
	engine::renderer::QuadSpritePtr m_backgroundSegments[BarSegment_Count]; //!< Background bar quad segments.
	math::Vector2    m_backgroundSegmentSize; //!< Dimensions of one background segment.
	math::VectorRect m_backgroundTextureRect; //!< Texture UV rectangle for the background.
	
	engine::renderer::TexturePtr    m_overlayTexture;
	engine::renderer::QuadSpritePtr m_overlaySegments[BarSegment_Count]; //!< Overlay bar quad segments.
	math::Vector2    m_overlaySegmentSize; //!< Dimensions of one overlay segment.
	math::VectorRect m_overlayTextureRect; //!< Texture UV rectangle for the overlay.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_PROGRESSBAR_H)
