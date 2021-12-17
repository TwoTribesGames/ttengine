#include <tt/fs/File.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/audio/xact/VolumeEvent.h>
#include <tt/audio/xact/VolumeEventInstance.h>
#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


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


VolumeEvent::VolumeEvent()
:
m_timeStamp(0.0f),
m_randomOffset(0.0f),
m_settingType(Setting_Equation),
m_equationType(Equation_Value),
m_operationType(Operation_Replace),
m_value(0.0f),
m_rangeMin(0.0f),
m_rangeMax(0.0f),
m_init(0.0f),
m_slope(0.0f),
m_slopeDelta(0.0f),
m_duration(0.004f),
m_repeats(0),
m_infinite(false),
m_frequency(1.0f)
{
}


VolumeEvent::~VolumeEvent()
{
}


void VolumeEvent::setTimeStamp(real p_time)
{
	if (p_time < 0.0f)
	{
		Volume_Warn("VolumeEvent::setTimeStamp: time stamp %f is less than 0.0; clamping\n", realToFloat(p_time));
		p_time = 0.0f;
	}
	m_timeStamp = p_time;
}


void VolumeEvent::setRandomOffset(real p_offset)
{
	if (p_offset < 0.0f)
	{
		Volume_Warn("VolumeEvent::setRandomOffset: offset %f is less than 0.0; clamping\n", realToFloat(p_offset));
		p_offset = 0.0f;
	}
	m_randomOffset = p_offset;
}


void VolumeEvent::setValue(real p_value)
{
	if (p_value > 6.0f)
	{
		Volume_Warn("VolumeEvent::setValue: volume %f larger than 6.0; clamping\n", realToFloat(p_value));
		p_value = 6.0f;
	}
	
	if (p_value < -96.0f)
	{
		Volume_Warn("VolumeEvent::setValue: volume %f less than -96.0; clamping\n", realToFloat(p_value));
		p_value = -96.0f;
	}
	
	m_value = p_value;
}


void VolumeEvent::setRangeMin(real p_min)
{
	if (p_min > 6.0f)
	{
		Volume_Warn("VolumeEvent::setRangeMin: volume %f larger than 6.0; clamping\n", realToFloat(p_min));
		p_min = 6.0f;
	}
	
	if (p_min < -96.0f)
	{
		Volume_Warn("VolumeEvent::setRangeMin: volume %f less than -96.0; clamping\n", realToFloat(p_min));
		p_min = -96.0f;
	}
	
	m_rangeMin = p_min;
}


void VolumeEvent::setRangeMax(real p_max)
{
	if (p_max > 6.0f)
	{
		Volume_Warn("VolumeEvent::setRangeMax: volume %f larger than 6.0; clamping\n", realToFloat(p_max));
		p_max = 6.0f;
	}
	
	if (p_max < -96.0f)
	{
		Volume_Warn("VolumeEvent::setRangeMax: volume %f less than -96.0; clamping\n", realToFloat(p_max));
		p_max = -96.0f;
	}
	
	m_rangeMax = p_max;
}


void VolumeEvent::setInitialValue(real p_init)
{
	if (p_init > 6.0f)
	{
		Volume_Warn("VolumeEvent::setInitialValue: volume %f larger than 6.0; clamping\n", realToFloat(p_init));
		p_init = 6.0f;
	}
	
	if (p_init < -96.0f)
	{
		Volume_Warn("VolumeEvent::setInitialValue: volume %f less than -96.0; clamping\n", realToFloat(p_init));
		p_init = -96.0f;
	}
	
	m_init = p_init;
}


void VolumeEvent::setDuration(real p_duration)
{
	if (p_duration < 0.004f)
	{
		Volume_Warn("VolumeEvent::setDuration: duration %f is less than 0.004 seconds; clamping\n", realToFloat(p_duration));
		p_duration = 0.004f;
	}
	m_duration = p_duration;
}


void VolumeEvent::setRepeats(int p_repeats)
{
	if (p_repeats < 0)
	{
		Volume_Warn("VolumeEvent::setCurve: repeats %d is less than 0; clamping\n", p_repeats);
		p_repeats = 0;
	}
	if (p_repeats > 254)
	{
		Volume_Warn("VolumeEvent::setCurve: curve %d is larger than 254; clamping\n", p_repeats);
		p_repeats = 254;
	}
	m_repeats = p_repeats;
}


void VolumeEvent::setFrequency(real p_frequency)
{
	if (p_frequency < 0.01f)
	{
		Volume_Warn("VolumeEvent::setFrequency: frequency %f is less than 0.01; clamping\n", realToFloat(p_frequency));
		p_frequency = 0.01f;
	}
	m_frequency = p_frequency;
}


