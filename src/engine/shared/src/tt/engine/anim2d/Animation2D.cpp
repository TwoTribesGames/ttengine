#include <string>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/Animation2D.h>
#include <tt/fs/File.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>



namespace tt {
namespace engine {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_versionAnimation2D = MAKE_VERSION(1, 5);
static const size_t s_sizeAnimation2D = 2 + 8 + 8 + 1 + 1 + 1 + 1 + 2; // version, duration, delay, timetype, direction, looping, tweentype, tagCount


Animation2D::TimeType Animation2D::timeTypeFromString( const std::string& p_name )
{
	if      (p_name == "linear")     return TimeType_Linear;
	else if (p_name == "easein")     return TimeType_EaseIn;
	else if (p_name == "easeout")    return TimeType_EaseOut;
	else if (p_name == "easeends")   return TimeType_EaseEnds;
	else if (p_name == "easecenter") return TimeType_EaseCenter;
	else
	{
		TT_PANIC("Unknown timetype '%s'.",
			p_name.c_str());
		return TimeType(0);
	}
}


std::string Animation2D::timeTypeToString(const TimeType& p_timeType)
{
	switch(p_timeType)
	{
	case TimeType_Linear:      return "linear";
	case TimeType_EaseIn:      return "easein";
	case TimeType_EaseOut:     return "easeout";
	case TimeType_EaseEnds:    return "easeends";
	case TimeType_EaseCenter:  return "easecenter";
	default: TT_PANIC("Unknown timetype"); return "unknown";
	}
}


Animation2D::DirectionType Animation2D::directionTypeFromString( const std::string& p_name )
{
	if      (p_name == "forward")         return DirectionType_Forward;
	else if (p_name == "backward")        return DirectionType_Backward;
	else if (p_name == "pingpong")        return DirectionType_PingPong;
	else if (p_name == "reversepingpong") return DirectionType_ReversePingPong;
	else
	{
		TT_PANIC("Unknown directiontype '%s'.",
			p_name.c_str());
		return DirectionType(0);
	}
}


std::string Animation2D::directionTypeToString(const DirectionType& p_directionType)
{
	switch(p_directionType)
	{
	case DirectionType_Forward:            return "forward";
	case DirectionType_Backward:           return "backward";
	case DirectionType_PingPong:           return "pingpong";
	case DirectionType_ReversePingPong:    return "reversepingpong";
	default: TT_PANIC("Unknown timetype"); return "unknown";
	}
}


Tag Animation2D::loadTag( const xml::XmlNode* p_node, const Tags& p_acceptedTags,
                          code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(Tag, Tag(), "loading tag");
	
	TT_ERR_ASSERT(p_node->getName() == "tag");
	
	std::string tagstr(xml::util::parseStr(p_node, "name", &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	
	Tag tag(tagstr);
	
	// check if the tag is accepted. If the acceptedTags is empty all tags are allowed.
	TT_ASSERTMSG(p_acceptedTags.empty() || p_acceptedTags.find(tag) != p_acceptedTags.end(),
	             "Unsupported Tag: %s", tagstr.c_str());
	
	return tag;
}


Animation2D::Animation2D()
:
m_active(false),
m_paused(false),
m_duration(1.0f),
m_time(0.0f),
m_internalTime(0.0f),
m_delta(1.0f),
m_delay(0.0f),
m_startTime(0.0f),
m_instantAtEnd(false),
m_durationRange(),
m_delayRange(),
m_timeType(TimeType_Linear),
m_tweenType(TweenType_QUAD),
m_directionType(DirectionType_Forward),
m_looping(false),
m_tags(),
m_strTags(),
m_tagged(true)
{
}


Animation2D::Animation2D(const Tags& p_tags)
:
m_active(false),
m_paused(false),
m_duration(1.0f),
m_time(0.0f),
m_internalTime(0.0f),
m_delta(1.0f),
m_delay(0.0f),
m_startTime(0.0f),
m_instantAtEnd(false),
m_durationRange(),
m_delayRange(),
m_timeType(TimeType_Linear),
m_tweenType(TweenType_QUAD),
m_directionType(DirectionType_Forward),
m_looping(false),
m_tags(p_tags),
m_strTags(),
m_tagged(false)
{
}


void Animation2D::update(real p_delta)
{
	if (m_active == false || m_paused || m_tagged == false)
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
		// quadratic (x^2)
		//m_time = pos * pos;
		m_time = Tween::tweenFactory(m_tweenType).easeIn(pos,0,1,1);
		break;

	case TimeType_EaseOut:
		// quaratic
		//m_time = -pos * (pos - 2.0f);
		m_time = Tween::tweenFactory(m_tweenType).easeOut(pos,0,1,1);
		break;

	case TimeType_EaseEnds:
		// cosine
		//m_time = -0.5f * (math::cos(pos * math::pi) - 1.0f);
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


void Animation2D::start()
{
	Tags emptyTags;
	return start(emptyTags);
}


void Animation2D::start( const Tags& p_tags )
{
	// when m_tags is empty always start
	bool doStart = m_tags.empty();
	
	
	// when neither is empty check the tags needed to start
	if(doStart == false)
	{
		doStart = true;
		for(Tags::const_iterator it(p_tags.begin()) ; it != p_tags.end() ; ++it)
		{
			if(m_tags.find(*it) == m_tags.end())
			{
				doStart = false;
				break;
			}
		}
	}
	
	// actualy start
	if(doStart)
	{
		if (m_active)
		{
			return;
		}
		setRanges(); // get random values from all ranges

		m_active = true;
		m_time = 0.0f;
		m_internalTime = 0.0f;
		m_startTime = m_delay;
		m_tagged = true;
		return;
	}
	else
	{
		m_tagged = false;
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
	m_time = 0.0f;
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
	TT_ASSERTMSG(m_active == false || m_timeType == p_type,
	             "Cannot change playback parameters while animation is active.");
	m_timeType = p_type;
}

void Animation2D::setTweenType( TweenType p_type )
{
	TT_ASSERTMSG(m_active == false || m_tweenType == p_type,
	             "Cannot change playback parameters while animation is active.");
	m_tweenType = p_type;
}

void Animation2D::setDirection(DirectionType p_type)
{
	TT_ASSERTMSG(m_active == false || m_directionType == p_type,
	             "Cannot change playback parameters while animation is active.");
	m_directionType = p_type;
}


void Animation2D::setLooping(bool p_loop)
{
	TT_ASSERTMSG(m_active == false || m_looping == p_loop,
	             "Cannot change playback parameters while animation is active.");
	m_looping = p_loop;
}


void Animation2D::setDurationRange(math::Range p_duration)
{
	TT_ASSERTMSG(p_duration.getMin() > 0.0f, "Duration should be larger than 0");
	TT_ASSERTMSG(m_active == false || m_durationRange == p_duration,
	             "Cannot change duration while animation is active.");
	m_durationRange = p_duration;
}


void Animation2D::setDuration(real p_duration)
{
	TT_ASSERTMSG(m_active == false, "Cannot change duration while animation is active.");
	m_durationRange.setMinMax(p_duration);
}


void Animation2D::setDelayRange(math::Range p_delay)
{
	TT_ASSERTMSG(m_active == false || m_delayRange == p_delay,
	             "Cannot change delay while animation is active.");
	m_delayRange = p_delay;
}


void Animation2D::setDelay(real p_delay)
{
	TT_ASSERTMSG(p_delay >= 0.0f, "Delay should be larger or equal than 0");
	TT_ASSERTMSG(m_active == false, "Cannot change delay while animation is active.");
	m_delayRange.setMinMax(p_delay);
}


bool Animation2D::load(const xml::XmlNode* p_node)
{
	Tags empty;
	return load(p_node, empty, empty);
}


bool Animation2D::load( const xml::XmlNode* p_node, 
                        const Tags& p_applyTags, 
                        const Tags& p_acceptedTags )
{
	TT_NULL_ASSERT(p_node);
	
	TT_ERR_CREATE("load");
	
	if (p_node->getAttribute("duration").empty())
	{
		TT_PANIC("Expected attribute 'duration' in node '%s'", 
			p_node->getName().c_str());
		return false;
	}
	
	const std::string& preset = p_node->getAttribute("preset");
	Preset presetType = Animation2D::Preset_OneShot;
	if (preset.empty() == false)
	{
		if      (preset == "oneshot")     presetType = Preset_OneShot;
		else if (preset == "looping")     presetType = Preset_Looping;
		else if (preset == "pingpong")    presetType = Preset_PingPong;
		else if (preset == "oscillating") presetType = Preset_Oscillating;
#ifndef TT_BUILD_FINAL
		else if (preset == "oscilating")
		{
			TT_PANIC("Found 'oscilating' in attribute 'preset' in node '%s', this should be 'oscillating'", 
				p_node->getName().c_str());
			return false;
		}
		else if (preset == "once")
		{
			TT_PANIC("Found 'once' in attribute 'preset' in node '%s', this should be 'oneshot'", 
				p_node->getName().c_str());
			return false;
		}
#endif
		else
		{
			TT_PANIC("Found unknown value '%s' in attribute 'preset' in node '%s'.",
				preset.c_str(), p_node->getName().c_str());
			return false;
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
			TT_PANIC("Found unknown value '%s' in attribute 'timing' in node '%s'.",
				timing.c_str(), p_node->getName().c_str());
			return false;
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
			TT_PANIC("Found unknown value '%s' in attribute 'direction' in node '%s'.",
				direction.c_str(), p_node->getName().c_str());
			return false;
		}
	}
	
	const std::string& looping = p_node->getAttribute("looping");
	bool loopType = false;
	if (looping.empty() == false)
	{
		loopType = str::parseBool(looping, &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Value '%s' of attribute 'looping' from node '%s' can't be converted to a bool.",
				looping.c_str(), 
				p_node->getName().c_str());
			return false;
		}
	}
	
	math::Range duration = str::parseRange(p_node->getAttribute("duration"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'duration' from node '%s' can't be converted to a Range.",
			p_node->getAttribute("duration").c_str(), 
			p_node->getName().c_str());
		return false;
	}
	
	const std::string& delayStr = p_node->getAttribute("delay");
	math::Range delay;
	if (delayStr.empty() == false)
	{
		delay = str::parseRange(delayStr, &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Value '%s' of attribute 'delay' from node '%s' can't be converted to a Range.",
				delayStr.c_str(), 
				p_node->getName().c_str());
			return false;
		}
	}
	
	if (preset.empty() == false)   setPreset(presetType);
	if (timing.empty() == false)   setTimeType(timingType);
	if (tweening.empty() == false) setTweenType(tweenType);
	if (direction.empty() == false)setDirection(directionType);
	if (looping.empty() == false)  setLooping(loopType);
	if (delayStr.empty() == false) setDelayRange(delay);
	
	setDurationRange(duration);
	setRanges();
	
	
	addTags(p_applyTags);
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "tag")
		{
			TT_ERR_SET_LOC("loading tag from animation");
			
			Tag tag(Animation2D::loadTag(child, p_acceptedTags, &errStatus));
			
			TT_ASSERTMSG(errStatus.hasError() == false, "Tag loading failed: %s", 
				         errStatus.getErrorMessage().c_str());
			if(errStatus.hasError()) return false;
			
			addTag(child->getAttribute("name"));
			addTag(tag);
		}
	}
	
	return true;
}


bool Animation2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);

