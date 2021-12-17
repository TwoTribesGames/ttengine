#include <tt/audio/xact/PitchEventInstance.h>
#include <tt/audio/xact/PlayWaveEventInstance.h>
#include <tt/audio/xact/Track.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/SoundInstance.h>
#include <tt/audio/xact/StopEventInstance.h>
#include <tt/audio/xact/VolumeEventInstance.h>
#include <tt/audio/helpers.h>
#include <tt/code/helpers.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


//#define TRACK_DEBUG
#ifdef TRACK_DEBUG
	#define Track_Printf TT_Printf
#else
	#define Track_Printf(...)
#endif


//#define TRACK_TRACE
#ifdef TRACK_TRACE
	#define Track_Trace TT_Printf
#else
	#define Track_Trace(...)
#endif


#define TRACK_WARN
#ifdef TRACK_WARN
	#define Track_Warn TT_Printf
#else
	#define Track_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

TrackInstance::TrackInstance(Track* p_track, SoundInstance* p_sound)
:
m_track(p_track),
m_sound(p_sound),
m_playEvent(0),
m_stopEvent(0),
m_volEvents(),
m_pitchEvents(),
m_pitch(0.0f),
m_volumeInDB(0.0f)
{
	Track_Trace("TrackInstance::TrackInstance: %p, %p\n", p_track, p_sound);
	
	TT_ASSERTMSG(m_track != 0, "TrackInstance::TrackInstance: track must not be 0");
	TT_ASSERTMSG(m_sound != 0, "TrackInstance::TrackInstance: sound must not be 0");

	m_volumeInDB = m_sound->getVolume() + m_track->getVolume();
}


TrackInstance::~TrackInstance()
{
	Track_Trace("TrackInstance::~TrackInstance\n");
	
	delete m_playEvent;
	delete m_stopEvent;
	
	code::helpers::freePointerContainer(m_volEvents);
	code::helpers::freePointerContainer(m_pitchEvents);
}


void TrackInstance::setPlayEvent(PlayWaveEventInstance* p_event)
{
	Track_Trace("TrackInstance::setPlayEvent: %p\n", p_event);
	
	if (p_event == 0)
	{
		Track_Warn("TrackInstance::setPlayEvent: event must not be 0\n");
		return;
	}
	
	m_playEvent = p_event;
}


void TrackInstance::setStopEvent(StopEventInstance* p_event)
{
	Track_Trace("TrackInstance::setStopEvent: %p\n", p_event);
	
	if (p_event == 0)
	{
		Track_Warn("TrackInstance::setStopEvent: event must not be 0\n");
		return;
	}
	
	m_stopEvent = p_event;
}


void TrackInstance::addVolumeEvent(VolumeEventInstance* p_event)
{
	Track_Trace("TrackInstance::addVolumeEvent: %p\n", p_event);
	
	if (p_event == 0)
	{
		Track_Warn("TrackInstance::addVolumeEvent: event must not be 0\n");
		return;
	}
	
	m_volEvents.push_back(p_event);
}


void TrackInstance::addPitchEvent(PitchEventInstance* p_event)
{
	Track_Trace("TrackInstance::addPitchEvent: %p\n", p_event);
	
	if (p_event == 0)
	{
		Track_Warn("TrackInstance::addPitchEvent: event must not be 0\n");
		return;
	}
	
	m_pitchEvents.push_back(p_event);
}


void TrackInstance::setPitch(real p_pitch)
{
	Track_Trace("TrackInstance::setPitch: %f\n", realToFloat(p_pitch));
	
	if (p_pitch > 12.0f)
	{
		Track_Warn("TrackInstance::setPitch: pitch %f larger than 12.0; clamping\n", realToFloat(p_pitch));
		p_pitch = 12.0f;
	}
	
	if (p_pitch < -12.0f)
	{
		Track_Warn("TrackInstance::setPitch: pitch %f less than -12.0; clamping\n", realToFloat(p_pitch));
		p_pitch = -12.0f;
	}
	
	m_pitch = p_pitch;
	
	// set pitch to play event
	if (m_playEvent != 0)
	{
		m_playEvent->setPitch(m_pitch);
	}
}


void TrackInstance::setVolume(real p_volumeInDB)
{
	Track_Trace("TrackInstance::setVolume: %f\n", realToFloat(p_volumeInDB));
	
	// Combine sound volume with track volume
	m_volumeInDB = p_volumeInDB + m_track->getVolume();
	
	// set volume to play event
	if (m_playEvent != 0)
	{
		m_playEvent->setVolume(m_volumeInDB);
	}
}


real TrackInstance::getVolume() const
{
	return m_volumeInDB;
}


real TrackInstance::getCategoryVolume() const
{
	return m_sound->getCategoryVolume();
}


real TrackInstance::getBaseVolume() const
{
	Track_Trace("TrackInstance::getBaseVolume\n");
	
	return m_sound->getVolume();
}


real TrackInstance::getReverbVolume() const
{
	return m_sound->getReverbVolume();
}