VolumeEvent* VolumeEvent::createVolumeEvent(const xml::XmlNode* p_node)
{
	TT_ERR_CREATE("createVolumeEvent");
	
	if (p_node == 0)
	{
		Volume_Warn("VolumeEvent::createVolumeEvent: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Set Volume Event")
	{
		Volume_Warn("VolumeEvent::createVolumeEvent: xml node '%s' is not a volume event node\n", p_node->getName().c_str());
		return 0;
	}
	
	VolumeEvent* event = new VolumeEvent;
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Event Header")
		{
			const std::string& timestampstr(child->getAttribute("Timestamp"));
			if (timestampstr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: no time stamp specified\n");
			}
			else
			{
				event->setTimeStamp(xml::fast_atof(timestampstr.c_str()) / 1000.0f);
			}
			
			const std::string& randoffstr(child->getAttribute("Random Offset"));
			if (randoffstr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: no random offset specified\n");
			}
			else
			{
				event->setRandomOffset(xml::fast_atof(randoffstr.c_str()) / 1000.0f);
			}
		}
		else if (child->getName() == "Equation")
		{
			event->setSettingType(Setting_Equation);
			
			const std::string& operatorstr(child->getAttribute("Operator"));
			if (operatorstr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: no equation operator specified\n");
			}
			else
			{
				const s32 op = str::parseS32(operatorstr, &errStatus);
				if (errStatus.hasError())
				{
					Volume_Warn("VolumeEvent::createVolumeEvent: invalid equation operator specified: '%s'\n", operatorstr.c_str());
				}
				else if (op == 0)
				{
					event->setOperationType(Operation_Replace);
				}
				else if (op == 1)
				{
					event->setOperationType(Operation_Add);
				}
				else
				{
					Volume_Warn("VolumeEvent::createVolumeEvent: unknown equation operator specified: '%d'\n", op);
				}
			}
			
			const xml::XmlNode* operand = child->getChild();
			if (operand == 0)
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: equation operand missing\n");
			}
			else
			{
				const std::string& conststr(operand->getAttribute("Constant"));
				const std::string& minstr  (operand->getAttribute("Min Random"));
				const std::string& maxstr  (operand->getAttribute("Max Random"));
				
				if (conststr.empty() == false)
				{
					event->setEquationType(Equation_Value);
					event->setValue(xml::fast_atof(conststr.c_str()) / 100.0f);
				}
				else if (minstr.empty() == false && maxstr.empty() == false)
				{
					event->setEquationType(Equation_Random);
					event->setRangeMin(xml::fast_atof(minstr.c_str()) / 100.0f);
					event->setRangeMax(xml::fast_atof(maxstr.c_str()) / 100.0f);
				}
				else
				{
					Volume_Warn("VolumeEvent::createVolumeEvent: missing operands\n");
				}
			}
		}
		else if (child->getName() == "Ramp")
		{
			event->setSettingType(Setting_Ramp);
			
			const std::string& initstr(child->getAttribute("Initial Value"));
			if (initstr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: missing initial value\n");
			}
			else
			{
				event->setInitialValue(xml::fast_atof(initstr.c_str()) / 100.0f);
			}
			
			const std::string& slopestr(child->getAttribute("Slope"));
			if (slopestr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: missing slope\n");
			}
			else
			{
				event->setSlope(xml::fast_atof(slopestr.c_str()));
			}
			
			const std::string& slopedeltastr(child->getAttribute("Slope Delta"));
			if (slopedeltastr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: missing slope delta\n");
			}
			else
			{
				event->setSlopeDelta(xml::fast_atof(slopedeltastr.c_str()));
			}
			
			const std::string& durationstr(child->getAttribute("Duration"));
			if (durationstr.empty())
			{
				Volume_Warn("VolumeEvent::createVolumeEvent: missing duration\n");
			}
			else
			{
				event->setDuration(xml::fast_atof(durationstr.c_str()));
			}
		}
		else
		{
			Volume_Warn("VolumeEvent::createVolumeEvent: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return event;
}


VolumeEventInstance* VolumeEvent::instantiate(TrackInstance* p_track)
{
	VolumeEventInstance* instance = new VolumeEventInstance(this, p_track);
	return instance;
}


bool VolumeEvent::load(const fs::FilePtr& p_file)
{
	// read members
	fs::readReal(p_file, &m_timeStamp);
	fs::readReal(p_file, &m_randomOffset);
	
	fs::readEnum<u8>(p_file, &m_settingType);
	fs::readEnum<u8>(p_file, &m_equationType);
	fs::readEnum<u8>(p_file, &m_operationType);
	
	fs::readReal(p_file, &m_value);
	fs::readReal(p_file, &m_rangeMin);
	fs::readReal(p_file, &m_rangeMax);
	
	fs::readReal(p_file, &m_init);
	fs::readReal(p_file, &m_slope);
	fs::readReal(p_file, &m_slopeDelta);
	fs::readReal(p_file, &m_duration);
	
	fs::readInteger(p_file, &m_repeats);
	fs::readBool(p_file, &m_infinite);
	fs::readReal(p_file, &m_frequency);
	
	return true;
}


bool VolumeEvent::save(const fs::FilePtr& p_file) const
{
	// write members
	fs::writeReal(p_file, m_timeStamp);
	fs::writeReal(p_file, m_randomOffset);
	
	fs::writeEnum<u8>(p_file, m_settingType);
	fs::writeEnum<u8>(p_file, m_equationType);
	fs::writeEnum<u8>(p_file, m_operationType);
	
	fs::writeReal(p_file, m_value);
	fs::writeReal(p_file, m_rangeMin);
	fs::writeReal(p_file, m_rangeMax);
	
	fs::writeReal(p_file, m_init);
	fs::writeReal(p_file, m_slope);
	fs::writeReal(p_file, m_slopeDelta);
	fs::writeReal(p_file, m_duration);
	
	fs::writeInteger(p_file, m_repeats);
	fs::writeBool(p_file, m_infinite);
	fs::writeReal(p_file, m_frequency);
	
	return true;
}

// Namespace end
}
}
}
