#if !defined(INC_TT_ENGINE_CACHE_RESOURCECACHE_H)
#define INC_TT_ENGINE_CACHE_RESOURCECACHE_H

#include <map>
#include <string>

#include <tt/engine/fwd.h>
#include <tt/engine/EngineID.h>
#include <tt/engine/cache/ResourceLog.h>
#include <tt/platform/tt_types.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace engine {
namespace cache {


template<class ResourceType>
class ResourceCache
{
public:
	typedef typename tt_ptr<ResourceType>::shared ResourcePtr;
	typedef typename tt_ptr<ResourceType>::weak   ResourceWeakPtr;
	typedef std::map<EngineID, ResourceWeakPtr, EngineIDLess> ResourceContainer;
	
	ResourceCache() {};
	~ResourceCache() {};
	
	static void startLog(const std::string& p_filename);
	static void stopLog();
	
	static bool exists(const std::string& p_resource, const std::string& p_namespace);
	static bool exists(const EngineID& p_id);
	
	static ResourcePtr get(const std::string& p_resource, const std::string& p_namespace,
	                       bool p_useDefault = false, u32 p_flags = 0);
	static ResourcePtr get(const EngineID& p_id, bool p_useDefault, u32 p_flags = 0);
	
	
	static ResourcePtr find(const std::string& p_resource, const std::string& p_namespace);
	static ResourcePtr find(const EngineID& p_id);
	
	static ResourcePtr getDefault();
	
	static void dump();
	static void dumpToFile(const std::string& p_filename, bool p_append = true);
	
	static tt::engine::EngineIDs getEngineIDs();
	static bool checkForChanges(const tt::engine::EngineIDs& p_engineIDs);
	
	// Reloads all changed assets. Returns the number of reloaded assets.
	static s32 reload();
	
	static const ResourceContainer& getAllResources() { return ms_resources; }
	
	static s32 getTotalMemSize();
	
private:
	static ResourcePtr load(const EngineID& p_id, bool p_useDefault, u32 p_flags);
	static void remove(ResourceType* p_resource);
	
	static ResourceContainer ms_resources;
	static thread::Mutex     ms_mutex;
	
	typedef std::map<EngineID, fs::time_type, EngineIDLess> TimeStamps;
	static TimeStamps ms_timestamps;
	
	static ResourceLog* ms_log;
};


// Namespace end
}
}
}

#include "ResourceCache.inl"


#endif // !defined(INC_TT_ENGINE_CACHE_RESOURCECACHE_H)
