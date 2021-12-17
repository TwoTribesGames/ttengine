#include <tt/code/helpers.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/pres/PresentationCache.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>
#include <tt/platform/tt_error.h>

#if !defined(TT_BUILD_FINAL)
#include <tt/system/Time.h>
#include <tt/platform/tt_printf.h>
//#define TT_PRESENTATION_CACHE_TIMER_ON
#endif


namespace tt {
namespace pres {

PresentationCache::CacheEntries                 PresentationCache::ms_cacheEntries;
PresentationCache::PointerToCacheEntryContainer PresentationCache::ms_pointerToCacheEntryContainer;
PresentationCache::TimeStamps                   PresentationCache::ms_timestamps;
bool                                            PresentationCache::ms_removePermanentObjects = false;


class CacheEntry
{
public:
	CacheEntry(const std::string& p_filename, const PresentationObjectPtr& p_object, const Tags& p_requiredTags,
	           const engine::renderer::TextureContainer& p_textures)
	:
	m_filename(p_filename),
	m_ref(0),
	m_object(p_object),
	m_requiredTags(p_requiredTags),
	m_textures(p_textures)
	{
	}
	
	inline void incRef() { ++m_ref; }
	inline void decRef() { TT_ASSERT(m_ref > 0); --m_ref; }
	
	inline u32 getRef() const { return m_ref; }
	inline const PresentationObjectPtr& getObject() { return m_object; }
	inline const std::string& getFilename() const { return m_filename; }
	inline const Tags& getRequiredTags() const { return m_requiredTags; }
	
private:
	CacheEntry(const CacheEntry& p_rhs);                  // Not implemented.
	const CacheEntry& operator=(const CacheEntry& p_rhs); // Not implemented
	
	const std::string                  m_filename;
	u32                                m_ref;
	PresentationObjectPtr              m_object;
	Tags                               m_requiredTags;
	engine::renderer::TextureContainer m_textures;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

PresentationObjectPtr PresentationCache::get(const std::string& p_filename,
                                             const Tags& p_requiredTags,
                                             PresentationMgr* p_mgr)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PRESENTATION_CACHE_TIMER_ON)
	u64 loadStart = system::Time::getInstance()->getMilliSeconds();
	bool fromCache = false;
#endif
	
	math::hash::Hash<32> presentationHash(p_filename);
	CacheEntries::iterator it(ms_cacheEntries.find(presentationHash));
	CacheEntry* entry = 0;
	if(it == ms_cacheEntries.end())
	{
		PresentationObjectPtr cachedObj(new PresentationObject(0));
		
		PresentationLoader& loader(p_mgr->getPresentationLoader());
		
		if (loader.load(cachedObj, p_filename, p_mgr->getTriggerFactory(), p_requiredTags) == false)
		{
			TT_WARNING("Missing presentation file '%s'\n", p_filename.c_str());
			return loader.createDefault(p_mgr);
		}
		
		if (p_mgr->isUsedForPrecache() && cachedObj->dontPrecache())
		{
			TT_Printf("[DONT PRECACHE] %s\n", p_filename.c_str());
			return PresentationObjectPtr();
		}
		
#if !defined(TT_BUILD_FINAL)
		{
			tt::fs::FilePtr file = tt::fs::open(p_filename + ".pres", tt::fs::OpenMode_Read);
			if (file != 0)
			{
				ms_timestamps[p_filename] = file->getWriteTime();
			}
		}
#endif
		
		entry = new CacheEntry(p_filename, cachedObj, p_requiredTags, cachedObj->getAndLoadAllUsedTextures());
		
		ms_cacheEntries.insert(std::make_pair(presentationHash, entry));
	}
	else
	{
#if !defined(TT_BUILD_FINAL) && defined(TT_PRESENTATION_CACHE_TIMER_ON)
		fromCache = true;
#endif
		entry = it->second;
	}
	
	
	const PresentationObjectPtr& cachedObject(entry->getObject());
	entry->incRef();
	
	TT_NULL_ASSERT(cachedObject);
	
	PresentationObjectPtr object(cachedObject->clone(&remove, p_mgr));
	
	ms_pointerToCacheEntryContainer.insert(std::make_pair(object.get(), entry));
	
	if(p_requiredTags.empty() == false)
	{
		PresentationMgr::checkRequiredTags(p_requiredTags, object->getTags().getAllUsedTags(), p_filename);
	}
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PRESENTATION_CACHE_TIMER_ON)
	u64 loadEnd = system::Time::getInstance()->getMilliSeconds();
	TT_Printf("PresentationCache::get for file %s: '%s' took: %u ms.\n",
		fromCache ? "[cached]" : "[loaded]", p_filename.c_str(), u32(loadEnd - loadStart));
#endif
	return object;
}


