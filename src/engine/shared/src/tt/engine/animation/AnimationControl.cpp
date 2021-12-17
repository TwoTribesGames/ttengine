#include <tt/engine/animation/AnimationControl.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace animation {


AnimationControl::AnimationControl()
:
m_startTime(0),
m_endTime(0),
m_loop(true),
m_mode(Mode_Play),
m_time(0),
m_fps(30),
m_paused(false)
{
}


void AnimationControl::load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Animation Control Properties");

	if(p_file->read(&m_startTime, sizeof(m_startTime)) != sizeof(m_startTime))
	{
		TT_ERR_AND_RETURN("Failed to load start time");
	}
	if(p_file->read(&m_endTime, sizeof(m_endTime)) != sizeof(m_endTime))
	{
		TT_ERR_AND_RETURN("Failed to load end time");
	}
	TT_ASSERT(m_startTime < m_endTime);

	u32 loopFlag(0);
	if(p_file->read(&loopFlag, sizeof(loopFlag)) != sizeof(loopFlag))
	{
		TT_ERR_AND_RETURN("Failed to read loop flag");
	}
	// Set looping
	m_loop = (loopFlag == 1);

	u32 type(0);
	if(p_file->read(&type, sizeof(type)) != sizeof(type))
	{
		TT_ERR_AND_RETURN("Failed to read type");
	}
	m_mode = static_cast<Mode>(type);
}


void AnimationControl::update(real p_elapsedTime)
{
	// Increase time
	if(m_paused == false && m_mode != Mode_SetTime)
	{
		m_time += (p_elapsedTime * m_fps);

		if(m_loop && m_time > m_endTime)
		{
			m_time -= (m_endTime - m_startTime);
		}
	}
}


// Namespace end
}
}
}
