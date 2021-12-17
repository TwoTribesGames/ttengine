#include <algorithm>

#include <tt/menu/elements/LinearTranslationAnimation.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

LinearTranslationAnimation::LinearTranslationAnimation()
{
}


LinearTranslationAnimation::~LinearTranslationAnimation()
{
}


void LinearTranslationAnimation::setStartPos(const math::Vector2& p_pos)
{
	m_startPos = p_pos;
}


math::Vector2 LinearTranslationAnimation::getStartPos() const
{
	return m_startPos;
}


void LinearTranslationAnimation::setDestinationPos(const math::Vector2& p_pos)
{
	m_destPos = p_pos;
}


math::Vector2 LinearTranslationAnimation::getDestinationPos() const
{
	return m_destPos;
}


void LinearTranslationAnimation::setDuration(s32 p_duration)
{
	m_duration = p_duration;
}


s32 LinearTranslationAnimation::getDuration() const
{
	return m_duration;
}


void LinearTranslationAnimation::update()
{
	if (m_frame >= m_duration)
	{
		return;
	}
	
	++m_frame;
	
	// Move the position
	m_pos += m_frameDelta;
	
	// Make sure we don't overshoot the destination position
	if (m_frameDelta.x > 0.0f)
	{
		m_pos.x = std::min(m_pos.x, m_destPos.x);
	}
	else
	{
		m_pos.x = std::max(m_pos.x, m_destPos.x);
	}
	
	if (m_frameDelta.y > 0.0f)
	{
		m_pos.y = std::min(m_pos.y, m_destPos.y);
	}
	else
	{
		m_pos.y = std::max(m_pos.y, m_destPos.y);
	}
}


math::Vector2 LinearTranslationAnimation::getPos() const
{
	return m_pos;
}


bool LinearTranslationAnimation::isDone() const
{
	return (m_frame == m_duration);
}


void LinearTranslationAnimation::start()
{
	m_frame = 0;
	
	m_frameDelta = m_destPos - m_startPos;
	m_frameDelta /= m_duration;
	
	m_pos = m_startPos;
}

// Namespace end
}
}
}
