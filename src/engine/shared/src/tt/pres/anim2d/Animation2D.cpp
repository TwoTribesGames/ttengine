#include <string>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/Animation2D.h>
#include <tt/pres/PresentationObject.h>
#include <tt/fs/File.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>



namespace tt {
namespace pres {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 8);
static const size_t s_size = 2 + 1 + 1 + 1 + 1 + 1; // version, timetype, direction, looping, static, tweentype


Animation2D::TimeType Animation2D::timeTypeFromString(const std::string& p_name)
{
	if      (p_name == "linear")     return TimeType_Linear;
	else if (p_name == "easein")     return TimeType_EaseIn;
	else if (p_name == "easeout")    return TimeType_EaseOut;
	else if (p_name == "easeends")   return TimeType_EaseEnds;
	else if (p_name == "easecenter") return TimeType_EaseCenter;
	else
	{
		TT_PANIC("Unknown timetype '%s'.", p_name.c_str());
		return TimeType(0);
	}
}


std::string Animation2D::timeTypeToString(TimeType p_timeType)
{
	switch (p_timeType)
	{
	case TimeType_Linear:      return "linear";
	case TimeType_EaseIn:      return "easein";
	case TimeType_EaseOut:     return "easeout";
	case TimeType_EaseEnds:    return "easeends";
	case TimeType_EaseCenter:  return "easecenter";
	default: TT_PANIC("Unknown timetype: %d", p_timeType); return "unknown";
	}
}


Animation2D::DirectionType Animation2D::directionTypeFromString(const std::string& p_name)
{
	if      (p_name == "forward")         return DirectionType_Forward;
	else if (p_name == "backward")        return DirectionType_Backward;
	else if (p_name == "pingpong")        return DirectionType_PingPong;
	else if (p_name == "reversepingpong") return DirectionType_ReversePingPong;
	else
	{
		TT_PANIC("Unknown directiontype '%s'.", p_name.c_str());
		return DirectionType(0);
	}
}


std::string Animation2D::directionTypeToString(DirectionType p_directionType)
{
	switch (p_directionType)
	{
	case DirectionType_Forward:            return "forward";
	case DirectionType_Backward:           return "backward";
	case DirectionType_PingPong:           return "pingpong";
	case DirectionType_ReversePingPong:    return "reversepingpong";
	default: TT_PANIC("Unknown timetype"); return "unknown";
	}
}


Animation2D::Animation2D()
:
m_active(false),
m_paused(false),
m_duration(1.0f),
m_delay(0.0f),
m_time(0.0f),
m_internalTime(0.0f),
m_delta(1.0f),
m_startTime(0.0f),
m_instantAtEnd(false),
m_timeType(TimeType_Linear),
m_tweenType(TweenType_QUAD),
m_directionType(DirectionType_Forward),
m_looping(false),
m_static(false),
m_hasNameOrTagMatch(false),
m_dataTags(),
m_id()
{
}


void Animation2D::update(real p_delta)
{
	if (m_active == false || m_paused || m_hasNameOrTagMatch == false || m_static)
	{
		return;
	}
	
	if (m_startTime > 0.0f)
	{
		if (p_delta >= m_startTime)
		{
			if(m_instantAtEnd == false)
			{
				m_internalTime = m_delta * (m_startTime - p_delta);
			}
			else
			{
				m_internalTime = 1.0f;
			}
			m_startTime = 0.0f;
		}
		else
		{
			m_startTime -= p_delta;
			return;
		}
	}
	
	// update internal time
	if(m_instantAtEnd == false)
	{
		m_internalTime += (m_delta * p_delta);
	}
	else
	{
		m_internalTime = 1.0f;
	}
	if (m_internalTime >= 1.0f)
	{
		if (m_looping)
		{
			m_internalTime -= 1.0f;
		}
		else
		{
			m_internalTime = 1.0f;
			m_active = false;
		}
		
		if (m_id.empty() == false)
		{
			if (m_presObj != 0)
			{
				if (m_looping) m_presObj->triggerSync(m_id);
				
				m_presObj->triggerEndSync(m_id);
			}
		}
	}
	
	// calculate position on the timeline
	real pos;
	
	switch (m_directionType)
	{
	case DirectionType_Forward:
		pos = m_internalTime;
		break;
	
	case DirectionType_Backward:
		pos = 1.0f - m_internalTime;
		break;
	
	case DirectionType_PingPong:
		pos = 1.0f - math::fabs((-0.5f + m_internalTime) * 2.0f);
		break;
	
	case DirectionType_ReversePingPong:
		pos = math::fabs((-0.5f + m_internalTime) * 2.0f);
		break;
	
	default:
		TT_PANIC("Unknown/unsupported direction type %d.", m_directionType);
		return;
	}
	
	// calculate animation time
	switch (m_timeType)
	{
	case TimeType_Linear:
		m_time = pos;
		break;
	
	case TimeType_EaseIn:
		m_time = Tween::tweenFactory(m_tweenType).easeIn(pos,0,1,1);
		break;
	
	case TimeType_EaseOut:
		m_time = Tween::tweenFactory(m_tweenType).easeOut(pos,0,1,1);
		break;
	
	case TimeType_EaseEnds:
		m_time = Tween::tweenFactory(m_tweenType).easeInOut(pos,0,1,1);
		break;
	
	case TimeType_EaseCenter:
		// sine
		if (pos <= 0.5f)
		{
			m_time = 0.5f * (math::sin(pos * math::pi) - 1.0f) + 0.5f;
		}
		else
		{
			m_time = -0.5f * (math::sin(pos * math::pi) - 1.0f) + 0.5f;
		}
		break;
	
	default:
		TT_PANIC("Unknown/unsupported time type %d\n", m_timeType);
		return;
	}
}


void Animation2D::start( const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name)
{
	if(m_dataTags.shouldPlay(p_tags, p_name))
	{
		if (m_active)
		{
			return;
		}
		setRanges(p_presObj); // get random values from all ranges
		
		m_active = m_static == false; // Don't set active when static
		if (m_directionType == DirectionType_Backward || m_directionType == DirectionType_ReversePingPong)
		{
			m_time = 1.0f;
		}
		else
		{
			m_time = 0.0f;
		}
		m_internalTime = 0.0f;
		m_startTime = m_delay;
		m_hasNameOrTagMatch = true;
		return;
	}
	else
	{
		m_hasNameOrTagMatch = false;
		m_active = false;
	}
}


void Animation2D::reset()
{
	if (m_paused)
	{
		resume();
	}
	if (m_active)
	{
		stop();
	}
	if (m_directionType == DirectionType_Backward || m_directionType == DirectionType_ReversePingPong)
	{
		m_time = 1.0f;
	}
	else
	{
		m_time = 0.0f;
	}
}


void Animation2D::setPreset(Preset p_preset)
{
	TT_ASSERTMSG(m_active == false, "Cannot change playback parameters while animation is active.");
	switch (p_preset)
	{
	case Preset_OneShot:
		m_timeType = TimeType_Linear;
		m_directionType = DirectionType_Forward;
		m_looping = false;
		break;
	
	case Preset_Looping:
		m_timeType = TimeType_Linear;
		m_directionType = DirectionType_Forward;
		m_looping = true;
		break;
	
	case Preset_PingPong:
		m_timeType = TimeType_Linear;
		m_directionType = DirectionType_PingPong;
		m_looping = true;
		break;
	
	case Preset_Oscillating:
		m_timeType = TimeType_EaseEnds;
		m_directionType = DirectionType_PingPong;
		m_looping = true;
		break;
	
	default:
		TT_PANIC("Unknown/unsupported preset '%d'", p_preset);
	}
}


void Animation2D::setTimeType(TimeType p_type)
{
	TT_ASSERTMSG(m_active == false, "Cannot change playback parameters while animation is active.");
	m_timeType = p_type;
}

void Animation2D::setTweenType( TweenType p_type )
{
	TT_ASSERTMSG(m_active == false, "Cannot change playback parameters while animation is active.");
	m_tweenType = p_type;
}

void Animation2D::setDirection(DirectionType p_type)
{
	TT_ASSERTMSG(m_active == false, "Cannot change playback parameters while animation is active.");
	m_directionType = p_type;
}


void Animation2D::setLooping(bool p_loop)
{
	TT_ASSERTMSG(m_active == false, "Cannot change playback parameters while animation is active.");
	m_looping = p_loop;
}


void Animation2D::setDurationRange(const pres::PresentationValue& p_duration)
{
	TT_ASSERTMSG(p_duration.getMin() > 0.0f, "Duration should be larger than 0");
	TT_ASSERTMSG(m_active == false, "Cannot change duration while animation is active.");
	m_duration = p_duration;
}


void Animation2D::setDuration(real p_duration)
{
	TT_ASSERTMSG(m_active == false, "Cannot change duration while animation is active.");
	m_duration.setValue(p_duration);
}


void Animation2D::setDelayRange(const pres::PresentationValue& p_delay)
{
	TT_ASSERTMSG(m_active == false, "Cannot change delay while animation is active.");
	m_delay = p_delay;
}


void Animation2D::setDelay(real p_delay)
{
	TT_ASSERTMSG(p_delay >= 0.0f, "Delay should be larger or equal than 0");
	TT_ASSERTMSG(m_active == false, "Cannot change delay while animation is active.");
	m_delay.setValue(p_delay);
}


bool Animation2D::load( const xml::XmlNode* p_node, 
                        const DataTags& p_applyTags, 
                        const Tags& p_acceptedTags,
                        code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "loading Animation2D from node " << p_node->getName());
	
