#include <algorithm>

#include <tt/platform/tt_error.h>
#include <tt/thread/Alarm.h>


namespace tt {
namespace thread {

Alarm::Alarms Alarm::ms_alarms;
u64           Alarm::ms_time = 0;


// ------------------------------------------------------------
// Public functions


Alarm::~Alarm()
{
	Alarms::iterator it = std::find(ms_alarms.begin(), ms_alarms.end(), this);
	if (it != ms_alarms.end())
	{
		ms_alarms.erase(it);
	}
}


Alarm* Alarm::createAlarm(u64 p_start, callback p_callback, void* p_userData, bool p_hardware)
{
	if (p_hardware)
	{
		TT_WARN("Hardware alarms not supported");
		return 0;
	}
	Alarm* alarm = new Alarm(p_start, 0, p_callback, p_userData, p_hardware);
	ms_alarms.push_back(alarm);
	return alarm;
}


Alarm* Alarm::createPeriodicAlarm(u64 p_start, u64 p_period, callback p_callback, void* p_userData, bool p_hardware)
{
	if (p_hardware)
	{
		TT_WARN("Hardware alarms not supported");
		return 0;
	}
	TT_WARNING(p_period != 0, "Period should be greater than 0.");
	
	Alarm* alarm = new Alarm(p_start, p_period, p_callback, p_userData, p_hardware);
	ms_alarms.push_back(alarm);
	return alarm;
}


void Alarm::update(u64 p_delta)
{
	ms_time += p_delta;
	for (;;)
	{
		Alarm* lowest = 0;
		
		// find timer with lowest time
		for (Alarms::iterator it = ms_alarms.begin(); it != ms_alarms.end(); ++it)
		{
			if ((*it)->m_hardware || (*it)->m_time == 0)
			{
				continue;
			}
			if ((*it)->m_time < ms_time)
			{
				if (lowest == 0 || (*it)->m_time < lowest->m_time)
				{
					lowest = (*it);
				}
			}
		}
		
		if (lowest != 0)
		{
			lowest->trigger();
		}
		else
		{
			// done
			break;
		}
	}
}


// ------------------------------------------------------------
// Private functions

Alarm::Alarm(u64 p_start, u64 p_period, callback p_callback, void* p_userData, bool p_hardware)
:
m_period(p_period),
m_time(ms_time + p_start + p_period),
m_callback(p_callback),
m_userData(p_userData),
m_hardware(p_hardware),
m_internal(0)
{
}


void Alarm::trigger()
{
	if (m_period != 0)
	{
		m_time += m_period;
	}
	else
	{
		m_time = 0;
	}
	
	if (m_callback != 0)
	{
		// WARNING: the callback function may delete this alarm
		// ensure no member variables are modified after the callback
		m_callback(m_userData);
	}
}


// namespace end
}
}
