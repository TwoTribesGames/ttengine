#include <tt/audio/xact/PitchEvent.h>
#include <tt/audio/xact/PlayWaveEvent.h>
#include <tt/audio/xact/StopEvent.h>
#include <tt/audio/xact/Track.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/VolumeEvent.h>
#include <tt/code/helpers.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>
#include <tt/xml/XmlNode.h>


//#define TRACK_DEBUG
#ifdef TRACK_DEBUG
	#define Track_Printf TT_Printf
#else
	#define Track_Printf(...)
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

Track::Track()
:
m_volumeInDB(0.0f),
m_playEvent(0),
m_stopEvent(0),
m_volEvents(),
m_pitchEvents()
{
}


Track::~Track()
{
	delete m_playEvent;
	delete m_stopEvent;
	
	code::helpers::freePointerContainer(m_volEvents);
	code::helpers::freePointerContainer(m_pitchEvents);
}


void Track::setVolume(real p_volumeInDB)
{
	if (p_volumeInDB > 6.0f)
	{
		Track_Warn("Track::setVolume: volume %f larger than 6.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		Track_Warn("Track::setVolume: volume %f less than -96.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = -96.0f;
	}
	
	m_volumeInDB = p_volumeInDB;
}


void Track::setPlayEvent(PlayWaveEvent* p_event)
{
	if (p_event == 0)
	{
		Track_Warn("Track::setPlayEvent: event must not be 0\n");
		return;
	}
	
	m_playEvent = p_event;
}


void Track::setStopEvent(StopEvent* p_event)
{
	if (p_event == 0)
	{
		Track_Warn("Track::setStopEvent: event must not be 0\n");
		return;
	}
	
	m_stopEvent = p_event;
}


void Track::addVolumeEvent(VolumeEvent* p_event)
{
	if (p_event == 0)
	{
		Track_Warn("Track::addVolumeEvent: event must not be 0\n");
		return;
	}
	
	m_volEvents.push_back(p_event);
}


void Track::addPitchEvent(PitchEvent* p_event)
{
	if (p_event == 0)
	{
		Track_Warn("Track::addPitchEvent: event must not be 0\n");
		return;
	}
	
	m_pitchEvents.push_back(p_event);
}


Track* Track::createTrack(const xml::XmlNode* p_node)
{
	if (p_node == 0)
	{
		Track_Warn("Track::createTrack: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Track")
	{
		Track_Warn("Track::createTrack: xml node '%s' is not a track node\n", p_node->getName().c_str());
		return 0;
	}
	
	Track* track = new Track;
	
	const std::string& volstr(p_node->getAttribute("Volume"));
	if (volstr.empty())
	{
		Track_Warn("Track::createTrack: no volume specified\n");
	}
	else
	{
		track->setVolume(xml::fast_atof(volstr.c_str()) / 100.0f);
	}
	
	bool hasPlayEvent = false;
	bool hasStopEvent = false;
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Play Wave Event")
		{
			if (hasPlayEvent)
			{
				Track_Warn("Track::createTrack: Track already has play wave event\n");
			}
			else
			{
				PlayWaveEvent* play = PlayWaveEvent::createPlayWaveEvent(child);
				
				if (play == 0)
				{
					Track_Warn("Track::createTrack: Error creating play wave event\n");
				}
				else
				{
					track->setPlayEvent(play);
					hasPlayEvent = true;
				}
			}
		}
		else if (child->getName() == "Stop Event")
		{
			if (hasStopEvent)
			{
				Track_Warn("Track::createTrack: Track already has stop event\n");
			}
			else
			{
				StopEvent* stop = StopEvent::createStopEvent(child);
				
				if (stop == 0)
				{
					Track_Warn("Track::createTrack: Error creating stop event\n");
				}
				else
				{
					track->setStopEvent(stop);
					hasStopEvent = true;
				}
			}
		}
		else if (child->getName() == "Set Pitch Event")
		{
			PitchEvent* pitch = PitchEvent::createPitchEvent(child);
			
			if (pitch == 0)
			{
				Track_Warn("Track::createTrack: Error creating pitch event\n");
			}
			else
			{
				track->addPitchEvent(pitch);
			}
		}
		else if (child->getName() == "Set Volume Event")
		{
			VolumeEvent* volume = VolumeEvent::createVolumeEvent(child);
			
			if (volume == 0)
			{
				Track_Warn("Track::createTrack: Error creating volume event\n");
			}
			else
			{
				track->addVolumeEvent(volume);
			}
		}
		else if (child->getName() == "Marker Event")
		{
			// not supported yet
		}
		else
		{
			Track_Warn("Track::createTrack: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return track;
}


TrackInstance* Track::instantiate(SoundInstance* p_sound)
{
	TrackInstance* instance = new TrackInstance(this, p_sound);
	
	if (m_playEvent != 0)
	{
		instance->setPlayEvent(m_playEvent->instantiate(instance));
	}
	
	if (m_stopEvent != 0)
	{
		instance->setStopEvent(m_stopEvent->instantiate(instance));
	}
	
	for (VolumeEvents::iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		instance->addVolumeEvent((*it)->instantiate(instance));
	}
	
	for (PitchEvents::iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		instance->addPitchEvent((*it)->instantiate(instance));
	}
	
	return instance;
}


bool Track::load(const fs::FilePtr& p_file)
{
	int  size;
	bool hasEvent;
	
	// read members
	fs::readReal(p_file, &m_volumeInDB);
	
	// read play event
	fs::readBool(p_file, &hasEvent);
	if (hasEvent)
	{
		TT_ASSERT(m_playEvent == 0);
		
		m_playEvent = new PlayWaveEvent;
		m_playEvent->load(p_file);
	}
	
	// read stop event
	fs::readBool(p_file, &hasEvent);
	if (hasEvent)
	{
		TT_ASSERT(m_stopEvent == 0);
		
		m_stopEvent = new StopEvent;
		m_stopEvent->load(p_file);
	}
	
	// read VolumeEvents
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		VolumeEvent* e = new VolumeEvent;
		e->load(p_file);
		
		m_volEvents.push_back(e);
	}
	
	// read PitchEvents
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		PitchEvent* e = new PitchEvent;
		e->load(p_file);
		
		m_pitchEvents.push_back(e);
	}
	
	return true;
}


bool Track::save(const fs::FilePtr& p_file) const
{
	// write members
	fs::writeReal(p_file, m_volumeInDB);
	
	// save play event
	fs::writeBool(p_file, m_playEvent != 0);
	if (m_playEvent != 0)
	{
		m_playEvent->save(p_file);
	}
	
	// save stop event
	fs::writeBool(p_file, m_stopEvent != 0);
	if (m_stopEvent != 0)
	{
		m_stopEvent->save(p_file);
	}
	
	// save VolumeEvents
	fs::writeInteger(p_file, m_volEvents.size());
	for (VolumeEvents::const_iterator it = m_volEvents.begin(); it != m_volEvents.end(); ++it)
	{
		(*it)->save(p_file);
	}
	
	// save PitchEvents
	fs::writeInteger(p_file, m_pitchEvents.size());
	for (PitchEvents::const_iterator it = m_pitchEvents.begin(); it != m_pitchEvents.end(); ++it)
	{
		(*it)->save(p_file);
	}
	
	return true;
}

// Namespace end
}
}
}