	p_node->setAttribute("duration", str::toStr(getDurationRange()));
	p_node->setAttribute("delay", str::toStr(getDelayRange()));
	switch (getTimeType())
	{
	case TimeType_Linear:     p_node->setAttribute("timing", "linear");     break;
	case TimeType_EaseIn:     p_node->setAttribute("timing", "easein");     break;
	case TimeType_EaseOut:    p_node->setAttribute("timing", "easeout");    break;
	case TimeType_EaseEnds:   p_node->setAttribute("timing", "easeends");   break;
	case TimeType_EaseCenter: p_node->setAttribute("timing", "easecenter"); break;
	default:
		TT_PANIC("Unknown/unsupported timing type %d.", getTimeType());
		return false;
	}

	switch (getDirection())
	{
	case DirectionType_Forward:         p_node->setAttribute("direction", "forward");         break;
	case DirectionType_Backward:        p_node->setAttribute("direction", "backward");        break;
	case DirectionType_PingPong:        p_node->setAttribute("direction", "pingpong");        break;
	case DirectionType_ReversePingPong: p_node->setAttribute("direction", "reversepingpong"); break;
	default:
		TT_PANIC("Unknown/unsupported direction type %d.", getDirection());
		return false;
	}
	p_node->setAttribute("looping", str::toStr(isLooping()));

