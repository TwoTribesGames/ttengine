#ifndef INC_TT_ENGINE_GLYPH_CUSTOMGLYPH_H
#define INC_TT_ENGINE_GLYPH_CUSTOMGLYPH_H


#include <tt/engine/glyph/GlyphAlpha8.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace glyph {

class CustomGlyph : public GlyphAlpha8
{
public:
	CustomGlyph(wchar_t p_char, const renderer::TexturePtr& p_texture, s32 p_yOffset = 0);
	virtual ~CustomGlyph();
	
	virtual void draw(s32 p_x, s32 p_y, renderer::TexturePainter& p_painter,
		const renderer::ColorRGBA& p_color) const;
	
private:
	renderer::TexturePtr m_texture;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_GLYPH_CUSTOMGLYPH_H)
