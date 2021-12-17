#include <algorithm>

#include <tt/audio/xact/PlayWaveEventInstance.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/utils.h>
#include <tt/audio/xact/WaveInstance.h>
#include <tt/audio/xact/Wave.h>
#include <tt/audio/helpers.h>
#include <tt/math/Random.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/snd.h>


//#define PLAY_DEBUG
#ifdef PLAY_DEBUG
	#define Play_Printf TT_Printf
#else
	#define Play_Printf(...)
#endif


//#define PLAY_TRACE
#ifdef PLAY_TRACE
	#define Play_Trace TT_Printf
#else
	#define Play_Trace(...)
#endif


#define PLAY_WARN
#ifdef PLAY_WARN
	#define Play_Warn TT_Printf
#else
	#define Play_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

PlayWaveEventInstance::PlayWaveEventInstance(PlayWaveEvent* p_playEvent, TrackInstance* p_track)
:
m_playEvent(p_playEvent),
m_track(p_track),
m_delay(0.0f),
m_loopCount(0),
m_breakLoop(false),
m_activeWave(0),
m_volumeInDB(0.0f),
m_volumeVariation(0.0f),
m_pitch(0.0f),
m_pan(0),
m_isDone(false),
m_paused(false),
m_isPositional(false),
m_position(math::Vector3::zero),
m_emitterRadiusInner(-1.0f),
m_emitterRadiusOuter(-1.0f)
{
	Play_Trace("PlayWaveEventInstance::PlayWaveEventInstance: [%p] %p, %p\n", this, p_playEvent, p_track);
	
	TT_ASSERTMSG(m_playEvent != 0, "PlayWaveEventInstance::PlayWaveEventInstance: play wave event must not be 0");
	TT_ASSERTMSG(m_track     != 0, "PlayWaveEventInstance::PlayWaveEventInstance: track must not be 0");
	
	m_volumeInDB = m_track->getVolume();
	
	if (m_playEvent->getPanVariationEnabled())
	{
		m_pan = getRandomPan();
	}
	
	if (m_playEvent->getVolumeVariationEnabled())
	{
		m_volumeVariation = getRandomVolume();
		
		switch (m_playEvent->getVolumeOperator())
		{
		case PlayWaveEvent::Operator_Add:     m_volumeInDB += m_volumeVariation; break;
		case PlayWaveEvent::Operator_Replace: m_volumeInDB = getRandomVolume();  break;
		default: break;
		}
	}
	
	if (m_playEvent->getPitchVariationEnabled())
	{
		m_pitch = getRandomPitch();
	}
}


void PlayWaveEventInstance::setVolume(real p_volumeInDB)
{
	Play_Trace("PlayWaveEventInstance::setVolume: [%p] %f\n", this, realToFloat(p_volumeInDB));
	
	m_volumeInDB = p_volumeInDB;
	
	if (m_playEvent->getVolumeVariationEnabled())
	{
		switch (m_playEvent->getVolumeOperator())
		{
		case PlayWaveEvent::Operator_Add:     m_volumeInDB += m_volumeVariation; break;
		case PlayWaveEvent::Operator_Replace: m_volumeInDB = getRandomVolume();  break;
		default: break;
		}
	}
	
	if (m_activeWave != 0)
	{
		m_activeWave->setVolume(m_volumeInDB, helpers::dBToRatio(m_track->getCategoryVolume()));
	}
}


void PlayWaveEventInstance::setPitch(real p_pitch)
{
	Play_Trace("PlayWaveEventInstance::setPitch: [%p] %f\n", this, p_pitch);
	
	if (p_pitch > 12.0f)
	{
		Play_Warn("PlayWaveEventInstance::setPitch: volume %d is larger than 12.0; clamping\n", realToFloat(p_pitch));
		p_pitch = 12.0f;
	}
	
	if (p_pitch < -12.0f)
	{
		Play_Warn("PlayWaveEventInstance::setPitch: volume %d is less than -12.0; clamping\n", realToFloat(p_pitch));
		p_pitch = -12.0f;
	}
	
	m_pitch = p_pitch;
	
	if (m_activeWave != 0)
	{
		m_activeWave->setPitch(m_pitch);
	}
}


void PlayWaveEventInstance::setPan(int p_pan)
{
	Play_Trace("PlayWaveEventInstance::setPan: [%p] %d\n", this, p_pan);
	
	if (p_pan > 359)
	{
		Play_Warn("PlayWaveEventInstance::setPan: pan %d is larger than 359; clamping\n", p_pan);
		p_pan = 359;
	}
	
	if (p_pan < 0)
	{
		Play_Warn("PlayWaveEventInstance::setPan: pan %d is less than 0; clamping\n", p_pan);
		p_pan = 0;
	}
	
	m_pan = p_pan;
	
	if (m_activeWave != 0)
	{
		m_activeWave->setPan(p_pan);
	}
}


