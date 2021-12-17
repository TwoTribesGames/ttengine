#if !defined(INC_TT_MENU_ELEMENTS_MARKER_H)
#define INC_TT_MENU_ELEMENTS_MARKER_H


#include <map>

#include <tt/engine/renderer/fwd.h>

#include <tt/math/Vector2.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Marker element. */
class Marker : public MenuElement
{
public:
	Marker(const std::string& p_name,
	       const MenuLayout&  p_layout);
	virtual ~Marker();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual Marker* clone() const;
	
protected:
	Marker(const Marker& p_rhs);
	
private:
	enum Corner
	{
		Corner_TopLeft,
		Corner_TopRight,
		Corner_BotLeft,
		Corner_BotRight,
		
		Corner_Count
	};
	
	
	void setTexture(real p_x0, real p_y0, real p_x1, real p_y1);
	void setUVs(const engine::renderer::QuadSpritePtr& p_quad,
	            s32 p_blockX, s32 p_blockY);
	
	// No assignment
	const Marker& operator=(const Marker&);
	
	
	engine::renderer::TexturePtr    m_texture;
	engine::renderer::QuadSpritePtr m_corners[Corner_Count];
	
	math::Vector2 m_topLeft;
	math::Vector2 m_botRight;
	math::Vector2 m_blockSize;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_MARKER_H)
