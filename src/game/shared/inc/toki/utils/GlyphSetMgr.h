#if !defined(INC_TOKI_UTILS_GLYPHSETMGR_H)
#define INC_TOKI_UTILS_GLYPHSETMGR_H


#include <tt/engine/glyph/fwd.h>
#include <toki/utils/types.h>

namespace toki {
namespace utils {



class GlyphSetMgr
{
public:
	static void loadAll();
	static void unloadAll();
	
	static const tt::engine::glyph::GlyphSetPtr& get(GlyphSetID p_id);
	
private:
	static tt::engine::glyph::GlyphSetPtr ms_glyphSets[GlyphSetID_Count];
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_UTILS_GLYPHSETMGR_H)
