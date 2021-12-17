#if !defined(INC_TT_MENU_ELEMENTS_FATIMAGE_H)
#define INC_TT_MENU_ELEMENTS_FATIMAGE_H


#include <tt/engine/renderer/fwd.h>

#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Graphical representation of the save FAT. */
class FATImage : public MenuElement
{
public:
	FATImage(const std::string& p_name,
	         const MenuLayout&  p_layout);
	virtual ~FATImage();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual FATImage* clone() const;
	
protected:
	FATImage(const FATImage& p_rhs);
	
private:
	engine::renderer::TexturePtr    m_image;
	engine::renderer::QuadSpritePtr m_imageQuad;
	
	
	// No assignment
	const FATImage& operator=(const FATImage&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_FATIMAGE_H)
