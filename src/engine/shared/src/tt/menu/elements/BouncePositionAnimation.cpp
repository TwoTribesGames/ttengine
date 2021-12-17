#include <tt/platform/tt_error.h>
#include <tt/math/math.h>
#include <tt/menu/elements/BouncePositionAnimation.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {
namespace elements {

//------------------------------------------------------------------------------
// Public member functions

BouncePositionAnimation::BouncePositionAnimation()
:
m_speed(1),
m_delta(0.0f),
m_frame(0.0f),
m_size(4.0f)
{
}


BouncePositionAnimation::~BouncePositionAnimation()
{
}


void BouncePositionAnimation::update()
{
	m_frame += m_delta;
	
	if (m_frame >= 1.0f)
	{
		m_frame -= 1.0f;
	}
}


void BouncePositionAnimation::setSpeed(s32 p_speed)
{
	TT_ASSERTMSG(p_speed > 0, "Animation speed must be larger than 0.");
	m_speed = p_speed;
}


s32 BouncePositionAnimation::getSpeed() const
{
	return m_speed;
}


s32 BouncePositionAnimation::getX() const
{
	return static_cast<s32>(m_frame * m_size);
}


s32 BouncePositionAnimation::getY() const
{
	return static_cast<s32>(m_frame * m_size);
}


void BouncePositionAnimation::start()
{
	m_frame = 0.0f;
	m_delta = 1.0f / m_speed;
	m_size  = 4.0f;
}

// Namespace end
}
}
}