	p_node->setAttribute("tweening",Tween::tweenTypeToString(getTweenType()));

	return true;
}


bool Animation2D::load(const fs::FilePtr& p_file)
{
	u8 buffer[s_sizeAnimation2D] = {0};
	if (p_file->read(buffer, static_cast<fs::size_type>(s_sizeAnimation2D)) != static_cast<fs::size_type>(s_sizeAnimation2D))
	{
		TT_PANIC("File '%s' too small.", p_file->getPath());
		return false;
	}
	size_t size = s_sizeAnimation2D;
	const u8* scratch = buffer;

	return Animation2D::load(scratch, size);
}


bool Animation2D::save(const fs::FilePtr& p_file) const
{
	u8 buffer[s_sizeAnimation2D] = {0};
	u8* scratch = buffer;
	size_t size = s_sizeAnimation2D;
	if (Animation2D::save(scratch, size) == false)
	{
		return false;
	}

	if (p_file->write(buffer, static_cast<fs::size_type>(s_sizeAnimation2D)) != static_cast<fs::size_type>(s_sizeAnimation2D))
	{
		TT_PANIC("Failed to save %d bytes to '%s'", s_sizeAnimation2D, p_file->getPath());
		return false;
	}
	return true;
}


bool Animation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	Tags empty;
	return load(p_bufferOUT, p_sizeOUT, empty, empty);
}


