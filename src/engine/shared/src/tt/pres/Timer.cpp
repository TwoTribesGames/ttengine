#include <tt/code/bufferutils.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/Timer.h>
#include <tt/xml/util/parse.h>


namespace tt {
namespace pres {


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 3);


Timer::Timer()
:
m_active(false),
m_paused(false),
m_duration(0.0f),
m_time(0.0f),
m_tags(),
m_tagged(false)
{
}


void Timer::update( real p_delta )
{
	if(m_active == false || m_paused || m_tagged == false) return;
	
	m_time -= p_delta;
	if(m_time <= 0)
	{
		m_active = false;
	}
}


void Timer::start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name)
{
	if(m_tags.shouldPlay(p_tags, p_name))
	{
		if (m_active)
		{
			return;
		}
		setRanges(p_presObj);
		
		m_time = m_duration;
		m_tagged = true;
		m_active = true;
		m_paused = false;
		return;
	}
	else
	{
		m_tagged = false;
		m_active = false;
	}
}


void Timer::stop()
{
	m_active = false;
}


void Timer::pause()
{
	if (m_active)
	{
		m_paused = true;
	}
}


void Timer::resume()
{
	if (m_active)
	{
		m_paused = false;
	}
}


void Timer::reset()
{
	if (m_paused)
	{
		resume();
	}
	if (m_active)
	{
		stop();
	}
}


void Timer::setDuration( real p_duration )
{
	TT_ASSERTMSG(p_duration > 0.0f, "Duration should be larger than 0");
	TT_ASSERTMSG(m_active == false, "Cannot change duration while timer is active.");
	m_duration.setValue(p_duration);
}


bool Timer::load( const xml::XmlNode* p_node, const DataTags& p_applyTags, const Tags& p_acceptedTags,
                  code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading Timer");
	TT_ERR_ASSERT(p_node->getName() == "time");
	
	m_duration = parsePresentationValue(p_node, "duration", 1.0f, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	m_tags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	setRanges(0);
	m_time = m_duration;
	
	return true;
}


bool Timer::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Saving Timer");
	
	if(getBufferSize() > p_sizeOUT)
	{
		TT_ERR_AND_RETURN("Not enough space in buffer need " << getBufferSize() 
		                  << " got " << p_sizeOUT);
	}
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	m_duration.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_tags.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	
	return true;
}


bool Timer::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                  const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                  code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading timer");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid version: code "
		<< GET_MAJOR_VERSION(s_version) << "." << GET_MINOR_VERSION(s_version) << ", data "
		<< GET_MAJOR_VERSION(version)   << "." << GET_MINOR_VERSION(version)   <<
		" -- please update your converter.");
	
	m_duration.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	m_tags.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	setRanges(0);
	
	return true;
}


size_t Timer::getBufferSize() const
{
	// duration + version
	size_t size(m_duration.getBufferSize() + 2 + m_tags.getBufferSize());
	return size;
}


void Timer::setRanges(PresentationObject* p_presObj)
{
	m_duration.updateValue(p_presObj);
}

//namespace end
}
}
