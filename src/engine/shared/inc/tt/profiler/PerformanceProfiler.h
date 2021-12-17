#ifndef INC_TT_PROFILER_PERFORMANCE_PROFILER_H
#define INC_TT_PROFILER_PERFORMANCE_PROFILER_H

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_printf.h>
#include <tt/profiler/PerformanceProfilerConstants.h>
#include <tt/system/Time.h>
#include <tt/str/toStr.h>

#include <vector>
#include <map>

namespace tt {
namespace profiler {

#ifndef TT_PROFILER_PRINTF
	#define TT_PROFILER_PRINTF(...)
#endif

#ifdef PERFORMANCE_PROFILER_ENABLED

struct ProfileInfo
{
	u64 elapsedTime;
	int indent;
	int frameIdx;
};

typedef std::vector<ProfileInfo> ProfileCollection;
typedef std::map<std::string, ProfileCollection> MeasureCollection;

/*! \brief Simple helper class for timing durations. */
class PerformanceProfiler
{
public:
	inline PerformanceProfiler(const char* p_functionName, const char* p_fileName, 
                        int p_line, const char* p_message, u64 p_threshold = 0)
	:
	m_functionName(0),
	m_fileName(0),
	m_message(0),
	m_line(0),
	m_startTime(0),
	m_threshold(0),
	m_valid(false)
	{
		if (m_enabled == false)
		{
			return;
		}

		m_functionName = p_functionName;
		m_fileName = p_fileName;
		m_message = p_message;
		m_line = p_line;
		m_threshold = p_threshold;
		m_valid = true;
		
		++m_indent;
		TT_ASSERT(m_indent < (m_maxProfilerDepth - 1));
		
		m_startTime = tt::system::Time::getInstance()->getMicroSeconds();
	}

	inline ~PerformanceProfiler()
	{
		if (m_valid == false)
		{
			return;
		}
		
		u64 endTime = tt::system::Time::getInstance()->getMicroSeconds();
		u64 elapsedTime = (endTime - m_startTime);

		ProfileInfo info;
		info.elapsedTime = elapsedTime;
		
		//if (elapsedTime > 4000)
		{
		//	TT_PANIC("%s %d\n", m_message, elapsedTime);
		}
		
		info.indent = m_indent;
		info.frameIdx = m_frameIdx;

		// make sure the filename isn't too long
		std::string file(m_fileName);
		if (file.length() > 50)
		{
			file = "..." + file.substr(file.length()-50);
		}
		
		// check if this key exists
		std::string key(m_message);
		key += " " + file;
		key += "(" + tt::str::toStr(m_line) + ")";
		
		MeasureCollection::iterator it = m_measures.find(key);
		if (it == m_measures.end())
		{
			ProfileCollection profileCollection;
			profileCollection.push_back(info);
			m_measures.insert(std::make_pair<std::string, ProfileCollection>(key, profileCollection));
		}
		else
		{
			(*it).second.push_back(info);
		}
		
		/*
		if (elapsedTime >= m_threshold)
		{
			for (int i = 0; i < m_indent; i++)
			{
				TT_PROFILER_PRINTF("-");
			}
			TT_PROFILER_PRINTF("%s(%d) %llu us. | %s: %s\n", m_fileName, m_line, 
				elapsedTime, m_functionName, m_message);
		}
		*/
		
		m_indent--;

	}

	inline static void enable() { m_enabled = true; }
	inline static void disable() { m_enabled = false; }
	inline static bool isEnabled() { return m_enabled; }

	static void update();

	static void outputProfileInfo();
	
	static void reset() { m_measures.clear(); }
	
private:
	static MeasureCollection m_measures;
	static const int m_maxProfilerDepth = 32;
	static bool      m_enabled;
	static int       m_frameIdx;

	const char* m_functionName;
	const char* m_fileName;
	const char* m_message;
	int         m_line;
	u64         m_startTime;
	u64         m_threshold;
	static int	m_indent;
	bool        m_valid;	// is this measure valid?
};

#endif	// PERFORMANCE_PROFILER_ENABLED

#ifdef PERFORMANCE_PROFILER_ENABLED
	#define PROFILE_PERFORMANCE_ISENABLED() \
				tt::profiler::PerformanceProfiler::isEnabled()

	#define PROFILE_PERFORMANCE_ON() \
				tt::profiler::PerformanceProfiler::enable()
				
	#define PROFILE_PERFORMANCE_UPDATE() \
	tt::profiler::PerformanceProfiler::update()

	#define PROFILE_PERFORMANCE_OUTPUT() \
				tt::profiler::PerformanceProfiler::outputProfileInfo()

	#define PROFILE_PERFORMANCE_RESET() \
				tt::profiler::PerformanceProfiler::reset()

	#define PROFILE_PERFORMANCE_OFF() \
				tt::profiler::PerformanceProfiler::disable()

	#define PROFILE_PERFORMANCE(p_message) \
				PROFILE_PERFORMANCE_N(performanceProfiler, (p_message), 0)

	#define PROFILE_PERFORMANCE_T(p_message, threshold) \
				PROFILE_PERFORMANCE_N(performanceProfiler, (p_message), (threshold))

	#define PROFILE_PERFORMANCE_N(name, p_message, threshold) \
				tt::profiler::PerformanceProfiler name(__FUNCTION__, __FILE__, __LINE__, (p_message), (threshold))
#else
	#define PROFILE_PERFORMANCE_ISENABLED(...)
	#define PROFILE_PERFORMANCE(...)
	#define PROFILE_PERFORMANCE_ON(...)
	#define PROFILE_PERFORMANCE_OFF(...)
	#define PROFILE_PERFORMANCE_UPDATE(...)
	#define PROFILE_PERFORMANCE_OUTPUT(...)
	#define PROFILE_PERFORMANCE_RESET(...)
	#define PROFILE_PERFORMANCE_T(...)
	#define PROFILE_PERFORMANCE_N(...)
#endif

// namespace
}
}

#endif	// INC_TT_PROFILER_PERFORMANCE_PROFILER_H
