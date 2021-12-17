#include <tt/pres/PresentationObject.h>
#include <tt/pres/TriggerBase.h>


namespace tt {
namespace pres {


bool TriggerBase::ms_triggersDisabled = false;


//--------------------------------------------------------------------------------------------------
// Public member functions

TriggerBase::TriggerBase(const TriggerInfo& p_triggerInfo)
:
m_active(false),
m_paused(false),
m_triggered(false),
m_looping(p_triggerInfo.looping),
m_duration(p_triggerInfo.duration),
m_delay(p_triggerInfo.delay),
m_time(0.0f),
m_startTime(0.0f),
m_tags(p_triggerInfo.dataTags),
m_tagged(false),
m_syncId(p_triggerInfo.syncId),
m_endSync(p_triggerInfo.endSync)
{
}


void TriggerBase::update(real p_delta)
{
	if (m_active == false || m_paused || m_tagged == false)
	{
		return;
	}
	
	if (m_startTime > 0)
	{
		m_startTime -= p_delta;
		if (ms_triggersDisabled && m_startTime < 0) m_triggered = true;
		return;
	}
	
	if (m_triggered == false)
	{
		m_triggered = true;
		
		if (ms_triggersDisabled == false) trigger();
	}
	
	// when duration = 0 the trigger stops immediately
	if (m_duration == 0)
	{
		if (m_triggered)
		{
			m_active = false;
			stop();
		}
		return;
	}
	
	m_time -= p_delta;
	if (m_time <= 0)
	{
		if (m_looping)
		{
			m_time += m_duration;
			reTrigger();
		}
		else
		{
			stop();
		}
	}
}


void TriggerBase::start(const Tags& p_tags, PresentationObject* p_presObj,
                        const std::string& p_name, bool p_restart)
{
	if (m_tags.shouldPlay(p_tags, p_name))
	{
		if (m_active)
		{
			return;
		}
		setRanges(p_presObj);
		
		m_time      = m_duration;
		m_startTime = m_delay;
		m_tagged    = true;
		m_paused    = false;
		m_triggered = false;
		m_active = (m_endSync) ? p_restart : true;
		
		// Check if we should start right away.
		if (m_active && m_startTime <= 0.0f)
		{
			m_triggered = true;
			
			if (ms_triggersDisabled == false) trigger();
		}
		return;
	}
	else
	{
		m_tagged = false;
		m_active = false;
		presentationEnded();
	}
}


void TriggerBase::stop()
{
	if (m_active == false)
	{
		return;
	}
	presentationEnded();
	m_active = false;
}


void TriggerBase::pause()
{
	if (m_active == false || m_paused)
	{
		return;
	}
	m_paused = true;
}


void TriggerBase::resume()
{
	if (m_active == false || m_paused == false)
	{
		return;
	}
	m_paused = false;
}


void TriggerBase::reset()
{
	if (m_paused)
	{
		resume();
	}
	if (m_active)
	{
		stop();
	}
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

TriggerBase::TriggerBase(const TriggerBase& p_rhs)
:
TriggerInterface(p_rhs),
m_active(p_rhs.m_active),
m_paused(p_rhs.m_paused),
m_triggered(p_rhs.m_triggered),
m_looping(p_rhs.m_looping),
m_duration(p_rhs.m_duration),
m_delay(p_rhs.m_delay),
m_time(p_rhs.m_time),
m_startTime(p_rhs.m_startTime),
m_tags(p_rhs.m_tags),
m_tagged(p_rhs.m_tagged),
m_syncId(p_rhs.m_syncId),
m_endSync(p_rhs.m_endSync)
{
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TriggerBase::setRanges(PresentationObject* p_presObj)
{
	m_duration.updateValue(p_presObj);
	m_delay.updateValue(p_presObj);
	
	if (m_syncId.empty() == false)
	{
		if (m_endSync)
		{
			p_presObj->addEndSyncedTrigger(m_syncId, this);
		}
		else
		{
			p_presObj->addSyncedTrigger(m_syncId, this);
		}
	}
}


// Namespace end
}
}
