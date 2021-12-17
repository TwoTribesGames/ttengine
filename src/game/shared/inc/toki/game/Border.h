#if !defined(INC_TOKI_GAME_BORDER_H)
#define INC_TOKI_GAME_BORDER_H


#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace game {

class Border;
typedef tt_ptr<Border>::shared BorderPtr;


/*! \brief Simple quad border around a specified rectangle. */
class Border
{
public:
	static BorderPtr create(real p_thickness, const tt::engine::renderer::ColorRGBA& p_color);
	~Border();
	
	void fitAroundRectangle(const tt::math::VectorRect& p_rect);
	void fitInsideRectangle(const tt::math::VectorRect& p_rect);
	void render();
	
	/*! \return The rectangle that this border fits around. */
	inline const tt::math::VectorRect& getInnerRect() const { return m_rect; }
	
	inline real                                   getThickness() const { return m_thickness; }
	inline const tt::engine::renderer::ColorRGBA& getColor()     const { return m_color;     }
	
private:
	Border(real p_thickness, const tt::engine::renderer::ColorRGBA& p_color);
	
	// No copying
	Border(const Border&);
	Border& operator=(const Border&);
	
	
	real                                m_thickness;
	tt::engine::renderer::ColorRGBA     m_color;
	tt::math::VectorRect                m_rect;
	tt::engine::renderer::QuadSpritePtr m_top;
	tt::engine::renderer::QuadSpritePtr m_bottom;
	tt::engine::renderer::QuadSpritePtr m_left;
	tt::engine::renderer::QuadSpritePtr m_right;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_BORDER_H)
