#include <tt/code/bufferutils.h>
#include <tt/pres/TimerStack.h>

namespace tt {
namespace pres {


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 0);


bool TimerStack::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "saving timer stack");
		
	if(getBufferSize() > p_sizeOUT)
	{
		TT_ERR_AND_RETURN("Not enough space in buffer need " << getBufferSize() 
		                  << " got " << p_sizeOUT);
	}
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u16>(m_allAnimations.size()), p_bufferOUT, p_sizeOUT);
	
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		if((*it)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false) 
		{
			TT_ERR_RETURN_ON_ERROR();
		}
	}
	return true;
}


bool TimerStack::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                       const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                       code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "loading timer stack");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid version: code "
		<< GET_MAJOR_VERSION(s_version) << "." << GET_MINOR_VERSION(s_version) << ", data "
		<< GET_MAJOR_VERSION(version)   << "." << GET_MINOR_VERSION(version)   <<
		" -- please update your converter.");
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
	const u16 timerCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < timerCount ; ++i)
	{
		TimerPtr timer(new Timer);
		timer->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
		
		m_allAnimations.push_back(timer);
		TT_ASSERT(m_activeAnimations.empty());
	}
	
	return true;
}


size_t TimerStack::getBufferSize() const
{
	size_t size(2 + 2); // number of animations + version
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		size += (*it)->getBufferSize(); 
	}
	return size;
}


void TimerStack::appendStack(TimerStack& p_other)
{
	// just append the whole stack to the end
	m_allAnimations.insert(m_allAnimations.end(), p_other.m_allAnimations.begin(), p_other.m_allAnimations.end());
	TT_ASSERT(m_activeAnimations.empty());
	p_other.m_allAnimations.clear();
	p_other.m_activeAnimations.clear();
}


TimerStack::TimerStack(const TimerStack& p_rhs)
:
anim2d::StackBase<Timer>(p_rhs)
{
}

//namespace end
}
}
