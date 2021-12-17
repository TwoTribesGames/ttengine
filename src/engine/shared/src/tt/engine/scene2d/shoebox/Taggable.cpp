#include <tt/code/helpers.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>

#include <tt/engine/scene2d/shoebox/Taggable.h>
#include <tt/engine/scene2d/shoebox/TagMgr.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {


//--------------------------------------------------------------------------------------------------
// Public member functions

Taggable::Taggable(const Taggable& p_rhs)
:
m_tags()
{
	for (Tags::const_iterator it = p_rhs.m_tags.begin(); it != p_rhs.m_tags.end(); ++it)
	{
		addTag(*it);
	}
}


Taggable::~Taggable()
{
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_DEV)
	// there seems to be a bug in the visual studio compiler regarding multiple inheritance
	// this somehow is a workaround
	Tags::const_iterator it = m_tags.begin();
	while (it != m_tags.end())
	{
		TagMgr::unregisterTaggable((*it), this);
		++it;
	}
#else
	for (Tags::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it)
	{
		TagMgr::unregisterTaggable((*it), this);
	}
#endif
}


const Taggable& Taggable::operator=(const Taggable& p_rhs)
{
	// Ignore self-assignment
	if (&p_rhs == this)
	{
		return *this;
	}
	
	for (Tags::iterator it = m_tags.begin(); it != m_tags.end(); ++it)
	{
		TagMgr::unregisterTaggable((*it), this);
	}
	code::helpers::freeContainer(m_tags);
	
	for (Tags::const_iterator it = p_rhs.m_tags.begin(); it != p_rhs.m_tags.end(); ++it)
	{
		addTag(*it);
	}
	return *this;
}


void Taggable::addTag(const std::string& p_tag)
{
	if (std::find(m_tags.begin(), m_tags.end(), p_tag) != m_tags.end())
	{
		TT_WARN("Attempt to add existing tag '%s' to Taggable object.", p_tag.c_str());
	}
	else
	{
		m_tags.push_back(p_tag);
		TagMgr::registerTaggable(p_tag, this);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
