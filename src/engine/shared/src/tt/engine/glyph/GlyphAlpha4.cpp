#include <tt/code/Buffer.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/engine/glyph/GlyphAlpha4.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace glyph {


///  \brief Constructor
///  \param  p_file File pointing to a glyph definition
GlyphAlpha4::GlyphAlpha4(tt::code::BufferReadContext* p_readContext)
:
Glyph(),
m_yoff(0),
m_pixels(0L)
{
	if (p_readContext == 0)
	{
		return;
	}
	
	namespace bu = tt::code::bufferutils;
	{
		u16 glyphChar = bu::get<u16>(p_readContext);
		m_char = static_cast<wchar_t>(glyphChar);
	}
	m_width     = bu::get<u16>(p_readContext);
	m_height    = bu::get<u16>(p_readContext);
	
	m_yoff      = bu::get<s16>(p_readContext);
	m_charwidth = bu::get<u16>(p_readContext);
	
	// Allocate glyph pixel memory
	const s32 pixeldata_length = ((m_width >> 1) * m_height) >> 1;
	m_pixels = new u16[pixeldata_length];
	
	for (s32 i = 0; i < pixeldata_length; ++i)
	{
		m_pixels[i] = bu::get<u16>(p_readContext);
	}
}


/// Destruct
GlyphAlpha4::~GlyphAlpha4()
{
	delete[] m_pixels;
}


/// \brief Render glyph to framebuffer 
/// \param  p_x    X position
/// \param  p_y    Y position
/// \param  p_fbuf framebuffer caps
void GlyphAlpha4::draw(s32 p_x,
                       s32 p_y,
                       renderer::TexturePainter& p_texturepntr,
                       const renderer::ColorRGBA& p_color)
const
{
	s32 starty = (p_y - getHeight() + m_yoff);
	s32 startx = p_x;
	tt::engine::renderer::ColorRGBA pixelColor(p_color);
	
	for ( s16 y = 0; y < getHeight(); ++y)
	{
		for ( s16 x = 0; x < getWidth(); x += 4)
		{
			u32 index = static_cast<u32>(y * (m_width >> 2) + (x >> 2));
			u16 fourpix = m_pixels[index];
			
			u8 a[4];
			decodeAlpha(fourpix, a);
			
			for (s32 subX = 0; subX < 4; ++subX)
			{
				if (a[subX] != 0)
				{
					pixelColor.a = a[subX];
					p_texturepntr.setPixel(
							static_cast<s32>(startx + x + subX),
							static_cast<s32>(starty + y),
							pixelColor);
				}
			}
		}
	}
}


//! \brief Set all pixels of glyph to zero - Rubiks hack
void GlyphAlpha4::erasePixels()
{
	s32 pixeldataWordLength = ((m_width >> 1) * m_height) >> 1;
	for (s32 c = 0; c < pixeldataWordLength; ++c)
	{
		m_pixels[c] = 0;
	}
}


s32 GlyphAlpha4::getXMin(s32 p_y) const
{
	const s32 width      = getWidth();
	const s32 startOfRow = p_y * (m_width >> 2);
	
	for (s32 x = 0; x < width; x += 4)
	{
		const u32 index      = static_cast<u32>(startOfRow + (x >> 2) );
		const u16 fourpixels = m_pixels[index];
		
		u8 a[4];
		decodeAlpha(fourpixels, a);
		
		for (s32 i = 0; i < 4; ++i)
		{
			if (a[i] > ms_alphaThreshold)
			{
				return x + i;
			}
		}
	}
	return width;
}


void GlyphAlpha4::printPixelRow(s32 p_y) const
{
	const s32 width      = getWidth();
	const s32 startOfRow = p_y * (m_width >> 2);
	
	for (s32 x = 0; x < width; x += 4)
	{
		const u32 index      = static_cast<u32>(startOfRow + (x >> 2));
		const u16 fourpixels = m_pixels[index];
		
		u8 a[4];
		decodeAlpha(fourpixels, a);
		
		for (s32 i = 0; i < 4; ++i)
		{
			if (a[i] > 0)
			{
				if (a[i] > ms_alphaThreshold)
				{
					TT_Printf("X");
				}
				else
				{
					TT_Printf("x");
				}
			}
			else
			{
				TT_Printf(" ");
			}
		}
	}
}


s32 GlyphAlpha4::getXMax(s32 p_y, bool p_checkAlpha) const
{
	const s32 width      = getWidth();
	const s32 startOfRow = p_y * (m_width >> 2);
	
	for (s32 x = width - 4; x >= 0; x -= 4)
	{
		const u32 index      = static_cast<u32>(startOfRow + (x >> 2));
		const u16 fourpixels = m_pixels[index];
		
		u8 a[4];
		decodeAlpha(fourpixels, a);
		
		for (s32 i = 3; i >= 0; --i)
		{ 
			if (p_checkAlpha ? a[i] > ms_alphaThreshold : a[i] > 0)
			{
				return (width - 1) - (x + i);
			}
		}
	}
	return width;
}


s32 GlyphAlpha4::getYMax() const
{
	return getHeight() - m_yoff;
}

// Namespace end
}
}
}
