#include <tt/audio/xact/StopEvent.h>
#include <tt/audio/xact/StopEventInstance.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/utils.h>
#include <tt/math/Random.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


//#define STOP_DEBUG
#ifdef STOP_DEBUG
	#define Stop_Printf TT_Printf
#else
	#define Stop_Printf(...)
#endif


#define STOP_WARN
#ifdef STOP_WARN
	#define Stop_Warn TT_Printf
#else
	#define Stop_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {


StopEventInstance::StopEventInstance(StopEvent* p_stopEvent, TrackInstance* p_track)
:
m_stopEvent(p_stopEvent),
m_track(p_track),
m_startEvent(0),
m_paused(false)
{
	TT_ASSERTMSG(m_stopEvent != 0, "StopEventInstance::StopEventInstance: stop event must not be 0");
	TT_ASSERTMSG(m_track     != 0, "StopEventInstance::StopEventInstance: track must not be 0");
}


StopEventInstance::~StopEventInstance()
{
	
}


bool StopEventInstance::play()
{
	m_startEvent = getTimeStamp();
	m_paused = false;
	return true;
}


bool StopEventInstance::stop()
{
	return true;
}


bool StopEventInstance::pause()
{
	m_paused = true;
	return true;
}


bool StopEventInstance::resume()
{
	m_paused = false;
	return true;
}


void StopEventInstance::update(real p_time)
{
	if (m_paused)
	{
		return;
	}
	if (p_time >= m_startEvent)
	{
		// stop
		
		switch (m_stopEvent->getStopBehavior())
		{
		case StopEvent::Behavior_Immediate:
			switch (m_stopEvent->getStopObject())
			{
			case StopEvent::Object_EntireCue:
				m_track->stopCue();
				break;
					
			case StopEvent::Object_TrackOnly:
				m_track->stop();
				break;
				
			default:
				break;
			}
			break;
			
		case StopEvent::Behavior_WithRelease:
			// unsupported
			break;
			
		default:
			break;
		}
	}
}


// Private Functions

real StopEventInstance::getTimeStamp() const
{
	real timestamp = m_stopEvent->getTimeStamp() * 1000;
	real offset = m_stopEvent->getRandomOffset() * 1000;
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(offset)));
	timestamp += rnd;
	return timestamp / 1000.0f;
}


} // namespace end
}
}