	TT_ERR_NULL_ASSERT(p_node);
	
	TT_ERR_ASSERTMSG(p_node->getAttribute("duration").empty() == false, 
		"Expected attribute 'duration' in node '" << p_node->getName() << "'");
	
	const std::string& preset = p_node->getAttribute("preset");
	Preset presetType = Animation2D::Preset_OneShot;
	if (preset.empty() == false)
	{
		if      (preset == "oneshot")     presetType = Preset_OneShot;
		else if (preset == "looping")     presetType = Preset_Looping;
		else if (preset == "pingpong")    presetType = Preset_PingPong;
		else if (preset == "oscillating") presetType = Preset_Oscillating;
		else
		{
#ifndef TT_BUILD_FINAL
			TT_ERR_ASSERTMSG(preset != "oscilating", 
				"Found 'oscilating' in attribute 'preset' in node '" << p_node->getName() << 
				"', this should be 'oscillating'");
			
			TT_ERR_ASSERTMSG(preset != "once", 
				"Found 'once' in attribute 'preset' in node '" << p_node->getName() << 
				"', this should be 'oneshot'");
#endif
			
			TT_ERR_AND_RETURN(
				"Found unknown value '" << preset << "' in attribute 'preset' in node '" 
				<< p_node->getName() << "'.");
		}
	}
	
