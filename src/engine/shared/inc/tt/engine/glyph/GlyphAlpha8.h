#ifndef INC_TT_LOCALIZATION_GLYPHALPHA8_H
#define INC_TT_LOCALIZATION_GLYPHALPHA8_H

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
class GlyphAlpha8 : public Glyph
{
public:
	GlyphAlpha8(tt::code::BufferReadContext* p_readContext);
	virtual ~GlyphAlpha8();
	
	virtual void draw(
			s32 p_x, 
			s32 p_y,
			engine::renderer::TexturePainter& p_texturepntr,
			const engine::renderer::ColorRGBA& p_color ) const;
	
	virtual void erasePixels();
	
	virtual s16 getAscenderHeight()  const { return static_cast<s16>(getHeight() - m_yoff); }
	virtual s16 getDescenderHeight() const { return static_cast<s16>(-m_yoff);              }
	
	virtual s32 getXMin(s32 p_y) const;
	using Glyph::getXMin;
	virtual s32 getXMax(s32 p_y, bool p_checkAlpha) const;
	using Glyph::getXMax;
	virtual s32 getYMax() const;
	
	virtual void printPixelRow(s32 p_y) const;
	void printPixelRowFromRight(s32 p_y) const;

protected:
	GlyphAlpha8()
	:
	m_yoff(0),
	m_pixels(0)
	{ }
	
	s16 m_yoff;
	u8* m_pixels;
	
private:
	// No copying
	GlyphAlpha8( const GlyphAlpha8& );
	const GlyphAlpha8& operator=( const GlyphAlpha8& );
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_LOCALIZATION_GLYPHALPHA8_H)
