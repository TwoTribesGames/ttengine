#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/script/Timer.h>
#include <toki/game/script/TimerMgr.h>
#include <toki/game/Game.h>
#include <toki/input/Recorder.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {

bool             TimerMgr::ms_initialized = false;
TimerMgr::Timers TimerMgr::ms_timers;


//--------------------------------------------------------------------------------------------------
// Public member functions

bool TimerMgr::isInitialized()
{
	return ms_initialized;
}


void TimerMgr::startTimer(const std::string& p_name, real p_timeout, const entity::EntityHandle& p_target)
{
	TT_ASSERT(ms_initialized);
	
	{
		const TimerPtr& timer = getTimer(p_name, p_target);
		if (timer != 0)
		{
			// timer already exist, overwrite with new timeout
			timer->setTimeout(p_timeout);
			return;
		}
	}
	
#if ENABLE_RECORDER_LOGGING
	std::ostream& log = AppGlobal::getInputRecorder()->log();
	log << "Started timer '" << p_name << "' (" << p_timeout << ") - " << p_target.getValue() << std::endl;
	log.flush();
#endif
	
	TimerPtr timer = TimerPtr(new Timer(p_name, p_timeout, p_target));
	
	if (tt::math::realLessEqual(p_timeout, 0.0f))
	{
		// Instantly fire
		timer->doCallback();
		return;
	}
	
	ms_timers[p_target].insert(std::make_pair(TimerHash(p_name), timer));
}


void TimerMgr::startCallbackTimer(const std::string& p_callback, real p_timeout,
                                  const entity::EntityHandle& p_target)
{
	TT_ASSERT(ms_initialized);
	
	{
		const TimerPtr& timer = getTimer(p_callback, p_target);
		if (timer != 0)
		{
			// timer already exist, overwrite with new timeout
			timer->setTimeout(p_timeout);
			return;
		}
	}
	
#if ENABLE_RECORDER_LOGGING
	std::ostream& log = AppGlobal::getInputRecorder()->log();
	log << "Started callback timer '" << p_callback << "' (" << p_timeout << ") - " << p_target.getValue() << std::endl;
	log.flush();
#endif
	
	TimerPtr timer = TimerPtr(new Timer(p_callback, p_timeout, p_target));
	timer->setIsSeparateCallback(true);
	
	if (tt::math::realLessEqual(p_timeout, 0.0f))
	{
		// Instantly fire
		timer->doCallback();
		return;
	}
	
	ms_timers[p_target].insert(std::make_pair(TimerHash(p_callback), timer));
}


void TimerMgr::stopTimer(const std::string& p_name, const entity::EntityHandle& p_target)
{
	Timers::iterator it = ms_timers.find(p_target);
	if (it != ms_timers.end())
	{
		EntityTimers& entityTimers = (*it).second;
		EntityTimers::iterator timersIt = entityTimers.find(TimerHash(p_name));
		if (timersIt != entityTimers.end())
		{
			entityTimers.erase(timersIt);
		}
		
		if (entityTimers.empty())
		{
			ms_timers.erase(it);
		}
	}
}


void TimerMgr::stopAllTimers(const entity::EntityHandle& p_target)
{
	Timers::iterator it = ms_timers.find(p_target);
	if (it != ms_timers.end())
	{
		ms_timers.erase(it);
	}
}


void TimerMgr::suspendTimer(const std::string& p_name, const entity::EntityHandle& p_target)
{
	const TimerPtr& timer = getTimer(p_name, p_target);
	if (timer != 0)
	{
		timer->setSuspended(true);
	}
}


void TimerMgr::suspendAllTimers(const entity::EntityHandle& p_target)
{
	Timers::iterator it = ms_timers.find(p_target);
	if (it != ms_timers.end())
	{
		for (EntityTimers::iterator timerIt = (*it).second.begin();
			timerIt != (*it).second.end(); ++timerIt)
		{
			(*timerIt).second->setSuspended(true);
		}
	}
}


void TimerMgr::resumeTimer(const std::string& p_name, const entity::EntityHandle& p_target)
{
	const TimerPtr& timer = getTimer(p_name, p_target);
	if (timer != 0)
	{
		timer->setSuspended(false);
	}
}


void TimerMgr::resumeAllTimers(const entity::EntityHandle& p_target)
{
	Timers::iterator it = ms_timers.find(p_target);
	if (it != ms_timers.end())
	{
		for (EntityTimers::iterator timerIt = (*it).second.begin();
			timerIt != (*it).second.end(); ++timerIt)
		{
			(*timerIt).second->setSuspended(false);
		}
	}
}


const TimerPtr& TimerMgr::getTimer(const std::string& p_name, const entity::EntityHandle& p_target)
{
	Timers::iterator it = ms_timers.find(p_target);
	if (it != ms_timers.end())
	{
		EntityTimers& entityTimers = (*it).second;
		EntityTimers::iterator timersIt = entityTimers.find(TimerHash(p_name));
		if (timersIt != entityTimers.end())
		{
			return (*timersIt).second;
		}
	}
	
	static TimerPtr empty;
	return empty;
}


void TimerMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_TimerMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the TimerMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	u32 timerCount = 0;
	for (Timers::iterator it = ms_timers.begin(); it != ms_timers.end(); ++it)
	{
		timerCount += static_cast<u32>((*it).second.size());
	}
	//TT_Printf("TimerMgr::serialize: Serializing %u timers.\n", timerCount);
	bu::put(timerCount, &context);
	
	for (Timers::iterator entityIt = ms_timers.begin(); entityIt != ms_timers.end(); ++entityIt)
	{
		const EntityTimers& timers = (*entityIt).second;
		for (EntityTimers::const_iterator timerIt = timers.begin(); timerIt != timers.end(); ++timerIt)
		{
			const TimerPtr& timer((*timerIt).second);
			
			bu::put(timer->getTimeout(),         &context);
			bu::put(timer->isSuspended(),        &context);
			bu::put(timer->getName(),            &context);
			bu::put(timer->isSeparateCallback(), &context);
			bu::putHandle(timer->getTarget(),    &context);
		}
	}
	
	context.flush();
}


void TimerMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_TimerMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the TimerMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	const u32 timerCount = bu::get<u32>(&context);
	//TT_Printf("TimerMgr::unserialize: Loading %u timers.\n", timerCount);
	
	ms_timers.clear();
	
	for (u32 i = 0; i < timerCount; ++i)
	{
		const real                 timeout            = bu::get<real       >(&context);
		const bool                 suspended          = bu::get<bool       >(&context);
		const std::string          name               = bu::get<std::string>(&context);
		const bool                 isSeparateCallback = bu::get<bool       >(&context);
		const entity::EntityHandle targetHandle       = bu::getHandle<entity::Entity>(&context);
		
		//TT_Printf("TimerMgr::unserialize: Timer %u: timeout %f | suspended: %s | name '%s' | target handle 0x%08X\n",
		//          i, timeout, suspended ? "true " : "false", name.c_str(), targetHandleValue);
		
		TimerPtr timer(new Timer(name, timeout, targetHandle));
		timer->setSuspended(suspended);
		timer->setIsSeparateCallback(isSeparateCallback);
		ms_timers[targetHandle][timer->getHash()] = timer;
	}
}


void TimerMgr::logTimers()
{
#if ENABLE_RECORDER_LOGGING
	std::ostream& log = AppGlobal::getInputRecorder()->log();
	using namespace game::entity;
	EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	
	for (Timers::iterator it = ms_timers.begin(); it != ms_timers.end(); ++it)
	{
		log << "Entity = " << it->first.getValue();
		Entity* target = entityMgr.getEntity((*it).first);
		if (target != 0)
		{
			EntityTimers& entityTimers = (*it).second;
			for (EntityTimers::iterator timersIt = entityTimers.begin();
			     timersIt != entityTimers.end(); ++timersIt)
			{
				const TimerPtr& timer = (*timersIt).second;

				log << " [" << timer->getName() << " - " << timer->getTimeout() << "]";
			}
		}
		log << std::endl;
	}
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TimerMgr::init()
{
	TT_ASSERT(ms_initialized == false);
	ms_initialized = true;
}


void TimerMgr::deinit()
{
	TT_ASSERT(ms_initialized);
	
	// Do deinit stuff here.
	tt::code::helpers::freeContainer(ms_timers);
	
	ms_initialized = false;
}


void TimerMgr::update(real p_elapsedTime)
{
	TT_ASSERT(ms_initialized);
	using namespace game::entity;
	EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	
	typedef std::vector<TimerPtr> TimerList;
	TimerList needsCallback;
	needsCallback.reserve(30);
	
	// First make a copy to make sure we don't get any updates of erased timers
	for (Timers::iterator it = ms_timers.begin(); it != ms_timers.end(); ++it)
	{
		Entity* target = entityMgr.getEntity((*it).first);
		if (target != 0)
		{
			EntityTimers& entityTimers = (*it).second;
			for (EntityTimers::iterator timersIt = entityTimers.begin();
			     timersIt != entityTimers.end(); ++timersIt)
			{
				const TimerPtr& timer = (*timersIt).second;
				if (timer->isAlive())
				{
					if (timer->updateTime(p_elapsedTime))
					{
						needsCallback.push_back(timer);
					}
				}
			}
		}
	}
	
	// Do the actual update
	for (TimerList::const_iterator it = needsCallback.begin(); it != needsCallback.end(); ++it)
	{
		const TimerPtr& ptr = (*it);
		
		// Check to make sure the timer ptr still exists.
		Timers::iterator findIt = ms_timers.find(ptr->getTarget());
		if (findIt != ms_timers.end())
		{
			EntityTimers& entityTimers = (*findIt).second;
			EntityTimers::iterator timersIt = entityTimers.find(ptr->getHash());
			if (timersIt != entityTimers.end() &&
			    (*timersIt).second == ptr)
			{
				// Found
				// Erease before doing the callback.
				entityTimers.erase(timersIt);
				ptr->doCallback();
			}
		}
	}
	
	// Now clean up the timers
	for (Timers::iterator it = ms_timers.begin(); it != ms_timers.end();)
	{
		Entity* target = entityMgr.getEntity((*it).first);
		if (target != 0)
		{
			EntityTimers& entityTimers = (*it).second;
			for (EntityTimers::iterator timersIt = entityTimers.begin(); timersIt != entityTimers.end();)
			{
				if ((*timersIt).second->isAlive())
				{
					++timersIt;
				}
				else
				{
					timersIt = entityTimers.erase(timersIt);
				}
			}
			if (entityTimers.empty())
			{
				it = ms_timers.erase(it);
			}
			else
			{
				++it;
			}
		}
		else
		{
			it = ms_timers.erase(it);
		}
	}
}


// Namespace end
}
}
}
