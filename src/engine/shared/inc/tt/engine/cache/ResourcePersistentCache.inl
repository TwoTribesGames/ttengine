#include <tt/code/helpers.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/file/ResourceHeader.h>
#include <tt/fs/File.h>
#include <tt/system/Time.h>
#include <tt/thread/CriticalSection.h>


// Resource Logging
#ifndef TT_BUILD_FINAL
#define RESOURCE_LOG_ENABLED
#endif

namespace tt {
namespace engine {
namespace cache {

template<class ResourceType>
typename ResourcePersistentCache<ResourceType>::ResourceContainer ResourcePersistentCache<ResourceType>::ms_resources;

template<class ResourceType>
thread::Mutex ResourcePersistentCache<ResourceType>::ms_mutex;

template<class ResourceType>
typename ResourcePersistentCache<ResourceType>::TimeStamps ResourcePersistentCache<ResourceType>::ms_timestamps;

#ifdef RESOURCE_LOG_ENABLED
template<class ResourceType>
ResourceLog* ResourcePersistentCache<ResourceType>::ms_log = 0;


template<class ResourceType>
void ResourcePersistentCache<ResourceType>::startLog(const std::string& p_filename)
{
	ms_log = new ResourceLog(p_filename);
}


template<class ResourceType>
void ResourcePersistentCache<ResourceType>::stopLog()
{
	delete ms_log;
	ms_log = 0;
}
#endif


template<class ResourceType>
bool ResourcePersistentCache<ResourceType>::exists(const std::string& p_resource, const std::string& p_namespace)
{
	return exists(EngineID(p_resource, p_namespace));
}


template<class ResourceType>
bool ResourcePersistentCache<ResourceType>::exists(const EngineID& p_id)
{
	return file::FileUtils::getInstance()->exists(p_id, ResourceType::fileType);
}


template<class ResourceType>
const typename ResourcePersistentCache<ResourceType>::ResourcePtr& ResourcePersistentCache<ResourceType>::get(
	const std::string& p_resource, const std::string& p_namespace, bool p_useDefault, u32 p_flags)
{
	return get(EngineID(p_resource, p_namespace), p_useDefault, p_flags);
}


template<class ResourceType>
const typename ResourcePersistentCache<ResourceType>::ResourcePtr& ResourcePersistentCache<ResourceType>::get(
	const EngineID& p_id, bool p_useDefault, u32 p_flags)
{
#ifdef RESOURCE_LOG_ENABLED
	if(ms_log != 0) ms_log->startEvent(p_id);
#endif
	
	thread::CriticalSection criticalSection(&ms_mutex);
	
	typename ResourceContainer::iterator it = ms_resources.find(p_id);
	
	if(it != ms_resources.end())
	{
		// Already in cache - return shared pointer
#ifdef RESOURCE_LOG_ENABLED
		const ResourcePtr& resource(it->second);
		TT_NULL_ASSERT(resource);
		ItemInfo item(CacheEvent_InCache, resource->getMemSize(), static_cast<s32>(resource.use_count()));
		if(ms_log != 0) ms_log->endEvent(item);
		return resource;
#else
		return it->second;
#endif
	}
	
	return load(p_id, p_useDefault, p_flags);
}


template<class ResourceType>
const typename ResourcePersistentCache<ResourceType>::ResourcePtr& ResourcePersistentCache<ResourceType>::find(
	const std::string& p_resourceName, const std::string& p_namespace)
{
	return find(EngineID(p_resourceName, p_namespace));
}


template<class ResourceType>
const typename ResourcePersistentCache<ResourceType>::ResourcePtr& ResourcePersistentCache<ResourceType>::find(const EngineID& p_id)
{
	thread::CriticalSection criticalSection(&ms_mutex);
	
	typename ResourceContainer::iterator it = ms_resources.find(p_id);
	
	if(it != ms_resources.end())
	{
		return it->second;
	}
	
	static ResourcePtr empty;
	return empty;
}


template<class ResourceType>
const typename ResourcePersistentCache<ResourceType>::ResourcePtr& ResourcePersistentCache<ResourceType>::getDefault()
{
	return get("default", "default");
}


template<class ResourceType>
void ResourcePersistentCache<ResourceType>::dump()
{
#if !defined(TT_BUILD_FINAL)
	thread::CriticalSection criticalSection(&ms_mutex);
	
	// FIXME: Remove duplicate code; see dumpToFile()
	if (ms_resources.empty())
	{
		// no data; early abort
		return;
	}
	
	TT_Printf("-- RESOURCE DUMP START (%s) --\n", 
		system::Time::getInstance()->getNowAsString().c_str());
	u32 totalSize = 0;
	u32 totalResources = 0;
	
	typedef std::multimap<int, std::string> SizeMap;
	SizeMap sortedMap;
	
	for(typename ResourceContainer::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		const ResourcePtr& t((*it).second);
		
		++totalResources;
		totalSize += t->getMemSize();
		sortedMap.insert(std::make_pair((t->getMemSize() / 1024), it->first.getName()));
	}
	
	// output statistics
	TT_Printf("\nTotal resources: %u\n", totalResources);
	
	TT_Printf("Total resource memory in use: %.3f MB\n\n", (totalSize/(1024.0f*1024.0f)));
	
	for (SizeMap::reverse_iterator it = sortedMap.rbegin(); it != sortedMap.rend(); ++it)
	{
		TT_Printf("\tsize: %6d KB\tname: '%s'\n", (*it).first, (*it).second.c_str());
	}
	
	TT_Printf("\n-- RESOURCE DUMP END --\n\n");
#endif
}


template<class ResourceType>
void ResourcePersistentCache<ResourceType>::dumpToFile(const std::string& p_filename, bool p_append)
{
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	thread::CriticalSection criticalSection(&ms_mutex);
	
	if (ms_resources.empty())
	{
		// no data; early abort
		return;
	}
	
	tt::fs::FilePtr f;
	
	if (p_append)
	{
		f = tt::fs::open(p_filename, tt::fs::OpenMode_Append);
	}
	else
	{
		f = tt::fs::open(p_filename, tt::fs::OpenMode_Write);
	}
	
	if (f == 0)
	{
		TT_PANIC("Cannot open log file '%s'", p_filename.c_str());
		return;
	}
	
	char buf[1024];
	
	sprintf(buf, "-- RESOURCE DUMP START (%s) --\r\n", 
		system::Time::getInstance()->getNowAsString().c_str());
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	u32 totalSize = 0;
	u32 totalResources = 0;
	
	typedef std::multimap<int, std::string> SizeMap;
	SizeMap sortedMap;
	
	for(typename ResourceContainer::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		const ResourcePtr& t((*it).second);
		
		++totalResources;
		totalSize += t->getMemSize();
		sortedMap.insert(std::make_pair((t->getMemSize() / 1024), it->first.getName()));
	}
	
	// output statistics
	sprintf(buf, "\r\nTotal resources: %u\r\n", totalResources);
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	
	sprintf(buf, "Total resource memory in use: %.3f MB\r\n\r\n", (totalSize/(1024.0f*1024.0f)));
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	
	for (SizeMap::const_reverse_iterator it = sortedMap.rbegin(); it != sortedMap.rend(); ++it)
	{
		sprintf(buf, "\tsize: %6d KB\tname: '%s'\r\n", (*it).first, (*it).second.c_str());
	
		f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	}
	
	sprintf(buf, "\r\n-- RESOURCE DUMP END --\r\n\r\n");
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
#else // #if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	(void)p_filename;
	(void)p_append;
#endif // #else //#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
}


template<class ResourceType>
tt::engine::EngineIDs ResourcePersistentCache<ResourceType>::getEngineIDs()
{
	thread::CriticalSection criticalSection(&ms_mutex);
	
	tt::engine::EngineIDs result;
	
	for(typename ResourceContainer::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		const ResourcePtr& resource((*it).second);
		
		if (resource != 0) 
		{
			result.push_back(resource->getEngineID());
		}
	}
	
	return result;
}


template<class ResourceType>
bool ResourcePersistentCache<ResourceType>::checkForChanges(const tt::engine::EngineIDs& p_engineIDs)
{
	for (tt::engine::EngineIDs::const_iterator it = p_engineIDs.begin(); it != p_engineIDs.end(); ++it)
	{
		const tt::engine::EngineID& id = (*it);
		
		if(id.valid() == false)
		{
			continue;
		}
		
		const tt::fs::time_type t(file::FileUtils::getInstance()->getLastWriteTime(id, ResourceType::fileType));
		
		if (t < 0)
		{
			// File not found
			continue;
		}
		
		{
			thread::CriticalSection criticalSection(&ms_mutex);
			
			if(t != ms_timestamps[id])
			{
				return true;
			}
		}
	}
	
	return false;
}


template<class ResourceType>
s32 ResourcePersistentCache<ResourceType>::reload()
{
	thread::CriticalSection criticalSection(&ms_mutex);
	
#if !defined(TT_BUILD_FINAL)
	const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	s32 resourceCount(0);
	
	for(typename ResourceContainer::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		const ResourcePtr& resource((*it).second);
		
		if (resource == 0)
		{
			continue;
		}
		
		EngineID id(resource->getEngineID());
		
		if(id.valid() == false)
		{
			continue;
		}
		
		fs::FilePtr file(file::FileUtils::getInstance()->getDataFile(id, ResourceType::fileType));
		
		if (file == 0)
		{
			// File not found
			TT_WARN("Resource Not Found: %s", id.toDebugString().c_str());
			continue;
		}
		
		if(file->getWriteTime() == ms_timestamps[id])
		{
			// File has not changed
			continue;
		}
		
		// Read header
		if(ResourceType::hasResourceHeader)
		{
			file::ResourceHeader header;
			
			if(file->read(&header, sizeof(header)) != sizeof(header))
			{
				TT_PANIC("[ENGINE] Failed to read resource header.");
				continue;
			}
			
			// Validate version
			if (header.checkVersion() == false)
			{
				continue;
			}
		}
		
		resource->load(file);
		ms_timestamps[id] = file->getWriteTime();
		++resourceCount;
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("Reloaded %d cached assets in %4u ms\n", resourceCount, u32(loadEnd - loadStart));
#endif
	
	return resourceCount;
}


template<class ResourceType>
void ResourcePersistentCache<ResourceType>::clear()
{
	tt::code::helpers::freeContainer(ms_resources);
}


template<class ResourceType>
s32 ResourcePersistentCache<ResourceType>::getTotalMemSize()
{
	thread::CriticalSection criticalSection(&ms_mutex);
	
	s32 totalSize(0);
	
	for (typename ResourceContainer::iterator it = ms_resources.begin(); it != ms_resources.end(); ++it)
	{
		const ResourcePtr& resource = (*it).second;
		if (resource != 0)
		{
			totalSize += resource->getMemSize();
		}
	}
	
	return totalSize;
}


////////////////////////////////////////////////////////////////
// Private


template<class ResourceType>
const typename ResourcePersistentCache<ResourceType>::ResourcePtr& ResourcePersistentCache<ResourceType>::load(
	const EngineID& p_id, bool p_useDefault, u32 p_flags)
{
	//const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
	
	fs::FilePtr file(file::FileUtils::getInstance()->getDataFile(p_id, ResourceType::fileType));
	
	static ResourcePtr empty;
	if (file == 0)
	{
		// File not found
		TT_WARN("Resource Not Found: %s", p_id.toDebugString().c_str());
		return p_useDefault ? getDefault() : empty;
	}
	
	// Read header
	if(ResourceType::hasResourceHeader)
	{
		file::ResourceHeader header;
		
		if(file->read(&header, sizeof(header)) != sizeof(header))
		{
			TT_PANIC("[ENGINE] Failed to read resource header.");
			return empty;
		}
		
		// Validate version
		if (header.checkVersion() == false)
		{
			return empty;
		}
	}
	
	// Create a new object for the resource
	ResourceType* raw(ResourceType::create(file, p_id, p_flags));
	
	if (raw != 0)
	{
		if (raw->load(file) == false)
		{
			TT_PANIC("[ENGINE] Failed to load: [%s]", p_id.toDebugString().c_str());
			delete raw;
			return empty;
		}
	}
	else
	{
		return empty;
	}
	
	// Transfer ownership to smart pointer
	TT_NULL_ASSERT(raw);
	ResourcePtr resource(raw);
	
	TT_NULL_ASSERT(resource);
	
	// Add to cache
	TT_ASSERT(ms_resources.find(p_id) == ms_resources.end());
	ms_resources [p_id] = resource;
	ms_timestamps[p_id] = file->getWriteTime();
	
#ifdef RESOURCE_LOG_ENABLED
	ItemInfo item(CacheEvent_Added, resource->getMemSize(), static_cast<s32>(resource.use_count()));
	if(ms_log != 0) ms_log->endEvent(item);
#endif
	
	//const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	//static u64 totalTime(0);
	//totalTime += (loadEnd - loadStart);
	//TT_Printf("Texture Loading: [%4u ms] [%5u ms] '%s'\n", u32(loadEnd - loadStart), u32(totalTime), file->getPath());
	
	return ms_resources[p_id];
}

// Namespace end
}
}
}