	const std::string& timing = p_node->getAttribute("timing");
	TimeType timingType = Animation2D::TimeType_Linear;
	if (timing.empty() == false)
	{
		if      (timing == "linear")     timingType = TimeType_Linear;
		else if (timing == "easein")     timingType = TimeType_EaseIn;
		else if (timing == "easeout")    timingType = TimeType_EaseOut;
		else if (timing == "easeends")   timingType = TimeType_EaseEnds;
		else if (timing == "easecenter") timingType = TimeType_EaseCenter;
		else
		{
			TT_ERR_AND_RETURN(
				"Found unknown value '" << timing << "' in attribute 'timing' in node '" 
				<< p_node->getName() << "'.");
		}
	}
	
	const std::string& tweening = p_node->getAttribute("tweening");
	TweenType tweenType = TweenType_QUAD;
	if (tweening.empty() == false)
	{
		tweenType = Tween::tweenTypeFromString(tweening);
	}
	
	const std::string& direction = p_node->getAttribute("direction");
	DirectionType directionType = Animation2D::DirectionType_Forward;
	if (direction.empty() == false)
	{
		if      (direction == "forward")         directionType = DirectionType_Forward;
		else if (direction == "backward")        directionType = DirectionType_Backward;
		else if (direction == "pingpong")        directionType = DirectionType_PingPong;
		else if (direction == "reversepingpong") directionType = DirectionType_ReversePingPong;
		else
		{
			TT_ERR_AND_RETURN(
				"Found unknown value '" << direction << "' in attribute 'direction' in node '" 
				<< p_node->getName() << "'.");
		}
	}
	
