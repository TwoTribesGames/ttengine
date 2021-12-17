#include <tt/code/helpers.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>

#include <tt/engine/scene2d/shoebox/Taggable.h>
#include <tt/engine/scene2d/shoebox/TagMgr.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

TagMgr::Taggables TagMgr::ms_taggables;


//--------------------------------------------------------------------------------------------------
// Public member functions

bool TagMgr::registerTaggable(const std::string& p_tag, Taggable* p_tagable)
{
	TT_NULL_ASSERT(p_tagable);
	TT_ASSERT(p_tag.empty() == false);
	
	ms_taggables.insert(std::make_pair(p_tag, p_tagable));
	return true;
}


bool TagMgr::unregisterTaggable(const std::string& p_tag, Taggable* p_tagable)
{
	const Taggables::iterator endIt = ms_taggables.upper_bound(p_tag);
	for (Taggables::iterator it = ms_taggables.lower_bound(p_tag); it != endIt; ++it)
	{
		if ((*it).second == p_tagable)
		{
			ms_taggables.erase(it);
			return true;
		}
	}
	TT_WARN("Tagable object %p with tag '%s' not found.", p_tagable, p_tag.c_str());
	return false;
}


bool TagMgr::sendEvent(const std::string& p_tag, const std::string& p_event,
                       const std::string& p_param)
{
	bool ret = true;
	const Taggables::iterator endIt = ms_taggables.upper_bound(p_tag);
	for (Taggables::iterator it = ms_taggables.lower_bound(p_tag); it != endIt; ++it)
	{
		if ((*it).second->handleEvent(p_event, p_param) == false)
		{
			TT_WARN("Event '%s' for tag '%s' resulted in an error by one of the taggable objects.",
			        p_event.c_str(), p_tag.c_str());
			ret = false;
		}
	}
	return ret;
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
