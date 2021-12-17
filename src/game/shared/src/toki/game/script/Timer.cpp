#include <tt/math/math.h>
#include <tt/platform/tt_error.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/Timer.h>

#include <toki/input/Recorder.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace game {
namespace script {

//--------------------------------------------------------------------------------------------------
// Public member functions

Timer::Timer(const std::string& p_name, real p_timeout, const entity::EntityHandle& p_target)
:
m_timeout(p_timeout),
m_suspended(false),
m_isSeparateCallback(false),
m_name(p_name),
m_hash(p_name),
m_target(p_target)
{
}


bool Timer::isAlive() const
{
	// Timed out
	if (tt::math::realLessEqual(m_timeout, 0.0f))
	{
		return false;
	}
	
	// Target entity is invalid
	return m_target.getPtr() != 0;
}


bool Timer::updateTime(real p_elapsedTime)
{
	game::entity::Entity* target = m_target.getPtr();
	
	if ((target != 0 && target->isSuspended()) || m_suspended)
	{
		return false;
	}
	
	m_timeout -= p_elapsedTime;
	
	return tt::math::realLessEqual(m_timeout, 0);
}


void Timer::doCallback()
{
	game::entity::Entity* target = m_target.getPtr();
	
	if (target != 0)
	{
		TT_ASSERTMSG(tt::math::realLessEqual(m_timeout, 0),
		             "Timer::doCallback for '%s' was scheduled but timeout (%f) is no longer 0. (Entity type: '%s')",
		             m_name.c_str(), m_timeout, target->getType().c_str());
		
	#if ENABLE_RECORDER_LOGGING
		std::ostream& log = AppGlobal::getInputRecorder()->log();
	
		log << "Timer went off for: " << m_name << " => " << m_target.getValue() << std::endl;
		log.flush();
	#endif
		
		if (m_isSeparateCallback)
		{
			target->getEntityScript()->callSqFun(m_name);
		}
		else
		{
			// Fire to onTimer callback (default)
			target->getEntityScript()->onTimer(m_name);
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