bool PlayWaveEventInstance::setPosition(const math::Vector3& p_position)
{
	m_isPositional = true;
	m_position     = p_position;
	
	return (m_activeWave != 0) ? m_activeWave->setPosition(p_position) : true;
}


bool PlayWaveEventInstance::setEmitterRadius(real p_inner, real p_outer)
{
	m_isPositional       = true;
	m_emitterRadiusInner = p_inner;
	m_emitterRadiusOuter = p_outer;
	
	return (m_activeWave != 0) ? m_activeWave->setEmitterRadius(p_inner, p_outer) : true;
}


bool PlayWaveEventInstance::play()
{
	Play_Trace("PlayWaveEventInstance::play [%p]\n", this);
	
	if (m_activeWave != 0)
	{
		TT_ASSERT(m_activeWave == 0);
		delete m_activeWave;
		m_activeWave = 0;
	}
	
	// prepare data
	m_loopCount  = 0;
	m_breakLoop  = false;
	m_delay      = m_playEvent->getStartDelay();
	m_isDone     = false;
	m_paused     = false;
	
	return true;
}


bool PlayWaveEventInstance::stop()
{
	Play_Trace("PlayWaveEventInstance::stop [%p]\n", this);
	
	// stop sound
	if (m_activeWave != 0)
	{
		if (((m_playEvent->getLoopCount() > 0 || shouldLoopInfinitely()) && 
		     (m_playEvent->getBreakLoop() == false)) &&
		    ((m_playEvent->getPlayListSize() == 1 && shouldLoopInfinitely()) == false))
		{
			// finish this wave, and then stop
			m_breakLoop = true;
		}
		else
		{
			m_activeWave->stop();
			delete m_activeWave;
			m_activeWave = 0;
			m_isDone = true;
		}
	}
	else
	{
		m_isDone = true;
	}
	
	return true;
}


bool PlayWaveEventInstance::pause()
{
	m_paused = true;
	if (m_activeWave != 0)
	{
		return m_activeWave->pause();
	}
	return true;
}


bool PlayWaveEventInstance::resume()
{
	m_paused = false;
	if (m_activeWave != 0)
	{
		return m_activeWave->resume();
	}
	return true;
}


