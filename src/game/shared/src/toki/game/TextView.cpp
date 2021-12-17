#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/TextView.h>

namespace toki {
namespace game {


TextView::TextView(s32 p_maxLines, const tt::math::Point2& p_position, real p_textLifetime)
:
m_position(p_position),
m_maxLines(p_maxLines),
m_textLifetime(p_textLifetime),
m_fadeTime(0.2f),
m_lines()
{}


void TextView::addLine(const TextLine& p_line)
{
	m_lines.push_back(p_line);
	m_lines.back().lifetime = m_textLifetime;

	if(m_lines.size() > static_cast<TextLines::size_type>(m_maxLines))
	{
		m_lines.pop_front();
	}
}


void TextView::update(real p_elapsedTime)
{
	s32 linesToRemove(0);

	for(TextLines::iterator it = m_lines.begin(); it != m_lines.end(); ++it)
	{
		it->lifetime -= p_elapsedTime;

		if(it->lifetime < m_fadeTime)
		{
			it->color.a = static_cast<u8>((it->lifetime / m_fadeTime) * 255.0f);
		}
		if(it->lifetime < 0)
		{
			++linesToRemove;
		}
	}

	for(int i = 0; i < linesToRemove; ++i)
	{
		m_lines.pop_front();
	}
}


void TextView::render() const
{
	static const s32 lineHeight = 15;
	tt::math::Point2 pos(m_position);

	using namespace tt::engine::renderer;
	Renderer* renderer = Renderer::getInstance();

	for(TextLines::const_iterator it = m_lines.begin(); it != m_lines.end(); ++it)
	{
		renderer->getDebug()->renderText(it->text, pos.x, pos.y, it->color);
		pos.y += lineHeight;
	}
}

}
}
