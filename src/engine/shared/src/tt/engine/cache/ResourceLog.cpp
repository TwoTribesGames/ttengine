#include <sstream>
#include <iomanip>
#include <cstring>

#include <tt/engine/cache/ResourceLog.h>

#include <tt/platform/tt_printf.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>
#include <tt/system/Time.h>


namespace tt {
namespace engine {
namespace cache {

static const char* header = "Event, Time (s), Load Time (ms), Size (KB), Total Mem (MB), "
                            "Use Count, Total Count, Name, Namespace, CRC\n";

ResourceLog::ResourceLog(const std::string& p_filename)
:
m_logFile(fs::open(p_filename + ".csv", fs::OpenMode_Write)),
m_startTime(system::Time::getInstance()->getMicroSeconds()),
m_usedMemory(0),
m_totalCount(0),
m_eventStarted(),
m_currentID()
{
	if (m_logFile != 0)
	{
		m_logFile->write(header, static_cast<fs::size_type>(std::strlen(header)));
	}
}


void ResourceLog::startEvent(const EngineID& p_id)
{
	m_eventStarted.push_back(system::Time::getInstance()->getMicroSeconds());
	m_currentID.push_back(p_id);
}


void ResourceLog::endEvent(const ItemInfo& p_info)
{
	const u64 now(system::Time::getInstance()->getMicroSeconds());
	
	const u32 loadingTime(static_cast<u32>(now - (m_eventStarted.back())));
	
	if(p_info.cacheEvent == CacheEvent_Added)
	{
		m_usedMemory += p_info.memSize;
		++m_totalCount;
	}
	else if(p_info.cacheEvent == CacheEvent_Removed)
	{
		m_usedMemory -= p_info.memSize;
		--m_totalCount;
	}
	
	static const real microToSeconds(1 / 1000000.0f);
	static const real microToMilliSec(1 / 1000.0f);
	static const real bytesToMegaBytes(1 / (1024.0f * 1024.0f));
	static const real bytesToKiloBytes(1 / 1024.0f);

	std::ostringstream entry;

	switch(p_info.cacheEvent)
	{
		case CacheEvent_Added  : entry << "ADD, "; break;
		case CacheEvent_Removed: entry << "RMV, "; break;
		case CacheEvent_InCache: entry << "---, "; break;
	}

	const u64 currentTime(now - m_startTime);
	
	entry << currentTime     * microToSeconds   << ", ";
	entry << loadingTime     * microToMilliSec  << ", ";
	entry << p_info.memSize  * bytesToKiloBytes << ", ";
	entry << m_usedMemory    * bytesToMegaBytes << ", ";
	entry << p_info.useCount                    << ", ";
	entry << m_totalCount                       << ", ";

#ifndef TT_BUILD_FINAL
	entry << m_currentID.back().getName()       << ", ";
	entry << m_currentID.back().getNamespace()  << ", ";
	entry << m_currentID.back().toString()      << std::endl;
#else
	entry << "unknown"                          << ", ";
	entry << "unknown"                          << ", ";
	entry << m_currentID.back().toString()      << std::endl;
#endif

	if (m_logFile == 0)
	{
		TT_Printf("%s\n", entry.str().c_str());
	}
	else
	{
		m_logFile->write(entry.str().c_str(), static_cast<fs::size_type>(entry.str().length()));
		m_logFile->flush();
	}
	
	m_eventStarted.pop_back();
	m_currentID.pop_back();
}

// Namespace end
}
}
}
