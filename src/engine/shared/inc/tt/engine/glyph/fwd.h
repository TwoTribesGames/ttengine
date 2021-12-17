#if !defined(INC_TT_ENGINE_GLYPH_FWD_H)
#define INC_TT_ENGINE_GLYPH_FWD_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace glyph {

class Glyph;
typedef tt_ptr<Glyph>::shared GlyphPtr;
typedef tt_ptr<Glyph>::weak   GlyphWeakPtr;

class GlyphSet;
typedef tt_ptr<GlyphSet>::shared GlyphSetPtr;
typedef tt_ptr<GlyphSet>::weak   GlyphSetWeakPtr;

}
}
}

#endif // !defined(INC_TT_ENGINE_GLYPH_FWD_H)