void PresentationCache::clear()
{
	code::helpers::freePairSecondContainer(ms_cacheEntries);
	code::helpers::freeContainer(ms_pointerToCacheEntryContainer);
	code::helpers::freeContainer(ms_timestamps);
}


tt::str::Strings PresentationCache::getFilenames()
{
	tt::str::Strings result;
	
	for (CacheEntries::const_iterator it = ms_cacheEntries.begin(); it != ms_cacheEntries.end(); ++it)
	{
		result.push_back((*it).second->getFilename());
	}
	
	return result;
}


bool PresentationCache::checkForChanges(const tt::str::Strings& p_filenames)
{
	for (tt::str::Strings::const_iterator it = p_filenames.begin(); it != p_filenames.end(); ++it)
	{
		tt::fs::FilePtr file = tt::fs::open((*it) + ".pres", tt::fs::OpenMode_Read);
		if (file == 0)
		{
			continue;
		}
		
		const tt::fs::time_type timestamp = file->getWriteTime();
		if (timestamp != ms_timestamps[*it])
		{
			return true;
		}
	}
	
	return false;
}


// FIXME: Reuse the ResourceCache class?
s32 PresentationCache::reload(const PresentationMgrPtr& p_mgr)
{
#if !defined(TT_BUILD_FINAL)
	const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	CacheEntries shouldReload;
	
	for (CacheEntries::const_iterator it = ms_cacheEntries.begin(); it != ms_cacheEntries.end(); ++it)
	{
		const std::string& filename((*it).second->getFilename());
		
		tt::fs::FilePtr file = tt::fs::open(filename + ".pres", tt::fs::OpenMode_Read);
		if (file == 0)
		{
			shouldReload.insert(*it);
			ms_timestamps[filename] = 0;
			continue;
		}
		
		const tt::fs::time_type timestamp = file->getWriteTime();
		if (timestamp != ms_timestamps[filename])
		{
			shouldReload.insert(*it);
			ms_timestamps[filename] = timestamp;
			continue;
		}
	}
	
	PresentationLoader& loader(p_mgr->getPresentationLoader());
	
	for (CacheEntries::const_iterator it = shouldReload.begin(); it != shouldReload.end(); ++it)
	{
		CacheEntry* entry = (*it).second;
		
		PresentationObjectPtr object(entry->getObject());
		const std::string& filename(entry->getFilename());
		object->resetAndClear();
		
		if (loader.load(object, filename, p_mgr->getTriggerFactory(), entry->getRequiredTags()) == false)
		{
			TT_WARNING("Missing presentation file '%s'\n", filename.c_str());
		}
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Reloaded %d cached presentations in %4u ms\n", shouldReload.size(), u32(loadEnd - loadStart));
#endif
	
	return static_cast<s32>(shouldReload.size());
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PresentationCache::remove(PresentationObject* p_object)
{
	TT_NULL_ASSERT(p_object);
	
	// Check if cache is cleared. If it is, simply delete object and return
	if (ms_pointerToCacheEntryContainer.empty())
	{
		TT_ASSERT(ms_cacheEntries.empty());
		delete p_object;
		return;
	}
	
	TT_NULL_ASSERT(p_object);
	
	PointerToCacheEntryContainer::iterator it(ms_pointerToCacheEntryContainer.find(p_object));
	if(it == ms_pointerToCacheEntryContainer.end())
	{
		TT_PANIC("Cannot find PresentationObject* in ms_pointerToCacheEntryContainer. "
		         "Probably called clear() while objects were still alive.");
		delete p_object;
		return;
	}
	
	CacheEntry* entry = (*it).second;
	TT_NULL_ASSERT(entry);
	
	CacheEntries::iterator entryIt(ms_cacheEntries.find(math::hash::Hash<32>(entry->getFilename())));
	if(entryIt == ms_cacheEntries.end())
	{
		TT_PANIC("Cannot find '%s' in ms_cachedPresObjects.", entry->getFilename().c_str());
		ms_pointerToCacheEntryContainer.erase(it);
		delete p_object;
		return;
	}
	
	entry->decRef();
	
	// reference count is zero, remove cacheentry (which contains the actual presentation object)
	if (entry->getRef() == 0 && (p_object->isPermanent() == false || ms_removePermanentObjects))
	{
		ms_cacheEntries.erase(entryIt);
		delete entry;
	}
	
	ms_pointerToCacheEntryContainer.erase(it);
	delete p_object;
}


//namespace end
}
}
