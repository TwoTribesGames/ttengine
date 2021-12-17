#include <tt/code/Buffer.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/engine/glyph/GlyphAlpha8.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace glyph {


///  \brief Constructor
///  \param  p_file File pointing to a glyph definition
GlyphAlpha8::GlyphAlpha8(tt::code::BufferReadContext* p_readContext)
:
Glyph(),
m_yoff(0),
m_pixels(0)
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
	const s32 pixeldata_length = (m_width * m_height);
	
	m_pixels = new u8[pixeldata_length];
	for (s32 i = 0; i < pixeldata_length; ++i)
	{
		m_pixels[i] = bu::get<u8>(p_readContext);
	}
}


/// Destruct
GlyphAlpha8::~GlyphAlpha8()
{
	delete[] m_pixels;
}


/// \brief Render glyph to framebuffer 
/// \param  p_x    X position
/// \param  p_y    Y position
/// \param  p_fbuf framebuffer caps
void GlyphAlpha8::draw(s32 p_x,
                       s32 p_y,
                       renderer::TexturePainter& p_texturepntr,
                       const renderer::ColorRGBA& p_color)
const
{
	// START debug draw.
#if 0
	{
		const s32 starty = (p_y - getHeight() + m_yoff);
		const s32 startx = p_x;
		
		renderer::TexturePainter& p_painter = p_texturepntr;
		const s32 width = getWidth();
		const s32 widthBorder = width - 1;
		const s32 height = getHeight();
		const s32 heightBorder = height - 1;
		for (s32 i = 0; i < width; ++i)
		{
			p_painter.setPixel(startx + i, starty + 0,            p_color);
			p_painter.setPixel(startx + i, starty + heightBorder, p_color);
		}
		for (s32 i = 0; i < height; ++i)
		{
			p_painter.setPixel(startx + 0,           starty + i, p_color);
			p_painter.setPixel(startx + widthBorder, starty + i, p_color);
		}
		const float stepY = (float)height / (float)width;
		
		for (s32 x = 0; x < width / 2; ++x)
		{
			s32 y = s32(float(stepY * x));
			p_painter.setPixel(startx + x,               starty + y,                p_color);
			p_painter.setPixel(startx + widthBorder - x, starty + y,                p_color);
			p_painter.setPixel(startx + x,               starty + heightBorder - y, p_color);
			p_painter.setPixel(startx + widthBorder - x, starty + heightBorder - y, p_color);
		}
		
		p_painter.setPixel(startx + width / 2, starty + height / 2, p_color);
	}
#endif
	// END debug draw.
	
	s32 starty = (p_y - getHeight() + m_yoff);
	s32 startx = p_x;
	
	//TT_Printf("GlyphAlpha8::draw - startx: %d, p_x: %d, starty: %d, p_y: %d, getHeight(): %d, m_yoff: %d\n",
	//          startx, p_x, starty, p_y, getHeight(), m_yoff);
	
	engine::renderer::ColorRGBA pixelColor(p_color);
	
	for ( s16 y = 0; y < getHeight(); ++y)
	{
		for ( s16 x = 0; x < getWidth(); ++x )
		{
			u32 index = static_cast<u32>((y * m_width) + x);
			pixelColor.a = m_pixels[index];
			
			if (pixelColor.a != 0)
			{
				p_texturepntr.setPixel(
					static_cast<s32>(startx + x),
					static_cast<s32>(starty + y),
					pixelColor);
			}
		}
	}
}


//! \brief Set all pixels of glyph to zero - Rubiks hack
void GlyphAlpha8::erasePixels()
{
	s32 pixeldata_length = (m_width * m_height);
	for (int c=0; c < pixeldata_length; ++c)
	{
		m_pixels[c] = 0;
	}
}


s32 GlyphAlpha8::getXMin(s32 p_y) const
{
	const s32 width = getWidth();
	const s32 startOfRow = (p_y * width);
	for (s32 x = 0; x < width; ++x)
	{
		const u32 index = static_cast<u32>(startOfRow + x);
		const u8 alpha  = m_pixels[index];
		if (alpha > ms_alphaThreshold)
		{
			return x;
		}
	}
	return width;
}


void GlyphAlpha8::printPixelRow(s32 p_y) const
{
	const s32 width      = getWidth();
	const s32 startOfRow = p_y * width;
	for (s32 x = 0; x < width; ++x)
	{
		u32 index = static_cast<u32>((startOfRow) + x);
		u8 alpha  = m_pixels[index];
		if (alpha > 0)
		{
			if (alpha > ms_alphaThreshold)
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


s32 GlyphAlpha8::getXMax(s32 p_y, bool p_checkAlpha) const
{
	const s32 width      = getWidth();
	const s32 startOfRow = p_y * width;
	for (s32 x = width - 1; x >= 0; --x)
	{
		const u32 index = static_cast<u32>(startOfRow + x);
		const u8 alpha  = m_pixels[index];
		if( p_checkAlpha ? alpha > ms_alphaThreshold : alpha > 0)
		{
			return (width - 1) - x;
		}
	}
	return width;
}


void GlyphAlpha8::printPixelRowFromRight(s32 p_y) const
{
	const s32 width = getWidth();
	const s32 startOfRow = (p_y * width);
	for (s32 x = width - 1; x >= 0; --x)
	{
		u32 index = static_cast<u32>(startOfRow + x);
		u8 alpha  = m_pixels[index];
		if (alpha > 0)
		{
			if (alpha > ms_alphaThreshold)
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


s32 GlyphAlpha8::getYMax() const
{
	return getHeight() - m_yoff;
}


// Namespace end
}
}
}
