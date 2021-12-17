#if !defined(INC_TT_GWEN_UTILS_H)
#define INC_TT_GWEN_UTILS_H


#include <Gwen/Structures.h>

#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace gwen {

inline engine::renderer::ColorRGBA toEngineColor(const Gwen::Color& p_color)
{
	return engine::renderer::ColorRGBA(p_color.r, p_color.g, p_color.b, p_color.a);
}


inline Gwen::Color toGwenColor(const engine::renderer::ColorRGBA& p_color)
{
	return Gwen::Color(p_color.r, p_color.g, p_color.b, p_color.a);
}

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_UTILS_H)
