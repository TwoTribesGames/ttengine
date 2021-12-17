#include <tt/audio/xact/StopEvent.h>
#include <tt/audio/xact/StopEventInstance.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>
#include <tt/xml/XmlNode.h>


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


StopEvent::StopEvent()
:
m_timeStamp(0.0f),
m_randomOffset(0.0f),
m_behavior(Behavior_Immediate),
m_object(Object_TrackOnly)
{
}


StopEvent::~StopEvent()
{
}


void StopEvent::setTimeStamp(real p_time)
{
	if (p_time < 0.0f)
	{
		Stop_Warn("StopEvent::setTimeStamp: time stamp %f is less than 0.0; clamping\n", realToFloat(p_time));
		p_time = 0.0f;
	}
	m_timeStamp = p_time;
}


void StopEvent::setRandomOffset(real p_offset)
{
	if (p_offset < 0.0f)
	{
		Stop_Warn("StopEvent::setRandomOffset: offset %f is less than 0.0; clamping\n", realToFloat(p_offset));
		p_offset = 0.0f;
	}
	m_randomOffset = p_offset;
}


StopEvent* StopEvent::createStopEvent(const xml::XmlNode* p_node)
{
	if (p_node == 0)
	{
		Stop_Warn("StopEvent::createStopEvent: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Stop Event")
	{
		Stop_Warn("StopEvent::createStopEvent: xml node '%s' is not a stop event node\n", p_node->getName().c_str());
		return 0;
	}
	
	StopEvent* event = new StopEvent;
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Event Header")
		{
			const std::string& timestampstr(child->getAttribute("Timestamp"));
			if (timestampstr.empty())
			{
				Stop_Warn("StopEvent::createStopEvent: no time stamp specified\n");
			}
			else
			{
				event->setTimeStamp(xml::fast_atof(timestampstr.c_str()) / 1000.0f);
			}
			
			const std::string& randoffstr(child->getAttribute("Random Offset"));
			if (randoffstr.empty())
			{
				Stop_Warn("StopEvent::createStopEvent: no random offset specified\n");
			}
			else
			{
				event->setRandomOffset(xml::fast_atof(randoffstr.c_str()) / 1000.0f);
			}
		}
		else
		{
			Stop_Warn("StopEvent::createStopEvent: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return event;
}


StopEventInstance* StopEvent::instantiate(TrackInstance* p_track)
{
	StopEventInstance* instance = new StopEventInstance(this, p_track);
	return instance;
}


bool StopEvent::load(const fs::FilePtr& p_file)
{
	// read members
	fs::readReal(p_file, &m_timeStamp);
	fs::readReal(p_file, &m_randomOffset);
	
	fs::readEnum<u8>(p_file, &m_behavior);
	fs::readEnum<u8>(p_file, &m_object);
	
	return true;
}


bool StopEvent::save(const fs::FilePtr& p_file) const
{
	// write members
	fs::writeReal(p_file, m_timeStamp);
	fs::writeReal(p_file, m_randomOffset);
	
	fs::writeEnum<u8>(p_file, m_behavior);
	fs::writeEnum<u8>(p_file, m_object);
	
	return true;
}

} // namespace end
}
}
