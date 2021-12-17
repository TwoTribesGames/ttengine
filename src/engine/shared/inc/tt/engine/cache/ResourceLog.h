#if !defined(INC_TT_ENGINE_CACHE_RESOURCELOG_H)
#define INC_TT_ENGINE_CACHE_RESOURCELOG_H


#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/engine/EngineID.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace cache {

enum CacheEvent
{
	CacheEvent_Added,
	CacheEvent_Removed,
	CacheEvent_InCache
};

struct ItemInfo
{
	CacheEvent cacheEvent;
	s32        memSize;
	s32        useCount;
	
	inline ItemInfo(CacheEvent p_event, s32 p_mem, s32 p_use)
	:
	cacheEvent(p_event),
	memSize(p_mem),
	useCount(p_use)
	{ }
};


class ResourceLog
{
public:
	ResourceLog(const std::string& p_filename);
	inline ~ResourceLog() { }

	void startEvent(const EngineID& p_id);
	void endEvent(const ItemInfo& p_info);

private:
	fs::FilePtr m_logFile;
	u64 m_startTime;
	s32 m_usedMemory;
	s32 m_totalCount;

	// Current Event Stacks
	std::vector<u64>      m_eventStarted;
	std::vector<EngineID> m_currentID;
};


// Namespace end
}
}
}



#endif // !defined(INC_TT_ENGINE_CACHE_RESOURCELOG_H)
