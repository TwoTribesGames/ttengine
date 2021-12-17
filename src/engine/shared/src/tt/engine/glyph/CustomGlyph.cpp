#include <tt/engine/glyph/CustomGlyph.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/mem/util.h>

namespace tt {
namespace engine {
namespace glyph {


CustomGlyph::CustomGlyph(wchar_t p_char, const renderer::TexturePtr& p_texture, s32 p_yOffset)
:
m_texture(p_texture)
{
	// Warn if outside custom range
	TT_WARNING(((p_char >= 0xE000) && (p_char < 0xff8f)),
		"Custom char is not inside Unicode standard custom range.");

	// Can't make an empty one
	TT_NULL_ASSERT(m_texture);

	// Set base class stuff
	m_char   = p_char;
	m_yoff   = static_cast<s16>(p_yOffset);
	m_width  = static_cast<u16>(m_texture->getWidth());
	m_height = static_cast<u16>(m_texture->getHeight());
	m_charwidth = m_width;

	// Fill with dummy data to prevent crashes when accessing the pixels buffer
	// Alternative solutions:
	// 1) Fill the buffer with alpha channel of used texture (might be the most correct one)
	// 2) Overload all functions that access m_pixels[]
	// 3) Check for m_pixels == 0 in all functions using it (in the base class),
	//    doing something appropriate when it's zero
	// However this works fine for now
	m_pixels = new u8[m_width * m_height];
	mem::fill8(m_pixels, 255, static_cast<mem::size_type>(m_width * m_height));

	// Efficiency improvement:
	// Use option 1 from above and use GlyphAlpha8::draw() -> no need to keep texture + getPixel()
}


CustomGlyph::~CustomGlyph() 
{
}


void CustomGlyph::draw(s32 p_x, s32 p_y, renderer::TexturePainter& p_painter,
					   const renderer::ColorRGBA&) const
{
	// Figure out where to start painting
	s32 starty = (p_y - getHeight() + m_yoff);
	s32 startx = p_x;

	// We do not want to draw outside the target (negative x, y)
	starty = std::max(starty, s32(0));
	startx = std::max(startx, s32(0));

	// Figure out amount of pixels to write
	s32 pixelsx = std::min(static_cast<s32>(m_width) , p_painter.getTextureWidth()  - startx);
	s32 pixelsy = std::min(static_cast<s32>(m_height), p_painter.getTextureHeight() - starty);

	// We cannot draw a negative amount of pixels
	pixelsx = std::max(pixelsx, s32(0));
	pixelsy = std::max(pixelsy, s32(0));
	
	// Get access to custom pixel data
	renderer::TexturePainter source = m_texture->lock();
	
	// Temp value to hold pixel color
	renderer::ColorRGBA pixelColor;
	
	for (s32 y = 0; y < pixelsy; ++y)
	{
		for(s32 x = 0; x < pixelsx; ++x)
		{
			// Get pixel from custom image
			source.getPixel(x,y, pixelColor);

			// Overwrite color information (make similar to other platforms)
			pixelColor.r = pixelColor.g = pixelColor.b = 255;
			
			// Write it to the destination texture (after alpha test)
			if(pixelColor.a > 5)
			{
				p_painter.setPixel(startx + x, starty + y, pixelColor);
			}
		}
	}
}


}
}
}
