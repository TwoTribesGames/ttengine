#include <tt/platform/tt_error.h>
#include <tt/menu/elements/Image.h>
#include <tt/menu/MenuDebug.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

Image::Image(const std::string& p_name,
             const MenuLayout&  p_layout,
             const std::string& p_filename,
             real               p_u,
             real               p_v,
             s32                p_imageWidth,
             s32                p_imageHeight)
:
MenuElement(p_name, p_layout),
m_image(),
m_filename(p_filename)
{
	MENU_CREATION_Printf("Image::Image: Element '%s': "
	                     "New Image using file '%s'.\n",
	                     getName().c_str(), m_filename.c_str());
	
	// Get the texture for the specified image
	// FIXME: SUPPORT NAMESPACES
	m_image = engine::renderer::TextureCache::get(m_filename,  "");
		
	if(m_image == 0)
	{
		TT_PANIC("Loading Image '%s' failed.", m_filename.c_str());
		m_image.reset();
		return;
	}
	
	// Create a quad for rendering the image
	m_imageQuad = engine::renderer::QuadSprite::createQuad(m_image);
	
	// Set up the quad
	math::clamp(p_u, 0.0f, 1.0f);
	math::clamp(p_v, 0.0f, 1.0f);
	
	const s32 imgW = p_imageWidth  == 0 ? m_image->getWidth()  : p_imageWidth;
	const s32 imgH = p_imageHeight == 0 ? m_image->getHeight() : p_imageHeight;
	
	m_imageQuad->setWidth(static_cast<real>(imgW));
	m_imageQuad->setHeight(static_cast<real>(imgH));
	
	/*
	m_imageQuad->setTexcoordLeft(p_u);
	m_imageQuad->setTexcoordTop(p_v);
	
	real u1 = p_u + (static_cast<real>(imgW) / m_image->getWidth());
	real v1 = p_v + (static_cast<real>(imgH) / m_image->getHeight());
	math::clamp(u1, 0.0f, 1.0f);
	math::clamp(v1, 0.0f, 1.0f);
	m_imageQuad->setTexcoordRight(u1);
	m_imageQuad->setTexcoordBottom(v1);
	*/
	
	// Set the minimum and requested dimensions
	setMinimumWidth(imgW);
	setMinimumHeight(imgH);
	setRequestedWidth(imgW);
	setRequestedHeight(imgH);
}


Image::~Image()
{
	MENU_CREATION_Printf("Image::~Image: Element '%s': "
	                     "Destructing image element.\n",
	                     getName().c_str());
}


void Image::render(const math::PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	m_imageQuad->setPosition(static_cast<real>(p_rect.getCenterPosition().x),
	                         static_cast<real>(p_rect.getCenterPosition().y),
	                         static_cast<real>(p_z));
	m_imageQuad->update();
	m_imageQuad->render();
}


void Image::setImage(const std::string& p_filename)
{
	// Nothing to do if filename remained the same
	if (p_filename == m_filename)
	{
		return;
	}
	
	// Attempt to load the specified image
	// FIXME: SUPPORT NAMESPACES
	engine::renderer::TexturePtr image = engine::renderer::TextureCache::get(p_filename, "");
	
	if(image == 0)
	{
		TT_PANIC("Loading Image '%s' failed.", p_filename.c_str());
		return;
	}
	
	// FIXME: Use QuadSprite frames
	real u0 = 0;// Deprecated: arrow->getTexcoordLeft();
	real u1 = 0;// Deprecated: arrow->getTexcoordRight();
	real v0 = 0;// Deprecated: arrow->getTexcoordTop();
	real v1 = 0;// Deprecated: arrow->getTexcoordBottom();
	
	real w = m_imageQuad->getWidth();
	real h = m_imageQuad->getHeight();
	
	// Update the quad and image
	m_filename = p_filename;
	m_image    = image;
	
	m_imageQuad->setTexture(m_image);
	
	// Restore the quad settings
	/*
	m_imageQuad->setTexcoordLeft(u0);
	m_imageQuad->setTexcoordRight(u1);
	m_imageQuad->setTexcoordTop(v0);
	m_imageQuad->setTexcoordBottom(v1);
	*/ (void)u0; (void)u1; (void)v0; (void)v1;
	
	m_imageQuad->setWidth(w);
	m_imageQuad->setHeight(h);
}


Image* Image::clone() const
{
	return new Image(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

Image::Image(const Image& p_rhs)
:
MenuElement(p_rhs),
m_image(p_rhs.m_image),
m_filename(p_rhs.m_filename)
{
	if (p_rhs.m_imageQuad != 0)
	{
		m_imageQuad.reset(new engine::renderer::QuadSprite(*(p_rhs.m_imageQuad)));
	}
}

// Namespace end
}
}
}
