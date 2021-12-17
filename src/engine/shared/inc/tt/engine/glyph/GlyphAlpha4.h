#ifndef INC_TT_ENGINE_GLYPH_GLYPHALPHA4_H
#define INC_TT_ENGINE_GLYPH_GLYPHALPHA4_H

#include <tt/code/fwd.h>
#include <tt/engine/glyph/Glyph.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace renderer {
	class TexturePainter;
	struct ColorRGBA;
}

namespace glyph {

/**
 * Unicode character container
 */
class GlyphAlpha4 : public Glyph
{
public:
	GlyphAlpha4(tt::code::BufferReadContext* p_readContext);
	virtual ~GlyphAlpha4();
	
	virtual void draw(s32 p_x,
	                  s32 p_y,
	                  renderer::TexturePainter& p_texturepntr,
	                  const renderer::ColorRGBA& p_color ) const;
	
	virtual void erasePixels();
	
	virtual s16 getAscenderHeight()  const { return static_cast<s16>(getHeight() - m_yoff); }
	virtual s16 getDescenderHeight() const { return static_cast<s16>(-m_yoff);              }
	
	virtual s32 getXMin(s32 p_y) const;
	using Glyph::getXMin;
	virtual s32 getXMax(s32 p_y, bool p_checkAlpha = true) const;
	using Glyph::getXMax;
	virtual s32 getYMax() const;
	
	virtual void printPixelRow(s32 p_y) const;
	
private:
	inline void decodeAlpha(u16 p_pixelData, u8* p_fourPixels) const
	{
		p_fourPixels[0] = static_cast<u8>((p_pixelData & 0xF000) >> 8);
		p_fourPixels[1] = static_cast<u8>((p_pixelData & 0x0F00) >> 4);
		p_fourPixels[2] = static_cast<u8>((p_pixelData & 0x00F0)     );
		p_fourPixels[3] = static_cast<u8>((p_pixelData & 0x000F) << 4);
		
		for (s32 i = 0; i < 4; ++i)
		{
			p_fourPixels[i] |= static_cast<u8>(p_fourPixels[i] >> 4);
		}
	}
	
	GlyphAlpha4( const GlyphAlpha4& );
	const GlyphAlpha4& operator=( const GlyphAlpha4& );
	
	s16     m_yoff;
	u16*    m_pixels;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_GLYPH_GLYPHALPHA4_H)
