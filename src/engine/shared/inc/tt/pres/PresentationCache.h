#if !defined(INC_TT_PRES_PRESENTATIONCACHE_H)
#define INC_TT_PRES_PRESENTATIONCACHE_H
#include <map>
#include <string>

#include <tt/platform/tt_error.h>
#include <tt/pres/fwd.h>
#include <tt/str/str.h>


namespace tt {
namespace pres {

class CacheEntry;

class PresentationCache
{
public:
	static PresentationObjectPtr get(const std::string& p_filename,
		const Tags& p_requiredTags,
		PresentationMgr* p_mgr);
	
	inline static void setRemovePermanentObjectsEnabled(bool p_enabled)
	{
		ms_removePermanentObjects = p_enabled;
	}
	
	static void clear();
	
	static tt::str::Strings getFilenames();
	static bool checkForChanges(const tt::str::Strings& p_filenames);
	static s32 reload(const PresentationMgrPtr& p_mgr);
	
private:
	//not implemented
	PresentationCache();
	~PresentationCache();
	
	static void remove(PresentationObject* p_object);
	
	typedef std::map<math::hash::Hash<32>, CacheEntry*> CacheEntries;
	typedef std::map<PresentationObject*, CacheEntry*> PointerToCacheEntryContainer;
	typedef std::map<std::string, tt::fs::time_type> TimeStamps;
	
	static CacheEntries                 ms_cacheEntries;
	static PointerToCacheEntryContainer ms_pointerToCacheEntryContainer;
	static TimeStamps                   ms_timestamps;
	
	static bool                         ms_removePermanentObjects;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_PRESENTATIONCACHE_H)
