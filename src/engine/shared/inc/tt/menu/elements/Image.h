#if !defined(INC_TT_MENU_ELEMENTS_IMAGE_H)
#define INC_TT_MENU_ELEMENTS_IMAGE_H


#include <tt/engine/renderer/fwd.h>

#include <tt/menu/elements/MenuElement.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Element that displays an image. */
class Image : public MenuElement
{
public:
	Image(const std::string& p_name,
	      const MenuLayout&  p_layout,
	      const std::string& p_filename,
	      real               p_u           = 0.0f,
	      real               p_v           = 0.0f,
	      s32                p_imageWidth  = 0,
	      s32                p_imageHeight = 0);
	virtual ~Image();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	void setImage(const std::string& p_filename);
	
	virtual Image* clone() const;
	
protected:
	Image(const Image& p_rhs);
	
private:
	// No assignment
	const Image& operator=(const Image&);
	
	
	engine::renderer::TexturePtr    m_image;
	engine::renderer::QuadSpritePtr m_imageQuad;
	std::string                     m_filename;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_IMAGE_H)
