#include <tt/audio/xact/PitchEvent.h>
#include <tt/audio/xact/PitchEventInstance.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/utils.h>
#include <tt/math/Random.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


//#define PITCH_DEBUG
#ifdef PITCH_DEBUG
	#define Pitch_Printf TT_Printf
#else
	#define Pitch_Printf(...)
#endif


#define PITCH_WARN
#ifdef PITCH_WARN
	#define Pitch_Warn TT_Printf
#else
	#define Pitch_Warn(...)
#endif

namespace tt {
namespace audio {
namespace xact {


PitchEventInstance::PitchEventInstance(PitchEvent* p_pitchEvent, TrackInstance* p_track)
:
m_pitchEvent(p_pitchEvent),
m_track(p_track),
m_nextStart(0.0f),
m_loopCount(0),
m_paused(false)
{
	TT_ASSERTMSG(m_pitchEvent != 0, "PitchEventInstance::PitchEventInstance: pitch event must not be 0");
	TT_ASSERTMSG(m_track      != 0, "PitchEventInstance::PitchEventInstance: track must not be 0");
}


PitchEventInstance::~PitchEventInstance()
{
	
}


bool PitchEventInstance::play()
{
	m_loopCount = 0;
	m_paused = false;
	m_nextStart = getTimeStamp();
	return true;
}


bool PitchEventInstance::stop()
{
	return true;
}


bool PitchEventInstance::pause()
{
	m_paused = true;
	return true;
}


bool PitchEventInstance::resume()
{
	m_paused = false;
	return true;
}


void PitchEventInstance::update(real p_time)
{
	// do stuff
	if (m_paused)
	{
		return;
	}
	
	switch (m_pitchEvent->getSettingType())
	{
	case PitchEvent::Setting_Equation:
		// update equation
		if (p_time >= m_nextStart)
		{
			if (m_pitchEvent->getInfinite() == false)
			{
				if (m_pitchEvent->getRepeats() < m_loopCount)
				{
					// early abort
					break;
				}
				++m_loopCount;
			}
			
			real pitch = 1.0f;
			switch (m_pitchEvent->getEquationType())
			{
			case PitchEvent::Equation_Value:
				pitch = m_pitchEvent->getValue();
				break;
				
			case PitchEvent::Equation_Random:
				pitch = getRandomPitch();
				break;
				
			default:
				break;
			}
			
			switch (m_pitchEvent->getOperationType())
			{
			case PitchEvent::Operation_Add:
				m_track->setPitch(m_track->getPitch() + pitch);
				break;
				
			case PitchEvent::Operation_Replace:
				m_track->setPitch(pitch);
				break;
				
			default:
				break;
			}
			
			m_nextStart += m_pitchEvent->getFrequency();
		}
		break;
		
	case PitchEvent::Setting_Ramp:
		// not supported
		break;
		
	default:
		break;
	}
}


// Private Functions

real PitchEventInstance::getRandomPitch() const
{
	real min = m_pitchEvent->getRangeMin() * 1000;
	real max = m_pitchEvent->getRangeMax() * 1000;
	real range = max - min;
	if (range <= 0.0f)
	{
		return min / 1000.0f;
	}
	
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(range)));
	rnd += min;
	return rnd / 1000.0f;
}


real PitchEventInstance::getTimeStamp() const
{
	real offset = m_pitchEvent->getRandomOffset() * 1000;
	if (offset == 0.0f)
	{
		return m_pitchEvent->getTimeStamp();
	}
	real timestamp = m_pitchEvent->getTimeStamp() * 1000;
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(offset)));
	timestamp += rnd;
	return timestamp / 1000.0f;
}


} // namespace end
}
}
