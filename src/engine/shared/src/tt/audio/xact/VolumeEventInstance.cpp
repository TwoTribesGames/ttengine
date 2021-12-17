#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/utils.h>
#include <tt/audio/xact/VolumeEvent.h>
#include <tt/audio/xact/VolumeEventInstance.h>
#include <tt/math/Random.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


//#define VOLUME_DEBUG
#ifdef VOLUME_DEBUG
	#define Volume_Printf TT_Printf
#else
	#define Volume_Printf(...)
#endif


#define VOLUME_WARN
#ifdef VOLUME_WARN
	#define Volume_Warn TT_Printf
#else
	#define Volume_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {


VolumeEventInstance::VolumeEventInstance(VolumeEvent* p_volumeEvent, TrackInstance* p_track)
:
m_volumeEvent(p_volumeEvent),
m_track(p_track),
m_nextStart(0),
m_loopCount(0),
m_paused(false)
{
	TT_ASSERTMSG(m_volumeEvent != 0, "VolumeEventInstance::VolumeEventInstance: volume event must not be 0");
	TT_ASSERTMSG(m_track       != 0, "VolumeEventInstance::VolumeEventInstance: track must not be 0");
}


bool VolumeEventInstance::play()
{
	m_loopCount = 0;
	m_nextStart = getTimeStamp();
	m_paused    = false;
	return true;
}


bool VolumeEventInstance::stop()
{
	return true;
}


bool VolumeEventInstance::pause()
{
	m_paused = true;
	return true;
}


bool VolumeEventInstance::resume()
{
	m_paused = false;
	return true;
}


void VolumeEventInstance::update(real p_time)
{
	if (m_paused)
	{
		return;
	}
	switch (m_volumeEvent->getSettingType())
	{
	case VolumeEvent::Setting_Equation:
		// update equation
		if (p_time >= m_nextStart)
		{
			if (m_volumeEvent->getInfinite() == false)
			{
				if (m_volumeEvent->getRepeats() < m_loopCount)
				{
					// early abort
					break;
				}
				++m_loopCount;
			}
			
			real volume = 0.0f;
			switch (m_volumeEvent->getEquationType())
			{
			case VolumeEvent::Equation_Value:
				volume = m_volumeEvent->getValue();
				break;
				
			case VolumeEvent::Equation_Random:
				volume = getRandomVolume();
				break;
				
			default:
				break;
			}
			
			switch (m_volumeEvent->getOperationType())
			{
			case VolumeEvent::Operation_Add:
				m_track->setVolume(m_track->getVolume() + volume);
				break;
				
			case VolumeEvent::Operation_Replace:
				m_track->setVolume(volume);
				break;
				
			default:
				break;
			}
			
			m_nextStart += m_volumeEvent->getFrequency();
		}
		break;
		
	case VolumeEvent::Setting_Ramp:
		// not supported
		break;
		
	default:
		break;
	}
}


// Private Functions

real VolumeEventInstance::getRandomVolume() const
{
	real min = m_volumeEvent->getRangeMin() * 1000.0f;
	real max = m_volumeEvent->getRangeMax() * 1000.0f;
	real range = max - min;
	if (range <= 0)
	{
		return min / 1000.0f;
	}
	
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(range)));
	rnd += min;
	return rnd / 1000.0f;
}


real VolumeEventInstance::getTimeStamp() const
{
	real offset = m_volumeEvent->getRandomOffset() * 1000;
	if (offset == 0.0f)
	{
		return m_volumeEvent->getTimeStamp();
	}
	
	real timestamp = m_volumeEvent->getTimeStamp() * 1000;
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(offset)));
	timestamp += rnd;
	return timestamp / 1000.0f;
}


} // namespace end
}
}
