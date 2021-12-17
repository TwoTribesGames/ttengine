#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/event/Event.h>
#include <toki/game/event/EventMgr.h>
#include <toki/game/event/Signal.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace event {

//--------------------------------------------------------------------------------------------------
// Public member functions

EventMgr::EventMgr()
:
m_curEventsIndex(0),
m_soundChecker(),
m_signalCount(0),
m_eventCount(0)
{
}


void EventMgr::update(real /*p_deltatime*/)
{
	m_eventCount = static_cast<s32>(m_events[m_curEventsIndex].size());
	
	const s32 prevEventsIndex = m_curEventsIndex;
	m_curEventsIndex = (m_curEventsIndex == 0) ? 1 : 0;
	
	SignalSet allSignals;
	for (Events::const_iterator it = m_events[prevEventsIndex].begin();
	     it != m_events[prevEventsIndex].end(); ++it)
	{
		SignalSet signals = (*it).process(m_soundChecker);
		allSignals.insert(signals.begin(), signals.end());
	}
	m_signalCount = static_cast<s32>(allSignals.size());
	
	// now handle all signals
	entity::EntityMgr& mgr = AppGlobal::getGame()->getEntityMgr();
	for (SignalSet::const_iterator it = allSignals.begin(); it != allSignals.end(); ++it)
	{
		entity::Entity* target = mgr.getEntity((*it).getTarget());
		TT_NULL_ASSERT(target);
		if (target != 0)
		{
			target->handleEvent(*((*it).getEvent()));
		}
	}
	
	m_events[prevEventsIndex].clear();	// allSignals now contains dangling pointers
}


void EventMgr::registerEvent(const Event& p_event)
{
	m_events[m_curEventsIndex].insert(p_event);
}


void EventMgr::unregisterEvent(const Event& p_event)
{
	Events& events = m_events[m_curEventsIndex];
	Events::iterator it = events.find(p_event);
	if (it != events.end())
	{
		events.erase(it);
	}
}

//----------------------------------------------------------------------------------------------------------------
// Private member functions



// Namespace end
}
}
}