bool TrackInstance::play()
{
	Track_Trace("TrackInstance::play\n");
	
	setVolume(m_sound->getVolume());
	
	bool success = true;
	
	if (m_playEvent != 0)
	{
		if (m_playEvent->play() == false)
		{
			Track_Warn("TrackInstance::play: unable to start play wave event\n");
			success = false;
		}
	}
	if (m_stopEvent != 0)
	{
		if (m_stopEvent->play() == false)
		{
			Track_Warn("TrackInstance::play: unable to start stop event\n");
			success = false;
		}
	}
	
	for (PitchEvents::iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		if ((*it)->play() == false)
		{
			Track_Warn("TrackInstance::play: unable to start pitch event\n");
			success = false;
		}
	}
	
	for (VolumeEvents::iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		if ((*it)->play() == false)
		{
			Track_Warn("TrackInstance::play: unable to start volume event\n");
			success = false;
		}
	}
	
	return success;
}


bool TrackInstance::stop()
{
	Track_Trace("TrackInstance::stop\n");
	
	bool success = true;
	
	if (m_stopEvent != 0)
	{
		if (m_stopEvent->stop() == false)
		{
			Track_Warn("TrackInstance::stop: unable to stop stop event\n");
			success = false;
		}
	}
	
	for (PitchEvents::iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		if ((*it)->stop() == false)
		{
			Track_Warn("TrackInstance::play: unable to stop pitch event\n");
			success = false;
		}
	}
	
	for (VolumeEvents::iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		if ((*it)->stop() == false)
		{
			Track_Warn("TrackInstance::play: unable to stop volume event\n");
			success = false;
		}
	}
	
	if (m_playEvent != 0)
	{
		if (m_playEvent->stop() == false)
		{
			Track_Warn("TrackInstance::stop: unable to stop play wave event\n");
			success = false;
		}
	}
	
	return success;
}


bool TrackInstance::pause()
{
	Track_Trace("TrackInstance::pause\n");
	
	bool success = true;
	
	if (m_stopEvent != 0)
	{
		if (m_stopEvent->pause() == false)
		{
			Track_Warn("TrackInstance::stop: unable to pause stop event\n");
			success = false;
		}
	}
	
	for (PitchEvents::iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		if ((*it)->pause() == false)
		{
			Track_Warn("TrackInstance::play: unable to pause pitch event\n");
			success = false;
		}
	}
	
	for (VolumeEvents::iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		if ((*it)->pause() == false)
		{
			Track_Warn("TrackInstance::play: unable to pause volume event\n");
			success = false;
		}
	}
	
	if (m_playEvent != 0)
	{
		if (m_playEvent->pause() == false)
		{
			Track_Warn("TrackInstance::stop: unable to pause play wave event\n");
			success = false;
		}
	}
	
	return success;
}


bool TrackInstance::resume()
{
	Track_Trace("TrackInstance::resume\n");
	
	bool success = true;
	
	if (m_stopEvent != 0)
	{
		if (m_stopEvent->resume() == false)
		{
			Track_Warn("TrackInstance::stop: unable to resume stop event\n");
			success = false;
		}
	}
	
	for (PitchEvents::iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		if ((*it)->resume() == false)
		{
			Track_Warn("TrackInstance::play: unable to resume pitch event\n");
			success = false;
		}
	}
	
	for (VolumeEvents::iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		if ((*it)->resume() == false)
		{
			Track_Warn("TrackInstance::play: unable to resume volume event\n");
			success = false;
		}
	}
	
	if (m_playEvent != 0)
	{
		if (m_playEvent->resume() == false)
		{
			Track_Warn("TrackInstance::stop: unable to resume play wave event\n");
			success = false;
		}
	}
	
	return success;
}


bool TrackInstance::stopCue()
{
	Track_Trace("TrackInstance::stopCue\n");
	
	return m_sound->stopCue();
}


void TrackInstance::update(real p_time)
{
	Track_Trace("TrackInstance::update: %f\n", p_time);
	
	if (m_stopEvent != 0)
	{
		m_stopEvent->update(p_time);
	}
	
	for (PitchEvents::iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		(*it)->update(p_time);
	}
	
	for (VolumeEvents::iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		(*it)->update(p_time);
	}
	
	if (m_playEvent != 0)
	{
		m_playEvent->update(p_time);
	}
}


void TrackInstance::updateVolume()
{
	Track_Trace("TrackInstance::updateVolume:\n");
	
	if (m_playEvent != 0)
	{
		m_playEvent->updateVolume(helpers::dBToRatio(getCategoryVolume()));
	}
}


void TrackInstance::updateReverbVolume()
{
	if (m_playEvent != 0)
	{
		m_playEvent->updateReverbVolume();
	}
}


bool TrackInstance::setPosition(const math::Vector3& p_position)
{
	return (m_playEvent != 0) ? m_playEvent->setPosition(p_position) : false;
}


bool TrackInstance::setEmitterRadius(real p_inner, real p_outer)
{
	return (m_playEvent != 0) ? m_playEvent->setEmitterRadius(p_inner, p_outer) : false;
}


bool TrackInstance::isDone() const
{
	Track_Trace("TrackInstance::isDone\n");
	
	if (m_playEvent != 0)
	{
		return m_playEvent->isDone();
	}
	if (m_stopEvent != 0)
	{
		return false;
	}
	return true;
}


snd::size_type TrackInstance::getPriority() const
{
	if(m_sound != 0)
	{
		return m_sound->getPriority();
	}
	return 0;
}

// Namespace end
}
}
}
