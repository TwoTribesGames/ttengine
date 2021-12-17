#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>

namespace tt {
namespace engine {
namespace renderer {

//--------------------------------------------------------------------------------------------------
// Public member functions

TexturePainter::TexturePainter(const TexturePainter& p_rhs)
:
m_texture         (p_rhs.m_texture         ),
m_width           (p_rhs.m_width           ),
m_height          (p_rhs.m_height          ),
m_bufferSize      (p_rhs.m_bufferSize      ),
m_texelSize       (p_rhs.m_texelSize       ),
m_pixelData       (p_rhs.m_pixelData       ),
m_pixelDataChanged(p_rhs.m_pixelDataChanged)
{
}


TexturePainter::~TexturePainter()
{
	m_texture->m_pixelDataChanged = m_pixelDataChanged;
}


bool TexturePainter::setPixel(s32 p_x, s32 p_y, ColorRGBA p_color)
{
	if (p_x < 0 || p_x >= getTextureWidth() ||
		p_y < 0 || p_y >= getTextureHeight())
	{
		TT_WARN("Position (%d, %d) out of bounds (0, 0) - (%d, %d).",
			p_x, p_y, getTextureWidth(), getTextureHeight());
		return false;
	}
	
	u32* pixels = reinterpret_cast<u32*>(m_pixelData);
	pixels += (p_y * m_width) + p_x;
	
	if (m_texture != 0 && m_texture->isPremultiplied())
	{
		p_color.premultiply();
	}

#ifdef TT_PLATFORM_WIN
	std::swap(p_color.r, p_color.b);
#endif
	
	mem::copy32(pixels, &p_color, sizeof(ColorRGBA));
	
	m_pixelDataChanged = true;
	
	return true;
}


bool TexturePainter::getPixel(s32 p_x, s32 p_y, ColorRGBA& p_color) const
{
	if (p_x < 0 || p_x >= getTextureWidth() ||
		p_y < 0 || p_y >= getTextureHeight())
	{
		TT_WARN("Position (%d, %d) out of bounds (0, 0) - (%d, %d).",
			p_x, p_y, getTextureWidth(), getTextureHeight());
		return false;
	}
	
	u32* pixels = reinterpret_cast<u32*>(m_pixelData);
	pixels += (p_y * m_width) + p_x;
	
	mem::copy32(&p_color, pixels, sizeof(ColorRGBA));
	
#ifdef TT_PLATFORM_WIN
	std::swap(p_color.r, p_color.b);
#endif
	
	//TT_WARNING(p_color.a == 255 || m_texture == 0 || m_texture->isPremultiplied() == false,
	//           "Can't return the same color was what was set when it has alpha and using premultiplied textures.");
	
	return true;
}


void TexturePainter::clear()
{
	mem::zero8(m_pixelData, static_cast<mem::size_type>(m_bufferSize));
	m_pixelDataChanged = true;
}


void TexturePainter::clear(const ColorRGBA& p_color)
{
	u32* pixels = reinterpret_cast<u32*>(m_pixelData);
	
	for (s32 y = 0; y < m_height; ++y)
	{
		for (s32 x = 0; x < m_width; ++x)
		{
			mem::copy32(pixels, &p_color, sizeof(ColorRGBA));
			++pixels;
		}
	}
	m_pixelDataChanged = true;
}


void TexturePainter::clearToWhite()
{
	mem::fill8(m_pixelData, 255, static_cast<mem::size_type>(m_bufferSize));
	m_pixelDataChanged = true;
}


void TexturePainter::setRegion(s32 p_width, s32 p_height, s32 p_offsetX, s32 p_offsetY, const u8* p_pixelData)
{
	if (p_offsetX + p_width > m_width)
	{
		TT_PANIC("Region is too wide; clipping not supported");
		return;
	}
	if (p_offsetY + p_height > m_height)
	{
		TT_PANIC("Region is too tall; clipping not supported");
		return;
	}
	u8* dst = m_pixelData;
	const u8* src = p_pixelData;
	u32 srcPitch = p_width * m_texelSize;
	u32 dstPitch = m_width * m_texelSize;
	
	for (s32 y = 0; y < p_height; ++y)
	{
		std::memcpy(&dst[((p_offsetY + y) * dstPitch) + (m_texelSize * p_offsetX)], &src[y * srcPitch], srcPitch);
	}
	m_pixelDataChanged = true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

TexturePainter::TexturePainter(Texture* p_texture)
	:
m_texture         (p_texture                    ),
m_width           (p_texture->getWidth()        ),
m_height          (p_texture->getHeight()       ),
m_bufferSize      (0                            ),
m_texelSize       (p_texture->getBytesPerPixel()),
m_pixelData       (0                            ),
m_pixelDataChanged(false                        )
{
	// Machine storage size
	m_bufferSize = m_width * m_height * m_texelSize;
	m_pixelData = p_texture->m_pixels;
	TT_NULL_ASSERT(m_pixelData);
}

// Namespace end
}
}
}
