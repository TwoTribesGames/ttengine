#if !defined(INC_TOKI_GAME_TEXTVIEW_H)
#define INC_TOKI_GAME_TEXTVIEW_H


#include <deque>

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Point2.h>


namespace toki {
namespace game {


struct TextLine
{
	std::string                     text;
	tt::engine::renderer::ColorRGBA color;
	real                            lifetime;

	TextLine(const std::string& p_text, const tt::engine::renderer::ColorRGBA& p_color)
	:
	text    (p_text),
	color   (p_color),
	lifetime(0)
	{ }
};


class TextView
{
public:
	TextView(s32 p_maxLines = 0,
	         const tt::math::Point2& p_position = tt::math::Point2::zero,
			 real p_textLifetime = 5.0f);
	~TextView() {}

	void addLine(const TextLine& p_line);

	void update(real p_elapsedTime);
	void render() const;

private:
	tt::math::Point2 m_position;
	s32              m_maxLines;
	real             m_textLifetime;
	real             m_fadeTime;

	typedef std::deque<TextLine> TextLines;
	TextLines m_lines;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_TEXTVIEW_H)
