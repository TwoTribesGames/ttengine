#include <tt/platform/tt_error.h>

#include <toki/game/movement/MovementSet.h>
#include <toki/game/movement/MovementSetCache.h>

namespace toki {
namespace game {
namespace movement {

MovementSetCache::Cache MovementSetCache::ms_cache;

//--------------------------------------------------------------------------------------------------
// Public member functions

MovementSetPtr MovementSetCache::get(const std::string& p_filename)
{
	MovementSetPtr movementSet(find(p_filename));
	
	if(movementSet != 0)
	{
		// Already in cache - return smart pointer
		return movementSet;
	}
	return load(p_filename);
}


MovementSetPtr MovementSetCache::find(const std::string& p_filename)
{
	Cache::iterator it = ms_cache.find(p_filename);
	
	if(it != ms_cache.end())
	{
		// Found the movementset
		TT_ASSERT(it->second.expired() == false);
		return it->second.lock();
	}
	
	return MovementSetPtr();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

MovementSetPtr MovementSetCache::load(const std::string& p_filename)
{
	MovementSet* raw = MovementSet::load(p_filename);
	TT_NULL_ASSERT(raw);
	if (raw != 0)
	{
		// Transfer ownership to smart pointer
		MovementSetPtr cacheEntry(raw, remove);
		
		// Add to cache
		ms_cache[p_filename] = MovementSetWeakPtr(cacheEntry);
		
		return cacheEntry;
	}
	
	return MovementSetPtr();
}


void MovementSetCache::remove(MovementSet* p_movementSet)
{
	if(p_movementSet == 0) return;
	
	if(ms_cache.empty())
	{
		TT_WARN("Tried to remove movementset from empty cache. Check (static) destruction order!");
		return;
	}
	
	// Search for this movementset in the cache
	Cache::iterator it = ms_cache.find(p_movementSet->getFilename());
	
	if(it != ms_cache.end())
	{
		TT_ASSERTMSG(it->second.expired(),
			"Trying to remove a movementset that still has references: %s.",
			p_movementSet->getFilename().c_str());
		
		// Remove from cache
		ms_cache.erase(it);
		
		// Free memory
		delete p_movementSet;
	}
	else
	{
		TT_PANIC("Cannot find in movementset cache: %s", p_movementSet->getFilename().c_str());
	}
}

// Namespace end
}
}
}