bool Animation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                        const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	if (p_sizeOUT < s_sizeAnimation2D)
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, s_sizeAnimation2D);
		return false;
	}

	using namespace code::bufferutils;

	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionAnimation2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d, Please update your converter",
			GET_MAJOR_VERSION(s_versionAnimation2D), GET_MINOR_VERSION(s_versionAnimation2D),
			GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}

	real durMin = be_get<real>(p_bufferOUT, p_sizeOUT);
	real durMax = be_get<real>(p_bufferOUT, p_sizeOUT);
	real delMin = be_get<real>(p_bufferOUT, p_sizeOUT);
	real delMax = be_get<real>(p_bufferOUT, p_sizeOUT);
	setDurationRange(math::Range(durMin, durMax));
	setDelayRange(math::Range(delMin, delMax));

	setTimeType (static_cast<TimeType     >(be_get<u8  >(p_bufferOUT, p_sizeOUT)));
	setTweenType(static_cast<TweenType    >(be_get<u8  >(p_bufferOUT, p_sizeOUT)));
	setDirection(static_cast<DirectionType>(be_get<u8  >(p_bufferOUT, p_sizeOUT)));
	setLooping  (                           be_get<bool>(p_bufferOUT, p_sizeOUT));
	setRanges();

	addTags(p_applyTags);

	const u16 tagCount = be_get<u16>(p_bufferOUT, p_sizeOUT);

	for (u16 i = 0 ; i < tagCount ; ++i)
	{
		std::string tagstr(be_get<std::string>(p_bufferOUT, p_sizeOUT));
		Tag tag(tagstr);
		
		// check if the tag is accepted. If the acceptedTags is empty all tags are allowed.
		TT_ASSERTMSG(p_acceptedTags.empty() || p_acceptedTags.find(tag) != p_acceptedTags.end(),
		             "Unsupported Tag: %s", tagstr.c_str());
		
		m_tags.insert(tag);
		//m_strTags.push_back(tagstr);
	}
	
	return true;
}


bool Animation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT) const
{
	if (p_sizeOUT < s_sizeAnimation2D)
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, s_sizeAnimation2D);
		return false;
	}
	
	using namespace code::bufferutils;
	be_put(s_versionAnimation2D, p_bufferOUT, p_sizeOUT);
	
	be_put(m_durationRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_durationRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_delayRange.getMin(),    p_bufferOUT, p_sizeOUT);
	be_put(m_delayRange.getMax(),    p_bufferOUT, p_sizeOUT);
	
	be_put(static_cast<u8>(m_timeType     ), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_tweenType    ), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_directionType), p_bufferOUT, p_sizeOUT);
	be_put(m_looping,                        p_bufferOUT, p_sizeOUT);
	
	be_put(static_cast<u16>(m_strTags.size()), p_bufferOUT, p_sizeOUT);// count of tags
	for(str::Strings::const_iterator tag(m_strTags.begin()) ; tag != m_strTags.end() ; ++tag)
	{
		be_put(*tag, p_bufferOUT, p_sizeOUT);
	}
	
	return true;
}


size_t Animation2D::getBufferSize() const
{
	size_t size(s_sizeAnimation2D);
	for(str::Strings::const_iterator tag(m_strTags.begin()) ; tag != m_strTags.end() ; ++tag)
	{
		size += 2 + tag->size();
	}
	return size;
}


void Animation2D::setRanges()
{
	tt::math::Random& rng(tt::math::Random::getStatic());

	m_duration = m_durationRange.getRandom(rng);
	m_delay    = m_delayRange.getRandom(rng);

	if(tt::math::realEqual(m_duration, 0.0f))
	{
		m_delta        = 0.0f;
		m_instantAtEnd = true;
	}
	else
	{
		m_delta        = 1.0f / m_duration;
		m_instantAtEnd = false;
	}
}


Animation2D::Animation2D(const Animation2D& p_rhs)
:
m_active(p_rhs.m_active),
m_paused(p_rhs.m_paused),
m_duration(p_rhs.m_duration),
m_time(p_rhs.m_time),
m_internalTime(p_rhs.m_internalTime),
m_delta(p_rhs.m_delta),
m_delay(p_rhs.m_delay),
m_startTime(p_rhs.m_startTime),
m_instantAtEnd(p_rhs.m_instantAtEnd),
m_durationRange(p_rhs.m_durationRange),
m_delayRange(p_rhs.m_delayRange),
m_timeType(p_rhs.m_timeType),
m_tweenType(p_rhs.m_tweenType),
m_directionType(p_rhs.m_directionType),
m_looping(p_rhs.m_looping),
m_tags(p_rhs.m_tags),
m_strTags(p_rhs.m_strTags),
m_tagged(p_rhs.m_tagged)
{
}

// Namespace end
}
}
}