void PlayWaveEventInstance::update(real p_time)
{
	Play_Trace("PlayWaveEventInstance::update: [%p] %f\n", this, realToFloat(p_time));
	if (m_isDone)
	{
		return;
	}
	
	Play_Printf("PlayWaveEventInstance::update: wait until time >= %f\n", realToFloat(m_delay));
	if (p_time >= m_delay)
	{
		// check if the currently playing sound has stopped
		if (m_activeWave != 0)
		{
			m_activeWave->update(p_time);
			if (m_activeWave->isPlaying() == false)
			{
				delete m_activeWave;
				m_activeWave = 0;
			}
		}
		
		// if there is no actively playing sound, start one
		if (m_activeWave == 0)
		{
			// Audio culling, do not start out-of-range sounds
			// Unless they are infinitely looping
			if(m_isPositional && shouldLoopInfinitely() == false && m_emitterRadiusOuter > 0.0f)
			{
				real distanceToListener = math::distance(snd::getListenerPosition(), m_position);

				if(distanceToListener > m_emitterRadiusOuter)
				{
					//TT_Printf("Culling audio based on distance (%g), radius = %g \n", distanceToListener, m_emitterRadiusOuter);
					m_isDone = true;
					return;
				}
			}
			
			if (m_breakLoop || shouldLoopInfinitely() == false)
			{
				if (m_breakLoop || (m_loopCount > m_playEvent->getLoopCount()))
				{
					// early abort
					Play_Printf("PlayWaveEventInstance::update: early abort: done\n");
					
					m_isDone = true;
					return;
				}
				++m_loopCount;
			}
			
			// update playlist
			Play_Printf("PlayWaveEventInstance::update: get next wave\n");
			Wave* nextWave(m_playEvent->getNextWave());
			if (nextWave == 0)
			{
				Play_Printf("PlayWaveEventInstance::update: next wave null: done\n");
				
				m_isDone = true;
				return;
			}
			m_activeWave = nextWave->instantiate();
			
			// update panning
			if (m_playEvent->getPanVariationEnabled() && m_playEvent->getPanNewOnLoop())
			{
				m_pan = getRandomPan();
			}
			
			// update volume
			if (m_playEvent->getVolumeVariationEnabled())
			{
				if (m_playEvent->getVolumeNewOnLoop())
				{
					switch (m_playEvent->getVolumeOperator())
					{
					case PlayWaveEvent::Operator_Add:
						// Subtract current variation, get new, add new variation
						m_volumeInDB -= m_volumeVariation;
						m_volumeVariation = getRandomVolume();
						m_volumeInDB += m_volumeVariation;
						break;
						
					case PlayWaveEvent::Operator_Replace:
						m_volumeInDB = getRandomVolume();
						break;
						
					default:
						break;
					}
				}
				else
				{
					m_volumeInDB = getRandomVolume();
				}
			}
			
			// update pitch
			if (m_playEvent->getPitchVariationEnabled())
			{
				if (m_playEvent->getPitchNewOnLoop())
				{
					switch (m_playEvent->getPitchOperator())
					{
					case PlayWaveEvent::Operator_Add:
						m_pitch += getRandomPitch();
						break;
						
					case PlayWaveEvent::Operator_Replace:
						m_pitch = getRandomPitch();
						break;
						
					default:
						break;
					}
				}
				else
				{
					m_pitch = getRandomPitch();
				}
			}
			
			if (m_activeWave != 0)
			{
				// set volume, pitch and panning and play wave
				//TT_Printf("PlayWaveEventInstance::update- m_volumeInDB: %f, m_track->getVolume(): %f\n",
				//          m_volumeInDB, m_track->getVolume());
				m_activeWave->update(p_time);
				
				if (m_isPositional)
				{
					m_activeWave->setPosition(m_position);
					if (math::realGreaterEqual(m_emitterRadiusInner, 0.0f) &&
					    math::realGreaterEqual(m_emitterRadiusOuter, 0.0f))
					{
						m_activeWave->setEmitterRadius(m_emitterRadiusInner, m_emitterRadiusOuter);
					}
				}
				
				m_activeWave->setVolume(m_volumeInDB, helpers::dBToRatio(m_track->getCategoryVolume()));
				
				m_activeWave->setPitch(m_pitch);
				m_activeWave->setPan(m_pan);
				m_activeWave->setReverbVolume(m_track->getReverbVolume());
				m_activeWave->play(m_playEvent->getPlayListSize() == 1 && shouldLoopInfinitely(), m_track->getPriority());
			}
		}
	}
}


void PlayWaveEventInstance::updateVolume(real p_normalizedCategoryVolume)
{
	Play_Trace("PlayWaveEventInstance::updateVolume [%p]\n", this);
	
	if (m_activeWave != 0)
	{
		m_activeWave->updateVolume(p_normalizedCategoryVolume);
	}
}


void PlayWaveEventInstance::updateReverbVolume()
{
	if (m_activeWave != 0)
	{
		m_activeWave->setReverbVolume(m_track->getReverbVolume());
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

real PlayWaveEventInstance::getRandomVolume() const
{
	Play_Trace("PlayWaveEventInstance::getRandomVolume [%p]\n", this);
	
	const real min = m_playEvent->getVolumeRangeMin() * 1000.0f;
	const real max = m_playEvent->getVolumeRangeMax() * 1000.0f;
	const real range = max - min;
	if (range <= 0)
	{
		return min / 1000.0f;
	}
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(range)));
	rnd += min;
	return rnd / 1000.0f;
}


real PlayWaveEventInstance::getRandomPitch() const
{
	Play_Trace("PlayWaveEventInstance::getRandomPitch [%p]\n", this);
	
	const real min = m_playEvent->getPitchRangeMin() * 1000;
	const real max = m_playEvent->getPitchRangeMax() * 1000;
	const real range = max - min;
	if (range <= 0)
	{
		return min / 1000.0f;
	}
	real rnd = static_cast<real>(getXactRandom().getNext(static_cast<u32>(range)));
	rnd += min;
	return rnd / 1000.0f;
}


int PlayWaveEventInstance::getRandomPan() const
{
	Play_Trace("PlayWaveEventInstance::getRandomPan [%p]\n", this);
	
	if (m_playEvent->getPanArc() <= 0)
	{
		return m_playEvent->getPanAngle();
	}
	
	int rnd = static_cast<int>(getXactRandom().getNext(static_cast<u32>(m_playEvent->getPanArc())));
	rnd += m_playEvent->getPanAngle();
	if (rnd >= 360)
	{
		rnd -= 360;
	}
	return rnd;
}


// Namespace end
}
}
}
