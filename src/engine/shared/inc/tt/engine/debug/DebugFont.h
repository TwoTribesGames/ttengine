#if !defined(INC_TT_ENGINE_DEBUG_DEBUGFONT_H)
#define INC_TT_ENGINE_DEBUG_DEBUGFONT_H


#include <tt/math/Point2.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace debug {


class DebugFont
{
private:
	static void initialize();
	static void destroy();

	static void setColor(const renderer::ColorRGBA& p_color);

	inline static void setScale(real p_scale) { ms_scale = p_scale; }

	static void draw(const std::string& p_msg, s32 p_x = 0, s32 p_y = 0);

	static inline void draw(const std::string&  p_msg, const math::Point2& p_pos) { draw(p_msg, p_pos.x, p_pos.y); }

	// Actually draw to the screen
	static void flush();

	DebugFont();
	~DebugFont();

	static renderer::TexturePtr buildFontTexture();
	static renderer::BatchQuad& getNextQuad();

	static renderer::QuadBufferPtr       ms_buffer;
	static renderer::BatchQuadCollection ms_quads;
	static renderer::ColorRGBA           ms_color;
	static u32                           ms_quadIndex;

	static bool ms_initialized;
	static real ms_scale;

	friend class DebugRenderer;
};


}
}
}

#endif // INC_TT_ENGINE_DEBUG_DEBUGFONT_H
