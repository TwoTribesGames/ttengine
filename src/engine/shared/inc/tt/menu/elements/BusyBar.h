#if !defined(INC_TT_MENU_ELEMENTS_BUSYBAR_H)
#define INC_TT_MENU_ELEMENTS_BUSYBAR_H


#include <tt/engine/renderer/fwd.h>
#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief A busy bar. */
class BusyBar : public MenuElement
{
public:
	typedef action_element_tag element_category;
	
	
	BusyBar(const std::string& p_name,
	        const MenuLayout&  p_layout);
	
	virtual ~BusyBar();
	
	virtual void update();
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual BusyBar* clone() const;
	
protected:
	BusyBar(const BusyBar& p_rhs);
	
private:
	void createBar();
	void renderDot(const math::PointRect& p_rect, s32 p_x, s32 p_z);
	
	// Cannot assign to menu elements
	const BusyBar& operator=(const BusyBar&);
	
	
	engine::renderer::TexturePtr    m_texture;
	engine::renderer::QuadSpritePtr m_quad;
	
	real m_counter;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_BUSYBAR_H)
