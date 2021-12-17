#ifndef INC_TT_ENGINE_GLYPH_GLYPH_H
#define INC_TT_ENGINE_GLYPH_GLYPH_H

#include <tt/engine/glyph/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {
	class TexturePainter;
	struct ColorRGBA;
}

namespace glyph {

/*! \brief Character container. */
class Glyph
{
public:
	Glyph();
	virtual ~Glyph();
	
	//! \return  Unicode value
	inline wchar_t getChar() const { return m_char; }
	
	//! \brief Image width of glyph (contrast to character width)
	inline u16 getWidth() const { return m_width; }
	
	//! \return Character pixel height  (variable within font!)
	inline u16 getHeight() const { return m_height; }
	
	/*! \return The offset the top pixel of the character has from the baseline. (Placement, not actual pixel data.)
	    \note see: http://en.wikipedia.org/wiki/Ascender_(typography) */
	virtual s16 getAscenderHeight()  const = 0;
	
	//! \return The offset the bottom pixel of the character has from the baseline. (Placement, not actual pixel data.)
	virtual s16 getDescenderHeight() const = 0;
	
	//! \return Character width, contrast to character image width, which is always a multiple of 4.
	inline u16 getCharacterWidth() const { return m_charwidth; }
	
	inline void setCharacterWidth(u16 p_newWidth) { m_charwidth = p_newWidth; }
	
	virtual void draw(s32 p_x, s32 p_y, renderer::TexturePainter& p_painter, 
	                  const renderer::ColorRGBA& p_color) const = 0;
	
	virtual void erasePixels() = 0;
	
	virtual s32 getXMin() const;
	virtual s32 getXMin(s32 p_y) const = 0;
	virtual s32 getXMax(bool p_checkAlpha = true) const;
	virtual s32 getXMax(s32 p_y, bool p_checkAlpha = true) const = 0;
	//virtual s32 getYMin() const = 0;
	virtual s32 getYMax() const = 0;
	
	virtual void printPixels() const;
	virtual void printPixelRow(s32 p_y) const = 0;
	
	inline static void setAlphaThreshold(u8 p_threshold) { ms_alphaThreshold = p_threshold; }
	
protected:
	
	wchar_t m_char;
	u16     m_width;
	u16     m_height;
	u16     m_charwidth;
	
	static u8 ms_alphaThreshold;
	
private:
	Glyph(const Glyph&);
	const Glyph& operator=(const Glyph&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_GLYPH_GLYPH_H)