	const std::string& looping = p_node->getAttribute("looping");
	bool loopType = false;
	if (looping.empty() == false)
	{
		loopType = str::parseBool(looping, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
	}
	
	pres::PresentationValue duration(pres::parsePresentationValue(p_node, "duration", 1.0f, &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	
	const std::string& delayStr = p_node->getAttribute("delay");
	pres::PresentationValue delay;
	if (delayStr.empty() == false)
	{
		delay.resetValue(delayStr, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
	}
	
	m_id   = p_node->getAttribute("id");
	
	if (preset.empty() == false)   setPreset(presetType);
	if (timing.empty() == false)   setTimeType(timingType);
	if (tweening.empty() == false) setTweenType(tweenType);
	if (direction.empty() == false)setDirection(directionType);
	if (looping.empty() == false)  setLooping(loopType);
	if (delayStr.empty() == false) setDelayRange(delay);
	
	m_static = false;
	
	setDurationRange(duration);
	setRanges(0);
	
	m_dataTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return true;
}


bool Animation2D::loadStatic(const xml::XmlNode* p_node,
                             const DataTags& p_applyTags,
                             const Tags& p_acceptedTags,
                             code::ErrorStatus* p_errStatus)
{
	m_delay.setValue(0.0f);
	m_duration.setValue(1.0f);
	m_static = true;
	m_time = 0.0f;
	
	return m_dataTags.load(p_node, p_applyTags, p_acceptedTags, p_errStatus);
}


bool Animation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                        const DataTags& p_applyTags, const Tags& p_acceptedTags,
                        code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "loading Animation2D binary ");
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small, got " << p_sizeOUT << " bytes, needs " << s_size << "\n");
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(version == s_version, "Invalid FrameAnimation version, code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			", Please update your presentation converter");
	
	m_delay.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_duration.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	
	setTimeType (static_cast<TimeType     >(be_get<u8  >(p_bufferOUT, p_sizeOUT)));
	setTweenType(static_cast<TweenType    >(be_get<u8  >(p_bufferOUT, p_sizeOUT)));
	setDirection(static_cast<DirectionType>(be_get<u8  >(p_bufferOUT, p_sizeOUT)));
	setLooping  (                           be_get<bool>(p_bufferOUT, p_sizeOUT));
	m_static =                              be_get<bool>(p_bufferOUT, p_sizeOUT);
	setRanges(0);
	
	m_id   = be_get<std::string>(p_bufferOUT, p_sizeOUT);
	
	m_dataTags.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return true;
}


bool Animation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT,
                       code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "Saving Animation2D binary ");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small, got " << p_sizeOUT << " bytes, needs " << s_size << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	m_delay.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_duration.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	be_put(static_cast<u8>(m_timeType     ), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_tweenType    ), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_directionType), p_bufferOUT, p_sizeOUT);
	be_put(m_looping,                        p_bufferOUT, p_sizeOUT);
	be_put(m_static,                         p_bufferOUT, p_sizeOUT);
	
	
	be_put(m_id,   p_bufferOUT, p_sizeOUT);
	
	m_dataTags.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return true;
}


size_t Animation2D::getBufferSize() const
{
	size_t size(s_size);
	return size + m_duration.getBufferSize() + m_delay.getBufferSize() + 2 + m_id.size() + m_dataTags.getBufferSize();
}


void Animation2D::setRanges(PresentationObject* p_presObj)
{
	TT_ASSERTMSG(m_active == false, "Can't set Ranges while animation is active");
	
	m_duration.updateValue(p_presObj);
	m_delay.updateValue(p_presObj);
	if(math::realEqual(m_duration, 0.0f))
	{
		m_delta = 0.0f;
		m_instantAtEnd = true;
	}
	else
	{
		m_delta = 1.0f / m_duration;
		m_instantAtEnd = false;
	}
	
	m_presObj = p_presObj;
}


Animation2D::Animation2D(const Animation2D& p_rhs)
:
m_active(p_rhs.m_active),
m_paused(p_rhs.m_paused),
m_duration(p_rhs.m_duration),
m_delay(p_rhs.m_delay),
m_time(p_rhs.m_time),
m_internalTime(p_rhs.m_internalTime),
m_delta(p_rhs.m_delta),
m_startTime(p_rhs.m_startTime),
m_instantAtEnd(p_rhs.m_instantAtEnd),
m_timeType(p_rhs.m_timeType),
m_tweenType(p_rhs.m_tweenType),
m_directionType(p_rhs.m_directionType),
m_looping(p_rhs.m_looping),
m_static(p_rhs.m_static),
m_hasNameOrTagMatch(p_rhs.m_hasNameOrTagMatch),
m_dataTags(p_rhs.m_dataTags),
m_id(p_rhs.m_id)
{
}

//namespace end
}
}
}
